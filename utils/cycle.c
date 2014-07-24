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
	int i, ret;
	int bank;
	uint32_t pinmask;
	struct timeval t1, t2;
	long usec;
	gpio_info *gg;
	long max;

	if (argc < 4) {
		fprintf(stderr, "usage: %s <num> <bank> <pin> [<pin> ...]\n", argv[0]);
		exit(1);
	}

	max = atoi(argv[1]);
	bank = atoi(argv[2]);
	pinmask = 0;
	for (i=3; i<argc; i++) {
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

	gettimeofday(&t1, NULL);
	for (i=0; i<max; i++) {
		gpio_set(gg);
		gpio_clear(gg);
	}
	gettimeofday(&t2, NULL);

	usec = 1e6 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec;
	printf("%li set/clears in %.3fms\n", max, usec / 1000.0);
	/* Freq in MHz = cycles / usec */
	printf("Frequency: %.3fMHz\n", (double)max / usec);

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
