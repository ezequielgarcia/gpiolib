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
	gpio_info *gg[32];
	int i, ret;
	int bank;
	int ngg;

	if (argc < 3) {
		fprintf(stderr, "usage: %s <bank> <pin> [<pin> ...]\n", argv[0]);
		exit(1);
	}

	signal(SIGINT, sigint);
	ret = gpio_init();
	if (ret) {
		fprintf(stderr, "gpio_init failed with %i\n", gpio_errno);
		exit(1);
	}

	bank = atoi(argv[1]);
	ngg = 0;
	for (i=2; i<argc; i++) {
		int t = atoi(argv[i]);
		gg[ngg] = gpio_attach(bank, bit(t), GPIO_IN);
		if (!gg[ngg]) {
			fprintf(stderr, "gpio_attach(%i) failed\n", t);
			exit(1);
		}
		ngg++;
	}

	for (;;) {
		for (i=0; i<ngg; i++)
			printf("%d%c", gpio_read(gg[i]), i == ngg-1 ? '\n' : ' ');

		usleep(50000);
	}

	for (i=0; i<ngg; i++) {
		ret = gpio_detach(gg[i]);
		if (ret) {
			fprintf(stderr, "gpio_detach failed with %i\n", gpio_errno);
			exit(1);
		}
	}

	ret = gpio_finish();
	if (ret) {
		fprintf(stderr, "gpio_finish failed with %i\n", gpio_errno);
		exit(1);
	}

	printf("Closed library\n");

	return 0;
}
