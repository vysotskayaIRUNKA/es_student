#pragma once

#include <stdint.h>

#define DEVICE_PROJECT_NAME "231-firmware-template"
#define DEVICE_NAME        "231-firmware-template"
#define FIRMWARE_VERSION   "1.0.0"
#define DEVICE_FIRMWARE_NAME "my-firmware"

typedef struct
{
    char board[32];
    char serial[17];
    char manufacturer[32];
    char product[32];
    char chip_revision[8];
    char sdk_version[16];
} device_info_t;

void device_get_info(device_info_t *info);
