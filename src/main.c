#include <scanner.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

#define XIAOMI_DATA_HANDLE 0x0036
#define XIAOMI_NOTIF_ENABLE_HANDLE 0x0038

LOG_MODULE_REGISTER(main);

typedef union
{
    struct data {
        uint16_t temp;
        uint8_t rh;
        uint16_t batt_v;
    } __attribute__((packed)) data;
    uint8_t raw[sizeof(struct data)];
} notification_data_t;

typedef struct
{
    struct bt_conn *connection;
} reader_ctx_t;

static reader_ctx_t ctx;

static void scanner_notify_callback(const bt_addr_le_t *addr, const char *addr_str, int8_t rssi)
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
        scanner_start(scanner_notify_callback);
    }
}

static uint8_t data_notification_callback(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *raw_data, uint16_t length)
{    
    if (length != sizeof(notification_data_t)) {
        LOG_ERR("Invalid received data size!");
        return BT_GATT_ITER_CONTINUE;
    }

    if (raw_data == NULL) {
        LOG_ERR("NULL data pointer!");
        return BT_GATT_ITER_CONTINUE;
    }

    notification_data_t n;
    memcpy(n.raw, raw_data, sizeof(n.raw));

    printk("{\"temperature\": %u.%02u, \"humidity\": %u, \"battery\": %u}\n", n.data.temp / 100, n.data.temp % 100, n.data.rh, n.data.batt_v);

    // LOG_INF("Temperature: %u.%02u*C, humidity: %u%%, battery voltage: %umV", n.data.temp / 100, n.data.temp % 100, n.data.rh, n.data.batt_v);

    return BT_GATT_ITER_CONTINUE;
}

static int xiaomi_enable_notifications(void)
{
    const uint16_t data = 0x0100; // Enable

    return bt_gatt_write_without_response(ctx.connection, XIAOMI_NOTIF_ENABLE_HANDLE, &data, sizeof(data), false);
}

static int xiaomi_subscribe_to_data(void)
{
    static struct bt_gatt_subscribe_params params = {
        .notify = data_notification_callback,
        .value = BT_GATT_CCC_NOTIFY,
        .ccc_handle = XIAOMI_DATA_HANDLE,
        .value_handle = XIAOMI_DATA_HANDLE
    };

    return bt_gatt_subscribe(ctx.connection, &params);
}

static void connected_callback(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Failed to connect, error %d (%s)!", err, bt_hci_err_to_str(err));

        /* Clean up */
        bt_conn_unref(ctx.connection);
        ctx.connection = NULL;

        scanner_start(scanner_notify_callback);

        return;
    }

    if (conn != ctx.connection) {
        return;
    }

    err = xiaomi_enable_notifications();
    if (err) {
        LOG_ERR("Failed to enable weather station notifications, error: %d!", err);
        return;
    }

    err = xiaomi_subscribe_to_data();
    if (err) {
        LOG_ERR("Failed to subscribe to data, error %d!", err);
        return;
    }

    LOG_INF("Connected!");
}

static void disconnected_callback(struct bt_conn *conn, uint8_t reason)
{
    LOG_WRN("Device disconnected, reason 0x%02X (%s)!", reason, bt_hci_err_to_str(reason));

    /* Clean up */
    bt_conn_unref(ctx.connection);
    ctx.connection = NULL;

    scanner_start(scanner_notify_callback);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected_callback,
    .disconnected = disconnected_callback
};

int main(void)
{
    int err;

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Failed to enable BLE, error %d!", err);
        return err;
    }

    err = scanner_start(scanner_notify_callback);
    if (err) {
        return err;
    }

    return 0;
}
