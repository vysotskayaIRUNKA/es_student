#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"

#include "device.h"
#include "command.h"

#include "led.h"

// main объявлен в main.c, заголовка у него нет — объявляем здесь
int main(void);

// ==================================================================
// Символы линкера — объявляются как extern char и берутся по адресу
// ==================================================================
extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

// ==================================================================
// Глобальные переменные — предмет опыта п2.1.4
// ==================================================================
uint32_t data_variable = 100;   // .data — значение хранится во флеш
uint32_t bss_variable;          // .bss  — в образе нет, обнуляется стартапом

// ==================================================================
// Вспомогательная строка таблицы mem_info
// ==================================================================
static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name,
           (unsigned)start,
           (unsigned)end,
           (unsigned)(end - start));
}

// ==================================================================
// mem_info — п2.1.3
// ==================================================================
void mem_info(void)
{
    // ---- границы регионов: задаёт чип, а не компоновщик ----
    const uintptr_t flash_start = XIP_BASE;
    const uintptr_t flash_size  = PICO_FLASH_SIZE_BYTES;
    const uintptr_t flash_end   = flash_start + flash_size;

    const uintptr_t sram_start  = SRAM_BASE;
    const uintptr_t sram_size   = 264 * 1024;   // RP2040 Datasheet, 2.6 Memory
    const uintptr_t sram_end    = sram_start + sram_size;

    const uintptr_t rom_start   = ROM_BASE;
    const uintptr_t rom_size    = 16 * 1024;    // RP2040 Datasheet, 2.6 Memory
    const uintptr_t rom_end     = rom_start + rom_size;

    // ---- границы образа: задаёт компоновщик ----
    const uintptr_t image_start = (uintptr_t)&__flash_binary_start;
    const uintptr_t image_end   = (uintptr_t)&__flash_binary_end;
    const uintptr_t boot2_start = (uintptr_t)&__boot2_start__;
    const uintptr_t boot2_end   = (uintptr_t)&__boot2_end__;
    const uintptr_t etext       = (uintptr_t)&__etext;
    const uintptr_t data_start  = (uintptr_t)&__data_start__;
    const uintptr_t data_end    = (uintptr_t)&__data_end__;
    const uintptr_t bss_start   = (uintptr_t)&__bss_start__;
    const uintptr_t bss_end     = (uintptr_t)&__bss_end__;
    const uintptr_t heap_limit  = (uintptr_t)&__HeapLimit;
    const uintptr_t stack_bot   = (uintptr_t)&__StackBottom;
    const uintptr_t stack_top   = (uintptr_t)&__StackTop;

    // ---- шапка ----
    printf("%-10s %-10s %-10s %8s\n", "area", "start", "end", "size");

    // ---- регионы ----
    row("flash", flash_start, flash_end);
    row("sram",  sram_start,  sram_end);
    row("rom",   rom_start,   rom_end);

    // ---- образ ----
    row("image", image_start, image_end);
    row("free",  image_end,   flash_end);

    // ---- секции ----
    row("boot2",       boot2_start, boot2_end);
    row("text",        boot2_end,   etext);

    // .data живёт в двух местах: хранится во флеш, работает в ОЗУ
    row("data flash",  etext,       etext + (data_end - data_start));
    row("data ram",    data_start,  data_end);

    row("bss",   bss_start, bss_end);
    row("heap",  bss_end,   heap_limit);
    row("stack", stack_bot, stack_top);

    // ---- итоги ----
    const uintptr_t image_size = image_end - image_start;
    const uintptr_t boot2_size = boot2_end - boot2_start;
    const uintptr_t text_size  = etext - boot2_end;
    const uintptr_t data_size  = data_end - data_start;
    const uintptr_t bss_size   = bss_end - bss_start;

    printf("\ntotal\n");
    printf("  flash image %8u = boot2 %u + text %u + data %u\n",
           (unsigned)image_size,
           (unsigned)boot2_size,
           (unsigned)text_size,
           (unsigned)data_size);

    printf("  flash free  %8u of %u\n",
           (unsigned)(flash_size - image_size),
           (unsigned)flash_size);

    printf("  ram used    %8u = data %u + bss %u\n",
           (unsigned)(data_size + bss_size),
           (unsigned)data_size,
           (unsigned)bss_size);

    printf("  ram free    %8u for heap and %u for stack\n",
           (unsigned)(heap_limit - bss_end),
           (unsigned)(stack_top - stack_bot));
}

