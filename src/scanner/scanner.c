#include "scanner.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(scanner);

typedef struct
{
    scanner_notify_callback_t callback;
} scanner_ctx_t;

static scanner_ctx_t ctx;

static void scanner_device_found_callback(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type, struct net_buf_simple *buf)
{
    static const char *const telink_prefix = "A4:C1:38";
    char addr_str[BT_ADDR_LE_STR_LEN];

    /* Discard non-connectable devices */
    if ((adv_type != BT_GAP_ADV_TYPE_ADV_IND) && (adv_type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND)) {
        return;
    }

    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

    /* Xiaomi weather stations use Telink chip, discard other devices */
    if (strncmp(addr_str, telink_prefix, strlen(telink_prefix)) != 0) {
        return;
    }

    /* Found connectable Telink device, notify */
    ctx.callback(addr, addr_str, rssi);
}

int scanner_start(scanner_notify_callback_t callback)
{
    int err;

    if (callback == NULL) {
        return -EINVAL;
    }
    ctx.callback = callback;

    struct bt_le_scan_param params = {
        .type = BT_LE_SCAN_TYPE_PASSIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW
    };

    err = bt_le_scan_start(&params, scanner_device_found_callback);
    if (err) {
        LOG_ERR("Failed to start BLE scan, error %d!", err);
        return err;
    }
    LOG_INF("BLE scan for Xiaomi weather stations started...");

    return 0;
}

int scanner_stop(void)
{
    int err;

    err = bt_le_scan_stop();
    if (err) {
        LOG_ERR("Failed to disable scanning, error %d!", err);
        return err;
    }

    LOG_INF("Scanning stopped");

    return 0; 
}
