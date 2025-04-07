#include "connection.h"
#include <scanner.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(connection);

typedef struct
{
    struct bt_conn *connection;
    xiaomi_frame_callback_t frame_callback;
} connection_ctx_t;

static connection_ctx_t ctx;

static void connected_callback(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Failed to connect, error %d!", err);

        /* Clean up */
        bt_conn_unref(ctx.connection);
        ctx.connection = NULL;

        scanner_start(connection_device_found_callback);

        return;
    }

    if (conn != ctx.connection) {
        return;
    }

    err = xiaomi_enable_notifications(ctx.connection, true);
    if (err) {
        LOG_ERR("Failed to enable weather station notifications, error: %d!", err);
        return;
    }

    err = xiaomi_subscribe_to_data(ctx.connection, ctx.frame_callback);
    if (err) {
        LOG_ERR("Failed to subscribe to data, error %d!", err);
        return;
    }

    LOG_INF("Connected!");
}

static void disconnected_callback(struct bt_conn *conn, uint8_t reason)
{
    LOG_WRN("Device disconnected, reason 0x%02X!", reason);

    /* Clean up */
    bt_conn_unref(ctx.connection);
    ctx.connection = NULL;

    scanner_start(connection_device_found_callback);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected_callback,
    .disconnected = disconnected_callback
};

int connection_init(xiaomi_frame_callback_t callback)
{
    if (callback == NULL) {
        return -EINVAL;
    }

    ctx.frame_callback = callback;

    return 0;
}

void connection_device_found_callback(const bt_addr_le_t *addr, const char *addr_str, int8_t rssi)
{
    int err;

    /* Discard scanner events if connection established. Scanning should
     * be disabled in such case anyway, but just to be sure. */
    if (ctx.connection != NULL) {
        return;
    }

    LOG_INF("Found connectable Telink device (%s, RSSI: %ddB), connecting...", addr_str, rssi);

    /* Disable scanning */
    err = scanner_stop();
    if (err) {
        return;
    }

    /* Try to connect to the device */
    err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT, &ctx.connection);
    if (err) {
        LOG_ERR("Failed to create connection to %s, error %d!", addr_str, err);
        scanner_start(connection_device_found_callback);
    }
}
