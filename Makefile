CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -MMD -MP
LDLIBS =
zipname = proj2

all: proj2

################################################################
proj2: main.o error.o sub_processes.o shared_memory.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

################################################################
run: proj2
	./proj2 2 10 4 10 10 10

test: run
	./kontrola-vystupu.sh < proj2.out


clean:
	rm -f *.o *.d $(zipname).zip

zip:
	zip $(zipname).zip *.c *.h Makefile

-include *.d

.PHONY: run test clean zip
