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

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "gpiolib.h"

#define ARRSIZE(x) (sizeof x / sizeof x[0])

/* All of these are AM335x-specific */
#define NBANKS			4
#define OFFSET_OUTPUT_ENABLE	0x134
#define OFFSET_DATAIN		0x138
#define OFFSET_CLEAR		0x190
#define OFFSET_SET		0x194
#define MMAP_SIZE		0x1000 /* 4kb */
#define GPIO_TO_PIN(bank, pin)	(32*(bank) + (pin))

/* Taken from TI AM335x Technical Reference Manual */
static const off_t gpio_bases[] = {
	[0] = 0x44E07000,
	[1] = 0x4804C000,
	[2] = 0x481AC000,
	[3] = 0x481AE000,
};

int gpio_errno;

static inline int bitcount(uint32_t x)
{
	int ret = 0;

	while (x) {
		x &= x-1;
		ret++;
	}

	return ret;
}

/* Addresses of each bank's mappings. Initialized to NULLs */
static void *mappings[NBANKS];
static int devmemfd = -1;
static bool ready;
static bool exported_gpios[NBANKS][32];

/*
 * Write the gpio number to a sysfs file. Most likely it should always
 * be /sys/class/gpio/export or /sys/class/gpio/unexport
 */
static int sys_gpio_helper(int bank, int pin, char *file)
{
	char buf[10];
	int fd, t;

	sprintf(buf, "%i", GPIO_TO_PIN(bank, pin));
	fd = open(file, O_WRONLY);
	if (fd < 0) {
		gpio_errno = fd;
		return 1;
	}

	t = strlen(buf) + 1;
	if (t != write(fd, buf, strlen(buf) + 1)) {
		gpio_errno = errno;
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}

static int sys_release_gpio(int bank, int pin)
{
	int ret;

	ret = sys_gpio_helper(bank, pin, "/sys/class/gpio/unexport");
	if (ret)
		return 1;

	exported_gpios[bank][pin] = false;
	return 0;
}

static int sys_request_gpio(int bank, int pin, gpio_dir dir)
{
	int ret;
	int fd, t;
	char buf[100];

	ret = sys_gpio_helper(bank, pin, "/sys/class/gpio/export");
	if (ret)
		return ret;

	exported_gpios[bank][pin] = true;

	sprintf(buf, "/sys/class/gpio/gpio%i/direction", GPIO_TO_PIN(bank, pin));
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		gpio_errno = errno;
		goto err_release;
	}

	if (dir == GPIO_IN) {
		t = write(fd, "in", 3);
		if (t != 3) {
			gpio_errno = errno;
			goto err_release;
		}
	} else {
		t = write(fd, "out", 4);
		if (t != 4) {
			gpio_errno = errno;
			goto err_release;
		}
	}
	close(fd);

	return 0;

err_release:
	sys_release_gpio(bank, pin);
	return 1;
}

static int sys_request_gpios(int bank, uint32_t pinmask, gpio_dir dir)
{
	int i;

	for (i = 0; i < 32; i++) {
		if ((pinmask & bit(i)) == 0)
			continue;

		if (sys_request_gpio(bank, i, dir))
			goto err;
	}

	return 0;

err:
	for (i--; i >= 0; i--) {
		if ((pinmask & bit(i)) == 0)
			continue;

		sys_release_gpio(bank, i);
	}

	return 1;
}

static int sys_release_gpios(int bank, uint32_t pinmask)
{
	int i;

	for (i = 0; i < 32; i++) {
		if ((pinmask & bit(i)) == 0)
			continue;

		sys_release_gpio(bank, i);
	}

	return 0;
}

static void cleanup()
{
	unsigned bank, pin;

	for (bank = 0; bank < NBANKS; bank++) {
		for (pin = 0; pin < 32; pin++) {
			if (exported_gpios[bank][pin]) {
				sys_release_gpio(bank, pin);
				exported_gpios[bank][pin] = false;
			}
		}
	}

	for (bank = 0; bank < NBANKS; bank++) {
		if (mappings[bank]) {
			munmap(mappings[bank], MMAP_SIZE);
			mappings[bank] = NULL;
		}
	}

	if (devmemfd != -1) {
		close(devmemfd);
		devmemfd = -1;
	}
}

int gpio_init(void)
{
	int i, fd;
	void *base;
	off_t base_offset;

	if (ready) {
		gpio_errno = EBUSY;
		return 1;
	}

	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		gpio_errno = errno;
		return 1;
	}

	atexit(cleanup);

	for (i = 0; i < NBANKS; i++) {
		base_offset = gpio_bases[i];
		base = mmap(0, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			    fd, base_offset);

		if (base == MAP_FAILED) {
			gpio_errno = errno;
			goto err_unmap;
		}

		mappings[i] = base;
	}

	devmemfd = fd;
	ready = true;

	return 0;

err_unmap:
	for (i--; i >= 0; i--) {
		munmap(mappings[i], MMAP_SIZE);
		mappings[i] = NULL;
	}

	close(fd);
	return 1;
}

gpio_info *gpio_attach(unsigned bank, uint32_t pinmask, gpio_dir direction)
{
	gpio_info *ret;
	volatile uint32_t *oe;
	void *base;

	if (!ready) {
		gpio_errno = ENODEV;
		return NULL;
	}

	/* Validate the arguments */
	if (bank >= ARRSIZE(gpio_bases) || pinmask == 0) {
		gpio_errno = EINVAL;
		return NULL;
	}

	if (direction != GPIO_IN && direction != GPIO_OUT) {
		gpio_errno = EINVAL;
		return NULL;
	}

	/* For input, only one pin is supported */
	if (direction == GPIO_IN && bitcount(pinmask) != 1) {
		gpio_errno = EINVAL;
		return NULL;
	}

	/* Let the kernel know we're using these GPIO lines */
	if (sys_request_gpios(bank, pinmask, direction))
		return NULL;

	base = mappings[bank];

	ret = malloc(sizeof *ret);
	if (!ret) {
		gpio_errno = ENOMEM;
		goto err_sysrelease;
	}
	memset(ret, 0, sizeof *ret);

	/* Check if the pin is properly muxed */
	oe = base + OFFSET_OUTPUT_ENABLE;
	if (direction == GPIO_IN) {
		/* All bits (the only one actually), should be set */
		if ((*oe & pinmask) != pinmask) {
			gpio_errno = ENODEV;
			goto err_free;
		}
	} else {
		/* All bits should be low */
		*oe &= ~pinmask;

		if ((*oe & pinmask) != 0) {
			gpio_errno = ENODEV;
			goto err_free;
		}
	}

	ret->bank	= bank;
	ret->direction	= direction;
	ret->mask	= pinmask;
	if (direction == GPIO_OUT) {
		ret->set	= base + OFFSET_SET;
		ret->clear	= base + OFFSET_CLEAR;
	} else {
		ret->datain	= base + OFFSET_DATAIN;
	}

	return ret;

err_free:
	free(ret);

err_sysrelease:
	sys_release_gpios(bank, pinmask);

	return NULL;
}

int gpio_detach(gpio_info *info)
{
	/* Failure? */
	sys_release_gpios(info->bank, info->mask);

	free(info);
	return 0;
}

int gpio_finish(void)
{
	if (!ready) {
		gpio_errno = EINVAL;
		return 1;
	}

	cleanup();

	ready = false;

	return 0;
}
