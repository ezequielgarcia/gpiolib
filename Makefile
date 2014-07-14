.PHONY:clean all
CFLAGS=-Wall -Wextra -O99
SHELL=/bin/bash
NAME=gpiolib

all: $(NAME).so test bang

bang: bang.c $(NAME).so
	$(CC) $(CFLAGS) $< $(NAME).so -o $@

test: test.c $(NAME).so
	$(CC) $(CFLAGS) $< $(NAME).so -o $@

%.so: %.o
	$(CC) $< -shared -o $@

%.o: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(NAME).so $(NAME).o test bang
