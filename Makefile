.PHONY:clean all
CFLAGS=-Wall -Wextra $(CFLAGS_EXTRA)
SHELL=/bin/bash
NAME=gpiolib

all: $(NAME).so test

test: test.c $(NAME).so
	$(CC) $< $(NAME).so -o $@

%.so: %.o
	$(CC) $< -shared -o $@

%.o: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(NAME).so $(NAME).o test
