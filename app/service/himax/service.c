

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i3c.h>
#include <zephyr/logging/log.h>

#include <interface/himax.h>

LOG_MODULE_REGISTER(himax, LOG_LEVEL_INF);

static void service(void) {
    himax_ver_t ver;

    k_sleep(K_MSEC(2000));

    himax_get_test();
    himax_get_version(&ver);
    LOG_INF("Himax Version: %d.%d.%d.%d.%d.%d",
            ver.major, ver.minor, ver.patch[0], ver.patch[1], ver.build[0], ver.build[1]);

    himax_set_resolution(HIMAX_RES_QQVGA_162_122);
    himax_set_frame_rate(5);

    while (1) {
        k_sleep(K_MSEC(1000));

        himax_get_user_distance(0);
    }
}

#define STACKSIZE 2048
#define PRIORITY 7

K_THREAD_DEFINE(himax_serv_id, STACKSIZE, service, NULL, NULL, NULL, PRIORITY,
                0, 0);
