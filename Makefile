CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -MMD -MP -g
LDLIBS =

all: proj2

################################################################
proj2: main.o error.o sub_processes.o shared_memory.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

################################################################
run: proj2
	./proj2 2 10 4 10 10 10

clean:
	rm -f *.o *.d $(xlogin).zip

zip:
	zip $(xlogin).zip *.c *.h Makefile

-include *.d

.PHONY: run clean zip
