#
# Student makefile for CS154 Project 3 Caching
#
CC = gcc
CFLAGS = -g -Wall -Werror -std=c99

all: csim test-trans tracegen

csim: csim.c caching.c caching.h
	$(CC) $(CFLAGS) -o csim csim.c caching.c -lm

test-trans: test-trans.c trans.o caching.c caching.h
	$(CC) $(CFLAGS) -o test-trans test-trans.c caching.c trans.o

tracegen: tracegen.c trans.o caching.c
	$(CC) $(CFLAGS) -O0 -o tracegen tracegen.c trans.o caching.c

trans.o: trans.c
	$(CC) $(CFLAGS) -O0 -c trans.c

#
# Clean the src dirctory
#
clean:
	rm -rf *.o
	rm -f csim
	rm -f test-trans tracegen
	rm -f trace.all trace.f*
	rm -f .csim_results .marker
