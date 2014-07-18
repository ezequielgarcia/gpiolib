#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "gpiolib.h"

int main(int argc, char **argv) {
	int bank;
	int i;
	uint32_t pinmask;
	gpio_info *gg;
	int sleep;
	int ret;

	if (argc < 4) {
		fprintf(stderr, "usage: %s <bank> <sleep> <pin> [<pin> ...]\n", argv[0]);
		exit(1);
	}

	bank  = atoi(argv[1]);
	sleep = atoi(argv[2]);

	pinmask = 0;
	for (i=3; i<argc; i++)
		pinmask |= bit(atoi(argv[i]));

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

	for (;;) {
		gpio_high(gg);
		if (sleep) usleep(sleep);
		gpio_low(gg);
		if (sleep) usleep(sleep);
	}

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
