#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/task_wdt/task_wdt.h>
#include <zephyr/zbus/zbus.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER( led_thread, CONFIG_APP_LED_LOG_LEVEL );

BUILD_ASSERT( CONFIG_APP_LED_WATCHDOG_TIMEOUT_SECONDS > CONFIG_APP_LED_MSGQ_TIMEOUT_SECONDS,
              "Watchdog timeout must be greater than trigger timeout" );

static void task_wdt_callback( int channel_id, void *user_data );
static void thread_listener_callback( const struct zbus_channel *chan );
static void gpio_setup( void );

ZBUS_LISTENER_DEFINE( led_thread_lis, thread_listener_callback );
ZBUS_CHAN_ADD_OBS( BUTTON_CHAN, led_thread_lis, 0 );

K_MSGQ_DEFINE( thread_msgq, sizeof( struct button_msg ), CONFIG_APP_LED_MSGQ_SIZE, 1 );

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS( led0 )

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET( LED0_NODE, gpios );

void task_wdt_callback( int channel_id, void *user_data )
{
    LOG_ERR( "Watchdog expired, Channel: %d, Thread: %s", channel_id, k_thread_name_get( (k_tid_t)user_data ) );
}

void thread_listener_callback( const struct zbus_channel *chan )
{
    const struct button_msg *message = zbus_chan_const_msg( chan );
    k_msgq_put( &thread_msgq, message, K_NO_WAIT );

    LOG_INF( "From listener -> Button event: %s", message->press == BUTTON_PRESS ? "PRESS" : "RELEASE" );
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
    struct button_msg button_message = { 0 };
    int               task_wdt_id    = -1;
    int               err            = 0;
    const uint32_t    wdt_timeout_ms = ( CONFIG_APP_LED_WATCHDOG_TIMEOUT_SECONDS * MSEC_PER_SEC );
    const k_timeout_t msgq_wait_ms   = K_MSEC( CONFIG_APP_LED_MSGQ_TIMEOUT_SECONDS * MSEC_PER_SEC );

    LOG_DBG( "LED module thread started" );

    task_wdt_id = task_wdt_add( wdt_timeout_ms, task_wdt_callback, (void *)k_current_get() );
    if( task_wdt_id < 0 )
    {
        LOG_ERR( "Failed to add task to watchdog: %d", task_wdt_id );
        return;
    }

    gpio_setup();

    while( true )
    {
        err = task_wdt_feed( task_wdt_id );
        if( err )
        {
            LOG_ERR( "Failed to feed the watchdog: %d", err );
            return;
        }

        err = k_msgq_get( &thread_msgq, &button_message, msgq_wait_ms );
        if( err < 0 )
        {
            continue;
        }

        if( button_message.press == BUTTON_PRESS )
        {
            gpio_pin_set_dt( &led, 1 );
        }
        else
        {
            gpio_pin_set_dt( &led, 0 );
        }
    }
}

K_THREAD_DEFINE( led_thread_id, CONFIG_APP_LED_THREAD_STACK_SIZE, module_thread_fn, NULL, NULL, NULL, 3, 0, 0 );
