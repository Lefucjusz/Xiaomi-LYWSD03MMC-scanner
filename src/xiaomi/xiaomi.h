#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <zephyr/bluetooth/bluetooth.h>

typedef union
{
    struct data_t {
        uint16_t temperature;
        uint8_t humidity;
        uint16_t batt_voltage;
    } __attribute__((packed)) data;
    uint8_t raw[sizeof(struct data_t)];
} xiaomi_frame_t;

typedef void (*xiaomi_frame_callback_t)(const xiaomi_frame_t *frame);

int xiaomi_enable_notifications(struct bt_conn *connection, bool enable);
int xiaomi_subscribe_to_data(struct bt_conn *connection, xiaomi_frame_callback_t callback);
