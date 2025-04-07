#pragma once

#include <xiaomi.h>

typedef struct
{
    void (*connection_established)(void);
    void (*connection_lost)(void);
    void (*frame_received)(const xiaomi_frame_t *frame);
} connection_callbacks_t;

int connection_init(const connection_callbacks_t *callbacks);

void connection_device_found_callback(const bt_addr_le_t *addr, const char *addr_str, int8_t rssi);
