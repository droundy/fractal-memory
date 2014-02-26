import glob, os

# create build environment
env = Environment()

# The following is needed because scons is stupid and removes the
# needed environment variables.  For more explanation, see:

# http://issues.lumiera.org/ticket/525
env["ENV"]["PKG_CONFIG_PATH"] = os.environ.get("PKG_CONFIG_PATH")
# determine compiler and linker flags for SDL
env.ParseConfig('pkg-config --cflags sdl2')
env.ParseConfig('pkg-config --libs sdl2')
env.ParseConfig('pkg-config --cflags SDL2_ttf')
env.ParseConfig('pkg-config --libs SDL2_ttf')
env.ParseConfig('pkg-config --libs freetype2')

# gather a list of source files
SOURCES = glob.glob('*.c')

# add additional compiler flags
env.Append(CCFLAGS = ['-g', '-Wall', '-Werror', '-std=c99', '-DSDL_ASSERT_LEVEL=2'])
# add additional libraries to link against
#env.Append(LIBS = ['SDL_mixer', 'SDL_image'])

# build target
# output executable will be "game"
env.Program(target = 'game', source = SOURCES)
