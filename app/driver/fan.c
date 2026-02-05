
#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(fan, LOG_LEVEL_WRN);

static const struct pwm_dt_spec fans[] = {
    PWM_DT_SPEC_GET(DT_NODELABEL(fan0)),
};


int app_fan_set(uint8_t idx, uint8_t duty) {
    int ret = 0;
    uint32_t pulse = 0;

    if (idx >= ARRAY_SIZE(fans)) {
        LOG_ERR("Invalid fan index: %d", idx);
        return -EINVAL;
    }

    if (duty > 100) {
        LOG_ERR("Invalid duty cycle: %d", duty);
        return -EINVAL;
    }

    pulse = (uint64_t)fans[idx].period * duty / 100;
    ret = pwm_set_pulse_dt(&fans[idx], pulse);
    if (ret < 0) {
        LOG_ERR("Failed to set PWM for fan: %d", ret);
        return ret;
    }

    return ret;
}

#include <zephyr/init.h>

static int init_config(void) {
    int ret = 0;

    LOG_INF("Initializing fan configuration...");

    for (size_t i = 0; i < ARRAY_SIZE(fans); i++) {
        if (!pwm_is_ready_dt(&fans[i])) {
            LOG_ERR("fan not ready");
            return -ENODEV;
        }

        ret = pwm_set_pulse_dt(&fans[i], 0);
        if (ret < 0) {
            LOG_ERR("Failed to set PWM for fan: %d", ret);
            return ret;
        }
    }

    return ret;
}

SYS_INIT(init_config, APPLICATION, 0);
