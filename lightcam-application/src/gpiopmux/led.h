#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

enum LED_COLOR { LED_NONE, LED_RED, LED_GREEN, LED_ALTERNATE };
enum LED_MODE { LED_OFF, LED_ON, LED_02HZ, LED_05HZ, LED_1HZ, LED_2HZ };


void setled(enum LED_COLOR ledcolor, enum LED_MODE ledmode);