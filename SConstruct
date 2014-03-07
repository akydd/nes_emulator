env = Environment(CCFLAGS='-Wall -Wextra')

source=['nes_emulator.c', 'ppu.o', 'cpu.o', 'loader.o', 'memory.o', 'controller.o', 'ppu_memory.o']

# targets
targetRelease=env.Program('nes_emulator', source, LIBS='SDL2')
Default(targetRelease)

# tests
env.Program('test_mem', ['test_mem.c', 'controller.o'])
env.Program('test_cpu', ['test_cpu.c', 'memory.o', 'controller.o'])
env.Program('test_controller', ['test_controller.c'])

# object files
env.Object('ppu.c')
env.Object('ppu_memory.c')
env.Object('controller.c')
env.Object('memory.c')
env.Object('cpu.c')
env.Object('loader.c')
