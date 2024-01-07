CFLAGS=-std=c99
CFLAGS+=-Wall -Wextra -Wshadow -Wno-unused-function
DEFS=
CC=gcc
INCS=-I. -Ithirdparty -Ishaders
OUTDIR=build
OUTEXT=
SHDC=sokol-shdc
SHDCFLAGS=--format sokol_impl --slang glsl330:glsl300es:hlsl4:metal_macos:metal_ios:wgsl
SHADERS=\
	shaders/sample-effect.glsl.h \
	shaders/sample-sdf.glsl.h \
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
	ifeq ($(backend), gles3)
		LIBS+=-lEGL
	endif
else ifeq ($(platform), macos)
        LIBS+=-framework Cocoa -framework QuartzCore -framework Metal -framework MetalKit
        CFLAGS+=-ObjC -x objective-c
else ifeq ($(platform), web)
	LIBS+=-sFULL_ES3
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
		CFLAGS+=-Og -g
	else
		CFLAGS+=-O3 -g0 -s
	endif
else ifeq ($(build), debug)
	CFLAGS+=-Og -ffast-math -g
else ifeq ($(build), release)
	CFLAGS+=-Ofast -fno-plt -g
	DEFS+=-DNDEBUG
endif

ifeq ($(build), debug)
	DEFS+=-DSOKOL_DEBUG
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
else ifeq ($(backend), gles3)
	DEFS+=-DSOKOL_GLES3
else ifeq ($(backend), d3d11)
	DEFS+=-DSOKOL_D3D11
else ifeq ($(backend), metal)
	DEFS+=-DSOKOL_METAL
else ifeq ($(backend), dummy)
	DEFS+=-DSOKOL_DUMMY_BACKEND
endif

SAMPLES=\
	build/sample-rectangle$(OUTEXT) \
	build/sample-primitives$(OUTEXT) \
	build/sample-blend$(OUTEXT) \
	build/sample-framebuffer$(OUTEXT) \
	build/sample-bench$(OUTEXT) \
	build/sample-sdf$(OUTEXT) \
	build/sample-effect$(OUTEXT)

all: $(SAMPLES)

shaders: $(SHADERS)

clean:
	rm -rf $(OUTDIR)

clean-shaders:
	rm -f $(SHADERS)

$(OUTDIR)/%$(OUTEXT): samples/%.c shaders/*.h thirdparty/*.h sokol_gp.h
	@mkdir -p $(OUTDIR)
	$(CC) -o $@ $< $(INCS) $(DEFS) $(CFLAGS) $(LIBS)

shaders/%.glsl.h: shaders/%.glsl
	$(SHDC) $(SHDCFLAGS) -i $^ -o $@

lint:
	clang-tidy sokol_gp.h -- $(INCS) $(DEFS) $(CFLAGS) -DSOKOL_IMPL -include sokol_gfx.h

update-thirdparty:
	wget -O thirdparty/sokol_app.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_app.h
	wget -O thirdparty/sokol_gfx.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_gfx.h
	wget -O thirdparty/sokol_glue.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_glue.h
	wget -O thirdparty/sokol_time.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_time.h
	wget -O thirdparty/sokol_log.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_log.h
	wget -O thirdparty/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

.PHONY: all shaders clean clean-shaders lint update-thirdparty

test: all
	./build/sample-rectangle$(OUTEXT)
	./build/sample-blend$(OUTEXT)
	./build/sample-primitives$(OUTEXT)
	./build/sample-effect$(OUTEXT)
	./build/sample-framebuffer$(OUTEXT)
	./build/sample-sdf$(OUTEXT)
	./build/sample-bench$(OUTEXT)
