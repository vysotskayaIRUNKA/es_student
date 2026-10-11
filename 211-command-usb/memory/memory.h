#pragma once

#include "pico/types.h"

extern uint32_t data_variable;
extern uint32_t bss_variable;

void mem_info(void);
void fw_info(void);
