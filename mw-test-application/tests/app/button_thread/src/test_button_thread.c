#include <zephyr/irq_offload.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/gpio/gpio_emul.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER( test_button_thread, LOG_LEVEL_DBG );

static void after_test( void * );
static void test_listener_callback( const struct zbus_channel *chan );

ZBUS_LISTENER_DEFINE( test_lis, test_listener_callback );
ZBUS_CHAN_ADD_OBS( BUTTON_CHAN, test_lis, 0 );

K_MSGQ_DEFINE( test_msgq, sizeof( struct button_msg ), 10, 1 );

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
    const struct button_msg *message = zbus_chan_const_msg( chan );
    k_msgq_put( &test_msgq, message, K_NO_WAIT );

    LOG_INF( "From listener -> Button event: %s", message->press == BUTTON_PRESS ? "PRESS" : "RELEASE" );
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

    // Wait for the button message
    struct button_msg message = { 0 };
    k_msgq_get( &test_msgq, &message, K_FOREVER );
    zassert_equal( message.press, BUTTON_PRESS, "Button press should be BUTTON_PRESS" );
    zassert_equal( message.button, BUTTON_1, "Button should be BUTTON_1" ); // TODO: Hardcoded button needs to be replaced
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

    // Wait for the button message
    struct button_msg message = { 0 };
    k_msgq_get( &test_msgq, &message, K_FOREVER );
    zassert_equal( message.press, BUTTON_RELEASE, "Button press should be BUTTON_RELEASE" );
    zassert_equal( message.button, BUTTON_1, "Button should be BUTTON_1" ); // TODO: Hardcoded button needs to be replaced
}

ZTEST_SUITE( integration, NULL, NULL, NULL, after_test, NULL );