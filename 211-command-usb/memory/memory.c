#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void mem_info(void)
{
    // шапка таблицы
    printf("%-10s %-10s %-10s %8s\n", "area", "start", "end", "size");

    // --- физическая память чипа ---
    row("flash", XIP_BASE, XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("sram",  SRAM_BASE, SRAM_BASE + 264 * 1024);
    row("rom",   ROM_BASE,  ROM_BASE  + 16  * 1024);

    // --- размещение образа во флеш ---
    extern uint32_t __flash_binary_start;
    extern uint32_t __flash_binary_end;
    extern uint32_t __boot2_start__;
    extern uint32_t __boot2_end__;
    extern uint32_t __etext;

    row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    row("free",  (uintptr_t)&__flash_binary_end,
        XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
    row("text",  (uintptr_t)&__boot2_end__,   (uintptr_t)&__etext);

    // --- .data: где хранится во флеш и где работает в ОЗУ ---
    extern uint32_t __data_start__;
    extern uint32_t __data_end__;

    uintptr_t data_ram_start = (uintptr_t)&__data_start__;
    uintptr_t data_ram_end   = (uintptr_t)&__data_end__;
    uintptr_t data_size      = data_ram_end - data_ram_start;

    row("data flash", (uintptr_t)&__etext, (uintptr_t)&__etext + data_size);
    row("data ram",   data_ram_start, data_ram_end);

    // --- bss / heap / stack ---
    extern uint32_t __bss_start__;
    extern uint32_t __bss_end__;
    extern uint32_t __HeapLimit;
    extern uint32_t __StackBottom;
    extern uint32_t __StackTop;

    row("bss",   (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
    row("heap",  (uintptr_t)&__bss_end__,   (uintptr_t)&__HeapLimit);
    row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

    // --- итоги ---
    printf("------------------------------------------------\n");

    uintptr_t image_size = (uintptr_t)&__flash_binary_end
    - (uintptr_t)&__flash_binary_start;
    uintptr_t free_flash = PICO_FLASH_SIZE_BYTES - image_size;
    uintptr_t ram_used   = data_size
    + ((uintptr_t)&__bss_end__ - (uintptr_t)&__bss_start__);
    uintptr_t ram_free   = ((uintptr_t)&__HeapLimit - (uintptr_t)&__bss_end__)
    + ((uintptr_t)&__StackTop - (uintptr_t)&__StackBottom);

    printf("image total: %8u\n", (unsigned)image_size);
    printf("free flash:  %8u / %u\n", (unsigned)free_flash,
           (unsigned)PICO_FLASH_SIZE_BYTES);
    printf("ram used:    %8u\n", (unsigned)ram_used);
    printf("ram free:    %8u\n", (unsigned)ram_free);
}
