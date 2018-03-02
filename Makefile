CFLAGS=gcc -g -std=gnu99 -Wall
CCFLAGS=-c

main: main.c
	${CFLAGS} ${OBJECTS} main.c -o main

clean:
	rm *.o main