// ==================================================================
// fw_info — п2.1.4
// ==================================================================
void fw_info(void)
{
    // ---- счётчики вызовов ----
    data_variable += 1;
    bss_variable  += 1;

    // ---- адреса функций со сброшенным признаком Thumb ----
    uintptr_t main_addr    = (uintptr_t)main    & ~1u;
    uintptr_t fw_info_addr = (uintptr_t)fw_info & ~1u;

    uint16_t main_code    = *(uint16_t *)main_addr;
    uint16_t fw_info_code = *(uint16_t *)fw_info_addr;

    // ---- локальная переменная и блок в куче ----
    uint32_t  stack_variable = 1946;
    uint32_t *heap_variable  = malloc(sizeof(uint32_t));

    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }

    // ---- шапка ----
    printf("object          address     value\n");

    // ---- код ----
    printf("main            0x%08x  0x%04x\n",
           (unsigned)main_addr, (unsigned)main_code);
    printf("fw_info         0x%08x  0x%04x\n",
           (unsigned)fw_info_addr, (unsigned)fw_info_code);

    // ---- таблица команд ----
    printf("commands        0x%08x\n", (unsigned)(uintptr_t)commands);

    for (uint i = 0; i < command_count; i++)
    {
        printf("- %-13s 0x%08x\n",
               commands[i].name,
               (unsigned)(uintptr_t)commands[i].handler);
    }

    // ---- константы паспорта ----
    printf("DEVICE_PROJECT  0x%08x  %s\n",
           (unsigned)(uintptr_t)DEVICE_PROJECT, DEVICE_PROJECT);
    printf("DEVICE_BOARD    0x%08x  %s\n",
           (unsigned)(uintptr_t)DEVICE_BOARD, DEVICE_BOARD);

    // ---- глобальные переменные ----
    printf("data_variable   0x%08x  %u\n",
           (unsigned)(uintptr_t)&data_variable, (unsigned)data_variable);
    printf("bss_variable    0x%08x  %u\n",
           (unsigned)(uintptr_t)&bss_variable, (unsigned)bss_variable);

    // ---- стек ----
    printf("stack_variable  0x%08x  %u\n",
           (unsigned)(uintptr_t)&stack_variable, (unsigned)stack_variable);

    // ---- куча ----
    if (heap_variable != NULL)
    {
        printf("heap_variable   0x%08x  %u\n",
               (unsigned)(uintptr_t)heap_variable, (unsigned)*heap_variable);

        free(heap_variable);
    }
    else
    {
        printf("heap_variable   NULL\n");
    }
}

#define VECTOR_TABLE 0x10000100
#define GPIO_IN ((volatile uint32_t *)0xd0000004)

void boot_info(void)
{
    const uint32_t *vectors = (const uint32_t *)VECTOR_TABLE;

    uint32_t stack_top = vectors[0];
    uint32_t reset_handler = vectors[1];

    uint32_t level = (*GPIO_IN >> led_pin()) & 1u;

    printf("vector table   0x%08x\n", (unsigned)VECTOR_TABLE);
    printf("  stack top    0x%08x\n", (unsigned)stack_top);
    printf("  reset        0x%08x\n", (unsigned)reset_handler);
    printf("  reset (even) 0x%08x\n", (unsigned)(reset_handler & ~1u));
    printf("gpio in        0x%08x\n", (unsigned)(uintptr_t)GPIO_IN);
    printf("  led bit      %u\n", (unsigned)level);
    printf("  gpio_get     %u\n", (unsigned)gpio_get(led_pin()));
}
