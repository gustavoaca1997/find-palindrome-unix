CFLAGS=gcc -g -std=c99
CCFLAGS=-c
OBJECTS=pal.o

main: main.c pal.o
	${CFLAGS} ${OBJECTS} main.c -o main

pal.o: pal.c pal.h
	${CFLAGS} ${CCFLAGS} pal.c -o pal.o

clean:
	rm *.o main