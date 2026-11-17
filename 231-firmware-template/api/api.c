#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "device.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"
#include "profiling.h"

typedef void (*api_callback_t)(const command_t *command);

typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

// Разбор числа из строки через strtoul (без sscanf — экономия образа).
static bool parse_u32(const char *text, uint32_t *value)
{
    if (text == NULL || *text == '\0')
    {
        return false;
    }

    char *end = NULL;
    unsigned long number = strtoul(text, &end, 10);

    if (end == text || *end != '\0')
    {
        return false;
    }

    if (number > 0xFFFFFFFFul)
    {
        return false;
    }

    *value = (uint32_t)number;
    return true;
}

// ---------- обработчики команд ----------

static void cmd_info(const command_t *command)
{
    (void)command;
    device_info_t info;
    device_get_info(&info);

    printf("name: %s\n", DEVICE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("board: %s\n", info.board);
    printf("serial: %s\n", info.serial);
    printf("manufacturer: %s\n", info.manufacturer);
    printf("product: %s\n", info.product);
    printf("chip: %s\n", info.chip_revision);
    printf("sdk: %s\n", info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    (void)command;
    printf("uptime: %llu ms\n", (unsigned long long)(time_us_64() / 1000));
}


/*static void cmd_main_time_exec(const command_t *command)
{
    (void)command;
    extern void main_time_exec_print(void);
    main_time_exec_print();
}

static void cmd_main_time_reset(const command_t *command)
{
    (void)command;
    extern void main_time_reset(void);
    main_time_reset();
}*/

// ---------- обработчики светодиода ----------

static void led_print_state(void)
{
    const char *name;
    switch (led_task_get_state())
    {
        case LED_STATE_OFF:   name = "off";   break;
        case LED_STATE_ON:    name = "on";    break;
        case LED_STATE_BLINK: name = "blink"; break;
        default:              name = "?";     break;
    }

    printf("led: %s, period %u ms\n", name, (unsigned)led_task_get_period_ms());
}

static void cmd_led_enable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_ON);
    led_print_state();
}

static void cmd_led_disable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_OFF);
    led_print_state();
}

static void cmd_led_blink(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_BLINK);
    led_print_state();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }

    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    led_print_state();
}

static void cmd_button(const command_t *command)
{
    (void)command;
    const char *state = button_task_is_pressed() ? "pressed" : "released";
    printf("button: %s, presses %u\n", state, (unsigned)button_task_get_press_count());
}

static void cmd_pi_start(const command_t *command)
{
    uint32_t terms = 1000000; // по умолчанию из примера задания

    if (command->argc > 1)
    {
        printf("error: usage pi_start [terms]\n");
        return;
    }

    if (command->argc == 1)
    {
        if (!parse_u32(command->argv[0], &terms))
        {
            printf("error: usage pi_start [terms]\n");
            return;
        }
    }

    if (terms == 0)
    {
        printf("error: pi_start: terms must be greater than zero\n");
        return;
    }

    if (!pi_task_start(terms))
    {
        printf("error: pi is busy\n");
        return;
    }

    printf("pi: started, %u terms\n", (unsigned)terms);
}

static void cmd_pi(const command_t *command)
{
    (void)command;

    switch (pi_task_get_state())
    {
        case PI_STATE_NOT_STARTED:
            printf("pi: not started\n");
            break;
        case PI_STATE_RUNNING:
            printf("pi: running\n");
            break;
        case PI_STATE_READY:
        {
            double val;
            uint32_t ms;
            if (pi_task_get_result(&val, &ms))
            {
                printf("pi: %.9f in %u ms\n", val, (unsigned)ms);
            }
            break;
        }
    }
}

static void cmd_profiling(const command_t *command)
{
    (void)command;
    profiling_result_t res;
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++)
    {
        if (profiling_get(i, &res))
        {
            uint32_t mean = res.count > 0 ? res.sum_us / res.count : 0;
            printf("%-10s count %-10u mean %u us  max %u us\n",
                   res.name, (unsigned)res.count, (unsigned)mean, (unsigned)res.max_us);
        }
    }
}

static void cmd_profiling_reset(const command_t *command)
{
    (void)command;
    profiling_reset();
    printf("profiling reset\n");
}

// ---------- таблица команд ----------

static const api_command_t api_commands[] =
{
    { "info",            "device passport",                                   cmd_info },
    { "uptime",          "time since reset",                                  cmd_uptime },
    { "profiling",  "superloop iteration: average and maximum",          cmd_profiling },
    { "profiling_reset", "reset maximum iteration",                           cmd_profiling_reset },
    { "led_enable",      "turn led on",                                       cmd_led_enable },
    { "led_disable",     "turn led off",                                      cmd_led_disable },
    { "led_blink",       "blink led",                                         cmd_led_blink },
    { "led_period",      "set blink period: led_period <ms>",                 cmd_led_period },
    {"button", "button state and press count", cmd_button},
    { "pi_start", "start calculating pi: pi_start [terms]", cmd_pi_start },
    { "pi",       "get pi result or state",                 cmd_pi },
};

#define API_COMMAND_COUNT (sizeof(api_commands) / sizeof(api_commands[0]))

// ---------- help ----------

static void api_print_help(void)
{
    printf("help             list of commands\n");
    for (uint32_t i = 0; i < API_COMMAND_COUNT; i++)
    {
        printf("%-16s %s\n", api_commands[i].name, api_commands[i].help);
    }
}

// ---------- api_handle ----------

void api_handle(const command_t *command)
{
    // 1. обрезанная команда не выполняется
    if (command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    // 2. help выполняет сам api
    if (strcmp(command->name, "help") == 0)
    {
        api_print_help();
        return;
    }

    // 3. поиск в таблице
    for (uint32_t i = 0; i < API_COMMAND_COUNT; i++)
    {
        if (strcmp(command->name, api_commands[i].name) == 0)
        {
            api_commands[i].callback(command);
            return;
        }
    }

    // 4. неизвестная команда
    printf("error: unknown command '%s', try help\n", command->name);
}
