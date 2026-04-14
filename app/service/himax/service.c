

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

#include <interface/himax.h>

LOG_MODULE_REGISTER(himax, LOG_LEVEL_INF);

static void service(void) {
    int ret = 0;
    himax_ver_t ver;

    k_sleep(K_MSEC(2000));

    // himax_get_test();
    // himax_get_version(&ver);
    // LOG_INF("Himax Version: %d.%d.%d.%d.%d.%d",
    //         ver.major, ver.minor, ver.patch[0], ver.patch[1], ver.build[0], ver.build[1]);

    // k_sleep(K_MSEC(2000));
    // himax_set_resolution(HIMAX_RES_QQVGA_162_122);

    // k_sleep(K_MSEC(2000));
    // himax_set_frame_rate(5);

    while (1) {
        int dis = 0;

        k_sleep(K_MSEC(1000));

        dis = himax_get_user_distance(0);
        if (dis >= 0) {
            LOG_INF("User 0 Distance Category: %d", dis);
        } else {
            LOG_ERR("Failed to get user distance");
            continue;
        }

        uint8_t duty = dis*30; // Simple mapping: 0->0%, 1->30%, 2->60%, 3->90%
        ret = app_fan_set(0, duty);
        if (ret != 0) {
            LOG_ERR("Failed to set fan duty cycle");
        } else {
            LOG_INF("Fan duty cycle set to %d%% based on distance category %d", duty, dis);
        }

        uint8_t ms = dis*300; // Simple mapping: 0->0ms, 1->300ms, 2->600ms, 3->900ms
        ms = (ms == 0) ? 10 : ms; // Ensure minimum blink interval
        extern void led_blink_set(uint16_t ms);
        led_blink_set(ms);
    }
}

#define STACKSIZE 2048
#define PRIORITY 7

K_THREAD_DEFINE(himax_serv_id, STACKSIZE, service, NULL, NULL, NULL, PRIORITY,
                0, 0);
