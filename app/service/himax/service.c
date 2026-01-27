

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i3c.h>
#include <zephyr/logging/log.h>

#include <interface/himax.h>

LOG_MODULE_REGISTER(himax, LOG_LEVEL_INF);

static void service(void) {

    k_sleep(K_MSEC(2000));

    himax_get_test();
    himax_get_version();

    while (1) {
        k_sleep(K_MSEC(1000));
    }
}

#define STACKSIZE 1024
#define PRIORITY 7

K_THREAD_DEFINE(himax_serv_id, STACKSIZE, service, NULL, NULL, NULL, PRIORITY,
                0, 0);
