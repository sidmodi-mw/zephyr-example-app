/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messages.h"
#include "led_thread.h"

#include <zephyr/irq_offload.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/gpio/gpio_emul.h>

LOG_MODULE_DECLARE( zbus, CONFIG_ZBUS_LOG_LEVEL );

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS( led0 )
#if !DT_NODE_HAS_STATUS_OKAY( LED0_NODE )
#error "Unsupported board: led0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET( LED0_NODE, gpios );

// Define the dummy button_thread_chan that will inject events into the led_thread
ZBUS_CHAN_DEFINE( button_thread_chan,               /* Name */
                  enum button_press,                /* Message type */
                  NULL,                             /* Validator */
                  NULL,                             /* User data */
                  ZBUS_OBSERVERS( led_thread_lis ), /* observers */
                  ZBUS_MSG_INIT( BUTTON_INVALID )   /* Initial value */
);

ZTEST( integration, test_button_press )
{
    enum button_press button_event = BUTTON_PRESS;
    int               gpio_val     = -1;

    // Wait for the led to be configured as an output by the thread under test
    while( gpio_pin_is_output_dt( &led ) == 0 )
    {
        k_sleep( K_MSEC( 10 ) );
    }

    // Emulate button press and check the LED value
    zbus_chan_pub( &button_thread_chan, &button_event, K_FOREVER );
    k_sleep( K_MSEC( 100 ) );
    gpio_val = gpio_emul_output_get( led.port, led.pin );
    zassert_equal( gpio_val, 1, "LED state should be ON (1), observed (%d)", gpio_val );
}

ZTEST( integration, test_button_release )
{
    enum button_press button_event = BUTTON_RELEASE;
    int               gpio_val     = -1;

    // Wait for the led to be configured as an output by the thread under test
    while( gpio_pin_is_output_dt( &led ) == 0 )
    {
        k_sleep( K_MSEC( 10 ) );
    }

    // Emulate button press and check the LED value
    zbus_chan_pub( &button_thread_chan, &button_event, K_FOREVER );
    k_sleep( K_MSEC( 100 ) );
    gpio_val = gpio_emul_output_get( led.port, led.pin );
    zassert_equal( gpio_val, 0, "LED state should be OFF (0), observed (%d)", gpio_val );
}

ZTEST_SUITE( integration, NULL, NULL, NULL, NULL, NULL );