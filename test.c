#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include "gpiolib.h"

int main(int argc, char **argv) {
	int i, j, ret;
	int bank;
	uint32_t pinmask;
	struct timeval t1, t2;
	gpio_info *gg;

	if (argc < 3) {
		fprintf(stderr, "usage: %s <bank> <pin> [<pin> ...]\n", argv[0]);
		exit(1);
	}

	bank = atoi(argv[1]);
	pinmask = 0;
	for (i=2; i<argc; i++) {
		int t = atoi(argv[i]);
		assert(t >= 0 && t < 32);
		pinmask |= bit(t);
	}

	printf("Testing with GPIO %i %i\n", bank, pinmask);

	ret = gpio_init();
	if (ret) {
		fprintf(stderr, "gpio_init failed with %i\n", gpio_errno);
		exit(1);
	}

	gg = gpio_attach(bank, pinmask, GPIO_OUT);
	if (!gg) {
		fprintf(stderr, "gpio_attach failed with %i\n", gpio_errno);
		exit(1);
	}

	printf("Initialized lib\n");
	printf("Testing on/off with 0.1 sec pause\n");

	ret = gpio_high_mask(gg, ~0);
	if (ret) {
		fprintf(stderr, "gpio_high failed with %i\n", gpio_errno);
		exit(1);
	}

	for (i=0; i<10; i++) {
		gpio_high_mask(gg, ~0);
		usleep(100000);
		gpio_low_mask(gg, ~0);
		usleep(100000);
	}

	for (i=0; i<10; i++) {
		for (j=0; j<32; j++) {
			if ((pinmask & bit(j)) == 0)
				continue;

			gpio_high_pin(gg, j);
			usleep(100000);
		}

		for (j=0; j<32; j++) {
			if ((pinmask & bit(j)) == 0)
				continue;

			gpio_low_pin(gg, j);
			usleep(100000);
		}
	}

	printf("Bitbanging\n");

	gettimeofday(&t1, NULL);
	for (i=0; i<10000000; i++) {
		gpio_high_mask(gg, ~0);
		gpio_low_mask(gg, ~0);
	}
	gettimeofday(&t2, NULL);

	printf("10M high/lows in %.3fms\n", 1000*(t2.tv_sec - t1.tv_sec) +
			(t2.tv_usec - t1.tv_usec) / 1000.0);

	ret = gpio_detach(gg);
	if (ret) {
		fprintf(stderr, "gpio_detach failed with %i\n", gpio_errno);
		exit(1);
	}

	ret = gpio_finish();
	if (ret) {
		fprintf(stderr, "gpio_finish failed with %i\n", gpio_errno);
		exit(1);
	}

	printf("Closed library\n");

	return 0;
}
