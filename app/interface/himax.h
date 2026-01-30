
#pragma once

#include <stdint.h>

typedef enum {
    HIMAX_RES_QQVGA_162_122 = 0x01, // 162x122
    HIMAX_RES_QVGA_324_224  = 0x02  // 324x224
} himax_resolution_t;

/* 距離類別枚舉，對應設計文件 [1] 的數值定義 */
typedef enum {
    DIST_NA   = 0, // Not Available / Not Detected
    DIST_NEAR = 1, // <= 5 ft
    DIST_MID  = 2, // 5 ft - 7 ft
    DIST_FAR  = 3, // >= 8 ft
    DIST_ERR  = -1 // Error
} himax_distance_t;

typedef struct himax_ver_t {
    uint8_t major;
    uint8_t minor;
    uint8_t patch[2];
    uint8_t build[2];
} himax_ver_t;

int himax_get_test(void);

int himax_get_version(himax_ver_t *ver);

int himax_set_resolution(himax_resolution_t res);

int himax_set_frame_rate(uint8_t fps);

int himax_get_user_distance(int user_id);
