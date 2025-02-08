/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messages.h"
#include "button_thread.h"

#include <zephyr/irq_offload.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/gpio/gpio_emul.h>

LOG_MODULE_DECLARE( zbus, CONFIG_ZBUS_LOG_LEVEL );

static void after_test( void * );
static void test_listener_callback( const struct zbus_channel *chan );

ZBUS_LISTENER_DEFINE( test_lis, test_listener_callback );
ZBUS_CHAN_ADD_OBS( button_thread_chan, test_lis, 3 );

K_MSGQ_DEFINE( test_msgq, sizeof( enum button_press ), 10, 1 );

#define SW0_NODE DT_ALIAS( sw0 )
#if !DT_NODE_HAS_STATUS_OKAY( SW0_NODE )
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR( SW0_NODE, gpios, { 0 } );

void after_test( void * )
{
    k_msgq_purge( &test_msgq );
}

void test_listener_callback( const struct zbus_channel *chan )
{
    const enum button_press *button_msg = zbus_chan_const_msg( chan );
    k_msgq_put( &test_msgq, button_msg, K_NO_WAIT );

    LOG_INF( "From listener -> Button event: %s", *button_msg == BUTTON_PRESS ? "PRESS" : "RELEASE" );
}

ZTEST( integration, test_button_press )
{
    // Wait for the button to be configured as an input by the thread under test
    while( gpio_pin_is_input_dt( &button ) == 0 )
    {
        k_sleep( K_MSEC( 10 ) );
    }

    // Emulate button press
    gpio_emul_input_set( button.port, button.pin, 1 );
    k_sleep( K_MSEC( 10 ) );
    gpio_emul_input_set( button.port, button.pin, 0 );

    // Wait for the button event
    enum button_press button_event = BUTTON_INVALID;
    k_msgq_get( &test_msgq, &button_event, K_FOREVER );
    zassert_equal( button_event, BUTTON_PRESS, "Button event should be BUTTON_PRESS" );
}

ZTEST( integration, test_button_release )
{
    // Wait for the button to be configured as an input by the thread under test
    while( gpio_pin_is_input_dt( &button ) == 0 )
    {
        k_sleep( K_MSEC( 10 ) );
    }

    // Emulate button release
    gpio_emul_input_set( button.port, button.pin, 0 );
    k_sleep( K_MSEC( 10 ) );
    gpio_emul_input_set( button.port, button.pin, 1 );

    // Wait for the button event
    enum button_press button_event = BUTTON_INVALID;
    k_msgq_get( &test_msgq, &button_event, K_FOREVER );
    zassert_equal( button_event, BUTTON_RELEASE, "Button event should be BUTTON_RELEASE" );
}

ZTEST_SUITE( integration, NULL, NULL, NULL, after_test, NULL );