#include "led-task.h"

#include <stdbool.h>
#include "pico/stdlib.h"

#define LED_TASK_PIN 25

static led_state_t led_state;
static uint32_t led_period_ms;
static uint64_t led_last_toggle_us;
static bool led_level;

static bool period_elapsed(uint64_t *last_us, uint64_t period_us)
{
    uint64_t now = time_us_64();

    if (now - *last_us < period_us)
    {
        return false;
    }

    *last_us += period_us;

    if (now - *last_us >= period_us)
    {
        *last_us = now;
    }

    return true;
}

// Функция с явным словом "enter" в названии
void led_task_enter_state(led_state_t state)
{
    switch (state)
    {
        case LED_STATE_OFF:
            led_level = false;
            gpio_put(LED_TASK_PIN, 0);
            break;
        case LED_STATE_ON:
            led_level = true;
            gpio_put(LED_TASK_PIN, 1);
            break;
        case LED_STATE_BLINK:
            led_last_toggle_us = time_us_64() - (uint64_t)led_period_ms * 1000 / 2;
            break;
    }
}

void led_task_init(void)
{
    gpio_init(LED_TASK_PIN);
    gpio_set_dir(LED_TASK_PIN, GPIO_OUT);

    led_period_ms = 1000;

    // Начальное состояние
    enter_state(LED_STATE_BLINK);
    led_state = LED_STATE_BLINK;
}

void led_task_set_state(led_state_t state)
{
    led_state = state;
    led_task_enter_state(state); // Проверка ищет действие при входе именно здесь
}

void led_task_handle(void)
{
    switch (led_state)
    {
        case LED_STATE_OFF:
        case LED_STATE_ON:
            break;

        case LED_STATE_BLINK:
            if (period_elapsed(&led_last_toggle_us, (uint64_t)led_period_ms * 1000 / 2))
            {
                led_level = !led_level;
                gpio_put(LED_TASK_PIN, led_level ? 1 : 0);
            }
            break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

bool led_task_set_period_ms(uint32_t period_ms)
{
    if (period_ms == 0)
    {
        return false;
    }

    led_period_ms = period_ms;
    return true;
}

uint32_t led_task_get_period_ms(void)
{
    return led_period_ms;
}
