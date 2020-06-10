CFLAGS?=-std=c99 -Wall -Wextra
DEFINES=
LIBS=-lSDL2 -lm
INCLUDES=-I.
OUTDIR=build
SHDC?=../scratch/sokol-tools-bin/bin/linux/sokol-shdc

# platform
ifndef platform
	platform=linux
endif
ifeq ($(platform), windows)
	CC?=x86_64-w64-mingw32-gcc
	DEFINES+=-DSDL_MAIN_HANDLED
	LIBS+=-lSDL2main -lopengl32 -ld3d11 -ldxgi -ldxguid
else
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
  backend=glcore33
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

DEPS=sokol_gctx.h sokol_gfx.h sokol_gfx_ext.h sokol_gp.h flextgl.h samples/sample_app.h

.PHONY: all clean shaders

all: sample-prims sample-blend sample-capture sample-fb sample-bench

clean:
	rm -rf $(OUTDIR)

shaders:
	@mkdir -p $(OUTDIR)
	$(SHDC) -i sokol_gp_shaders.glsl -o $(OUTDIR)/sokol_gp_shaders.glsl.h -l glsl330:glsl100:glsl300es:hlsl5:metal_macos

ifeq ($(platform), windows)

$(OUTDIR)/%.exe: $(DEPS) samples/%.c
	@mkdir -p $(OUTDIR)
	$(CC) -o $@ $(subst .exe,.c,$(subst $(OUTDIR),samples,$@)) $(INCLUDES) $(DEFINES) $(CFLAGS) $(LIBS)

%:
	@${MAKE} --no-print-directory $(OUTDIR)/$@.exe

else

$(OUTDIR)/%: $(DEPS) samples/%.c
	@mkdir -p $(OUTDIR)
	$(CC) -o $@ $(subst $(OUTDIR),samples,$@).c $(INCLUDES) $(DEFINES) $(CFLAGS) $(LIBS)

%:
	@${MAKE} --no-print-directory $(OUTDIR)/$@

endif