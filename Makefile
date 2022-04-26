CFLAGS=-std=c99
CFLAGS+=-Wall -Wextra -Wshadow -Wno-unused-function
DEFS=
CC=gcc
INCS=-I. -Ithirdparty -Ishaders
OUTDIR=build
OUTEXT=
SHDC=sokol-shdc
SHDCFLAGS=--format sokol_impl --slang glsl330:glsl100:glsl300es:hlsl4:metal_macos:wgpu
SAMPLES=\
	sample-rectangle\
	sample-primitives\
	sample-blend\
	sample-framebuffer\
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
	LIBS+=-lkernel32 -luser32 -lshell32 -lgdi32 -ld3d11 -ldxgi
	OUTEXT=.exe
else ifeq ($(platform), linux)
	DEFS+=-D_GNU_SOURCE
	CFLAGS+=-pthread
	LIBS+=-lX11 -lXi -lXcursor -lGL -ldl -lm
else ifeq ($(platform), macos)
        LIBS+=-framework Cocoa -framework QuartzCore -framework Metal -framework MetalKit
        CFLAGS+=-ObjC -x objective-c
else ifeq ($(platform), web)
	LIBS+=-sUSE_WEBGL2=1
	CC=emcc
	OUTEXT=.html
	CFLAGS+=--shell-file=samples/sample-shell.html --embed-file images
endif

# build type
ifndef build
	build=debug
endif
ifeq ($(platform), web)
	ifeq ($(build), debug)
		CFLAGS+=-O1 -fno-inline -g
	else
		CFLAGS+=-Oz -g0 -flto
		CFLAGS+=-Wl,--strip-all,--gc-sections,--lto-O3
	endif
else ifeq ($(build), debug)
	CFLAGS+=-Og -g
else ifeq ($(build), release)
	CFLAGS+=-O3 -g -ffast-math -fno-plt -flto
	DEFS+=-DNDEBUG
endif

# backend
ifndef backend
	ifeq ($(platform), windows)
		backend=d3d11
	else ifeq ($(platform), macos)
		backend=metal
	else ifeq ($(platform), web)
		backend=gles3
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
