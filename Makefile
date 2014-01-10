CC=gcc
CFLAGS=-Wall -Wextra

OBJ=memory.o cpu.o ppu_memory.o ppu.o loader.o


nes_emulator: $(OBJ) nes_emulator.o
	$(CC) $(CFLAGS) -o nes_emulator $(OBJ) nes_emulator.o



test_mem: test_mem.o
	$(CC) $(CFLAGS) -o test_mem test_mem.o

test_instructions: memory.o test_instructions.o
	$(CC) $(CFLAGS) -o test_instructions memory.o test_instructions.o

test_cpu: memory.o test_cpu.o
	$(CC) $(CFLAGS) -o test_cpu memory.o test_cpu.o



nes_emulator.o: memory.h cpu.h ppu_memory.h ppu.h loader.h nes_emulator.c


memory.o: memory.h memory.c
cpu.o: memory.h cpu.h cpu.c
ppu.o: memory.h ppu_memory.h ppu.c
ppu_memory.o: ppu_memory.h ppu_memory.c
loader.o: memory.h ppu_memory.h loader.h loader.c


test_mem.o:memory.c test_mem.c
test_instructions.o:memory.h cpu.c test_instructions.c
test_cpu.o:memory.h cpu.c test_cpu.c

clean:
	rm -rf *.o
