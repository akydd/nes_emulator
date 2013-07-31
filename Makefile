CC=gcc
CFLAGS=-Wall

OBJ=memory.o cpu.o instructions.o

test_mem: memory.o test_mem.o
	$(CC) $(CFLAGS) -o test_mem memory.o test_mem.o

test: $(OBJ) test.o
	$(CC) $(CFLAGS) -o test $(OBJ) test.o

test_cpu: cpu.o test_cpu.o
	$(CC) $(CFLAGS) -o test_cpu cpu.o test_cpu.o

memory.o: memory.h memory.c
cpu.o: memory.h cpu.h cpu.c
instructions.o: memory.h cpu.h instructions.h instructions.c

test_mem.o:memory.h test_mem.c
test.o:memory.h cpu.h instructions.h test.c
test_cpu.o:cpu.h test_cpu.c

clean:
	rm -rf *.o
