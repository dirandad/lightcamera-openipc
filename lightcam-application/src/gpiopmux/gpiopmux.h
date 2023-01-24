
#ifndef REGINFO_LIGHT_H
#define REGINFO_LIGHT_H

#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>



int gpio_get_value(char *gpio, bool *val);

int gpio_set_value(char *gpio, bool *val);

int gpio_set_direction(char *gpio, bool *direction);

bool pmux_get_cmd(uint32_t addr, uint32_t *data);

bool pmux_set_cmd(uint32_t addr, uint32_t *data);

int gpio_config_interrupt(char *gpio);

#endif /* REGINFO_LIGHT_H */

