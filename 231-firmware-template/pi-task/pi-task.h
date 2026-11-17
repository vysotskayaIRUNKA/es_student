#pragma once

#include <stdint.h>
#include <stdbool.h>

// Состояния задачи расчёта
typedef enum {
    PI_STATE_NOT_STARTED, // не запускался
    PI_STATE_RUNNING,     // считает
    PI_STATE_READY,       // готово
} pi_state_t;

// Инициализация задачи (сброс состояния)
void pi_task_init(void);

// Запуск расчёта на n членов ряда.
// Возвращает false, если расчёт уже идёт.
bool pi_task_start(uint32_t terms);

// Выполняет порцию расчёта (вызывается в суперцикле)
void pi_task_handle(void);

// Возвращает текущее состояние задачи
pi_state_t pi_task_get_state(void);

// Забирает результат: возвращает true, если готово,
// и записывает значение и время расчёта по указателям.
bool pi_task_get_result(double *pi_value, uint32_t *elapsed_ms);
