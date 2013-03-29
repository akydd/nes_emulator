CC=gcc
CFLAGS=-Wall

SRC=cpu.c instructions.c test.c
OBJ=cpu.o instructions.o test.o

test: $(OBJ)
	$(CC) $(CFLAGS) -o test $(OBJ)

cpu.o: cpu.h cpu.c
instructions.o: cpu.h instructions.h instructions.c

test.o:cpu.h instructions.h test.c
