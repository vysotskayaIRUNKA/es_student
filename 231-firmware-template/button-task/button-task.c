#include "button-task.h"
#include "led-task.h"
#include "pico/stdlib.h"

#define BUTTON_PIN 15
#define DEBOUNCE_TIME_US 20000 // 20 мс

// Состояния машины кнопки
typedef enum {
    BUTTON_STATE_RELEASED,    // отпущена
    BUTTON_STATE_PRESS_MAYBE, // нажатие?
    BUTTON_STATE_PRESSED,     // нажата
    BUTTON_STATE_RELEASE_MAYBE // отпускание?
} button_state_t;

static button_state_t button_state;
static uint64_t button_timer_us;
static uint32_t press_count;

void button_task_init(void)
{
    // Настройка вывода на вход с подтяжкой к питанию
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    button_state = BUTTON_STATE_RELEASED;
    button_timer_us = 0;
    press_count = 0;
}

void button_task_handle(void)
{
    // Нажатая кнопка замыкает вывод на землю и дает 0 (false)
    bool is_level_pressed = !gpio_get(BUTTON_PIN);
    uint64_t now_us = time_us_64();

    switch (button_state)
    {
        case BUTTON_STATE_RELEASED:
            if (is_level_pressed)
            {
                button_timer_us = now_us;
                button_state = BUTTON_STATE_PRESS_MAYBE;
            }
            break;

        case BUTTON_STATE_PRESS_MAYBE:
            if (!is_level_pressed)
            {
                // Это был дребезг
                button_state = BUTTON_STATE_RELEASED;
            }
            else if (now_us - button_timer_us >= DEBOUNCE_TIME_US)
            {
                // Нажатие подтверждено
                button_state = BUTTON_STATE_PRESSED;
                press_count++;

                // Переключаем светодиод
                led_task_next_state();
            }
            break;

        case BUTTON_STATE_PRESSED:
            if (!is_level_pressed)
            {
                button_timer_us = now_us;
                button_state = BUTTON_STATE_RELEASE_MAYBE;
            }
            break;

        case BUTTON_STATE_RELEASE_MAYBE:
            if (is_level_pressed)
            {
                // Это был дребезг при отпускании
                button_state = BUTTON_STATE_PRESSED;
            }
            else if (now_us - button_timer_us >= DEBOUNCE_TIME_US)
            {
                // Отпускание подтверждено
                button_state = BUTTON_STATE_RELEASED;
            }
            break;
    }
}

bool button_task_is_pressed(void)
{
    return (button_state == BUTTON_STATE_PRESSED || button_state == BUTTON_STATE_RELEASE_MAYBE);
}

uint32_t button_task_get_press_count(void)
{
    return press_count;
}
