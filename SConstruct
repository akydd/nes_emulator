env = Environment(CCFLAGS='-Wall -Wextra')

# get build mode from command line
validModes = {\
	'release':[],\
	'blargg':['BLARGG'],\
	'debug':['DEBUG'],\
	'debug_cpu':['DEBUG_CPU'],\
	'debug_mem':['DEBUG_MEM'],\
	'debug_ppu':['DEBUG_PPU'],\
	'debug_controller':['DEBUG_CONTROLLER'],\
	'debug_all':['DEBUG', 'DEBUG_CPU', 'DEBUG_PPU', 'DEBUG_MEM', 'BLARGG', 'DEBUG_CONTROLLER']\
}


AddOption('--mode', dest='mode', action='store', type='string', default='release')
modes = env.GetOption('mode')

for mode in modes.split(","):
	mode = mode.strip()
	if not (mode in validModes.keys()):
		print 'Error: expected one of ' + ', '.join(validModes.keys())
		Exit(1)
	else:
		env.Append(CPPDEFINES = validModes[mode])
		print '**** Compiling in ' + mode + ' mode...'

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
