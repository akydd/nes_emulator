DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS=-Wall -Wextra -MD -MP -DDEBUG -DDEBUG_PPU -I/usr/include/SDL2
else
	CFLAGS=-Wall -Wextra -MD -MP -I/usr/include/SDL2
endif


CC=gcc

TEST_SRC = $(wildcard test*.c)
UNUSED_SRC=ppu_registers.c
EXCLUDE=$(TEST_SRC) $(UNUSED_SRC)

SRC = $(filter-out $(EXCLUDE), $(wildcard *.c))
LIBFLAGS=-lSDL2

nes_emulator: $(SRC:%.c=%.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBFLAGS)

test_mem: $(TEST_SRC:%.c=%.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBFLAGS)

test_ppu_mem: $(TEST_SRC:%.c=%.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBFLAGS)

clean:
	rm -rf *.o

-include $(SRC:%.c=%.d)
