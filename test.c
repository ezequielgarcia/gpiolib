#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "gpiolib.h"

int main(int argc, char **argv) {
	int i, ret;
	int bank, pin;
	struct timeval t1, t2;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <bank> <pin>\n", argv[0]);
		exit(1);
	}

	bank = atoi(argv[1]);
	pin  = atoi(argv[2]);

	printf("Testing with GPIO %i %i\n", bank, pin);

	ret = gpio_init(bank, pin);
	if (ret) {
		fprintf(stderr, "gpio_init failed with %i\n", ret);
		exit(1);
	}

	printf("Initialized lib\n");
	printf("Testing on/off with 1 sec pause\n");

	ret = gpio_high();
	if (ret) {
		fprintf(stderr, "gpio_high failed with %i\n", ret);
		exit(1);
	}

	for (i=0; i<5; i++) {
		gpio_high();
		sleep(1);
		gpio_low();
		sleep(1);
	}

	printf("Bitbanging\n");

	gettimeofday(&t1, NULL);
	for (i=0; i<10000000; i++) {
		gpio_high();
		gpio_low();
	}
	gettimeofday(&t2, NULL);

	printf("10M high/lows in %.3fms\n", 1000*(t2.tv_sec - t1.tv_sec) +
			(t2.tv_usec - t1.tv_usec) / 1000.0);

	ret = gpio_finish();
	if (ret) {
		fprintf(stderr, "gpio_finish failed with %i\n", ret);
		exit(1);
	}

	printf("Closed library\n");

	return 0;
}
