#include <zephyr/irq_offload.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/gpio/gpio_emul.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER( test_led_thread, LOG_LEVEL_DBG );

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS( led0 )
#if !DT_NODE_HAS_STATUS_OKAY( LED0_NODE )
#error "Unsupported board: led0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET( LED0_NODE, gpios );

ZTEST( integration, test_button_press )
{
    struct button_msg button_message = { .press = BUTTON_PRESS };
    int               gpio_val       = -1;

    // Wait for the led to be configured as an output by the thread under test
    while( gpio_pin_is_output_dt( &led ) == 0 )
    {
        k_sleep( K_MSEC( 10 ) );
    }

    // Simulate a button press by publishing to the channel and check the LED value
    zbus_chan_pub( &BUTTON_CHAN, &button_message, K_FOREVER );
    k_sleep( K_MSEC( 100 ) );
    gpio_val = gpio_emul_output_get( led.port, led.pin );
    zassert_equal( gpio_val, 1, "LED state should be ON (1), observed (%d)", gpio_val );
}

ZTEST( integration, test_button_release )
{
    struct button_msg button_message = { .press = BUTTON_RELEASE };
    int               gpio_val       = -1;

    // Wait for the led to be configured as an output by the thread under test
    while( gpio_pin_is_output_dt( &led ) == 0 )
    {
        k_sleep( K_MSEC( 10 ) );
    }

    // Simulate a button press by publishing to the channel and check the LED value
    zbus_chan_pub( &BUTTON_CHAN, &button_message, K_FOREVER );
    k_sleep( K_MSEC( 100 ) );
    gpio_val = gpio_emul_output_get( led.port, led.pin );
    zassert_equal( gpio_val, 0, "LED state should be OFF (0), observed (%d)", gpio_val );
}

ZTEST_SUITE( integration, NULL, NULL, NULL, NULL, NULL );
