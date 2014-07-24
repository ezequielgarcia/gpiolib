#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "gpiolib.h"

void sigint(int s __attribute__((unused))) {
	gpio_finish();
	exit(0);
}

int main(int argc, char **argv) {
	int i, j, ret;
	int bank;
	uint32_t pinmask;
	struct timeval t1, t2;
	double msec;
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

	signal(SIGINT, sigint);
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

	for (i=0; i<10; i++) {
		gpio_set(gg);
		usleep(100000);
		gpio_clear(gg);
		usleep(100000);
	}

	for (i=0; i<10; i++) {
		for (j=0; j<32; j++) {
			if ((pinmask & bit(j)) == 0)
				continue;

			gpio_set_pin(gg, j);
			usleep(100000);
		}

		for (j=0; j<32; j++) {
			if ((pinmask & bit(j)) == 0)
				continue;

			gpio_clear_pin(gg, j);
			usleep(100000);
		}
	}

	printf("Bitbanging\n");

	gettimeofday(&t1, NULL);
	for (i=0; i<10e6; i++) {
		gpio_set(gg);
		gpio_clear(gg);
	}
	gettimeofday(&t2, NULL);

	msec = 1000 * (t2.tv_sec - t1.tv_sec) +
		(t2.tv_usec - t1.tv_usec) / 1000.0;
	printf("10M set/clears in %.3fms\n", msec);
	printf("Frequency: %.3fMHz\n", 10e3 / msec);

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
