/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messages.h"
#include "uart_thread.h"

#include <zephyr/irq_offload.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/serial/uart_emul.h>

LOG_MODULE_DECLARE( zbus, CONFIG_ZBUS_LOG_LEVEL );

static void after_test( void * );
static void test_listener_callback( const struct zbus_channel *chan );

ZBUS_LISTENER_DEFINE( test_lis, test_listener_callback );
ZBUS_CHAN_ADD_OBS( uart_thread_chan, test_lis, 3 );

K_MSGQ_DEFINE( test_msgq, sizeof( struct uart_msg ), 10, 1 );

// Get UART configuration from the devicetree
#define UART_NODE DT_ALIAS( datauart )
#if !DT_NODE_HAS_STATUS_OKAY( UART_NODE )
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct device *uart_device = DEVICE_DT_GET( UART_NODE );

void after_test( void * )
{
    k_msgq_purge( &test_msgq );
}

void test_listener_callback( const struct zbus_channel *chan )
{
    const struct uart_msg *uart_message = zbus_chan_const_msg( chan );
    k_msgq_put( &test_msgq, uart_message, K_NO_WAIT );
}

ZTEST( integration, test_uart_rx_lump_data )
{
    // Emulate UART Rx Data
    uint8_t test_data[UART_MESSAGE_BUFFER_SIZE] = { 0 };
    memset( test_data, 0x69, sizeof( test_data ) );
    uart_emul_put_rx_data( uart_device, test_data, sizeof( test_data ) );

    // Wait for the button event
    struct uart_msg uart_message = { 0 };
    k_msgq_get( &test_msgq, &uart_message, K_FOREVER );
    zassert_mem_equal( test_data, uart_message.data, uart_message.len, "Rx data does not match test data." );
}

ZTEST( integration, test_uart_rx_frag_data )
{
    // Emulate UART Rx Data
    uint8_t test_data[UART_MESSAGE_BUFFER_SIZE] = { 0 };
    memset( test_data, 0x69, sizeof( test_data ) );

    for( unsigned int i = 0; i < sizeof( test_data ); i++ )
    {
        uart_emul_put_rx_data( uart_device, &test_data[i], 1 );
    }

    // Wait for the button event
    struct uart_msg uart_message = { 0 };
    k_msgq_get( &test_msgq, &uart_message, K_FOREVER );
    zassert_mem_equal( test_data, uart_message.data, uart_message.len, "Rx data does not match test data." );
}

ZTEST_SUITE( integration, NULL, NULL, NULL, after_test, NULL );
