CC=gcc
CFLAGS=-Wall

OBJ=memory.o cpu.o instructions.o


nes_emulator: $(OBJ) nes_emulator.o
	$(CC) $(CFLAGS) -o nes_emulator $(OBJ) nes_emulator.o



test_mem: memory.o test_mem.o
	$(CC) $(CFLAGS) -o test_mem memory.o test_mem.o

test_instructions: $(OBJ) test_instructions.o
	$(CC) $(CFLAGS) -o test_instructions $(OBJ) test_instructions.o

test_cpu: memory.o cpu.o test_cpu.o
	$(CC) $(CFLAGS) -o test_cpu memory.o cpu.o test_cpu.o



nes_emulator.o: memory.h cpu.h nes_emulator.c


memory.o: memory.h memory.c
cpu.o: memory.h cpu.h cpu.c
instructions.o: memory.h cpu.h instructions.h instructions.c


test_mem.o:memory.h test_mem.c
test_instructions.o:memory.h cpu.h instructions.h test_instructions.c
test_cpu.o:memory.h cpu.h test_cpu.c

clean:
	rm -rf *.o
