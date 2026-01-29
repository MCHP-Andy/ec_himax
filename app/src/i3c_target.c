/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <stdio.h>
#include <zephyr/sys/util.h>
#include "zephyr/drivers/i3c.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(i3c_target, LOG_LEVEL_INF);


static int tgt0_write_requested_cb(struct i3c_target_config *config);
static int tgt0_write_received_cb(struct i3c_target_config *config, uint8_t val);
static int tgt0_read_requested_cb(struct i3c_target_config *config, uint8_t *val);
static int tgt0_read_processed_cb(struct i3c_target_config *config, uint8_t *val);
static int tgt0_stop_cb(struct i3c_target_config *config);

struct i3c_target_callbacks tgt0_cbs= {
    .write_requested_cb   = tgt0_write_requested_cb,
    .write_received_cb    = tgt0_write_received_cb,
    .read_requested_cb    = tgt0_read_requested_cb,
    .read_processed_cb    = tgt0_read_processed_cb,
    .stop_cb              = tgt0_stop_cb,
};

struct i3c_target_config tgt0_cfg;
uint8_t tgt_tx_buff[88] = {
                           0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,  0x8,
                           0x9,  0xa,  0xb,  0xc,  0xd,  0xe,  0xf,  0x10,
                           0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                           0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
                           0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                           0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
                           0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
                           0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
                           0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
                           0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
                           0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8};

static int tgt0_write_requested_cb(struct i3c_target_config *config)
{
    printf("[%s]\n", __FUNCTION__);
    return 0;
}

static int tgt0_write_received_cb(struct i3c_target_config *config, uint8_t val)
{
    printf("[%s] Data => 0x%02x\n", __FUNCTION__, val);
    return 0;
}

static int tgt0_read_requested_cb(struct i3c_target_config *config, uint8_t *val)
{
    printf("[%s]\n", __FUNCTION__);
    return 0;
}

static int tgt0_read_processed_cb(struct i3c_target_config *config, uint8_t *val)
{
    printf("[%s]\n", __FUNCTION__);
    return 0;
}

static int tgt0_stop_cb(struct i3c_target_config *config)
{
    printf("[%s]\n", __FUNCTION__);
    return 0;
}


static int i3c_target(void)
{
    int ret = 0;
    const struct device *const target_dev = DEVICE_DT_GET(DT_NODELABEL(i3c1));

    if (!device_is_ready(target_dev)) {
		/*if device is not ready, try initializing it*/
		ret = device_init(target_dev);
		if (0 != ret) {
			LOG_ERR("Target[%s] init failed with error %d", target_dev->name, ret);
		} else {
			LOG_INF("Target[%s] init success", target_dev->name);
		}
	} else {
		LOG_INF("Target[%s] ready", target_dev->name);
	}

    tgt0_cfg.callbacks = &tgt0_cbs;
	ret = i3c_target_register(target_dev, &tgt0_cfg);
	if (ret) {
		LOG_ERR("Target[%s] register failed", target_dev->name);
	} else {
		LOG_INF("Target[%s] register success", target_dev->name);
	}

    return 0;
}

#define STACKSIZE 4096
#define PRIORITY 7

K_THREAD_DEFINE(i3c_target_id, STACKSIZE, i3c_target, NULL, NULL, NULL,
		PRIORITY, 0, 0);
