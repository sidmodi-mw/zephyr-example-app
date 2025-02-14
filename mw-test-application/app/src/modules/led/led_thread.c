#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/drivers/gpio.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER( led_thread, CONFIG_ZBUS_LOG_LEVEL );

static void thread_listener_callback( const struct zbus_channel *chan );
static void gpio_setup( void );

ZBUS_LISTENER_DEFINE( led_thread_lis, thread_listener_callback );
ZBUS_CHAN_ADD_OBS( BUTTON_CHAN, led_thread_lis, 3 );

K_MSGQ_DEFINE( thread_msgq, sizeof( enum button_press ), 10, 1 );

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS( led0 )

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET( LED0_NODE, gpios );

void thread_listener_callback( const struct zbus_channel *chan )
{
    const enum button_press *button_msg = zbus_chan_const_msg( chan );
    k_msgq_put( &thread_msgq, button_msg, K_NO_WAIT );

    LOG_INF( "From listener -> Button event: %s", *button_msg == BUTTON_PRESS ? "PRESS" : "RELEASE" );
}

void gpio_setup( void )
{
    int ret;

    if( !gpio_is_ready_dt( &led ) )
    {
        return;
    }

    ret = gpio_pin_configure_dt( &led, GPIO_OUTPUT_INACTIVE );
    if( ret < 0 )
    {
        return;
    }

    printk( "Set up LED at %s pin %d\n", led.port->name, led.pin );
}

void module_thread_fn( void )
{
    enum button_press button_event = BUTTON_PRESS;
    gpio_setup();

    while( 1 )
    {
        k_msgq_get( &thread_msgq, &button_event, K_FOREVER );
        if( button_event == BUTTON_PRESS )
        {
            gpio_pin_set_dt( &led, 1 );
        }
        else
        {
            gpio_pin_set_dt( &led, 0 );
        }
    }
}

K_THREAD_DEFINE( led_thread_id, 1024, module_thread_fn, NULL, NULL, NULL, 3, 0, 0 );
