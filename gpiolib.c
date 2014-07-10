#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "gpiolib.h"
#include <stdio.h>

#define ARRSIZE(x) (sizeof x / sizeof x[0])

#define OFFSET_OUTPUT_ENABLE	0x134
#define OFFSET_CLEAR		0x190
#define OFFSET_SET		0x194
#define MMAP_SIZE		0x1000 /* 4kb */

struct gpiodata {
	int fd;
	unsigned bank, pin;
	volatile uint32_t *base;
	volatile uint32_t *oe;
	volatile uint32_t *set_addr;
	volatile uint32_t *clear_addr;
};

static struct gpiodata data = { .fd = -1 };

static inline uint32_t bit(int i) {
	return ((uint32_t)1) << i;
}

static off_t gpio_bases[] = {
	[0] = 0x44E07000,
	[1] = 0x4804C000,
	[2] = 0x481AC000,
	[3] = 0x481AE000,
};

int gpio_init(unsigned bank, unsigned pin) {
	off_t base_physical;
	volatile void *base;
	volatile uint32_t *oe;
	int fd, ret;

	if (data.fd != -1)
		return EBUSY;

	if (bank >= ARRSIZE(gpio_bases) || pin > 31) {
		ret = EINVAL;
		goto err;
	}

	base_physical = gpio_bases[bank];

	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		ret = fd;
		goto err;
	}

	base = mmap(0, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
		    base_physical);

	if (base == MAP_FAILED) {
		ret = errno;
		goto err_close_fd;
	}

	/* Enable the pin as output (set a 0 on the corresponding bit) */
	oe = base + OFFSET_OUTPUT_ENABLE;
	*oe &= ~bit(pin);

	data.fd		= fd;
	data.bank	= bank;
	data.pin	= pin;
	data.base	= base;
	data.oe		= oe;
	data.set_addr	= base + OFFSET_SET;
	data.clear_addr	= base + OFFSET_CLEAR;
	return 0;

err_close_fd:
	close(fd);

err:
	data.fd = -1;
	return ret;
}

int gpio_finish() {
	if (data.fd == -1)
		return EBADFD;

	/* Disable pin (set as input)? */
	*data.oe |= bit(data.pin);

	munmap((void*)data.base, MMAP_SIZE);
	close(data.fd);
	data.fd = -1;
	return 0;
}

int gpio_low() {
	if (data.fd == -1)
		return EBADFD;

	*data.clear_addr = bit(data.pin);
	return 0;
}

int gpio_high() {
	if (data.fd == -1)
		return EBADFD;

	*data.set_addr = bit(data.pin);
	return 0;
}
