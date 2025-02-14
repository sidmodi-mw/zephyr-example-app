/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/task_wdt/task_wdt.h>
#include <zephyr/zbus/zbus.h>

#include "zbus_channels.h"

// Register log module
LOG_MODULE_REGISTER( button, CONFIG_APP_BUTTON_LOG_LEVEL );

BUILD_ASSERT( CONFIG_APP_BUTTON_WATCHDOG_TIMEOUT_SECONDS > CONFIG_APP_BUTTON_MSGQ_TIMEOUT_SECONDS,
              "Watchdog timeout must be greater than trigger timeout" );

// Declare all the internal functions
static void task_wdt_callback( int channel_id, void *user_data );
static void button_setup( void );

// Define the internal message queue
K_MSGQ_DEFINE( button_msgq, sizeof( struct button_msg ), CONFIG_APP_BUTTON_MSGQ_SIZE, 1 );

// Get button configuration from the devicetree sw0 alias. This is mandatory.
#define SW0_NODE DT_ALIAS( sw0 )
#if !DT_NODE_HAS_STATUS_OKAY( SW0_NODE )
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR( SW0_NODE, gpios, { 0 } );
static struct gpio_callback      button_cb_data;

// ----------------------------- Function definitions -----------------------------
void task_wdt_callback( int channel_id, void *user_data )
{
    LOG_ERR( "Watchdog expired, Channel: %d, Thread: %s", channel_id, k_thread_name_get( (k_tid_t)user_data ) );
}

void button_pressed( const struct device *dev, struct gpio_callback *cb, uint32_t pins )
{
    int               val          = gpio_pin_get_dt( &button );
    enum button_press button_event = val == 1 ? BUTTON_PRESS : BUTTON_RELEASE;
    struct button_msg button_msg   = { .button = BUTTON_1, .press = button_event }; // TODO: Hardcoded button needs to be replaced
    k_msgq_put( &button_msgq, &button_msg, K_NO_WAIT );
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
    int               task_wdt_id    = -1;
    int               err            = 0;
    const uint32_t    wdt_timeout_ms = ( CONFIG_APP_BUTTON_WATCHDOG_TIMEOUT_SECONDS * MSEC_PER_SEC );
    const k_timeout_t msgq_wait_ms   = K_MSEC( CONFIG_APP_BUTTON_MSGQ_TIMEOUT_SECONDS * MSEC_PER_SEC );
    struct button_msg message        = { 0 };

    LOG_DBG( "Button module thread started" );

    task_wdt_id = task_wdt_add( wdt_timeout_ms, task_wdt_callback, (void *)k_current_get() );
    if( task_wdt_id < 0 )
    {
        LOG_ERR( "Failed to add task to watchdog: %d", task_wdt_id );
        return;
    }

    button_setup();

    while( true )
    {
        err = task_wdt_feed( task_wdt_id );
        if( err )
        {
            LOG_ERR( "Failed to feed the watchdog: %d", err );
            return;
        }

        err = k_msgq_get( &button_msgq, &message, msgq_wait_ms );
        if( err < 0 )
        {
            continue;
        }

        zbus_chan_pub( &BUTTON_CHAN, &message, K_NO_WAIT );
    }
}

K_THREAD_DEFINE( button_thread_id, CONFIG_APP_BUTTON_THREAD_STACK_SIZE, button_thread, NULL, NULL, NULL, 3, 0, 0 );
