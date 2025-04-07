#include <scanner.h>
#include <connection.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/drivers/auxdisplay.h>

LOG_MODULE_REGISTER(main);

static const struct device *const lcd = DEVICE_DT_GET(DT_NODELABEL(hd44780));

static int auxdisplay_init(void)
{
    if (!device_is_ready(lcd)) {
        return -ENODEV;
    }

    return auxdisplay_cursor_set_enabled(lcd, false);
}

static void auxdisplay_printf(const char *fmt, ...)
{
    va_list arg_list;
    char msg_buf[32];
    int len;

    va_start(arg_list, fmt);
    len = vsnprintf(msg_buf, sizeof(msg_buf), fmt, arg_list);
    va_end(arg_list);

    auxdisplay_write(lcd, msg_buf, len);
}

static void connection_established_callback(void)
{
    auxdisplay_clear(lcd);
    auxdisplay_printf("Connected!");
}

static void connection_lost_callback(void)
{
    auxdisplay_clear(lcd);
    auxdisplay_printf("Scanning...");
}

static void frame_received_callback(const xiaomi_frame_t *frame)
{
    auxdisplay_cursor_position_set(lcd, AUXDISPLAY_POSITION_ABSOLUTE, 0, 0);
    auxdisplay_printf("Temp: %u.%02uC", frame->data.temperature / 100, frame->data.temperature % 100);
    auxdisplay_cursor_position_set(lcd, AUXDISPLAY_POSITION_ABSOLUTE, 0, 1);
    auxdisplay_printf("RH: %u%%", frame->data.humidity);

    printk("{\"temperature\": %u.%02u, \"humidity\": %u, \"battery\": %u}\n", 
            frame->data.temperature / 100, 
            frame->data.temperature % 100, 
            frame->data.humidity, 
            frame->data.batt_voltage);
}

static const connection_callbacks_t connection_callbacks = {
    .connection_established = connection_established_callback,
    .connection_lost = connection_lost_callback,
    .frame_received = frame_received_callback
};

int main(void)
{
    int err;

    err = auxdisplay_init();
    if (err) {
        LOG_ERR("Failed to initialize HD44780 display, error %d!", err);
        return err;
    }

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Failed to enable BLE, error %d!", err);
        return err;
    }

    err = connection_init(&connection_callbacks);
    if (err) {
        LOG_ERR("Failed to initialize connection, error %d!", err);
        return err;
    }

    err = scanner_start(connection_device_found_callback);
    if (err) {
        return err;
    }

    auxdisplay_printf("Scanning...");

    return 0;
}
