CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic -g -O3 -D_GNU_SOURCE -I/usr/include/GL `sdl2-config --cflags`
LDFLAGS=`sdl2-config --libs` -lGL -lGLEW -lm
OBJDIR=obj/
BUILDOBJS=main.o binary.o dirtyregion.o graphics.o input.o lifegraphics.o loadfile.o stack.o threadlife.o vector3.o

all: gameoflife

gameoflife: main.o binary.o dirtyregion.o graphics.o input.o lifegraphics.o loadfile.o stack.o threadlife.o vector3.o
	$(CC) $(BUILDOBJS) -o ../gameoflife $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

binary.o: binary.c
	$(CC) $(CFLAGS) -c binary.c	

dirtyregion.o: dirtyregion.c
	$(CC) $(CFLAGS) -c dirtyregion.c

graphics.o: graphics.c
	$(CC) $(CFLAGS) -c graphics.c

input.o: input.c
	$(CC) $(CFLAGS) -c input.c

lifegraphics.o: lifegraphics.c
	$(CC) $(CFLAGS) -c lifegraphics.c

loadfile.o: loadfile.c
	$(CC) $(CFLAGS) -c loadfile.c

stack.o: stack.c
	$(CC) $(CFLAGS) -c stack.c

threadlife.o: threadlife.c
	$(CC) $(CFLAGS) -c threadlife.c

vector3.o: vector3.c
	$(CC) $(CFLAGS) -c vector3.c

clean:
	rm *.o

