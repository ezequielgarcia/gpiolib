.PHONY:clean all utils
CFLAGS=-Wall -Wextra -O99
SHELL=/bin/bash

MODS=$(patsubst %.c,%,$(wildcard *.c))

all: $(MODS)

%: %.c ../gpiolib.o
	$(CC) -I$(INC) $(CFLAGS) ../gpiolib.o $< -o $@

clean:
	rm -f $(MODS)
