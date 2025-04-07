#pragma once

#include <zephyr/bluetooth/bluetooth.h>

typedef void (*scanner_notify_callback_t)(const bt_addr_le_t *addr, const char *addr_str, int8_t rssi);

int scanner_start(scanner_notify_callback_t callback);

int scanner_stop(void);
