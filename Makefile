CC=gcc
CFLAGS=-Wall -Wextra

OBJ=memory.o cpu.o ppu_memory.o ppu.o ppu_registers.o loader.o


nes_emulator: $(OBJ) nes_emulator.o
	$(CC) $(CFLAGS) -o nes_emulator $(OBJ) nes_emulator.o


test_mem: test_mem.o
	$(CC) $(CFLAGS) -o test_mem test_mem.o

test_ppu_mem: test_ppu_mem.o
	$(CC) $(CFLAGS) -o test_ppu_mem test_ppu_mem.o

test_cpu: memory.o test_cpu.o
	$(CC) $(CFLAGS) -o test_cpu memory.o test_cpu.o



nes_emulator.o: memory.h cpu.h ppu_memory.h ppu.h loader.h ppu_registers.h nes_emulator.c


memory.o: memory.h memory.c
cpu.o: memory.h ppu_registers.h cpu.h cpu.c
ppu.o: memory.h ppu_memory.h ppu.c
ppu_memory.o: ppu_memory.h ppu_memory.c
ppu_registers.o: ppu_registers.h ppu_registers.c
loader.o: memory.h ppu_memory.h loader.h loader.c


test_mem.o:memory.c test_mem.c
test_ppu_mem.o:ppu_memory.c test_ppu_mem.c
test_cpu.o:memory.h cpu.c test_cpu.c

clean:
	rm -rf *.o
