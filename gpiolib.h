/*
 * Userspace GPIO driver for the TI AM335x chip family
 *
 * GNU GPL?
 */

#ifndef __GPIO_LIB_H
#define __GPIO_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>	/* uint32_t */

typedef enum {
	GPIO_IN,
	GPIO_OUT
} gpio_dir;

struct gpio_info;
typedef struct gpio_info gpio_info;

/* errno-like variable, used to signal errors from the library */
extern int gpio_errno;

int gpio_init();

/* Attach a gpio to be used */
gpio_info *gpio_attach(unsigned bank, uint32_t pinmask, gpio_dir direction);

/* Detach. Takes a gpio_info pointer previously returned by gpio_attach */
int gpio_detach(gpio_info *info);

/* Teardown the mappings */
int gpio_finish();

/* Set gpio value to low/high, direction must be GPIO_OUT */
int gpio_low(gpio_info *info);
int gpio_high(gpio_info *info);

/* Set some of the pins */
int gpio_low_mask(gpio_info *info, uint32_t mask);
int gpio_high_mask(gpio_info *info, uint32_t mask);

/* Set only one pin */
int gpio_low_pin(gpio_info *info, int pin);
int gpio_high_pin(gpio_info *info, int pin);

/* Read gpio value, direction must be GPIO_IN */
int gpio_read(gpio_info *info);

uint32_t bit(int i);

#ifdef __cplusplus
}
#endif

#endif

