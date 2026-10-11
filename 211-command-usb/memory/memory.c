// memory.c

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"
#include "memory.h"

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

// Символы линкера: у них нет значения, есть только адрес.
// Поэтому тип — char, а не uint32_t.
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

void mem_info(void)
{
    // --- шапка таблицы ---
    printf("%-10s %-10s %-10s %8s\n", "area", "start", "end", "size");
    printf("------------------------------------------------\n");

    // --- физическая память чипа ---
    row("flash", XIP_BASE, XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("sram",  SRAM_BASE, SRAM_END);          // 0x20000000 .. 0x20042000
    row("rom",   ROM_BASE,  ROM_BASE + 16 * 1024);

    // --- размещение образа во флеш ---
    uintptr_t flash_start = (uintptr_t)&__flash_binary_start;
    uintptr_t flash_end   = (uintptr_t)&__flash_binary_end;
    uintptr_t boot2_start = (uintptr_t)&__boot2_start__;
    uintptr_t boot2_end   = (uintptr_t)&__boot2_end__;
    uintptr_t etext       = (uintptr_t)&__etext;

    row("image",  flash_start, flash_end);
    row("free",   flash_end,   XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("boot2",  boot2_start, boot2_end);
    row("text",   boot2_end,   etext);

    // --- .data: хранение во флеш и работа в ОЗУ ---
    uintptr_t data_ram_start = (uintptr_t)&__data_start__;
    uintptr_t data_ram_end   = (uintptr_t)&__data_end__;
    uintptr_t data_size      = data_ram_end - data_ram_start;

    row("data flash", etext,          etext + data_size);
    row("data ram",   data_ram_start, data_ram_end);

    // --- bss / heap / stack ---
    uintptr_t bss_start    = (uintptr_t)&__bss_start__;
    uintptr_t bss_end      = (uintptr_t)&__bss_end__;
    uintptr_t heap_limit   = (uintptr_t)&__HeapLimit;
    uintptr_t stack_bottom = (uintptr_t)&__StackBottom;
    uintptr_t stack_top    = (uintptr_t)&__StackTop;

    row("bss",   bss_start,    bss_end);
    row("heap",  bss_end,      heap_limit);
    row("stack", stack_bottom, stack_top);

    // --- итоги ---
    printf("------------------------------------------------\n");

    uintptr_t image_size = flash_end - flash_start;
    uintptr_t free_flash = PICO_FLASH_SIZE_BYTES - image_size;
    uintptr_t ram_used   = data_size + (bss_end - bss_start);
    uintptr_t ram_free   = (heap_limit - bss_end) + (stack_top - stack_bottom);

    printf("%-10s %8u\n", "image",  (unsigned)image_size);
    printf("%-10s %8u / %u\n", "free", (unsigned)free_flash,
           (unsigned)PICO_FLASH_SIZE_BYTES);
    printf("%-10s %8u\n", "ram used", (unsigned)ram_used);
    printf("%-10s %8u\n", "ram free", (unsigned)ram_free);
}
