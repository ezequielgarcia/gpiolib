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

/* Use this to construct the bitmasks */
static inline uint32_t bit(int i) {
	return ((uint32_t)1) << i;
}

/*
 * Don't use this structure. Treat gpio_info as an opaque object subject
 * to change. The only reason this is here is performance.
 */
typedef struct {
	gpio_dir direction;
	unsigned bank;
	uint32_t mask;

	volatile uint32_t *set;
	volatile uint32_t *clear;
	volatile uint32_t *datain;
} gpio_info;

/* errno-like variable, used to signal errors from the library */
extern int gpio_errno;

int gpio_init();

/* Attach a gpio to be used */
gpio_info *gpio_attach(unsigned bank, uint32_t pinmask, gpio_dir direction);

/* Detach. Takes a gpio_info pointer previously returned by gpio_attach */
int gpio_detach(gpio_info *info);

/* Teardown the mappings */
int gpio_finish();

/* Set/clear gpio values, direction must be GPIO_OUT */
static inline int gpio_clear(gpio_info *info) {
	*info->clear = info->mask;
	return 0;
}
static inline int gpio_set(gpio_info *info) {
	*info->set = info->mask;
	return 0;
}

/* Set/clear some of the pins */
static inline int gpio_clear_mask(gpio_info *info, uint32_t mask) {
	*info->clear = (info->mask & mask);
	return 0;
}
static inline int gpio_set_mask(gpio_info *info, uint32_t mask) {
	*info->set = (info->mask & mask);
	return 0;
}

/* Set/clear only one pin */
static inline int gpio_clear_pin(gpio_info *info, int pin) {
	return gpio_clear_mask(info, bit(pin));
}
static inline int gpio_set_pin(gpio_info *info, int pin) {
	return gpio_set_mask(info, bit(pin));
}

/* Read gpio value, direction must be GPIO_IN */
static inline int gpio_read(gpio_info *info) {
	uint32_t datain = *info->datain;
	return datain & info->mask ? 1 : 0;
}

#ifdef __cplusplus
}
#endif

#endif

