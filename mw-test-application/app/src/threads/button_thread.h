#pragma once

#include <zephyr/zbus/zbus.h>

void button_thread( void );

ZBUS_CHAN_DECLARE( button_thread_chan );
