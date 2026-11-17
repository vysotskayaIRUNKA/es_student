#include "stdio-text-protocol.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

// Всё состояние протокола закрыто в этом файле (static).
static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint32_t line_len;
static bool line_overflow;

static command_t command;

void stdio_text_protocol_init(void)
{
    line_len = 0;
    line_overflow = false;
    line[0] = '\0';
}

// Делит строку на слова на месте: пробелы заменяются '\0',
// в name и argv пишутся указатели на начала слов.
static void split_line(void)
{
    command.name = NULL;
    command.argc = 0;
    command.truncated = line_overflow;

    uint32_t i = 0;

    // пропускаем ведущие пробелы
    while (i < line_len && line[i] == ' ')
    {
        i++;
    }

    if (i >= line_len)
    {
        // пустая строка
        command.name = NULL;
        return;
    }

    command.name = &line[i];

    while (i < line_len)
    {
        // слово
        while (i < line_len && line[i] != ' ')
        {
            i++;
        }

        // конец слова: если это не конец строки — ставим '\0'
        if (i < line_len)
        {
            line[i] = '\0';
            i++;
        }

        // пропускаем пробелы между словами
        while (i < line_len && line[i] == ' ')
        {
            i++;
        }

        if (i >= line_len)
        {
            break;
        }

        // следующее слово — аргумент
        if (command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS)
        {
            command.argv[command.argc] = &line[i];
            command.argc++;
        }
        else
        {
            // аргументов больше, чем помещается
            command.truncated = true;
            break;
        }
    }
}

const command_t *stdio_text_protocol_handle(void)
{
    // читаем символ без ожидания; нет символа — возвращаем NULL
    int c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT)
    {
        return NULL;
    }

    // конец строки
    if (c == '\r' || c == '\n')
    {
        // пустую строку пропускаем
        if (line_len == 0 && !line_overflow)
        {
            return NULL;
        }

        // переводим строку в терминале
        putchar('\n');

        // завершаем буфер
        line[line_len < STDIO_TEXT_PROTOCOL_LINE_SIZE ? line_len : STDIO_TEXT_PROTOCOL_LINE_SIZE - 1] = '\0';

        // делим строку на слова
        split_line();

        // готовим буфер к следующей строке
        line_len = 0;
        line_overflow = false;

        return command.name != NULL ? &command : NULL;
    }

    // Backspace: 0x08 (Ctrl+H) или 0x7f (DEL)
    if (c == 0x08 || c == 0x7f)
    {
        if (line_len > 0)
        {
            line_len--;
            // стираем символ на экране: шаг назад, пробел, шаг назад
            printf("\b \b");
        }
        return NULL;
    }

    // печатный символ: кладём в буфер и повторяем в терминал
    if (c >= 0x20 && c < 0x7f)
    {
        if (line_len < STDIO_TEXT_PROTOCOL_LINE_SIZE - 1)
        {
            line[line_len] = (char)c;
            line_len++;
            putchar(c);
        }
        else
        {
            // буфер полон — помечаем строку как переполненную
            line_overflow = true;
            putchar(c);
        }
    }

    return NULL;
}
