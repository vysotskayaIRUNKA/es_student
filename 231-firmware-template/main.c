#include <stdio.h>
#include "pico/stdlib.h"
#include "stdio-text-protocol.h"
#include "api.h"
#include "led-task.h"

int main(void)
{
	stdio_init_all();
	stdio_text_protocol_init();
	led_task_init();

	while (true)
	{
		const command_t *command = stdio_text_protocol_handle();
		if (command != NULL)
		{
			api_handle(command);
		}

		led_task_handle();
	}
}
