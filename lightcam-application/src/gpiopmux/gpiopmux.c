#include "gpiopmux.h"
#include "mmap.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

  /* ERROR Number Definitions           */

enum REG_OPS { OP_READ, OP_WRITE };

// usage: mem_reg(regs[reg_num]->address, &val, OP_READ)

bool mem_reg(uint32_t addr, uint32_t *data, enum REG_OPS op) {
    static int mem_fd;
    static char *loaded_area;
    static uint32_t loaded_offset;
    static uint32_t loaded_size;

    uint32_t offset = addr & 0xffff0000;
    uint32_t size = 0xffff;
	
    if (!addr || (loaded_area && offset != loaded_offset)) {
        int res = munmap(loaded_area, loaded_size);
        if (res) {
            fprintf(stderr, "read_mem_reg error: %s (%d)\n", strerror(errno),
                    errno);
        }
    }

    if (!addr) {
        close(mem_fd);
        return true;
    }

    if (!mem_fd) {
        mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        if (mem_fd < 0) {
            fprintf(stderr, "can't open /dev/mem\n");
            return false;
        }
    }
	

    volatile char *mapped_area;
    if (offset != loaded_offset) {
        mapped_area =
            mmap(NULL, // Any adddress in our space will do
                 size, // Map length
                 PROT_READ |
                     PROT_WRITE, // Enable reading & writting to mapped memory
                 MAP_SHARED,     // Shared with other processes
                 mem_fd,         // File to map
                 offset          // Offset to base address
            );


        if (mapped_area == MAP_FAILED) {
            fprintf(stderr, "read_mem_reg mmap error: %s (%d)\n",
                    strerror(errno), errno);
            return false;
        }
        loaded_area = (char *)mapped_area;
        loaded_size = size;
        loaded_offset = offset;
		

		
    } else
        mapped_area = loaded_area;

    if (op == OP_READ)
        *data = *(volatile uint32_t *)(mapped_area + (addr - offset));
    else if (op == OP_WRITE)
        *(volatile uint32_t *)(mapped_area + (addr - offset)) = *data;


    return true;
}



bool pmux_get_cmd(uint32_t addr, uint32_t *data) {

	bool retval;
	retval = mem_reg(addr, data, OP_READ);
	return retval;

}


bool pmux_set_cmd(uint32_t addr, uint32_t *data) {
	
	bool retval;
	retval = mem_reg(addr, data, OP_WRITE);
	return retval;

}	


// op = 0 : read
// op = 1 : set direction
// op = 2 : write
int gpio_manipulate(char *gpio, bool *val, int op) {
    int GPIO_Groups = 0;
    size_t GPIO_Base = 0;
    size_t GPIO_Offset = 0;
	
	// specific for HI3518EV300
	GPIO_Base = 0x120B0000;
    GPIO_Offset = 0x1000;
    GPIO_Groups = 9;

	int direction = 0;

	if(op==1)
		direction = (*val ? 1 : 0);
	
	if(op==2)
		direction = 1;

    const char *gpio_num = gpio;
    //gpio_mux_by(gpio_num, -1, NULL);

    int group, num;
    if (sscanf(gpio_num, "%d_%d", &group, &num) != 2) {
        num = strtoul(gpio_num, NULL, 10);
        group = num / 8;
        num %= 8;
    }
    if (group > GPIO_Groups || num > 7)
        return EXIT_FAILURE;
	
    size_t mask = 1 << (2 + num);

    uint32_t address = GPIO_Base + (group * GPIO_Offset) + mask;

    if (op > 0) { // write or set direction

        size_t daddress = GPIO_Base + (group * GPIO_Offset) + 0x400;
        uint32_t direct;
        if (!mem_reg(daddress, &direct, OP_READ)) {
            fprintf(stderr, "read reg %#x error\n", address);
            return EXIT_FAILURE;
        }

		
        if (((direct >> num) & 1U) != direction) {
            // If current state is input, set bit it make it output
			if (direction == 1)
				direct |= 1UL << num;
			else
				direct &= ~(1UL << num);
				
            if (!mem_reg(daddress, &direct, OP_WRITE)) {
                fprintf(stderr, "write reg %#x error\n", address);
                return EXIT_FAILURE;
            }
        }
	}
	if ( op == 2) // write
	{
        uint32_t cmd = *val << num;

        if (!mem_reg(address, &cmd, OP_WRITE)) {
            printf("write reg %#x error\n", address);
            return EXIT_FAILURE;
        }
    }
	if ( op == 0) // read
	{
		uint32_t cmd = 0;
        if (!mem_reg(address, &cmd, OP_READ)) {
            printf("read reg %#x error\n", address);
            return EXIT_FAILURE;
        }
		*val = (cmd >> num) == 1;
    }

    return EXIT_SUCCESS;
}

int gpio_get_value(char *gpio, bool *val) {
		
    return gpio_manipulate(gpio, val, 0);
}

int gpio_set_value(char *gpio, bool *val) {

    return gpio_manipulate(gpio, val, 2);
}

int gpio_set_direction(char *gpio, bool *direction) {

    return gpio_manipulate(gpio, direction, 1);
}

int gpio_config_interrupt(char *gpio) {
	
	char *gpio_num_str = gpio;

    int group, num;
    if (sscanf(gpio_num_str, "%d_%d", &group, &num) != 2) {
        num = strtoul(gpio_num_str, NULL, 10);
        group = num / 8;
        num %= 8;
    }
	
	int nogpio = group * 8 + num;
	
	char nogpio_str[5];
	
	sprintf(nogpio_str, "%d", nogpio);
	
	char gpio_path[30];
	
	sprintf(gpio_path, "/sys/class/gpio/gpio%d/edge", nogpio);
	
	int fd = open(gpio_path, O_WRONLY);
	
	if (fd == -1) {
		fd = open("/sys/class/gpio/export", O_WRONLY);
		if (fd == -1) {
			fprintf(stderr, "Unable to open /sys/class/gpio/export\n");
			printf( "Unable to open /sys/class/gpio/export\n");
			return 1;
		}

		if (write(fd, nogpio_str, strlen(nogpio_str)) != strlen(nogpio_str)) {
			fprintf(stderr, "Error writing to /sys/class/gpio/export\n");
			printf("Error writing to /sys/class/gpio/export\n");
			close(fd);
			return 1;
		}

		close(fd);
		
		fd = open(gpio_path, O_WRONLY);
	}

    if (fd == -1) {
        fprintf(stderr, "Unable to open %s\n", gpio_path);
        printf("Unable to open %s\n", gpio_path);
		return 1;
    }

    if (write(fd, "both", 4) != 4) {
        fprintf(stderr, "Error writing to %s\n", gpio_path);
        printf("Error writing to %s\n", gpio_path);
		close(fd);
		return 1;
    }
    close(fd);
	
	return 0;

}