#include "pi-task.h"
#include "pico/stdlib.h"

// Около 100 членов ряда: 100 * 4.7 мкс ≈ 0.47 мс, укладываемся в 0.5 мс
#define PI_TERMS_PER_CALL 100

static pi_state_t pi_state;

static uint32_t pi_k;      // текущий член
static uint32_t pi_n;      // всего членов
static double pi_sum;
static double pi_sign;

static double pi_result;
static uint32_t pi_elapsed_ms;

static uint32_t pi_start_time_us;

void pi_task_init(void)
{
    pi_state = PI_STATE_NOT_STARTED;
    pi_k = 0;
    pi_n = 0;
    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_result = 0.0;
    pi_elapsed_ms = 0;
}

bool pi_task_start(uint32_t terms)
{
    if (pi_state == PI_STATE_RUNNING)
    {
        return false; // уже считает
    }

    if (terms == 0)
    {
        return false; // нечего считать
    }

    pi_n = terms;
    pi_k = 0;
    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_state = PI_STATE_RUNNING;
    pi_start_time_us = time_us_32();

    return true;
}

void pi_task_handle(void)
{
    if (pi_state != PI_STATE_RUNNING)
    {
        return;
    }

    // Определяем конец порции
    uint32_t end;
    // Защита от переполнения 32-битного сложения (Ф5)
    if (pi_n - pi_k < PI_TERMS_PER_CALL)
    {
        end = pi_n;
    }
    else
    {
        end = pi_k + PI_TERMS_PER_CALL;
    }

    // Считаем ряд Лейбница
    for (; pi_k < end; pi_k++)
    {
        pi_sum += pi_sign / (2.0 * (double)pi_k + 1.0);
        pi_sign = -pi_sign;
    }

    // Проверяем, закончили ли
    if (pi_k == pi_n)
    {
        pi_result = 4.0 * pi_sum;
        pi_elapsed_ms = (time_us_32() - pi_start_time_us) / 1000;
        pi_state = PI_STATE_READY;
    }
}

pi_state_t pi_task_get_state(void)
{
    return pi_state;
}

bool pi_task_get_result(double *pi_value, uint32_t *elapsed_ms)
{
    if (pi_state != PI_STATE_READY)
    {
        return false;
    }

    if (pi_value != NULL)
    {
        *pi_value = pi_result;
    }
    if (elapsed_ms != NULL)
    {
        *elapsed_ms = pi_elapsed_ms;
    }

    return true;
}
