#include <scanner.h>
#include <connection.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/hci.h>

LOG_MODULE_REGISTER(main);

static void frame_received_callback(const xiaomi_frame_t *frame)
{
    printk("{\"temperature\": %u.%02u, \"humidity\": %u, \"battery\": %u}\n", 
            frame->data.temperature / 100, 
            frame->data.temperature % 100, 
            frame->data.humidity, 
            frame->data.batt_voltage);
}

int main(void)
{
    int err;

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Failed to enable BLE, error %d!", err);
        return err;
    }

    err = connection_init(frame_received_callback);
    if (err) {
        LOG_ERR("Failed to initialize connection, error %d!", err);
        return err;
    }

    err = scanner_start(connection_device_found_callback);
    if (err) {
        return err;
    }

    return 0;
}
