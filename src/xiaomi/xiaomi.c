#include "xiaomi.h"
#include <errno.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

#define XIAOMI_DATA_HANDLE 0x0036
#define XIAOMI_NOTIFICATIONS_ENABLE_HANDLE 0x0038

LOG_MODULE_REGISTER(xiaomi);

typedef struct 
{
    xiaomi_frame_callback_t callback;
} xiaomi_ctx_t;

static xiaomi_ctx_t ctx;

static int xiaomi_parse_data(xiaomi_frame_t *frame, const void *data, size_t size)
{
    if ((frame == NULL) || (data == NULL)) {
        return -EINVAL;
    }

    if (size != sizeof(*frame)) {
        return -EMSGSIZE;
    }

    memcpy(frame->raw, data, size);

    return 0;
}

static uint8_t xiaomi_notification_callback(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *data, uint16_t length)
{
    ARG_UNUSED(conn);
    ARG_UNUSED(params);

    int err;
    xiaomi_frame_t frame;

    err = xiaomi_parse_data(&frame, data, length);
    if (err) {
        LOG_ERR("Failed to parse new frame, error %d", err);
        return BT_GATT_ITER_CONTINUE;
    }

    if (ctx.callback != NULL) {
        ctx.callback(&frame);
    }

    return BT_GATT_ITER_CONTINUE;
}

int xiaomi_enable_notifications(struct bt_conn *connection, bool enable)
{
    uint16_t data;

    if (connection == NULL) {
        return -EINVAL;
    }

    data = enable ? 0x0100 : 0x0000;

    return bt_gatt_write_without_response(connection, XIAOMI_NOTIFICATIONS_ENABLE_HANDLE, &data, sizeof(data), false);
}

int xiaomi_subscribe_to_data(struct bt_conn *connection, xiaomi_frame_callback_t callback)
{
    if ((connection == NULL) || (callback == NULL)) {
        return -EINVAL;
    }

    ctx.callback = callback;

    static struct bt_gatt_subscribe_params params = {
        .notify = xiaomi_notification_callback,
        .value = BT_GATT_CCC_NOTIFY,
        .ccc_handle = XIAOMI_DATA_HANDLE,
        .value_handle = XIAOMI_DATA_HANDLE
    };

    return bt_gatt_subscribe(connection, &params);
}
