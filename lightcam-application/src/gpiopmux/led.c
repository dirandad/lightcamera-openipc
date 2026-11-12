#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "gpiopmux.h"
#include "led.h"

static pthread_t thread_id;
static int thread_running = 0;

// current state, to do nothing when setled() is called again with the same state
static enum LED_COLOR current_color = LED_NONE;
static enum LED_MODE current_mode = LED_OFF;
static int current_valid = 0;

struct freqled_data{
	enum LED_COLOR ledcolor;
	enum LED_MODE ledmode;
};

// Blinking thread. Cancelled by setled() (usleep is a cancellation point).
void *freqled(void *args){

	char *gpiored   = "1_6";
	char *gpiogreen = "1_4";

	// copy the arguments, then free them: before 2026-11 the thread freed
	// args and kept reading them (use after free)
	struct freqled_data data = *(struct freqled_data *) args;
	free(args);

	int freq=100000;

	switch(data.ledmode)
	{
		case LED_02HZ:
			freq = 100000;
			break;
		case LED_05HZ:
			freq = 250000;
			break;
		case LED_1HZ:
			freq = 500000;
			break;
		case LED_2HZ:
			freq = 1000000;
			break;
		default:
			break;
	}

	bool val = true;
	char *gpio = gpiogreen;

	while(true)
	{

		switch(data.ledcolor)
		{
			case LED_ALTERNATE:
				val = true;
				gpio = (gpio == gpiored ? gpiogreen : gpiored);
				break;
			case LED_RED:
				gpio = gpiored;
				break;
			case LED_GREEN:
				gpio = gpiogreen;
				break;
			default:
				break;
		}

		gpio_set_value(gpio, &val);

		usleep(freq);


		switch(data.ledcolor)
		{
			case LED_ALTERNATE:
				val = false;
				gpio_set_value(gpio, &val);
				break;
			case LED_RED:
			case LED_GREEN:
				val=!val;
				break;
			default:
				break;
		}

	}
	return NULL;
}

void setled(enum LED_COLOR ledcolor, enum LED_MODE ledmode)
{
	char *gpionone  = NULL;
	char *gpiored   = "1_6";
	char *gpiogreen = "1_4";

	// The main loop calls setled() about once per second. Before 2026-11, each
	// call restarted the blinking thread and never joined the old one: one
	// thread stack was lost per call ("memory leak" in lightcam.c).
	if (current_valid && ledcolor == current_color && ledmode == current_mode)
		return;
	current_color = ledcolor;
	current_mode = ledmode;
	current_valid = 1;

	// stop the blinking thread if any, and wait for it
	if(thread_running)
	{
		pthread_cancel(thread_id);
		pthread_join(thread_id, NULL);
		thread_running = 0;
	}

	bool val = false;

	// switch off led
	gpio_set_value(gpiored, &val);
	gpio_set_value(gpiogreen, &val);

	char *gpio = gpionone;

	switch(ledcolor)
	{
		case LED_NONE:
		case LED_ALTERNATE:
			gpio = gpionone;
			break;
		case LED_RED:
			gpio = gpiored;
			break;
		case LED_GREEN:
			gpio = gpiogreen;
			break;
	}

	switch(ledmode)
	{
		case LED_OFF:
			break;
		case LED_ON:
			val = true;
			if(gpio)
				gpio_set_value(gpio, &val);
			break;
		case LED_02HZ:
		case LED_05HZ:
		case LED_1HZ:
		case LED_2HZ: ;
			struct freqled_data *args = (struct freqled_data *)malloc(sizeof(struct freqled_data));
			if (!args)
				break;
			args->ledcolor = ledcolor;
			args->ledmode = ledmode;
			if (pthread_create(&thread_id, NULL, freqled, (void *) args) == 0)
				thread_running = 1;
			else
				free(args);
			break;
	}
}
