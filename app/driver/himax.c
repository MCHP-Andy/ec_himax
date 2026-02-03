
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i3c.h>
#include <zephyr/logging/log.h>

#include <interface/himax.h>

LOG_MODULE_REGISTER(himax_drv, LOG_LEVEL_INF);

#define HIMAX_DELAY 50
#define HIMAX_WAKEUP_DELAY_MS 20
#define HIMAX_PROCESS_DELAY_MS 50

#define HIMAX_WRITE_ADDR 0x01
#define HIMAX_READ_ADDR 0x00

#define HIMAX_HEADER_SIZE 4

/* 定義 OTA 命令代碼 [3] */
#define FEAT_OTA_OP    0x50
#define FEAT_OTA_UPG   0x51

#define CMD_JUMP2UPG   0x08
#define CMD_START      0x09
#define CMD_DATA       0x0A
#define CMD_END        0x0B
#define CMD_REBOOT     0x05

static struct i3c_device_desc *target = NULL;
static const struct gpio_dt_spec wk_gpio = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), himax_wk_gpios);

static int himax_write(uint8_t *data, uint32_t len, bool pec_en, bool hdr_en) {
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

static int himax_read(uint8_t *data, uint32_t len, bool pec_en, bool hdr_en) {
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

int himax_get_test(void) {
    bool pec_en = false;
    bool hdr_en = false;
    int ret = 0;

    uint8_t tx_data[] = {0x04, 0x02, 0x05, 0x05, 0x01, 0x00, 0x01, 0x04, 0x00};
    ret = himax_write(tx_data, sizeof(tx_data), pec_en, hdr_en);
    if (ret)
        LOG_ERR("himax_write error: %d", ret);

    k_sleep(K_MSEC(HIMAX_DELAY));

    uint8_t rx_data[13] = {0};
    ret = himax_read(rx_data, sizeof(rx_data), pec_en, hdr_en);
    if (ret)
        LOG_ERR("himax_read error: %d", ret);

    return 0;
}

int himax_get_version(himax_ver_t *ver) {
    bool pec_en = false;
    bool hdr_en = false;
    int ret = 0;

    if (ver == NULL) {
        LOG_ERR("Invalid version structure");
        return -EINVAL;
    }
    

    uint8_t tx_data[] = {0x01, 0x01, 0x05, 0x00};
    ret = himax_write(tx_data, sizeof(tx_data), pec_en, hdr_en);
    if (ret)
        LOG_ERR("himax_write error: %d", ret);

    k_sleep(K_MSEC(HIMAX_DELAY));

    uint8_t rx_data[10] = {0};
    ret = himax_read(rx_data, sizeof(rx_data), pec_en, hdr_en);
    if (ret)
        LOG_ERR("himax_read error: %d", ret);

    LOG_DBG("Himax readback: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            rx_data[0], rx_data[1], rx_data[2], rx_data[3], rx_data[4],
            rx_data[5], rx_data[6], rx_data[7], rx_data[8], rx_data[9]);
    
    ver->major = rx_data[4];
    ver->minor = rx_data[5];
    ver->patch[0] = rx_data[6];
    ver->patch[1] = rx_data[7];
    ver->build[0] = rx_data[8];
    ver->build[1] = rx_data[9];

    return 0;
}

/**
 * 設定感測器解析度 (Set Sensor Resolution)
 *
 * I3C Command: 01 15 05 01 [Value]
 * 參考來源: Design Document Section 7.1 [1], Flow chart [2]
 */
int himax_set_resolution(himax_resolution_t res) {
    int ret = 0;
    bool pec_en = false;
    bool hdr_en = false;

    /* 檢查參數有效性 */
    if (res != HIMAX_RES_QQVGA_162_122 && res != HIMAX_RES_QVGA_324_224) {
        LOG_ERR("Invalid resolution value");
        return -EINVAL;
    }

    /* 建構命令封包 (5 Bytes) */
    /* Feature(01) | Cmd(15) | Flag(05) | Len(01) | Value(res) */
    uint8_t cmd_buffer[] = {0x01, 0x15, 0x05, 0x01, (uint8_t)res};

    /* 1. GPIO High: 喚醒裝置進入 Special Mode */
    gpio_pin_set_dt(&wk_gpio, 1);

    /* 2. Delay 20ms: 等待裝置喚醒 */
    k_sleep(K_MSEC(HIMAX_WAKEUP_DELAY_MS));

    /* 3. 發送設定命令 */
    ret = himax_write(cmd_buffer, sizeof(cmd_buffer), pec_en, hdr_en);
    if (ret != 0) {
        LOG_ERR("Failed to set resolution: %d", ret);
        // 即使失敗也要執行 GPIO Low 復原狀態
    }

    /* 4. Delay 50ms: 等待 WE2 處理設定 */
    k_sleep(K_MSEC(HIMAX_PROCESS_DELAY_MS));

    /* 5. GPIO Low: 結束操作，回到 Sleep Mode */
    gpio_pin_set_dt(&wk_gpio, 0);

    if (ret == 0) {
        LOG_INF("Resolution set to %s",
                (res == HIMAX_RES_QVGA_324_224) ? "QVGA" : "QQVGA");
    }

    return ret;
}

/**
 * 設定幀率 (Set Frame Rate)
 *
 * I3C Command: 01 16 05 01 [FPS]
 * 參考來源: Design Document Section 7.1 [2]
 */
int himax_set_frame_rate(uint8_t fps) {
    int ret = 0;
    bool pec_en = false;
    bool hdr_en = false;

    /* 參數檢查: FPS 範圍 1~5 (參考來源 [2]) */
    if (fps < 1 || fps > 5) {
        LOG_ERR("Invalid FPS: %d (Range: 1-5)", fps);
        return -EINVAL;
    }

    /* 建構命令封包 (5 Bytes) */
    /* Feature(01) | Cmd(16) | Flag(05) | Len(01) | Value(fps) */
    uint8_t cmd_buffer[] = {0x01, 0x16, 0x05, 0x01, fps};

    /* 1. GPIO High: 喚醒裝置 */
    gpio_pin_set_dt(&wk_gpio, 1);

    /* 2. Delay 20ms */
    k_sleep(K_MSEC(HIMAX_WAKEUP_DELAY_MS));

    /* 3. 發送設定命令 */
    ret = himax_write(cmd_buffer, sizeof(cmd_buffer), pec_en, hdr_en);
    if (ret != 0) {
        LOG_ERR("Failed to set frame rate: %d", ret);
    }

    /* 4. Delay 50ms: 等待 WE2 處理 */
    k_sleep(K_MSEC(HIMAX_PROCESS_DELAY_MS));

    /* 5. GPIO Low: 釋放裝置 */
    gpio_pin_set_dt(&wk_gpio, 0);

    if (ret == 0) {
        LOG_INF("Frame Rate set to %d FPS", fps);
    }

    return ret;
}

#define HIMAX_METADATA_CMD_LEN 4
#define HIMAX_METADATA_RESP_LEN 54
#define HIMAX_DELAY_MS 10

/* 定義 I3C Read Metadata 命令 Payload [2] */
static uint8_t metadata_cmd[2][4] = {
    {0x01, 0x13, 0x05, 0x00},
    {0x01, 0x14, 0x05, 0x00},
};

/**
 * 讀取 Metadata 的主要函式
 * @param buf 用於儲存讀取回來的 54 bytes 資料緩衝區
 */
static int himax_read_metadata(uint8_t *pbuf) {
    int ret = 0;
    bool pec_en = false; // 根據需求設定
    bool hdr_en = false; // 根據需求設定
    uint8_t buf[HIMAX_METADATA_RESP_LEN] = {0};

    /* 1. Set GPIO High to wake up WE2 (Enter Special Mode) [1] */
    gpio_pin_set_dt(&wk_gpio, 1);

    /* 等待裝置喚醒，設計文件建議 20ms，此處可依實際情況調整 [3] */
    k_sleep(K_MSEC(100));

    ret =
        himax_write(&metadata_cmd[0][0], sizeof(metadata_cmd[0]), pec_en, hdr_en);
    if (ret != 0) {
        LOG_ERR("Failed to write metadata command: %d", ret);
        goto exit_sequence;
    }

    k_sleep(K_MSEC(50));

    /* 2. Send "Read Metadata" Command [2] */
    /* 使用提供的 himax_write API */
    ret =
        himax_write(&metadata_cmd[1][0], sizeof(metadata_cmd[1]), pec_en, hdr_en);
    if (ret != 0) {
        LOG_ERR("Failed to write metadata command: %d", ret);
        goto exit_sequence;
    }

    /* 3. Delay for WE2 to prepare data [3, 4] */
    k_sleep(K_MSEC(50));

    /* 4. Read Metadata Response [2, 5] */
    /* 使用提供的 himax_read API，讀取 54 bytes */
    ret = himax_read(buf, HIMAX_METADATA_RESP_LEN, pec_en, hdr_en);
    if (ret != 0) {
        LOG_ERR("Failed to read metadata: %d", ret);
    } else {
        LOG_INF("Metadata Read Success. Header: %02x %02x %02x %02x", buf[0],
                buf[1], buf[2], buf[3]);

        memcpy(pbuf, buf, HIMAX_METADATA_RESP_LEN);
    }

exit_sequence:
    /* 5. Set GPIO Low to finish (Return to Sleep Mode) [1] */
    gpio_pin_set_dt(&wk_gpio, 0);

    return ret;
}

/* 索引查找表 (Lookup Table) */
static const uint8_t user_distance_indices[] = {
    11, // User 0
    24, // User 1
    31, // User 2
    38, // User 3
    45, // User 4
    52  // User 5
};

/**
 * 讀取指定使用者的距離分類
 *
 * @param user_id   輸入 0 (主要使用者) 或 1~5 (其他人員)
 * @return          回傳距離枚舉 (0~3)，若失敗則回傳 -1
 */
int himax_get_user_distance(int user_id) {
    int ret = 0;
#define HIMAX_METADATA_MAX_SIZE                                                \
    60 // 加大 Buffer 以容納 User 5 (Index 52 + 4 Header = 56)
    uint8_t rx_buffer[HIMAX_METADATA_MAX_SIZE] = {0};

    /* 1. 參數驗證 */
    if (user_id < 0 || user_id > 5) {
        LOG_ERR("Invalid User ID: %d (Must be 0-5)", user_id);
        return DIST_ERR;
    }

    ret = himax_read_metadata(rx_buffer);
    if (ret) {
        return DIST_NA;
    }

    /* 7. 解析數據 */
    // 計算 Buffer Index: Header Offset (4) + Payload Index
    uint8_t buffer_idx = user_distance_indices[user_id];

    // 安全檢查：防止存取越界
    if (buffer_idx >= sizeof(rx_buffer)) {
        LOG_ERR("Buffer overflow access attempt at index %d", buffer_idx);
        return DIST_ERR;
    }

    uint8_t distance_value = rx_buffer[buffer_idx];

    // 驗證數值有效性 (0, 1, 2, 3)
    if (distance_value > 3) {
        LOG_WRN("Unexpected distance value: %d", distance_value);
        return DIST_NA; // 視為無效
    }

    LOG_INF("User %d Distance: %d", user_id, distance_value);
    return (int)distance_value;
}

/* CRC16 計算函式 (需實作 MCRF4XX 算法) */
static uint16_t calculate_crc16(uint8_t *data, uint32_t len) {
    return 0;
}

/* 封裝並發送 OTA I2C 命令 */
static int send_ota_cmd(const struct device *i2c_dev, uint8_t feature, uint8_t cmd, uint8_t *payload, uint16_t pay_len) {
    uint8_t buffer[256 + 10]; // 根據 Payload 大小調整
    uint16_t crc;
    
    // 1. 建構 Header
    buffer[0] = feature;
    buffer[1] = cmd;
    buffer[2] = pay_len & 0xFF;        // Length LSB
    buffer[3] = (pay_len >> 8) & 0xFF; // Length MSB
    
    // 2. 複製 Payload
    if (pay_len > 0 && payload != NULL) {
        memcpy(&buffer[4], payload, pay_len);
    }
    
    // 3. 計算 CRC (範圍: 從 Feature 到 Payload 結束) [3]
    // Checksum range = buf ... buf[N+3]
    crc = calculate_crc16(buffer, 4 + pay_len);
    
    // 4. 填入 CRC (Little Endian)
    buffer[4 + pay_len] = crc & 0xFF;
    buffer[4 + pay_len + 1] = (crc >> 8) & 0xFF;
    
    // 5. 透過 I2C 發送 (注意：不是 I3C)
    // i2c_write(i2c_dev, buffer, 4 + pay_len + 2, I2C_ADDR_WE2);
    return 0; 
}

/* 完整更新流程 */
void himax_fw_update_flow(const struct device *i2c_dev, uint8_t *fw_image, uint32_t fw_size) {
    // Set to i2c mode

    // Step 1: Jump to OTA
    send_ota_cmd(i2c_dev, FEAT_OTA_OP, CMD_JUMP2UPG, NULL, 0);
    k_sleep(K_MSEC(100)); // 等待重啟

    // Step 2: Start OTA
    send_ota_cmd(i2c_dev, FEAT_OTA_UPG, CMD_START, NULL, 0);
    
    // Step 3: Send Data (Chunking)
    uint32_t offset = 0;
    uint16_t chunk_size = 200; // 依據 I2C buffer 限制設定
    while (offset < fw_size) {
        uint16_t len = (fw_size - offset > chunk_size) ? chunk_size : (fw_size - offset);
        send_ota_cmd(i2c_dev, FEAT_OTA_UPG, CMD_DATA, &fw_image[offset], len);
        offset += len;
        // 建議加入簡單延遲或讀取 Status 確保寫入完成
        k_sleep(K_MSEC(5)); 
    }
    
    // Step 4: End OTA
    send_ota_cmd(i2c_dev, FEAT_OTA_UPG, CMD_END, NULL, 0);
    k_sleep(K_MSEC(100)); // 等待驗證

    // Step 5: Reboot
    send_ota_cmd(i2c_dev, FEAT_OTA_OP, CMD_REBOOT, NULL, 0);

    // Set to i3c mode
}


#include <zephyr/init.h>
static int hinax_init(void) {
    LOG_INF("Himax driver initialized");

    const struct device *const host_dev = DEVICE_DT_GET(DT_PARENT(DT_NODELABEL(himax_dev)));
    const struct i3c_device_id devid =
        I3C_DEVICE_ID_DT(DT_NODELABEL(himax_dev)); // {.pid = 0x02c400130000};
    target = i3c_device_find(host_dev, &devid);
    LOG_INF("Target found: %p", target);

    gpio_pin_configure_dt(&wk_gpio, GPIO_OUTPUT_INACTIVE);

    // himax_set_resolution(HIMAX_RES_QQVGA_162_122);
    // himax_set_frame_rate(5);

    return 0;
}

SYS_INIT(hinax_init, APPLICATION, 0);
