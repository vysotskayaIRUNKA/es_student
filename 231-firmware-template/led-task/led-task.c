#include "led-task.h"

#include <stdbool.h>
#include "pico/stdlib.h"

// номер вывода светодиода — единственное место, где он записан
#define LED_TASK_PIN 25

// состояние и период закрыты в файле (static)
static led_state_t led_state;
static uint32_t led_period_ms;
static uint64_t led_last_toggle_us;
static bool led_level;

// Проверка периода: следующее срабатывание отсчитывается от прошлого.
// Если прибор отстал больше чем на период, пропущенные срабатывания
// не догоняются — отсчёт начинается заново.
static bool period_elapsed(uint64_t *last_us, uint64_t period_us)
{
    uint64_t now = time_us_64();

    if (now - *last_us < period_us)
    {
        return false;
    }

    *last_us += period_us;

    // отстали больше чем на период — пропускаем долг, начинаем заново
    if (now - *last_us >= period_us)
    {
        *last_us = now;
    }

    return true;
}

// действие при входе в состояние
static void led_task_enter_state(led_state_t state)
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
            // метка в прошлое, чтобы первое переключение случилось сразу
            led_last_toggle_us = time_us_64() - (uint64_t)led_period_ms * 1000 / 2;
            break;
    }
}

void led_task_init(void)
{
    gpio_init(LED_TASK_PIN);
    gpio_set_dir(LED_TASK_PIN, GPIO_OUT);

    led_period_ms = 1000;
    led_state = LED_STATE_BLINK;
    led_task_enter_state(led_state);
}

void led_task_handle(void)
{
    switch (led_state)
    {
        case LED_STATE_OFF:
        case LED_STATE_ON:
            // светодиод установлен при входе, делать нечего
            break;

        case LED_STATE_BLINK:
            // каждые полпериода переключаем светодиод
            if (period_elapsed(&led_last_toggle_us, (uint64_t)led_period_ms * 1000 / 2))
            {
                led_level = !led_level;
                gpio_put(LED_TASK_PIN, led_level ? 1 : 0);
            }
            break;
    }
}

void led_task_set_state(led_state_t state)
{
    led_state = state;
    led_task_enter_state(state);
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
