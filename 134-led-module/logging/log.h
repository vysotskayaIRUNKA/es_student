#ifndef LOG_H
#define LOG_H

#include <stdio.h>

// Имя устройства и версия прошивки
#define DEVICE_NAME      "pico-led"
#define FIRMWARE_VERSION "1.0.0"

// Уровни логирования
#define LOG_LEVEL_ERR  1
#define LOG_LEVEL_INF  2
#define LOG_LEVEL_DBG  3

// Порог по умолчанию — можно переопределить ДО включения log.h
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INF
#endif

// Функции — реализация в log.c
void log_version(void);
void log_prefix(const char *level, const char *function, int line);

// Макросы логирования
#define LOG_ERR(...)                                \
do                                              \
{                                               \
    if (LOG_LEVEL >= LOG_LEVEL_ERR)             \
    {                                           \
        log_prefix("err", __func__, __LINE__);  \
        printf(__VA_ARGS__);                    \
    }                                           \
} while (0)

#define LOG_INF(...)                                \
do                                              \
{                                               \
    if (LOG_LEVEL >= LOG_LEVEL_INF)             \
    {                                           \
        log_prefix("inf", __func__, __LINE__);  \
        printf(__VA_ARGS__);                    \
    }                                           \
} while (0)

#define LOG_DBG(...)                                \
do                                              \
{                                               \
    if (LOG_LEVEL >= LOG_LEVEL_DBG)             \
    {                                           \
        log_prefix("dbg", __func__, __LINE__);  \
        printf(__VA_ARGS__);                    \
    }                                           \
} while (0)

#endif // LOG_H
