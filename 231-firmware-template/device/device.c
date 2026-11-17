#include "device.h"

#include <stdio.h>

void device_print_passport(void)
{
    printf("name: %s\n", DEVICE_PROJECT_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("firmware: %s\n", DEVICE_FIRMWARE_NAME);
}
