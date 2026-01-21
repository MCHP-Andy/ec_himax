#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i3c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/util_macro.h>


LOG_MODULE_REGISTER(i3c_host, LOG_LEVEL_INF);

#if DT_NODE_HAS_STATUS(DT_NODELABEL(i3c0), okay)
const struct device *const host_dev = DEVICE_DT_GET(DT_NODELABEL(i3c0));
#else
#error "Node is disabled"
#endif

#define HIMAX_DELAY 50

#define HIMAX_WRITE_ADDR 0x01
#define HIMAX_READ_ADDR 0x00

static int himax_write(struct i3c_device_desc *target, uint8_t *data,
                       uint32_t len, bool pec_en, bool hdr_en) {
    int ret = 0;
    uint8_t addr = HIMAX_WRITE_ADDR;

    if ((target == NULL) || (data == NULL) || (len <= 0)) {
        LOG_ERR("Invalid inputs!!");
        goto exit_himax_write;
    }

    if (target->dynamic_addr == 0) {
        LOG_ERR("Invalid Target!!");
        goto exit_himax_write;
    }

    struct i3c_msg msg[2];

    msg[0].buf = &addr;
    msg[0].len = 1;
    msg[0].flags = I3C_MSG_WRITE;
#ifdef USE_MCHP_H3_CHANGES
    if (true == pec_en) {
        msg[0].flags |= I3C_MSG_PEC;
    }
    if (true == hdr_en) {
        msg[0].flags |= I3C_MSG_HDR;
    }
#endif

    msg[1].buf = (uint8_t *)data;
    msg[1].len = len;
    msg[1].flags = I3C_MSG_RESTART | I3C_MSG_WRITE | I3C_MSG_STOP;
#ifdef USE_MCHP_H3_CHANGES
    if (true == pec_en) {
        msg[1].flags |= I3C_MSG_PEC;
    }
    if (true == hdr_en) {
        msg[1].flags |= I3C_MSG_HDR;
    }
#endif

    ret = i3c_transfer(target, msg, 2);

exit_himax_write:
    return ret;
}

static int himax_read(struct i3c_device_desc *target, uint8_t *data,
                      uint32_t len, bool pec_en, bool hdr_en) {
    int ret = 0;
    uint8_t addr = HIMAX_READ_ADDR;

    if ((target == NULL) || (data == NULL) || (len <= 0)) {
        LOG_ERR("Invalid inputs!!");
        goto exit_himax_write;
    }

    if (target->dynamic_addr == 0) {
        LOG_ERR("Invalid Target!!");
        goto exit_himax_write;
    }

    struct i3c_msg msg[2];

    msg[0].buf = &addr;
    msg[0].len = 1;
    msg[0].flags = I3C_MSG_WRITE;
#ifdef USE_MCHP_H3_CHANGES
    if (true == pec_en) {
        msg[0].flags |= I3C_MSG_PEC;
    }
    if (true == hdr_en) {
        msg[0].flags |= I3C_MSG_HDR;
    }
#endif
    msg[1].buf = (uint8_t *)data;
    msg[1].len = len;
    msg[1].flags = I3C_MSG_RESTART | I3C_MSG_READ | I3C_MSG_STOP;
#ifdef USE_MCHP_H3_CHANGES
    if (true == pec_en) {
        msg[1].flags |= I3C_MSG_PEC;
    }
    if (true == hdr_en) {
        msg[1].flags |= I3C_MSG_HDR;
    }
#endif

    ret = i3c_transfer(target, msg, 2);

exit_himax_write:
    return ret;
}

static int himax_get_test(struct i3c_device_desc *target) {
    bool pec_en = false;
    bool hdr_en = false;
    int ret = 0;

    uint8_t tx_data[] = {0x04, 0x02, 0x05, 0x05, 0x01, 0x00, 0x01, 0x04, 0x00};
    ret = himax_write(target, tx_data, sizeof(tx_data), pec_en, hdr_en);
    if (ret)
        LOG_ERR("himax_write error: %d", ret);

    k_sleep(K_MSEC(HIMAX_DELAY));

    uint8_t rx_data[13] = {0};
    ret = himax_read(target, rx_data, sizeof(rx_data), pec_en, hdr_en);
    if (ret)
        LOG_ERR("himax_read error: %d", ret);

    return 0;
}

static int himax_get_version(struct i3c_device_desc *target) {
    bool pec_en = false;
    bool hdr_en = false;
    int ret = 0;

    uint8_t tx_data[] = {0x01, 0x01, 0x05, 0x00};
    ret = himax_write(target, tx_data, sizeof(tx_data), pec_en, hdr_en);
    if (ret)
        LOG_ERR("himax_write error: %d", ret);

    k_sleep(K_MSEC(HIMAX_DELAY));

    uint8_t rx_data[10] = {0};
    ret = himax_read(target, rx_data, sizeof(rx_data), pec_en, hdr_en);
    if (ret)
        LOG_ERR("himax_read error: %d", ret);

    LOG_INF("Himax readback: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            rx_data[0], rx_data[1], rx_data[2], rx_data[3], rx_data[4],
            rx_data[5], rx_data[6], rx_data[7], rx_data[8], rx_data[9]);

    return 0;
}

static void i3c_test(void) {
    int ret = 0;

    k_sleep(K_SECONDS(2));

    if (!device_is_ready(host_dev)) {
        /*if device is not ready, try initializing it*/
        ret = device_init(host_dev);
        if (0 != ret) {
            LOG_ERR("Host[%s] init failed with error %d", host_dev->name, ret);
        } else {
            LOG_INF("Host[%s] init success", host_dev->name);
        }
    } else {
        LOG_INF("Host[%s] ready", host_dev->name);
    }

    const struct i3c_device_id devid = I3C_DEVICE_ID_DT(DT_NODELABEL(himax_dev));// {.pid = 0x02c400130000};
    struct i3c_device_desc *target = NULL;
    target = i3c_device_find(host_dev, &devid);
    LOG_INF("Target found: %p", target);

    if (target != NULL) {
        himax_get_test(target);

        k_sleep(K_MSEC(HIMAX_DELAY));

        himax_get_version(target);
    }
}

#define STACKSIZE 4096
#define PRIORITY 7

K_THREAD_DEFINE(i3c_test_id, STACKSIZE, i3c_test, NULL, NULL, NULL, PRIORITY, 0,
                0);
