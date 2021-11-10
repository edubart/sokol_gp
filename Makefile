CFLAGS?=-std=c99 -Wall -Wextra
DEFINES=
CC=gcc
LIBS=-lSDL2 -lm
INCLUDES=-I.
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
SAMPLE_SHADERS=\
	samples/sample-effect.glsl.h\
	samples/sample-sdf.glsl.h

# platform
ifndef platform
	platform=linux
endif
ifeq ($(platform), windows)
	CC:=x86_64-w64-mingw32-gcc
	DEFINES+=-DSDL_MAIN_HANDLED
	LIBS+=-lSDL2main -lopengl32 -ld3d11 -ldxgi -ldxguid
	OUTEXT=.exe
else
	OUTEXT=
	CC?=gcc
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
	DEFINES+=-DNDEBUG
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
	DEFINES+=-DSOKOL_GLCORE33
else ifeq ($(backend), gles2)
	DEFINES+=-DSOKOL_GLES2
else ifeq ($(backend), gles3)
	DEFINES+=-DSOKOL_GLES3
else ifeq ($(backend), d3d11)
	DEFINES+=-DSOKOL_D3D11
else ifeq ($(backend), metal)
	DEFINES+=-DSOKOL_METAL
else ifeq ($(backend), dummy)
	DEFINES+=-DSOKOL_DUMMY_BACKEND
endif

.PHONY: all clean shaders


all: $(SAMPLES)

clean:
	rm -rf $(OUTDIR)
	rm -f *.log *.dxvk-cache

sokolgp-shaders:
	@mkdir -p $(OUTDIR)
	$(SHDC) $(SHDCFLAGS) -i sokol_gp_shaders.glsl -o $(OUTDIR)/sokol_gp_shaders.glsl.h

$(SAMPLES): %:
	@mkdir -p $(OUTDIR)
	$(CC) -o $(OUTDIR)/$@$(OUTEXT) samples/$@.c $(INCLUDES) $(DEFINES) $(CFLAGS) $(LIBS)

samples/%.glsl.h: samples/%.glsl
	$(SHDC) $(SHDCFLAGS) -i $^ -o $@

sample-shaders: $(SAMPLE_SHADERS)
