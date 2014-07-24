.PHONY:clean all
CFLAGS=-Wall -Wextra -O99
SHELL=/bin/bash
NAME=gpiolib

all: $(NAME).o test bang input cycle

%: %.c $(NAME).o
	$(CC) $(CFLAGS) $< $(NAME).o -o $@

%.o: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(NAME).so $(NAME).o test bang input cycle
