
/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "zbus_channels.h"

#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/task_wdt/task_wdt.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/kernel.h>

LOG_MODULE_REGISTER( uart_thread, CONFIG_APP_UART_LOG_LEVEL );

BUILD_ASSERT( CONFIG_APP_UART_WATCHDOG_TIMEOUT_SECONDS > CONFIG_APP_UART_PIPE_READ_TIMEOUT_SECONDS,
              "Watchdog timeout must be greater than trigger timeout" );

// Get UART configuration from the devicetree
#define UART_NODE DT_ALIAS( datauart )
#if !DT_NODE_HAS_STATUS_OKAY( UART_NODE )
#error "Unsupported board: datauart devicetree alias is not defined"
#endif
static const struct device *uart_device = DEVICE_DT_GET( UART_NODE );

// UART Buffers
#define UART_RX_BUFFER_SIZE  256
#define UART_PIPE_SIZE       ( 5 * UART_RX_BUFFER_SIZE )
#define UART_RECEIVE_TIMEOUT 20000
uint8_t g_rxDoubleBuffer[2][UART_RX_BUFFER_SIZE] = { 0 };
uint8_t g_activeBuffer                           = 0;

static void task_wdt_callback( int channel_id, void *user_data );
static void uart_cb( const struct device *p_dev, struct uart_event *p_evt, void *p_user_data );
static void uart_data_handler( struct uart_event *evt );
static int  uart_init( void );
static void uart_thread( void );

K_PIPE_DEFINE( uart_pipe, UART_PIPE_SIZE, 4 );

void task_wdt_callback( int channel_id, void *user_data )
{
    LOG_ERR( "Watchdog expired, Channel: %d, Thread: %s", channel_id, k_thread_name_get( (k_tid_t)user_data ) );
}

void uart_cb( const struct device *p_dev, struct uart_event *p_evt, void *p_user_data )
{
    uint8_t writeFlagInv = g_activeBuffer == 0 ? 1 : 0;
    LOG_DBG( "UART EVENT - 0x%02x", p_evt->type );

    switch( p_evt->type )
    {
    case UART_RX_RDY:
        uart_data_handler( p_evt );
        break;
    case UART_RX_BUF_REQUEST:
        // Provide next Buffer to be used for continuous receive
        LOG_DBG( "UART_RX_BUF_REQUEST EVENT - 0x%02x", writeFlagInv );
        uart_rx_buf_rsp( p_dev, g_rxDoubleBuffer[writeFlagInv], sizeof( g_rxDoubleBuffer[0] ) );
        break;
    case UART_RX_BUF_RELEASED:
        // Buffer Released - Update Write Flag
        LOG_DBG( "UART_RX_BUF_RELEASED EVENT - 0x%02x", writeFlagInv );
        g_activeBuffer = writeFlagInv;
        break;
    case UART_RX_STOPPED:
        switch( p_evt->data.rx_stop.reason )
        {
        case UART_ERROR_OVERRUN:
            LOG_ERR( "UART interrupt: Stop Reason - UART_ERROR_OVERRUN" );
            break;
        case UART_ERROR_PARITY:
            LOG_ERR( "UART interrupt: Stop Reason - UART_ERROR_PARITY" );
            break;
        case UART_ERROR_FRAMING:
            LOG_ERR( "UART interrupt: Stop Reason - UART_ERROR_FRAMING" );
            break;
        case UART_BREAK:
            LOG_ERR( "UART interrupt: Stop Reason - UART_BREAK" );
            break;
        case UART_ERROR_COLLISION:
            LOG_ERR( "UART interrupt: Stop Reason - UART_ERROR_COLLISION" );
            break;
        case UART_ERROR_NOISE:
            LOG_ERR( "UART interrupt: Stop Reason - UART_ERROR_NOISE" );
            break;
        }
        break;
    case UART_RX_DISABLED:
        uart_rx_enable( p_dev, g_rxDoubleBuffer[writeFlagInv], sizeof( g_rxDoubleBuffer[0] ), UART_RECEIVE_TIMEOUT );
        break;
    default:
        break;
    }
}

inline void uart_data_handler( struct uart_event *evt )
{
    k_pipe_write( &uart_pipe, &evt->data.rx.buf[evt->data.rx.offset], evt->data.rx.len, K_NO_WAIT );
}

int uart_init( void )
{
    int err = 0;

    /* Configure UART */
    if( !device_is_ready( uart_device ) )
    {
        LOG_ERR( "UART device not ready\r\n" );
        return -1;
    }

    /* Register UART Callback Function */
    err = uart_callback_set( uart_device, uart_cb, NULL );
    if( err )
    {
        return err;
    }

    /* Intialise Buffers */
    g_activeBuffer = 0;

    return 0;
}

void uart_thread( void )
{
    struct uart_msg   uart_message   = { 0 };
    size_t            bytes_read     = 0;
    int               ret            = -1;
    int               task_wdt_id    = -1;
    const uint32_t    wdt_timeout_ms = ( CONFIG_APP_UART_WATCHDOG_TIMEOUT_SECONDS * MSEC_PER_SEC );
    const k_timeout_t pipe_wait_ms   = K_MSEC( CONFIG_APP_UART_PIPE_READ_TIMEOUT_SECONDS * MSEC_PER_SEC );

    LOG_DBG( "UART module thread started" );

    task_wdt_id = task_wdt_add( wdt_timeout_ms, task_wdt_callback, (void *)k_current_get() );
    if( task_wdt_id < 0 )
    {
        LOG_ERR( "Failed to add task to watchdog: %d", task_wdt_id );
        return;
    }

    uart_init();

    // Start UART Receive
    uart_rx_enable( uart_device, g_rxDoubleBuffer[g_activeBuffer], sizeof( g_rxDoubleBuffer[0] ), UART_RECEIVE_TIMEOUT );

    // Collect sizeof( uart_message.data ) bytes before publishing
    uart_message.len = sizeof( uart_message.data );
    while( true )
    {
        ret = task_wdt_feed( task_wdt_id );
        if( ret )
        {
            LOG_ERR( "Failed to feed the watchdog: %d", ret );
            return;
        }

        ret = k_pipe_read( &uart_pipe, uart_message.data, uart_message.len - bytes_read, pipe_wait_ms );
        if( ret < 0 )
        {
            // Error
            continue;
        }

        LOG_DBG( "Received %d bytes on UART", ret );
        bytes_read += ret;

        if( bytes_read == uart_message.len )
        {
            // Publish received data
            zbus_chan_pub( &UART_CHAN, &uart_message, K_NO_WAIT );
        }
    }
}

K_THREAD_DEFINE( uart_thread_id, CONFIG_APP_UART_THREAD_STACK_SIZE, uart_thread, NULL, NULL, NULL, 3, 0, 0 );
