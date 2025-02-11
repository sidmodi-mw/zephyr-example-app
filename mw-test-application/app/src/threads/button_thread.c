/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "messages.h"

#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>

LOG_MODULE_REGISTER( button_thread, CONFIG_ZBUS_LOG_LEVEL );

static void button_setup( void );

ZBUS_CHAN_DEFINE( button_thread_chan,             /* Name */
                  enum button_press,              /* Message type */
                  NULL,                           /* Validator */
                  NULL,                           /* User data */
                  ZBUS_OBSERVERS_EMPTY,           /* observers */
                  ZBUS_MSG_INIT( BUTTON_INVALID ) /* Initial value major 0, minor 1, build 2 */
);

K_MSGQ_DEFINE( button_msgq, sizeof( enum button_press ), 10, 1 );

// Get button configuration from the devicetree sw0 alias. This is mandatory.
#define SW0_NODE DT_ALIAS( sw0 )
#if !DT_NODE_HAS_STATUS_OKAY( SW0_NODE )
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR( SW0_NODE, gpios, { 0 } );
static struct gpio_callback      button_cb_data;

// ----------------------------- Function definitions -----------------------------

void button_pressed( const struct device *dev, struct gpio_callback *cb, uint32_t pins )
{
    int               val          = gpio_pin_get_dt( &button );
    enum button_press button_event = val == 1 ? BUTTON_PRESS : BUTTON_RELEASE;
    k_msgq_put( &button_msgq, &button_event, K_NO_WAIT );
}

void button_setup( void )
{
    int ret = 0;

    if( !gpio_is_ready_dt( &button ) )
    {
        printk( "Error: button device %s is not ready\n", button.port->name );
        return;
    }

    ret = gpio_pin_configure_dt( &button, GPIO_INPUT );
    if( ret != 0 )
    {
        printk( "Error %d: failed to configure %s pin %d\n", ret, button.port->name, button.pin );
        return;
    }

    ret = gpio_pin_interrupt_configure_dt( &button, GPIO_INT_EDGE_BOTH );
    if( ret != 0 )
    {
        printk( "Error %d: failed to configure interrupt on %s pin %d\n", ret, button.port->name, button.pin );
        return;
    }

    gpio_init_callback( &button_cb_data, button_pressed, BIT( button.pin ) );
    gpio_add_callback( button.port, &button_cb_data );
    printk( "Set up button at %s pin %d\n", button.port->name, button.pin );
}

void button_thread( void )
{
    button_setup();

    enum button_press button_event = BUTTON_PRESS;

    while( 1 )
    {
        k_msgq_get( &button_msgq, &button_event, K_FOREVER );
        zbus_chan_pub( &button_thread_chan, &button_event, K_SECONDS( 1 ) );
    }
}

void test_thread( void )
{
    while( 1 )
    {
        gpio_emul_input_set( button.port, button.pin, 1 );
        k_sleep( K_MSEC( 250 ) );
        gpio_emul_input_set( button.port, button.pin, 0 );
        k_sleep( K_MSEC( 250 ) );
    }
}

K_THREAD_DEFINE( test_thread_id, 1024, test_thread, NULL, NULL, NULL, 3, 0, 0 );
K_THREAD_DEFINE( button_thread_id, 1024, button_thread, NULL, NULL, NULL, 3, 0, 0 );
