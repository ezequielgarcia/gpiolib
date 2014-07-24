/*
 * Userspace GPIO driver for the TI AM335x chip family
 *
 * Copyright (C) 2014 Vanguardia Sur - http://www.vanguardiasur.com.ar/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
static inline uint32_t bit(int i)
{
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

/* Attach a gpio to be used */
gpio_info *gpio_attach(unsigned bank, uint32_t pinmask, gpio_dir direction);

/* Detach. Takes a gpio_info pointer previously returned by gpio_attach */
int gpio_detach(gpio_info *info);

/* Prepare the library */
int gpio_init(void);

/* Teardown the mappings */
int gpio_finish(void);

/* Set/clear gpio values, direction must be GPIO_OUT */
static inline int gpio_clear(gpio_info *info)
{
	*info->clear = info->mask;
	return 0;
}

static inline int gpio_set(gpio_info *info)
{
	*info->set = info->mask;
	return 0;
}

/* Set/clear some of the pins */
static inline int gpio_clear_mask(gpio_info *info, uint32_t mask)
{
	*info->clear = (info->mask & mask);
	return 0;
}

static inline int gpio_set_mask(gpio_info *info, uint32_t mask)
{
	*info->set = (info->mask & mask);
	return 0;
}

/* Set/clear only one pin */
static inline int gpio_clear_pin(gpio_info *info, int pin)
{
	return gpio_clear_mask(info, bit(pin));
}

static inline int gpio_set_pin(gpio_info *info, int pin)
{
	return gpio_set_mask(info, bit(pin));
}

/* Read gpio value, direction must be GPIO_IN */
static inline int gpio_read(gpio_info *info)
{
	uint32_t datain = *info->datain;
	return datain & info->mask ? 1 : 0;
}

#ifdef __cplusplus
}
#endif

#endif

