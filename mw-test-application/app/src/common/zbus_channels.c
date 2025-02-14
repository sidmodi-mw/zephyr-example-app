
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "zbus_channels.h"

ZBUS_CHAN_DEFINE( BUTTON_CHAN,          /* Name */
                  struct button_msg,    /* Message type */
                  NULL,                 /* Validator */
                  NULL,                 /* User data */
                  ZBUS_OBSERVERS_EMPTY, /* observers */
                  ZBUS_MSG_INIT( 0 )    /* Initial value major 0, minor 1, build 2 */
);

ZBUS_CHAN_DEFINE( UART_CHAN,            /* Name */
                  struct uart_msg,      /* Message type */
                  NULL,                 /* Validator */
                  NULL,                 /* User data */
                  ZBUS_OBSERVERS_EMPTY, /* observers */
                  ZBUS_MSG_INIT( 0 )    /* Initial value major 0, minor 1, build 2 */
);