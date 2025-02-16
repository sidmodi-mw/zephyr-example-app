/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "app_version.h"

LOG_MODULE_REGISTER( app, CONFIG_APP_LOG_LEVEL );

static bool print_channel_data_iterator( const struct zbus_channel *chan, void *user_data )
{
    int *count = user_data;

    LOG_INF( "%d - Channel %s:", *count, zbus_chan_name( chan ) );
    LOG_INF( "      Message size: %d", zbus_chan_msg_size( chan ) );
    LOG_INF( "      Observers:" );

    ++( *count );

    struct zbus_channel_observation *observation;

    for( int16_t i = chan->data->observers_start_idx, limit = chan->data->observers_end_idx; i < limit; ++i )
    {
        STRUCT_SECTION_GET( zbus_channel_observation, i, &observation );

        __ASSERT( observation != NULL, "observation must be not NULL" );

        LOG_INF( "      - %s", observation->obs->name );
    }

    struct zbus_observer_node *obs_nd, *tmp;

    SYS_SLIST_FOR_EACH_CONTAINER_SAFE( &chan->data->observers, obs_nd, tmp, node )
    {
        LOG_INF( "      - %s", obs_nd->obs->name );
    }

    return true;
}

static bool print_observer_data_iterator( const struct zbus_observer *obs, void *user_data )
{
    int *count = user_data;

    LOG_INF( "%d - %s %s", *count, obs->type == ZBUS_OBSERVER_LISTENER_TYPE ? "Listener" : "Subscriber", zbus_obs_name( obs ) );

    ++( *count );

    return true;
}

#include <zephyr/task_wdt/task_wdt.h>
static int watchdog_init( void )
{
    __ASSERT( ( task_wdt_init( NULL ) == 0 ), "Task watchdog init failure" );
    return 0;
}

int main( void )
{
    int count = 0;

    LOG_INF( "App version: %s-%s", APP_VERSION_STRING, STRINGIFY( APP_BUILD_VERSION ) );

    LOG_INF( "Channel list:" );
    zbus_iterate_over_channels_with_user_data( print_channel_data_iterator, &count );

    count = 0;

    LOG_INF( "Observers list:" );
    zbus_iterate_over_observers_with_user_data( print_observer_data_iterator, &count );

    return 0;
}

SYS_INIT( watchdog_init, POST_KERNEL, 0 );