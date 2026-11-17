#include <stdio.h>
#include "pico/stdlib.h"

#include "stdio-text-protocol.h"
#include "api.h"
#include "led-task.h"

static uint64_t main_time_sum_us;
static uint64_t main_time_max_us;
static uint32_t main_time_count;

void main_time_reset(void)
{
	main_time_sum_us = 0;
	main_time_max_us = 0;
	main_time_count = 0;
}

void main_time_exec_print(void)
{
	uint64_t avg = main_time_count ? main_time_sum_us / main_time_count : 0;
	printf("main_time_exec: avg %llu us, max %llu us\n",
		   (unsigned long long)avg,
		   (unsigned long long)main_time_max_us);
}

int main(void)
{
	stdio_init_all();
	stdio_text_protocol_init();

	// Только запуск задачи
	led_task_init();

	main_time_reset();

	while (true)
	{
		uint64_t start = time_us_64();

		const command_t *command = stdio_text_protocol_handle();
		if (command != NULL)
		{
			api_handle(command);
		}

		// Только вызов обработчика
		led_task_handle();

		uint64_t dt = time_us_64() - start;
		main_time_sum_us += dt;
		if (dt > main_time_max_us)
		{
			main_time_max_us = dt;
		}
		main_time_count++;
	}
}
