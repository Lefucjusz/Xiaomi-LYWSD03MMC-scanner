#pragma once

#include <xiaomi.h>

int connection_init(xiaomi_frame_callback_t callback);

void connection_device_found_callback(const bt_addr_le_t *addr, const char *addr_str, int8_t rssi);
