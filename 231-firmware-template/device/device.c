#include "device.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/unique_id.h"

void device_get_info(device_info_t *info)
{
    if (info == NULL) return;

    // Заполняем поля структуры, НИЧЕГО не печатаем
    snprintf(info->board, sizeof(info->board), "pico");
    pico_get_unique_board_id_string(info->serial, sizeof(info->serial));

    snprintf(info->manufacturer, sizeof(info->manufacturer), "Raspberry Pi");
    snprintf(info->product, sizeof(info->product), "RP2040");

    // Версия кристалла
    uint32_t chip_id = *(volatile uint32_t *)(0x40000000 + 0x4000);
    snprintf(info->chip_revision, sizeof(info->chip_revision), "B%u", (unsigned)((chip_id >> 4) & 0xF));

    // Версия SDK
    snprintf(info->sdk_version, sizeof(info->sdk_version), "%d.%d.%d",
             PICO_SDK_VERSION_MAJOR, PICO_SDK_VERSION_MINOR, PICO_SDK_VERSION_REVISION);
}
