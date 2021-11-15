CFLAGS=-std=c99
CFLAGS+=-Wall -Wextra -Wshadow -Wmissing-prototypes -Wstrict-prototypes
DEFS=
LIBS=-lSDL2 -lm
INCS=-I. -Ithirdparty -Ishaders
OUTDIR=build
SHDC=sokol-shdc
SHDCFLAGS=--format sokol_impl --slang glsl330:glsl100:glsl300es:hlsl4:metal_macos:wgpu
SAMPLES=\
	sample-prims\
	sample-blend\
	sample-capture\
	sample-fb\
	sample-bench\
	sample-sdf\
	sample-effect
SHADERS=\
	shaders/sample-effect.glsl.h\
	shaders/sample-sdf.glsl.h\
	shaders/sokol_gp.glsl.h

# platform
ifndef platform
	platform=linux
endif
ifeq ($(platform), windows)
	CC=x86_64-w64-mingw32-gcc
	DEFS+=-DSDL_MAIN_HANDLED
	LIBS+=-lSDL2main -lopengl32 -ld3d11 -ldxgi -ldxguid
	OUTEXT=.exe
else
	OUTEXT=
	CC=gcc
	LIBS+=-lGL -ldl
endif

# build type
ifndef build
	build=debug
endif
ifeq ($(build), debug)
	CFLAGS+=-Og -g
else ifeq ($(build), release)
	CFLAGS+=-O3 -g -ffast-math -fno-plt -flto
	DEFS+=-DNDEBUG
endif

# backend
ifndef backend
	ifeq ($(platform), windows)
		backend=d3d11
	else
		backend=glcore33
	endif
endif
ifeq ($(backend), glcore33)
	DEFS+=-DSOKOL_GLCORE33
else ifeq ($(backend), gles2)
	DEFS+=-DSOKOL_GLES2
else ifeq ($(backend), gles3)
	DEFS+=-DSOKOL_GLES3
else ifeq ($(backend), d3d11)
	DEFS+=-DSOKOL_D3D11
else ifeq ($(backend), metal)
	DEFS+=-DSOKOL_METAL
else ifeq ($(backend), dummy)
	DEFS+=-DSOKOL_DUMMY_BACKEND
endif

.PHONY: all clean shaders


all: $(SAMPLES)

shaders: $(SHADERS)

clean:
	rm -rf $(OUTDIR)

$(SAMPLES): %:
	@mkdir -p $(OUTDIR)
	$(CC) -o $(OUTDIR)/$@$(OUTEXT) samples/$@.c $(INCS) $(DEFS) $(CFLAGS) $(LIBS)

shaders/%.glsl.h: shaders/%.glsl
	$(SHDC) $(SHDCFLAGS) -i $^ -o $@
