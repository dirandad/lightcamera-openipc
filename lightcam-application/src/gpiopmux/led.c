#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "gpiopmux.h"
#include "led.h"

pthread_t thread_id;


struct freqled_data{
	enum LED_COLOR ledcolor;
	enum LED_MODE ledmode;
};

// A normal C function that is executed as a thread 
// when its name is specified in pthread_create()
void *freqled(void *args){

	char *gpiored   = "1_6";
	char *gpiogreen = "1_4";

	struct freqled_data *data = (struct freqled_data *) args;
	
	int freq=100000;
	
	switch(data->ledmode)
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
	}
	
	bool val = true;
	char *gpio = gpiogreen;
	
	while(true)
	{
		
		switch(data->ledcolor)
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
		}
		
		gpio_set_value(gpio, &val);

		usleep(freq);
		
				
		switch(data->ledcolor)
		{
			case LED_ALTERNATE:
				val = false;
				gpio_set_value(gpio, &val);
				break;
			case LED_RED:		
			case LED_GREEN:	
				val=!val;
				break;
		}
		
		
	}
			
}

// A normal C function that is executed as a thread 
// when its name is specified in pthread_create()
void setled(enum LED_COLOR ledcolor, enum LED_MODE ledmode)
{
	char *gpionone  = NULL;
	char *gpiored   = "1_6";
	char *gpiogreen = "1_4";
	
	bool val = false;
	
	// switch off led
	gpio_set_value(gpiored, &val);
	gpio_set_value(gpiogreen, &val);
		
	char *gpio;
		
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
	
	// stop thread if exists
	if(thread_id)
	{
		pthread_cancel(thread_id);
		
		thread_id = NULL;
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
			args->ledcolor = ledcolor;
			args->ledmode = ledmode;
			//printf("gpio:%s\n", args->gpio);
			//printf("ledmode:%d\n", args->ledmode);
			pthread_create(&thread_id, NULL, freqled, (void *) args);
			break;
	}
}

