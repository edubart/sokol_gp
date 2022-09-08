/*
Minimal efficient cross platform 2D graphics painter for Sokol GFX.
sokol_gp - v0.1.0 - 31/Dec/2021
Eduardo Bart - edub4rt@gmail.com
https://github.com/edubart/sokol_gp

# Sokol GP

Minimal efficient cross platform 2D graphics painter in pure C
using modern graphics API through the excellent [Sokol GFX](https://github.com/floooh/sokol) library.

Sokol GP, or in short SGP, stands for Sokol Graphics Painter.

![sample-primitives](https://raw.githubusercontent.com/edubart/sokol_gp/master/screenshots/sample-primitives.png)

## Features

* Made and optimized only for **2D rendering only**, no 3D support.
* Minimal, in a pure single C header.
* Use modern unfixed pipeline graphics APIs for more efficiency.
* Cross platform (backed by Sokol GFX).
* D3D11/OpenGL 3.3/Metal/WebGPU graphics backends (through Sokol GFX).
* **Automatic batching** (merge recent draw calls into batches automatically).
* **Batch optimizer** (rearranges the ordering of draw calls to batch more).
* Uses preallocated memory (no allocations at runtime).
* Supports drawing basic 2D primitives (rectangles, triangles, lines and points).
* Supports the classic 2D color blending modes (color blend, add, modulate, multiply).
* Supports 2D space transformations and changing 2D space coordinate systems.
* Supports drawing the basic primitives (rectangles, triangles, lines and points).
* Supports multiple texture bindings.
* Supports custom fragment shaders with 2D primitives.
* Can be mixed with projects that are already using Sokol GFX.

## Why?

Sokol GFX is an excellent library for rendering using unfixed pipelines
of modern graphics cards, but it is too complex to use for simple 2D drawing,
and it's API is too generic and specialized for 3D rendering. To draw 2D stuff, the programmer
usually needs to setup custom shaders when using Sokol GFX, or use its Sokol GL
extra library, but Sokol GL also has an API with 3D design in mind, which
incurs some costs and limitations.

This library was created to draw 2D primitives through Sokol GFX with ease,
and by not considering 3D usage it is optimized for 2D rendering only,
furthermore it features an **automatic batch optimizer**, more details of it will be described below.

## Automatic batch optimizer

When drawing the library creates a draw command queue of all primitives yet to be drawn,
every time a new draw command is added the batch optimizer looks back up to the last
8 recent draw commands (this is adjustable), and try to rearrange and merge drawing commands
if it finds a previous draw command that meets the following criteria:

* The new draw command and previous command uses the *same primitive pipeline*
* The new draw command and previous command uses the *same shader uniforms*
* The new draw command and previous command uses the *same texture bindings*
* The new draw command and previous command does not have another intermediary
draw command *that overlaps* in-between them.

By doing this the batch optimizer is able for example to merge textured draw calls,
even if they were drawn with other intermediary different textures draws between them.
The effect is more efficiency when drawing, because less draw calls will be dispatched
to the GPU,

This library can avoid a lot of work of making an efficient 2D drawing batching system,
by automatically merging draw calls behind the scenes at runtime,
thus the programmer does not need to manage batched draw calls manually,
nor he needs to sort batched texture draw calls,
the library will do this seamlessly behind the scenes.

The batching algorithm is fast, but it has `O(n)` CPU complexity for every new draw command added,
where `n` is the `SGP_BATCH_OPTIMIZER_DEPTH` configuration.
In experiments using `8` as the default is a good default,
but you may want to try out different values depending on your case.
Using values that are too high is not recommended, because the algorithm may take too long
scanning previous draw commands, and that may consume more CPU resources.

The batch optimizer can be disabled by setting `SGP_BATCH_OPTIMIZER_DEPTH` to 0,
you can use that to measure its impact.

In the samples directory of this repository there is a
benchmark example that tests drawing with the bath optimizer enabled/disabled.
On my machine that benchmark was able to increase performance in a 2.2x factor when it is enabled.
In some private game projects the gains of the batch optimizer proved to increase FPS performance
above 1.5x by just replacing the graphics backend with this library, with no internal
changes to the game itself.

## Design choices

The library has some design choices with performance in mind that will be discussed briefly here.

Like Sokol GFX, Sokol GP will never do any allocation in the draw loop,
so when initializing you must configure beforehand the maximum size of the
draw command queue buffer and the vertices buffer.

All the 2D space transformation (functions like `sgp_rotate`) are done by the CPU and not by the GPU,
this is intentionally to avoid adding extra overhead in the GPU, because typically the number
of vertices of 2D applications are not that large, and it is more efficient to perform
all the transformation with the CPU right away rather than pushing extra buffers to the GPU
that ends up using more bandwidth of the CPU<->GPU bus.
In contrast 3D applications usually dispatches vertex transformations to the GPU using a vertex shader,
they do this because the amount of vertices of 3D objects can be very large
and it is usually the best choice, but this is not true for 2D rendering.

Many APIs to transform the 2D space before drawing a primitive are available, such as
translate, rotate and scale. They can be used as similarly as the ones available in 3D graphics APIs,
but they are crafted for 2D only, for example when using 2D we don't need to use a 4x4 or 3x3 matrix
to perform vertex transformation, instead the code is specialized for 2D and can use a 2x3 matrix,
saving extra CPU float computations.

All pipelines always use a texture associated with it, even when drawing non textured primitives,
because this minimizes graphics pipeline changes when mixing textured calls and non textured calls,
improving efficiency.

The library is coded in the style of Sokol GFX headers, reusing many macros from there,
you can change some of its semantics such as custom allocator, custom log function, and some
other details, read `sokol_gfx.h` documentation for more on that.

## Usage

Copy `sokol_gp.h` along with other Sokol headers to the same folder. Setup Sokol GFX
as you usually would, then add call to `sgp_setup(desc)` just after `sg_setup(desc)`, and
call to `sgp_shutdown()` just before `sg_shutdown()`. Note that you should usually check if
SGP is valid after its creation with `sgp_is_valid()` and exit gracefully with an error if not.

In your frame draw function add `sgp_begin(width, height)` before calling any SGP
draw function, then draw your primitives. At the end of the frame (or framebuffer) you
should **ALWAYS call** `sgp_flush()` between a Sokol GFX begin/end render pass,
the `sgp_flush()` will dispatch all draw commands to Sokol GFX. Then call `sgp_end()` immediately
to discard the draw command queue.

An actual example of this setup will be shown below.

## Quick usage example

The following is a quick example on how to this library with Sokol GFX and Sokol APP:

```c
// This is an example on how to set up and use Sokol GP to draw a filled rectangle.

// Includes Sokol GFX, Sokol GP and Sokol APP, doing all implementations.
#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"

#include <stdio.h> // for fprintf()
#include <stdlib.h> // for exit()
#include <math.h> // for sinf() and cosf()

// Called on every frame of the application.
static void frame(void) {
    // Get current window size.
    int width = sapp_width(), height = sapp_height();
    float ratio = width/(float)height;

    // Begin recording draw commands for a frame buffer of size (width, height).
    sgp_begin(width, height);
    // Set frame buffer drawing region to (0,0,width,height).
    sgp_viewport(0, 0, width, height);
    // Set drawing coordinate space to (left=-ratio, right=ratio, top=1, bottom=-1).
    sgp_project(-ratio, ratio, 1.0f, -1.0f);

    // Clear the frame buffer.
    sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
    sgp_clear();

    // Draw an animated rectangle that rotates and changes its colors.
    float time = sapp_frame_count() * sapp_frame_duration();
    float r = sinf(time)*0.5+0.5, g = cosf(time)*0.5+0.5;
    sgp_set_color(r, g, 0.3f, 1.0f);
    sgp_rotate_at(time, 0.0f, 0.0f);
    sgp_draw_filled_rect(-0.5f, -0.5f, 1.0f, 1.0f);

    // Begin a render pass.
    sg_pass_action pass_action = {0};
    sg_begin_default_pass(&pass_action, width, height);
    // Dispatch all draw commands to Sokol GFX.
    sgp_flush();
    // Finish a draw command queue, clearing it.
    sgp_end();
    // End render pass.
    sg_end_pass();
    // Commit Sokol render.
    sg_commit();
}

// Called when the application is initializing.
static void init(void) {
    // Initialize Sokol GFX.
    sg_desc sgdesc = {.context = sapp_sgcontext()};
    sg_setup(&sgdesc);
    if(!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol GFX context!\n");
        exit(-1);
    }

    // Initialize Sokol GP, adjust the size of command buffers for your own use.
    sgp_desc sgpdesc = {0};
    sgp_setup(&sgpdesc);
    if(!sgp_is_valid()) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error_message(sgp_get_last_error()));
        exit(-1);
    }
}

// Called when the application is shutting down.
static void cleanup(void) {
    // Cleanup Sokol GP and Sokol GFX resources.
    sgp_shutdown();
    sg_shutdown();
}

// Implement application main through Sokol APP.
sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .window_title = "Triangle (Sokol GP)",
        .sample_count = 4, // Enable anti aliasing.
    };
}
```

To run this example, first copy the `sokol_gp.h` header alongside with other Sokol headers
to the same folder, then compile with any C compiler using the proper linking flags (read `sokol_gfx.h`).

## Complete Examples

In folder `samples` you can find the following complete examples covering all APIs of the library:

* [sample-primitives.c](https://github.com/edubart/sokol_gp/blob/master/samples/sample-primitives.c): This is an example showing all drawing primitives and transformations APIs.
* [sample-blend.c](https://github.com/edubart/sokol_gp/blob/master/samples/sample-blend.c): This is an example showing all blend modes between 3 rectangles.
* [sample-framebuffer.c](https://github.com/edubart/sokol_gp/blob/master/samples/sample-framebuffer.c): This is an example showing how to use multiple `sgp_begin()` with frame buffers.
* [sample-sdf.c](https://github.com/edubart/sokol_gp/blob/master/samples/sample-sdf.c): This is an example on how to create custom shaders.
* [sample-effect.c](https://github.com/edubart/sokol_gp/blob/master/samples/sample-effect.c): This is an example on how to use custom shaders for 2D drawing.
* [sample-bench.c](https://github.com/edubart/sokol_gp/blob/master/samples/sample-bench.c): This is a heavy example used for benchmarking purposes.

These examples are used as the test suite for the library, you can build them by typing `make`.

## Error handling

It is possible that after many draw calls the command or vertex buffer may overflow,
in that case the library will set an error error state and will continue to operate normally,
but when flushing the drawing command queue with `sgp_flush()` no draw command will be dispatched.
This can happen because the library uses pre allocated buffers, in such
cases the issue can be fixed by increasing the prefixed command queue buffer and the vertices buffer
when calling `sgp_setup()`.

Making invalid number of push/pops of `sgp_push_transform()` and `sgp_pop_transform()`,
or nesting too many `sgp_begin()` and `sgp_end()` may also lead to errors, that
is a usage mistake.

You can enable the `SOKOL_DEBUG` macro in such cases to debug, or handle
the error programmatically by reading `sgp_get_last_error()` after calling `sgp_end()`.
It is also advised to leave `SOKOL_DEBUG` enabled when developing with Sokol, so you can
catch mistakes early.

## Blend modes

The library supports the most usual blend modes used in 2D, which are the following:

- `SGP_BLENDMODE_NONE` - No blending (`dstRGBA = srcRGBA`).
- `SGP_BLENDMODE_BLEND` - Alpha blending (`dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))` and `dstA = srcA + (dstA * (1-srcA))`)
- `SGP_BLENDMODE_ADD` - Color add (`dstRGB = (srcRGB * srcA) + dstRGB` and `dstA = dstA`)
- `SGP_BLENDMODE_MOD` - Color modulate (`dstRGB = srcRGB * dstRGB` and `dstA = dstA`)
- `SGP_BLENDMODE_MUL` - Color multiply (`dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA))` and `dstA = (srcA * dstA) + (dstA * (1-srcA))`)

## Changing 2D coordinate system

You can change the screen area to draw by calling `sgp_viewport(x, y, width, height)`.
You can change the coordinate system of the 2D space by calling `sgp_project(left, right, top, bottom)`,
with it.

## Transforming 2D space

You can translate, rotate or scale the 2D space before a draw call, by using the transformation
functions the library provides, such as `sgp_translate(x, y)`, `sgp_rotate(theta)`, etc.
Check the cheat sheet or the header for more.

To save and restore the transformation state you should call `sgp_push_transform()` and
later `sgp_pop_transform()`.

## Drawing primitives

The library provides drawing functions for all the basic primitives, that is,
for points, lines, triangles and rectangles, such as `sgp_draw_line()` and `sgp_draw_filled_rect()`.
Check the cheat sheet or the header for more.
All of them have batched variations.

## Color modulation

All common pipelines have color modulation, and you can modulate
a color before a draw by setting the current state color with `sgp_set_color(r,g,b,a)`,
later you should reset the color to default (white) with `sgp_reset_color()`.

## Custom shaders

When using a custom shader, you must create a pipeline for it with `sgp_make_pipeline(desc)`,
using shader, blend mode and a draw primitive associated with it. Then you should
call `sgp_set_pipeline()` before the shader draw call. You are responsible for using
the same blend mode and drawing primitive as the created pipeline.

Custom uniforms can be passed to the shader with `sgp_set_uniform(data, size)`,
where you should always pass a pointer to a struct with exactly the same schema and size
as the one defined in the shader.

Although you can create custom shaders for each graphics backend manually,
it is advised should use the Sokol shader compiler [SHDC](https://github.com/floooh/sokol-tools/blob/master/docs/sokol-shdc.md),
because it can generate shaders for multiple backends from a single `.glsl` file,
and this usually works well.

By default the library uniform buffer per draw call has just 4 float uniforms
(`SGP_UNIFORM_CONTENT_SLOTS` configuration), and that may be too low to use with custom shaders.
This is the default because typically newcomers may not want to use custom 2D shaders,
and increasing a larger value means more overhead.
If you are using custom shaders please increase this value to be large enough to hold
the number of uniforms of your largest shader.

## Library configuration

The following macros can be defined before including to change the library behavior:

- `SGP_BATCH_OPTIMIZER_DEPTH` - Number of draw commands that the batch optimizer looks back at. Default is 8.
- `SGP_UNIFORM_CONTENT_SLOTS` - Maximum number of floats that can be stored in each draw call uniform buffer. Default is 4.
- `SGP_TEXTURE_SLOTS` - Maximum number of textures that can be bound per draw call. Default is 4.

## License

MIT, see LICENSE file or the end of `sokol_gp.h` file.
*/

#if defined(SOKOL_IMPL) && !defined(SOKOL_GP_IMPL)
#define SOKOL_GP_IMPL
#endif

#ifndef SOKOL_GP_INCLUDED
#define SOKOL_GP_INCLUDED 1

#ifndef SOKOL_GFX_INCLUDED
#error "Please include sokol_gfx.h before sokol_gp.h"
#endif

/* Number of draw commands that the batch optimizer looks back at.
8 is a fair default value, but could be tuned per application.
1 makes the batch optimizer try to merge only the very last draw call.
0 disables the batch optimizer
*/
#ifndef SGP_BATCH_OPTIMIZER_DEPTH
#define SGP_BATCH_OPTIMIZER_DEPTH 8
#endif

/* Number of draw commands that the batch optimizer looks back. */
#ifndef SGP_UNIFORM_CONTENT_SLOTS
#define SGP_UNIFORM_CONTENT_SLOTS 4
#endif

/* Number of texture slots that can be bound in a pipeline. */
#ifndef SGP_TEXTURE_SLOTS
#define SGP_TEXTURE_SLOTS 4
#endif

#if defined(SOKOL_API_DECL) && !defined(SOKOL_GP_API_DECL)
#define SOKOL_GP_API_DECL SOKOL_API_DECL
#endif
#ifndef SOKOL_GP_API_DECL
#if defined(_WIN32) && defined(SOKOL_DLL) && defined(SOKOL_GP_IMPL)
#define SOKOL_GP_API_DECL __declspec(dllexport)
#elif defined(_WIN32) && defined(SOKOL_DLL)
#define SOKOL_GP_API_DECL __declspec(dllimport)
#else
#define SOKOL_GP_API_DECL extern
#endif
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* List of possible error codes. */
typedef enum sgp_error {
    SGP_NO_ERROR = 0,
    SGP_ERROR_SOKOL_INVALID,
    SGP_ERROR_VERTICES_FULL,
    SGP_ERROR_UNIFORMS_FULL,
    SGP_ERROR_COMMANDS_FULL,
    SGP_ERROR_VERTICES_OVERFLOW,
    SGP_ERROR_TRANSFORM_STACK_OVERFLOW,
    SGP_ERROR_TRANSFORM_STACK_UNDERFLOW,
    SGP_ERROR_STATE_STACK_OVERFLOW,
    SGP_ERROR_STATE_STACK_UNDERFLOW,
    SGP_ERROR_ALLOC_FAILED,
    SGP_ERROR_MAKE_VERTEX_BUFFER_FAILED,
    SGP_ERROR_MAKE_WHITE_IMAGE_FAILED,
    SGP_ERROR_MAKE_COMMON_SHADER_FAILED,
    SGP_ERROR_MAKE_COMMON_PIPELINE_FAILED,
} sgp_error;

/* Blend modes. */
typedef enum sgp_blend_mode {
    SGP_BLENDMODE_NONE = 0, /* No blending.
                               dstRGBA = srcRGBA */
    SGP_BLENDMODE_BLEND,    /* Alpha blending.
                               dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))
                               dstA = srcA + (dstA * (1-srcA)) */
    SGP_BLENDMODE_ADD,      /* Color add.
                               dstRGB = (srcRGB * srcA) + dstRGB
                               dstA = dstA */
    SGP_BLENDMODE_MOD,      /* Color modulate.
                               dstRGB = srcRGB * dstRGB
                               dstA = dstA */
    SGP_BLENDMODE_MUL,      /* Color multiply.
                               dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA))
                               dstA = (srcA * dstA) + (dstA * (1-srcA)) */
    _SGP_BLENDMODE_NUM
} sgp_blend_mode;

typedef struct sgp_isize {
    int w, h;
} sgp_isize;

typedef struct sgp_irect {
    int x, y, w, h;
} sgp_irect;

typedef struct sgp_rect {
    float x, y, w, h;
} sgp_rect;

typedef struct sgp_textured_rect {
    sgp_rect dst;
    sgp_rect src;
} sgp_textured_rect;

typedef struct sgp_vec2 {
    float x, y;
} sgp_vec2;

typedef sgp_vec2 sgp_point;

typedef struct sgp_line {
    sgp_point a, b;
} sgp_line;

typedef struct sgp_triangle {
    sgp_point a, b, c;
} sgp_triangle;

typedef struct sgp_mat2x3 {
    float v[2][3];
} sgp_mat2x3;

typedef struct sgp_color {
    float r, g, b, a;
} sgp_color;

typedef struct sgp_uniform {
    uint32_t size;
    float content[SGP_UNIFORM_CONTENT_SLOTS];
} sgp_uniform;

typedef struct sgp_images_uniform {
    uint32_t count;
    sg_image images[SGP_TEXTURE_SLOTS];
} sgp_images_uniform;

/* SGP draw state. */
typedef struct sgp_state {
    sgp_isize frame_size;
    sgp_irect viewport;
    sgp_irect scissor;
    sgp_mat2x3 proj;
    sgp_mat2x3 transform;
    sgp_mat2x3 mvp;
    sgp_color color;
    sgp_images_uniform images;
    sgp_uniform uniform;
    sgp_blend_mode blend_mode;
    sg_pipeline pipeline;
    uint32_t _base_vertex;
    uint32_t _base_uniform;
    uint32_t _base_command;
} sgp_state;

/* Structure that defines SGP setup parameters. */
typedef struct sgp_desc {
    uint32_t max_vertices;
    uint32_t max_commands;
} sgp_desc;

/* Structure that defines SGP custom pipeline creation parameters. */
typedef struct sgp_pipeline_desc {
    sg_shader_desc shader;              /* Sokol shader description. */
    sg_primitive_type primitive_type;   /* Draw primitive type (triangles, lines, points, etc). Default is triangles. */
    sgp_blend_mode blend_mode;          /* Color blend mode. Default is no blend. */
} sgp_pipeline_desc;

/* Initialization and de-initialization. */
SOKOL_GP_API_DECL void sgp_setup(const sgp_desc* desc);                 /* Initializes the SGP context, and should be called after `sg_setup`. */
SOKOL_GP_API_DECL void sgp_shutdown(void);                              /* Destroys the SGP context. */
SOKOL_GP_API_DECL bool sgp_is_valid(void);                              /* Checks if SGP context is valid, should be checked after `sgp_setup`. */

/* Error handling. */
SOKOL_GP_API_DECL sgp_error sgp_get_last_error(void);                   /* Returns last SGP error. */
SOKOL_GP_API_DECL const char* sgp_get_error_message(sgp_error error);   /* Returns a message with SGP error description. */

/* Custom pipeline creation. */
SOKOL_GP_API_DECL sg_pipeline sgp_make_pipeline(const sgp_pipeline_desc* desc); /* Creates a custom shader pipeline to be used with SGP. */

/* Draw command queue management. */
SOKOL_GP_API_DECL void sgp_begin(int width, int height);    /* Begins a new SGP draw command queue. */
SOKOL_GP_API_DECL void sgp_flush(void);                     /* Dispatch current Sokol GFX draw commands. */
SOKOL_GP_API_DECL void sgp_end(void);                       /* End current draw command queue, discarding it. */

/* 2D coordinate space projection */
SOKOL_GP_API_DECL void sgp_project(float left, float right, float top, float bottom); /* Set the coordinate space boundary in the current viewport. */
SOKOL_GP_API_DECL void sgp_reset_project(void);                                       /* Resets the coordinate space to default (coordinate of the viewport). */

/* 2D coordinate space transformation. */
SOKOL_GP_API_DECL void sgp_push_transform(void);                            /* Saves current transform matrix, to be restored later with a pop. */
SOKOL_GP_API_DECL void sgp_pop_transform(void);                             /* Restore transform matrix to the same value of the last push. */
SOKOL_GP_API_DECL void sgp_reset_transform(void);                           /* Resets the transform matrix to identity (no transform). */
SOKOL_GP_API_DECL void sgp_translate(float x, float y);                     /* Translates the 2D coordinate space. */
SOKOL_GP_API_DECL void sgp_rotate(float theta);                             /* Rotates the 2D coordinate space around the origin. */
SOKOL_GP_API_DECL void sgp_rotate_at(float theta, float x, float y);        /* Rotates the 2D coordinate space around a point. */
SOKOL_GP_API_DECL void sgp_scale(float sx, float sy);                       /* Scales the 2D coordinate space around the origin. */
SOKOL_GP_API_DECL void sgp_scale_at(float sx, float sy, float x, float y);  /* Scales the 2D coordinate space around a point. */

/* State change for custom pipelines. */
SOKOL_GP_API_DECL void sgp_set_pipeline(sg_pipeline pipeline);              /* Sets current draw pipeline. */
SOKOL_GP_API_DECL void sgp_reset_pipeline(void);                            /* Resets to the current draw pipeline to default (builtin pipelines). */
SOKOL_GP_API_DECL void sgp_set_uniform(const void* data, uint32_t size);    /* Sets uniform buffer for a custom pipeline. */
SOKOL_GP_API_DECL void sgp_reset_uniform(void);                             /* Resets uniform buffer to default (current state color). */

/* State change functions for the common pipelines. */
SOKOL_GP_API_DECL void sgp_set_blend_mode(sgp_blend_mode blend_mode);       /* Sets current blend mode. */
SOKOL_GP_API_DECL void sgp_reset_blend_mode(void);                          /* Resets current blend mode to default (no blending). */
SOKOL_GP_API_DECL void sgp_set_color(float r, float g, float b, float a);   /* Sets current color modulation. */
SOKOL_GP_API_DECL void sgp_reset_color(void);                               /* Resets current color modulation to default (white). */
SOKOL_GP_API_DECL void sgp_set_image(int channel, sg_image image);          /* Sets current bound texture in a texture channel. */
SOKOL_GP_API_DECL void sgp_unset_image(int channel);                        /* Remove current bound texture in a texture channel (no texture). */
SOKOL_GP_API_DECL void sgp_reset_image(int channel);                        /* Resets current bound texture in a channel to default (white texture). */

/* State change functions for all pipelines. */
SOKOL_GP_API_DECL void sgp_viewport(int x, int y, int w, int h);            /* Sets the screen area to draw into. */
SOKOL_GP_API_DECL void sgp_reset_viewport(void);                            /* Reset viewport to default values (0, 0, width, height). */
SOKOL_GP_API_DECL void sgp_scissor(int x, int y, int w, int h);             /* Set clip rectangle in the viewport. */
SOKOL_GP_API_DECL void sgp_reset_scissor(void);                             /* Resets clip rectangle to default (viewport bounds). */
SOKOL_GP_API_DECL void sgp_reset_state(void);                               /* Reset all state to default values. */

/* Drawing functions. */
SOKOL_GP_API_DECL void sgp_clear(void);                                                                         /* Clears the current viewport using the current state color. */
SOKOL_GP_API_DECL void sgp_draw_points(const sgp_point* points, uint32_t count);                                /* Draws points in a batch. */
SOKOL_GP_API_DECL void sgp_draw_point(float x, float y);                                                        /* Draws a single point. */
SOKOL_GP_API_DECL void sgp_draw_lines(const sgp_line* lines, uint32_t count);                                   /* Draws lines in a batch. */
SOKOL_GP_API_DECL void sgp_draw_line(float ax, float ay, float bx, float by);                                   /* Draws a single line. */
SOKOL_GP_API_DECL void sgp_draw_lines_strip(const sgp_point* points, uint32_t count);                           /* Draws a strip of lines. */
SOKOL_GP_API_DECL void sgp_draw_filled_triangles(const sgp_triangle* triangles, uint32_t count);                /* Draws triangles in a batch. */
SOKOL_GP_API_DECL void sgp_draw_filled_triangle(float ax, float ay, float bx, float by, float cx, float cy);    /* Draws a single triangle. */
SOKOL_GP_API_DECL void sgp_draw_filled_triangles_strip(const sgp_point* points, uint32_t count);                /* Draws strip of triangles. */
SOKOL_GP_API_DECL void sgp_draw_filled_rects(const sgp_rect* rects, uint32_t count);                            /* Draws a batch of rectangles. */
SOKOL_GP_API_DECL void sgp_draw_filled_rect(float x, float y, float w, float h);                                /* Draws a single rectangle. */
SOKOL_GP_API_DECL void sgp_draw_textured_rects(const sgp_rect* rects, uint32_t count);                          /* Draws a batch of textured rectangles. */
SOKOL_GP_API_DECL void sgp_draw_textured_rect(float x, float y, float w, float h);                              /* Draws a single textured rectangle. */
SOKOL_GP_API_DECL void sgp_draw_textured_rects_ex(int channel, const sgp_textured_rect* rects, uint32_t count); /* Draws a batch textured rectangle, each from a source region. */
SOKOL_GP_API_DECL void sgp_draw_textured_rect_ex(int channel, sgp_rect dest_rect, sgp_rect src_rect);           /* Draws a single textured rectangle from a source region. */

/* Querying functions. */
SOKOL_GP_API_DECL sgp_state* sgp_query_state(void); /* Returns the current draw state. */
SOKOL_GP_API_DECL sgp_desc sgp_query_desc(void);    /* Returns description of the current SGP context. */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GP_INCLUDED

#ifdef SOKOL_GP_IMPL
#ifndef SOKOL_GP_IMPL_INCLUDED
#define SOKOL_GP_IMPL_INCLUDED

#ifndef SOKOL_GFX_IMPL_INCLUDED
#error "Please include sokol_gfx.h implementation before sokol_gp.h implementation"
#endif

#include <string.h>
#include <math.h>
#include <stddef.h>

#ifndef SOKOL_LIKELY
#ifdef __GNUC__
#define SOKOL_LIKELY(x) __builtin_expect(x, 1)
#define SOKOL_UNLIKELY(x) __builtin_expect(x, 0)
#else
#define SOKOL_LIKELY(x) (x)
#define SOKOL_UNLIKELY(x) (x)
#endif
#endif

enum {
    _SGP_INIT_COOKIE = 0xCAFED0D,
    _SGP_DEFAULT_MAX_VERTICES = 65536,
    _SGP_DEFAULT_MAX_COMMANDS = 16384,
    _SGP_MAX_STACK_DEPTH = 64
};

typedef struct _sgp_vertex {
    sgp_vec2 position;
    sgp_vec2 texcoord;
} _sgp_vertex;

typedef struct _sgp_region {
    float x1, y1, x2, y2;
} _sgp_region;

typedef struct _sgp_draw_args {
    sg_pipeline pip;
    sgp_images_uniform images;
    _sgp_region region;
    uint32_t uniform_index;
    uint32_t vertex_index;
    uint32_t num_vertices;
} _sgp_draw_args;

typedef union _sgp_command_args {
    _sgp_draw_args draw;
    sgp_irect viewport;
    sgp_irect scissor;
} _sgp_command_args;

typedef enum _sgp_command_type {
    SGP_COMMAND_NONE = 0,
    SGP_COMMAND_DRAW,
    SGP_COMMAND_VIEWPORT,
    SGP_COMMAND_SCISSOR
} _sgp_command_type;

typedef struct _sgp_command {
    _sgp_command_type cmd;
    _sgp_command_args args;
} _sgp_command;

typedef struct _sgp_context {
    uint32_t init_cookie;
    sgp_error last_error;
    sgp_desc desc;

    // resources
    sg_shader shader;
    sg_buffer vertex_buf;
    sg_image white_img;
    sg_pipeline pipelines[_SG_PRIMITIVETYPE_NUM*_SGP_BLENDMODE_NUM];

    // command queue
    uint32_t cur_vertex;
    uint32_t cur_uniform;
    uint32_t cur_command;
    uint32_t num_vertices;
    uint32_t num_uniforms;
    uint32_t num_commands;
    _sgp_vertex* vertices;
    sgp_uniform* uniforms;
    _sgp_command* commands;

    // state tracking
    sgp_state state;

    // matrix stack
    uint32_t cur_transform;
    uint32_t cur_state;
    sgp_mat2x3 transform_stack[_SGP_MAX_STACK_DEPTH];
    sgp_state state_stack[_SGP_MAX_STACK_DEPTH];
} _sgp_context;

static _sgp_context _sgp;

static const sgp_mat2x3 _sgp_mat3_identity = {{
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f}
}};

static const sgp_color _sgp_white_color = {1.0f, 1.0f, 1.0f, 1.0f};

static void _sgp_set_error(sgp_error error) {
    _sgp.last_error = error;
    SOKOL_LOG(sgp_get_error_message(error));
}

#if defined(SOKOL_GLCORE33)
static const char* _sgp_vs_source =
"#version 330\n"
"layout(location=0) in vec4 coord;\n"
"out vec2 texUV;\n"
"void main() {\n"
"    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
"    texUV = coord.zw;\n"
"}\n";
static const char* _sgp_fs_source =
"#version 330\n"
"uniform sampler2D iChannel0;\n"
"uniform vec4 iColor;\n"
"in vec2 texUV;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"    fragColor = texture(iChannel0, texUV) * iColor;\n"
"}\n";
#elif defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
static const char* _sgp_vs_source =
"attribute vec4 coord;\n"
"varying vec2 texUV;\n"
"void main() {\n"
"    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
"    texUV = coord.zw;\n"
"}\n";
static const char* _sgp_fs_source =
"precision mediump float;\n"
"uniform sampler2D iChannel0;\n"
"uniform vec4 iColor;\n"
"varying vec2 texUV;\n"
"void main() {\n"
"    gl_FragColor = texture2D(iChannel0, texUV) * iColor;\n"
"}\n";
#elif defined(SOKOL_D3D11)
static const char* _sgp_vs_source =
"struct vs_out {\n"
"    float2 texUV: TEXCOORD0;\n"
"    float4 pos: SV_Position;\n"
"};\n"
"vs_out main(float4 coord: POSITION) {\n"
"    vs_out outp;\n"
"    outp.pos = float4(coord.xy, 0.0f, 1.0f);\n"
"    outp.texUV = coord.zw;\n"
"    return outp;\n"
"}\n";
static const char* _sgp_fs_source =
"Texture2D<float4> iChannel0: register(t0);\n"
"SamplerState smp: register(s0);\n"
"uniform float4 iColor;\n"
"float4 main(float2 texUV: TEXCOORD0): SV_Target0 {\n"
"    return iChannel0.Sample(smp, texUV) * iColor;\n"
"}\n";
#elif defined(SOKOL_METAL)
static const char* _sgp_vs_source =
"#include <metal_stdlib>\n"
"#include <simd/simd.h>\n"
"using namespace metal;\n"
"struct main0_out {\n"
"    float2 texUV [[user(locn0)]];\n"
"    float4 gl_Position [[position]];\n"
"};\n"
"struct main0_in {\n"
"    float4 coord [[attribute(0)]];\n"
"};\n"
"vertex main0_out _main(main0_in in [[stage_in]]) {\n"
"    main0_out out = {};\n"
"    out.gl_Position = float4(in.coord.xy, 0.0, 1.0);\n"
"    out.texUV = in.coord.zw;\n"
"    return out;\n"
"}\n";
static const char* _sgp_fs_source =
"#include <metal_stdlib>\n"
"#include <simd/simd.h>\n"
"using namespace metal;\n"
"struct main0_out {\n"
"    float4 fragColor [[color(0)]];\n"
"};\n"
"struct main0_in {\n"
"    float2 texUV [[user(locn0)]];\n"
"};\n"
"fragment main0_out _main(main0_in in [[stage_in]], constant float4& iColor [[buffer(0)]], texture2d<float> iChannel0 [[texture(0)]], sampler texSmplr [[sampler(0)]]) {\n"
"    main0_out out = {};\n"
"    out.fragColor = iChannel0.sample(texSmplr, in.texUV) * iColor;\n"
"    return out;\n"
"}\n";
#elif defined(SOKOL_WGPU)
#define _SGP_BYTECODE
/*
    #version 450
    layout(location = 0) in vec4 coord;
    layout(location = 0) out vec2 texUV;
    void main() {
        gl_Position = vec4(coord.xy, 0.0, 1.0);
        texUV = coord.zw;
    }
*/
static const uint8_t _sgp_vs_bytecode[1160] = {
    0x03,0x02,0x23,0x07,0x00,0x00,0x01,0x00,0x08,0x00,0x08,0x00,0x22,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x11,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x0b,0x00,0x06,0x00,
    0x02,0x00,0x00,0x00,0x47,0x4c,0x53,0x4c,0x2e,0x73,0x74,0x64,0x2e,0x34,0x35,0x30,
    0x00,0x00,0x00,0x00,0x0e,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
    0x0f,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x6d,0x61,0x69,0x6e,
    0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x1f,0x00,0x00,0x00,
    0x07,0x00,0x03,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x00,0x08,0x00,
    0x07,0x00,0x00,0x00,0x73,0x6f,0x6b,0x6f,0x6c,0x5f,0x67,0x70,0x5f,0x73,0x68,0x61,
    0x64,0x65,0x72,0x73,0x2e,0x67,0x6c,0x73,0x6c,0x00,0x00,0x00,0x03,0x00,0x37,0x00,
    0x02,0x00,0x00,0x00,0xc2,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x2f,0x2f,0x20,0x4f,
    0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,
    0x20,0x63,0x6c,0x69,0x65,0x6e,0x74,0x20,0x76,0x75,0x6c,0x6b,0x61,0x6e,0x31,0x30,
    0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,
    0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x63,0x6c,0x69,0x65,0x6e,0x74,0x20,0x6f,0x70,
    0x65,0x6e,0x67,0x6c,0x31,0x30,0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,
    0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x74,0x61,0x72,
    0x67,0x65,0x74,0x2d,0x65,0x6e,0x76,0x20,0x76,0x75,0x6c,0x6b,0x61,0x6e,0x31,0x2e,
    0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,
    0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x74,0x61,0x72,0x67,0x65,0x74,0x2d,0x65,0x6e,
    0x76,0x20,0x6f,0x70,0x65,0x6e,0x67,0x6c,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,
    0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x65,0x6e,
    0x74,0x72,0x79,0x2d,0x70,0x6f,0x69,0x6e,0x74,0x20,0x6d,0x61,0x69,0x6e,0x0a,0x23,
    0x6c,0x69,0x6e,0x65,0x20,0x31,0x0a,0x00,0x05,0x00,0x04,0x00,0x05,0x00,0x00,0x00,
    0x6d,0x61,0x69,0x6e,0x00,0x00,0x00,0x00,0x05,0x00,0x06,0x00,0x0d,0x00,0x00,0x00,
    0x67,0x6c,0x5f,0x50,0x65,0x72,0x56,0x65,0x72,0x74,0x65,0x78,0x00,0x00,0x00,0x00,
    0x06,0x00,0x06,0x00,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x67,0x6c,0x5f,0x50,
    0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x00,0x06,0x00,0x07,0x00,0x0d,0x00,0x00,0x00,
    0x01,0x00,0x00,0x00,0x67,0x6c,0x5f,0x50,0x6f,0x69,0x6e,0x74,0x53,0x69,0x7a,0x65,
    0x00,0x00,0x00,0x00,0x06,0x00,0x07,0x00,0x0d,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
    0x67,0x6c,0x5f,0x43,0x6c,0x69,0x70,0x44,0x69,0x73,0x74,0x61,0x6e,0x63,0x65,0x00,
    0x06,0x00,0x07,0x00,0x0d,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x67,0x6c,0x5f,0x43,
    0x75,0x6c,0x6c,0x44,0x69,0x73,0x74,0x61,0x6e,0x63,0x65,0x00,0x05,0x00,0x03,0x00,
    0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x04,0x00,0x13,0x00,0x00,0x00,
    0x63,0x6f,0x6f,0x72,0x64,0x00,0x00,0x00,0x05,0x00,0x04,0x00,0x1f,0x00,0x00,0x00,
    0x74,0x65,0x78,0x55,0x56,0x00,0x00,0x00,0x48,0x00,0x05,0x00,0x0d,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,0x00,0x05,0x00,
    0x0d,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
    0x48,0x00,0x05,0x00,0x0d,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,
    0x03,0x00,0x00,0x00,0x48,0x00,0x05,0x00,0x0d,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
    0x0b,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x47,0x00,0x03,0x00,0x0d,0x00,0x00,0x00,
    0x02,0x00,0x00,0x00,0x47,0x00,0x04,0x00,0x13,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,0x1f,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x13,0x00,0x02,0x00,0x03,0x00,0x00,0x00,0x21,0x00,0x03,0x00,
    0x04,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x16,0x00,0x03,0x00,0x08,0x00,0x00,0x00,
    0x20,0x00,0x00,0x00,0x17,0x00,0x04,0x00,0x09,0x00,0x00,0x00,0x08,0x00,0x00,0x00,
    0x04,0x00,0x00,0x00,0x15,0x00,0x04,0x00,0x0a,0x00,0x00,0x00,0x20,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x0a,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,
    0x01,0x00,0x00,0x00,0x1c,0x00,0x04,0x00,0x0c,0x00,0x00,0x00,0x08,0x00,0x00,0x00,
    0x0b,0x00,0x00,0x00,0x1e,0x00,0x06,0x00,0x0d,0x00,0x00,0x00,0x09,0x00,0x00,0x00,
    0x08,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x0e,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x0e,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x15,0x00,0x04,0x00,
    0x10,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,
    0x10,0x00,0x00,0x00,0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x12,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x12,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x17,0x00,0x04,0x00,
    0x14,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,
    0x08,0x00,0x00,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,
    0x08,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,0x20,0x00,0x04,0x00,
    0x1c,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x1e,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x1e,0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x36,0x00,0x05,0x00,
    0x03,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
    0xf8,0x00,0x02,0x00,0x06,0x00,0x00,0x00,0x08,0x00,0x04,0x00,0x07,0x00,0x00,0x00,
    0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,0x09,0x00,0x00,0x00,
    0x15,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x51,0x00,0x05,0x00,0x08,0x00,0x00,0x00,
    0x19,0x00,0x00,0x00,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51,0x00,0x05,0x00,
    0x08,0x00,0x00,0x00,0x1a,0x00,0x00,0x00,0x15,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
    0x50,0x00,0x07,0x00,0x09,0x00,0x00,0x00,0x1b,0x00,0x00,0x00,0x19,0x00,0x00,0x00,
    0x1a,0x00,0x00,0x00,0x17,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x41,0x00,0x05,0x00,
    0x1c,0x00,0x00,0x00,0x1d,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x11,0x00,0x00,0x00,
    0x3e,0x00,0x03,0x00,0x1d,0x00,0x00,0x00,0x1b,0x00,0x00,0x00,0x4f,0x00,0x07,0x00,
    0x14,0x00,0x00,0x00,0x21,0x00,0x00,0x00,0x15,0x00,0x00,0x00,0x15,0x00,0x00,0x00,
    0x02,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x3e,0x00,0x03,0x00,0x1f,0x00,0x00,0x00,
    0x21,0x00,0x00,0x00,0xfd,0x00,0x01,0x00,0x38,0x00,0x01,0x00,
};
/*
    #version 450
    layout(set = 0, binding = 4, std140) uniform fs_in {
        vec4 iColor;
    } in;
    layout(location = 0, set = 2, binding = 0) uniform sampler2D iChannel0;
    layout(location = 0) out vec4 fragColor;
    layout(location = 0) in vec2 texUV;
    void main() {
        fragColor = texture(iChannel0, texUV) * in.iColor;
    }
*/
static const uint8_t _sgp_fs_bytecode[1060] = {
    0x03,0x02,0x23,0x07,0x00,0x00,0x01,0x00,0x08,0x00,0x08,0x00,0x1f,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x11,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x0b,0x00,0x06,0x00,
    0x02,0x00,0x00,0x00,0x47,0x4c,0x53,0x4c,0x2e,0x73,0x74,0x64,0x2e,0x34,0x35,0x30,
    0x00,0x00,0x00,0x00,0x0e,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
    0x0f,0x00,0x07,0x00,0x04,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x6d,0x61,0x69,0x6e,
    0x00,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x10,0x00,0x03,0x00,
    0x05,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x07,0x00,0x03,0x00,0x01,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x07,0x00,0x08,0x00,0x07,0x00,0x00,0x00,0x73,0x6f,0x6b,0x6f,
    0x6c,0x5f,0x67,0x70,0x5f,0x73,0x68,0x61,0x64,0x65,0x72,0x73,0x2e,0x67,0x6c,0x73,
    0x6c,0x00,0x00,0x00,0x03,0x00,0x37,0x00,0x02,0x00,0x00,0x00,0xc2,0x01,0x00,0x00,
    0x01,0x00,0x00,0x00,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,
    0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x63,0x6c,0x69,0x65,0x6e,0x74,0x20,
    0x76,0x75,0x6c,0x6b,0x61,0x6e,0x31,0x30,0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,
    0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x63,
    0x6c,0x69,0x65,0x6e,0x74,0x20,0x6f,0x70,0x65,0x6e,0x67,0x6c,0x31,0x30,0x30,0x0a,
    0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,
    0x73,0x73,0x65,0x64,0x20,0x74,0x61,0x72,0x67,0x65,0x74,0x2d,0x65,0x6e,0x76,0x20,
    0x76,0x75,0x6c,0x6b,0x61,0x6e,0x31,0x2e,0x30,0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,
    0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,0x65,0x73,0x73,0x65,0x64,0x20,0x74,
    0x61,0x72,0x67,0x65,0x74,0x2d,0x65,0x6e,0x76,0x20,0x6f,0x70,0x65,0x6e,0x67,0x6c,
    0x0a,0x2f,0x2f,0x20,0x4f,0x70,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x50,0x72,0x6f,0x63,
    0x65,0x73,0x73,0x65,0x64,0x20,0x65,0x6e,0x74,0x72,0x79,0x2d,0x70,0x6f,0x69,0x6e,
    0x74,0x20,0x6d,0x61,0x69,0x6e,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,0x31,0x0a,0x00,
    0x05,0x00,0x04,0x00,0x05,0x00,0x00,0x00,0x6d,0x61,0x69,0x6e,0x00,0x00,0x00,0x00,
    0x05,0x00,0x05,0x00,0x0b,0x00,0x00,0x00,0x66,0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,
    0x72,0x00,0x00,0x00,0x05,0x00,0x05,0x00,0x0f,0x00,0x00,0x00,0x69,0x43,0x68,0x61,
    0x6e,0x6e,0x65,0x6c,0x30,0x00,0x00,0x00,0x05,0x00,0x04,0x00,0x13,0x00,0x00,0x00,
    0x74,0x65,0x78,0x55,0x56,0x00,0x00,0x00,0x05,0x00,0x05,0x00,0x16,0x00,0x00,0x00,
    0x66,0x73,0x5f,0x70,0x61,0x72,0x61,0x6d,0x73,0x00,0x00,0x00,0x06,0x00,0x05,0x00,
    0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x69,0x43,0x6f,0x6c,0x6f,0x72,0x00,0x00,
    0x05,0x00,0x03,0x00,0x18,0x00,0x00,0x00,0x5f,0x32,0x34,0x00,0x47,0x00,0x04,0x00,
    0x0b,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x0f,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x0f,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x0f,0x00,0x00,0x00,0x21,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x13,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,0x00,0x05,0x00,
    0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x23,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x47,0x00,0x03,0x00,0x16,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x18,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x18,0x00,0x00,0x00,0x21,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x13,0x00,0x02,0x00,
    0x03,0x00,0x00,0x00,0x21,0x00,0x03,0x00,0x04,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
    0x16,0x00,0x03,0x00,0x08,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x17,0x00,0x04,0x00,
    0x09,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x0a,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x0a,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x19,0x00,0x09,0x00,
    0x0c,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x1b,0x00,0x03,0x00,0x0d,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x0e,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x17,0x00,0x04,0x00,
    0x11,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x12,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x11,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x12,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x1e,0x00,0x03,0x00,
    0x16,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x17,0x00,0x00,0x00,
    0x02,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x17,0x00,0x00,0x00,
    0x18,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x15,0x00,0x04,0x00,0x19,0x00,0x00,0x00,
    0x20,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x19,0x00,0x00,0x00,
    0x1a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x1b,0x00,0x00,0x00,
    0x02,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x36,0x00,0x05,0x00,0x03,0x00,0x00,0x00,
    0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0xf8,0x00,0x02,0x00,
    0x06,0x00,0x00,0x00,0x08,0x00,0x04,0x00,0x07,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,0x0d,0x00,0x00,0x00,0x10,0x00,0x00,0x00,
    0x0f,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,0x11,0x00,0x00,0x00,0x14,0x00,0x00,0x00,
    0x13,0x00,0x00,0x00,0x57,0x00,0x05,0x00,0x09,0x00,0x00,0x00,0x15,0x00,0x00,0x00,
    0x10,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x41,0x00,0x05,0x00,0x1b,0x00,0x00,0x00,
    0x1c,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x1a,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,
    0x09,0x00,0x00,0x00,0x1d,0x00,0x00,0x00,0x1c,0x00,0x00,0x00,0x85,0x00,0x05,0x00,
    0x09,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x15,0x00,0x00,0x00,0x1d,0x00,0x00,0x00,
    0x3e,0x00,0x03,0x00,0x0b,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0xfd,0x00,0x01,0x00,
    0x38,0x00,0x01,0x00,
};
#elif defined(SOKOL_DUMMY_BACKEND)
static const char _sgp_vs_source[] = "";
static const char _sgp_fs_source[] = "";
#else
#error "Please define one of SOKOL_GLCORE33, SOKOL_GLES2, SOKOL_GLES3, SOKOL_D3D11, SOKOL_METAL, SOKOL_WGPU or SOKOL_DUMMY_BACKEND!"
#endif

static sg_blend_state _sgp_blend_state(sgp_blend_mode blend_mode) {
    sg_blend_state blend;
    memset(&blend, 0, sizeof(sg_blend_state));
    switch(blend_mode) {
        case SGP_BLENDMODE_NONE:
            blend.enabled = false;
            blend.src_factor_rgb = SG_BLENDFACTOR_ONE;
            blend.dst_factor_rgb = SG_BLENDFACTOR_ZERO;
            blend.op_rgb = SG_BLENDOP_ADD;
            blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
            blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
            blend.op_alpha = SG_BLENDOP_ADD;
            break;
        case SGP_BLENDMODE_BLEND:
            blend.enabled = true;
            blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
            blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend.op_rgb = SG_BLENDOP_ADD;
            blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
            blend.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend.op_alpha = SG_BLENDOP_ADD;
            break;
        case SGP_BLENDMODE_ADD:
            blend.enabled = true;
            blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
            blend.dst_factor_rgb = SG_BLENDFACTOR_ONE;
            blend.op_rgb = SG_BLENDOP_ADD;
            blend.src_factor_alpha = SG_BLENDFACTOR_ZERO;
            blend.dst_factor_alpha = SG_BLENDFACTOR_ONE;
            blend.op_alpha = SG_BLENDOP_ADD;
            break;
        case SGP_BLENDMODE_MOD:
            blend.enabled = true;
            blend.src_factor_rgb = SG_BLENDFACTOR_DST_COLOR;
            blend.dst_factor_rgb = SG_BLENDFACTOR_ZERO;
            blend.op_rgb = SG_BLENDOP_ADD;
            blend.src_factor_alpha = SG_BLENDFACTOR_ZERO;
            blend.dst_factor_alpha = SG_BLENDFACTOR_ONE;
            blend.op_alpha = SG_BLENDOP_ADD;
            break;
        case SGP_BLENDMODE_MUL:
            blend.enabled = true;
            blend.src_factor_rgb = SG_BLENDFACTOR_DST_COLOR;
            blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend.op_rgb = SG_BLENDOP_ADD;
            blend.src_factor_alpha = SG_BLENDFACTOR_DST_ALPHA;
            blend.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend.op_alpha = SG_BLENDOP_ADD;
            break;
        default:
            blend.enabled = false;
            blend.src_factor_rgb = SG_BLENDFACTOR_ONE;
            blend.dst_factor_rgb = SG_BLENDFACTOR_ZERO;
            blend.op_rgb = SG_BLENDOP_ADD;
            blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
            blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
            blend.op_alpha = SG_BLENDOP_ADD;
            SOKOL_UNREACHABLE;
            break;
    }
    return blend;
}

static sg_pipeline _sgp_make_pipeline(sg_primitive_type primitive_type, sgp_blend_mode blend_mode, sg_shader shader) {
    sg_blend_state blend = _sgp_blend_state(blend_mode);

    if(primitive_type == _SG_PRIMITIVETYPE_DEFAULT)
        primitive_type = SG_PRIMITIVETYPE_TRIANGLES;

    // create pipeline
    sg_pipeline_desc pip_desc;
    memset(&pip_desc, 0, sizeof(sg_pipeline_desc));
    pip_desc.shader = shader;
    pip_desc.layout.buffers[0].stride = sizeof(_sgp_vertex);
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT4;
    pip_desc.colors[0].pixel_format = _sg.desc.context.color_format;
    pip_desc.colors[0].blend = blend;
    pip_desc.primitive_type = primitive_type;

    sg_pipeline pip = sg_make_pipeline(&pip_desc);
    if(pip.id != SG_INVALID_ID && sg_query_pipeline_state(pip) != SG_RESOURCESTATE_VALID) {
        sg_destroy_pipeline(pip);
        pip.id = SG_INVALID_ID;
    }
    return pip;
}

static sg_pipeline _sgp_lookup_pipeline(sg_primitive_type primitive_type, sgp_blend_mode blend_mode) {
    uint32_t pip_index = (primitive_type * _SGP_BLENDMODE_NUM) + blend_mode;
    if(_sgp.pipelines[pip_index].id != SG_INVALID_ID)
        return _sgp.pipelines[pip_index];

    sg_pipeline pip = _sgp_make_pipeline(primitive_type, blend_mode, _sgp.shader);
    if(pip.id != SG_INVALID_ID)
        _sgp.pipelines[pip_index] = pip;
    return pip;
}

static sg_shader _sgp_make_common_shader(void) {
    sg_shader_desc shader_desc;
    memset(&shader_desc, 0, sizeof(shader_desc));
    shader_desc.attrs[0].name = "coord";
    shader_desc.attrs[0].sem_name = "POSITION";
    shader_desc.attrs[0].sem_index = 0;
#ifdef _SGP_BYTECODE
    shader_desc.vs.byte_code = _sgp_vs_bytecode;
    shader_desc.vs.byte_code_size = sizeof(_sgp_vs_bytecode);
#else
    shader_desc.vs.source = _sgp_vs_source;
#endif
#ifdef _SGP_BYTECODE
    shader_desc.fs.byte_code = _sgp_fs_bytecode;
    shader_desc.fs.byte_code_size = sizeof(_sgp_fs_bytecode);
#else
    shader_desc.fs.source = _sgp_fs_source;
#endif
    shader_desc.fs.uniform_blocks[0].size = sizeof(sgp_color);
    shader_desc.fs.uniform_blocks[0].uniforms[0].name = "iColor";
    shader_desc.fs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_FLOAT4;
    shader_desc.fs.images[0].name = "iChannel0";
    shader_desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
    return sg_make_shader(&shader_desc);
}

void sgp_setup(const sgp_desc* desc) {
    SOKOL_ASSERT(_sgp.init_cookie == 0);

    if(!sg_isvalid()) {
        _sgp_set_error(SGP_ERROR_SOKOL_INVALID);
        return;
    }

    // init
    _sgp.init_cookie = _SGP_INIT_COOKIE;
    _sgp.last_error = SGP_NO_ERROR;

    // set desc default values
    _sgp.desc = *desc;
    _sgp.desc.max_vertices = _sg_def(desc->max_vertices, _SGP_DEFAULT_MAX_VERTICES);
    _sgp.desc.max_commands = _sg_def(desc->max_commands, _SGP_DEFAULT_MAX_COMMANDS);

    // allocate buffers
    _sgp.num_vertices = _sgp.desc.max_vertices;
    _sgp.num_commands = _sgp.desc.max_commands;
    _sgp.num_uniforms = _sgp.desc.max_commands;
    _sgp.vertices = (_sgp_vertex*) _sg_malloc(_sgp.num_vertices * sizeof(_sgp_vertex));
    _sgp.uniforms = (sgp_uniform*) _sg_malloc(_sgp.num_uniforms * sizeof(sgp_uniform));
    _sgp.commands = (_sgp_command*) _sg_malloc(_sgp.num_commands * sizeof(_sgp_command));
    if(!_sgp.commands || !_sgp.uniforms || !_sgp.commands) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_ALLOC_FAILED);
        return;
    }
    memset(_sgp.vertices, 0, _sgp.num_vertices * sizeof(_sgp_vertex));
    memset(_sgp.uniforms, 0, _sgp.num_uniforms * sizeof(sgp_uniform));
    memset(_sgp.commands, 0, _sgp.num_commands * sizeof(_sgp_command));

    // create vertex buffer
    sg_buffer_desc vertex_buf_desc;
    memset(&vertex_buf_desc, 0, sizeof(sg_buffer_desc));
    vertex_buf_desc.size = (size_t)(_sgp.num_vertices * sizeof(_sgp_vertex));
    vertex_buf_desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
    vertex_buf_desc.usage = SG_USAGE_STREAM;

    _sgp.vertex_buf = sg_make_buffer(&vertex_buf_desc);
    if(sg_query_buffer_state(_sgp.vertex_buf) != SG_RESOURCESTATE_VALID) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_MAKE_VERTEX_BUFFER_FAILED);
        return;
    }

    // create white texture
    uint32_t pixels[4];
    memset(pixels, 0xFF, sizeof(pixels));
    sg_image_desc white_img_desc;
    memset(&white_img_desc, 0, sizeof(sg_image_desc));
    white_img_desc.type = SG_IMAGETYPE_2D;
    white_img_desc.width = 2;
    white_img_desc.height = 2;
    white_img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    white_img_desc.min_filter = SG_FILTER_NEAREST;
    white_img_desc.mag_filter = SG_FILTER_NEAREST;
    white_img_desc.data.subimage[0][0].ptr = pixels;
    white_img_desc.data.subimage[0][0].size = sizeof(pixels);
    white_img_desc.label = "sgp-white-texture";

    _sgp.white_img = sg_make_image(&white_img_desc);
    if(sg_query_image_state(_sgp.white_img) != SG_RESOURCESTATE_VALID) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_MAKE_WHITE_IMAGE_FAILED);
        return;
    }

    // create common shader
    _sgp.shader = _sgp_make_common_shader();
    if(sg_query_shader_state(_sgp.shader) != SG_RESOURCESTATE_VALID) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_MAKE_COMMON_SHADER_FAILED);
        return;
    }

    // create common pipelines
    bool pips_ok = true;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_NONE).id != SG_INVALID_ID;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_BLEND).id != SG_INVALID_ID;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_POINTS, SGP_BLENDMODE_NONE).id != SG_INVALID_ID;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_POINTS, SGP_BLENDMODE_BLEND).id != SG_INVALID_ID;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINES, SGP_BLENDMODE_NONE).id != SG_INVALID_ID;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINES, SGP_BLENDMODE_BLEND).id != SG_INVALID_ID;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, SGP_BLENDMODE_NONE).id != SG_INVALID_ID;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, SGP_BLENDMODE_BLEND).id != SG_INVALID_ID;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, SGP_BLENDMODE_NONE).id != SG_INVALID_ID;
    pips_ok = pips_ok && _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, SGP_BLENDMODE_BLEND).id != SG_INVALID_ID;
    if(!pips_ok) {
        sgp_shutdown();
        _sgp_set_error(SGP_ERROR_MAKE_COMMON_PIPELINE_FAILED);
        return;
    }
}

void sgp_shutdown(void) {
    if(_sgp.init_cookie == 0) return; // not initialized
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state == 0);
    if(_sgp.vertices)
        _sg_free(_sgp.vertices);
    if(_sgp.uniforms)
        _sg_free(_sgp.uniforms);
    if(_sgp.commands)
        _sg_free(_sgp.commands);
    for(uint32_t i=0;i<_SG_PRIMITIVETYPE_NUM*_SGP_BLENDMODE_NUM;++i) {
        sg_pipeline pip = _sgp.pipelines[i];
        if(pip.id != SG_INVALID_ID)
            sg_destroy_pipeline(pip);
    }
    if(_sgp.shader.id != SG_INVALID_ID)
        sg_destroy_shader(_sgp.shader);
    if(_sgp.vertex_buf.id != SG_INVALID_ID)
        sg_destroy_buffer(_sgp.vertex_buf);
    if(_sgp.white_img.id != SG_INVALID_ID)
        sg_destroy_image(_sgp.white_img);
    memset(&_sgp, 0, sizeof(_sgp_context));
}

bool sgp_is_valid(void) {
    return _sgp.init_cookie == _SGP_INIT_COOKIE;
}

sgp_error sgp_get_last_error(void) {
    return _sgp.last_error;
}

const char* sgp_get_error_message(sgp_error error_code) {
    switch(error_code) {
        case SGP_NO_ERROR:
            return "No error";
        case SGP_ERROR_SOKOL_INVALID:
            return "Sokol is not initialized";
        case SGP_ERROR_VERTICES_FULL:
            return "SGP vertices buffer is full";
        case SGP_ERROR_UNIFORMS_FULL:
            return "SGP uniform buffer is full";
        case SGP_ERROR_COMMANDS_FULL:
            return "SGP command buffer is full";
        case SGP_ERROR_VERTICES_OVERFLOW:
            return "SGP vertices buffer overflow";
        case SGP_ERROR_TRANSFORM_STACK_OVERFLOW:
            return "SGP transform stack overflow";
        case SGP_ERROR_TRANSFORM_STACK_UNDERFLOW:
            return "SGP transform stack underflow";
        case SGP_ERROR_STATE_STACK_OVERFLOW:
            return "SGP state stack overflow";
        case SGP_ERROR_STATE_STACK_UNDERFLOW:
            return "SGP state stack underflow";
        case SGP_ERROR_ALLOC_FAILED:
            return "SGP failed to allocate buffers";
        case SGP_ERROR_MAKE_VERTEX_BUFFER_FAILED:
            return "SGP failed to create vertex buffer";
        case SGP_ERROR_MAKE_WHITE_IMAGE_FAILED:
            return "SGP failed to create white image";
        case SGP_ERROR_MAKE_COMMON_SHADER_FAILED:
            return "SGP failed to create the common shader";
        case SGP_ERROR_MAKE_COMMON_PIPELINE_FAILED:
            return "SGP failed to create the common pipeline";
        default:
            return "Invalid error code";
    }
}

sg_pipeline sgp_make_pipeline(const sgp_pipeline_desc* desc) {
    sg_pipeline pip = {SG_INVALID_ID};
    sg_shader shader = sg_make_shader(&desc->shader);
    if(sg_query_shader_state(shader) == SG_RESOURCESTATE_VALID)
        pip = _sgp_make_pipeline(desc->primitive_type, desc->blend_mode, shader);
    else if(shader.id != SG_INVALID_ID)
        sg_destroy_shader(shader);
    return pip;
}

static inline sgp_mat2x3 _sgp_default_proj(int width, int height) {
    // matrix to convert screen coordinate system
    // to the usual the coordinate system used on the backends
    sgp_mat2x3 mat = {{
        {2.0f/(float)width,                0.0f, -1.0f},
        {      0.0f,        -2.0f/(float)height,  1.0f}
    }};
    return mat;
}

void sgp_begin(int width, int height) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(_sgp.cur_state >= _SGP_MAX_STACK_DEPTH)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_OVERFLOW);
        return;
    }

    // begin reset last error
    _sgp.last_error = SGP_NO_ERROR;

    // save current state
    _sgp.state_stack[_sgp.cur_state++] = _sgp.state;

    // reset to default state
    _sgp.state.frame_size.w = width; _sgp.state.frame_size.h = height;
    _sgp.state.viewport.x = 0; _sgp.state.viewport.y = 0;
    _sgp.state.viewport.w = width; _sgp.state.viewport.h = height;
    _sgp.state.scissor.x = 0; _sgp.state.scissor.y = 0;
    _sgp.state.scissor.w = -1; _sgp.state.scissor.h = -1;
    _sgp.state.proj = _sgp_default_proj(width, height);
    _sgp.state.transform = _sgp_mat3_identity;
    _sgp.state.mvp = _sgp.state.proj;
    _sgp.state.color = _sgp_white_color;
    memset(&_sgp.state.uniform, 0, sizeof(sgp_uniform));
    _sgp.state.uniform.size = sizeof(sgp_color);
    _sgp.state.uniform.content[0] = _sgp_white_color.r;
    _sgp.state.uniform.content[1] = _sgp_white_color.g;
    _sgp.state.uniform.content[2] = _sgp_white_color.b;
    _sgp.state.uniform.content[3] = _sgp_white_color.a;
    _sgp.state.blend_mode = SGP_BLENDMODE_NONE;
    _sgp.state._base_vertex = _sgp.cur_vertex;
    _sgp.state._base_uniform = _sgp.cur_uniform;
    _sgp.state._base_command = _sgp.cur_command;

    _sgp.state.images.count = 1;
    _sgp.state.images.images[0] = _sgp.white_img;
    sg_image img = {SG_INVALID_ID};
    for(int i=1;i<SGP_TEXTURE_SLOTS;++i) {
        _sgp.state.images.images[i] = img;
    }
}

static void _sgp_get_pipeline_uniform_count(sg_pipeline pip, int* vs_uniform_count, int* fs_uniform_count) {
    _sg_pipeline_t* p = _sg_lookup_pipeline(&_sg.pools, pip.id);
    SOKOL_ASSERT(p);
    *vs_uniform_count = p ? p->shader->cmn.stage[SG_SHADERSTAGE_VS].num_uniform_blocks : 0;
    *fs_uniform_count = p ? p->shader->cmn.stage[SG_SHADERSTAGE_FS].num_uniform_blocks : 0;
}

void sgp_flush(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    uint32_t end_command = _sgp.cur_command;
    uint32_t end_vertex = _sgp.cur_vertex;

    // rewind indexes
    _sgp.cur_vertex = _sgp.state._base_vertex;
    _sgp.cur_uniform = _sgp.state._base_uniform;
    _sgp.cur_command = _sgp.state._base_command;

    // draw nothing on errors
    if(_sgp.last_error != SGP_NO_ERROR)
        return;

    // nothing to be drawn
    if(end_command <= _sgp.state._base_command)
        return;

    // upload vertices
    uint32_t base_vertex = _sgp.state._base_vertex;
    uint32_t num_vertices = (end_vertex - base_vertex) * sizeof(_sgp_vertex);
    sg_range vertex_range = {&_sgp.vertices[base_vertex], num_vertices};
    int offset = sg_append_buffer(_sgp.vertex_buf, &vertex_range);
    if(sg_query_buffer_overflow(_sgp.vertex_buf)) {
        _sgp_set_error(SGP_ERROR_VERTICES_OVERFLOW);
        return;
    }

    const uint32_t SG_IMPOSSIBLE_ID = 0xffffffffU;
    uint32_t cur_pip_id = SG_IMPOSSIBLE_ID;
    uint32_t cur_uniform_index = SG_IMPOSSIBLE_ID;
    uint32_t cur_imgs_id[SGP_TEXTURE_SLOTS];
    for(int i=0;i<SGP_TEXTURE_SLOTS;++i)
        cur_imgs_id[i] = SG_IMPOSSIBLE_ID;

    // define the resource bindings
    sg_bindings bind;
    memset(&bind, 0, sizeof(sg_bindings));
    bind.vertex_buffers[0] = _sgp.vertex_buf;
    bind.vertex_buffer_offsets[0] = offset;

    // flush commands
    for(uint32_t i = _sgp.state._base_command; i < end_command; ++i) {
        _sgp_command* cmd = &_sgp.commands[i];
        switch(cmd->cmd) {
            case SGP_COMMAND_VIEWPORT: {
                sgp_irect* args = &cmd->args.viewport;
                sg_apply_viewport(args->x, args->y, args->w, args->h, true);
                break;
            }
            case SGP_COMMAND_SCISSOR: {
                sgp_irect* args = &cmd->args.scissor;
                sg_apply_scissor_rect(args->x, args->y, args->w, args->h, true);
                break;
            }
            case SGP_COMMAND_DRAW: {
                _sgp_draw_args* args = &cmd->args.draw;
                if(args->num_vertices == 0)
                    break;
                bool apply_bindings = false;
                // pipeline
                if(args->pip.id != cur_pip_id) {
                    // when pipeline changes we need to re-apply uniforms and bindings
                    cur_uniform_index = SG_IMPOSSIBLE_ID;
                    apply_bindings = true;
                    cur_pip_id = args->pip.id;
                    sg_apply_pipeline(args->pip);
                }
                // bindings
                for(uint32_t j=0;j<SGP_TEXTURE_SLOTS;++j) {
                    uint32_t img_id = j < args->images.count ? args->images.images[j].id : (uint32_t)SG_INVALID_ID;
                    if(cur_imgs_id[j] != img_id) {
                        // when an image binding change we need to re-apply bindings
                        cur_imgs_id[j] = img_id;
                        bind.fs_images[j].id = img_id;
                        apply_bindings = true;
                    }
                }
                if(apply_bindings)
                    sg_apply_bindings(&bind);
                // uniforms
                if(cur_uniform_index != args->uniform_index) {
                    cur_uniform_index = args->uniform_index;
                    sgp_uniform* uniform = &_sgp.uniforms[cur_uniform_index];
                    if(uniform->size > 0) {
                        sg_range uniform_range = {&uniform->content, uniform->size};
                        int vs_uniform_count, fs_uniform_count;
                        _sgp_get_pipeline_uniform_count(args->pip, &vs_uniform_count, &fs_uniform_count);
                        // apply uniforms on vertex shader only when needed
                        if(vs_uniform_count > 0)
                            sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &uniform_range);
                        // apply uniforms on fragment shader
                        if(fs_uniform_count > 0)
                            sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &uniform_range);
                    }
                }
                //  draw
                sg_draw((int)(args->vertex_index - base_vertex), (int)args->num_vertices, 1);
                break;
            }
            case SGP_COMMAND_NONE: {
                // this command was optimized away
                break;
            }
        }
    }
}

void sgp_end(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(_sgp.cur_state <= 0)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_UNDERFLOW);
        return;
    }

    // restore old state
    _sgp.state = _sgp.state_stack[--_sgp.cur_state];
}

static inline sgp_mat2x3 _sgp_mul_proj_transform(sgp_mat2x3* proj, sgp_mat2x3* transform) {
    // this actually multiply matrix projection and transform matrix in an optimized way
    float x = proj->v[0][0], y = proj->v[1][1];
    sgp_mat2x3 m = {{
        {x*transform->v[0][0], x*transform->v[0][1], x*transform->v[0][2]+proj->v[0][2]},
        {y*transform->v[1][0], y*transform->v[1][1], y*transform->v[1][2]+proj->v[1][2]}
    }};
    return m;
}

void sgp_project(float left, float right, float top, float bottom) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    float w = right - left;
    float h = top - bottom;
    sgp_mat2x3 proj = {{
        {2.0f/w,   0.0f,  -(right+left)/w},
        {0.0f,   2.0f/h,  -(top+bottom)/h}
    }};
    _sgp.state.proj = proj;
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_reset_project(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.proj = _sgp_default_proj(_sgp.state.viewport.w, _sgp.state.viewport.h);
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_push_transform(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(_sgp.cur_transform >= _SGP_MAX_STACK_DEPTH)) {
        _sgp_set_error(SGP_ERROR_TRANSFORM_STACK_OVERFLOW);
        return;
    }
    _sgp.transform_stack[_sgp.cur_transform++] = _sgp.state.transform;
}

void sgp_pop_transform(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(_sgp.cur_transform <= 0)) {
        _sgp_set_error(SGP_ERROR_TRANSFORM_STACK_UNDERFLOW);
        return;
    }
    _sgp.state.transform = _sgp.transform_stack[--_sgp.cur_transform];
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_reset_transform(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.transform = _sgp_mat3_identity;
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_translate(float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    // multiply by translate matrix:
    // 1.0f, 0.0f,    x,
    // 0.0f, 1.0f,    y,
    // 0.0f, 0.0f, 1.0f,
    _sgp.state.transform.v[0][2] += x*_sgp.state.transform.v[0][0] + y*_sgp.state.transform.v[0][1];
    _sgp.state.transform.v[1][2] += x*_sgp.state.transform.v[1][0] + y*_sgp.state.transform.v[1][1];
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_rotate(float theta) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    float sint = sinf(theta), cost = cosf(theta);
    // multiply by rotation matrix:
    // cost, -sint, 0.0f,
    // sint,  cost, 0.0f,
    // 0.0f,  0.0f, 1.0f,
    sgp_mat2x3 transform = {{
       {cost*_sgp.state.transform.v[0][0]+sint*_sgp.state.transform.v[0][1], -sint*_sgp.state.transform.v[0][0]+cost*_sgp.state.transform.v[0][1], _sgp.state.transform.v[0][2]},
       {cost*_sgp.state.transform.v[1][0]+sint*_sgp.state.transform.v[1][1], -sint*_sgp.state.transform.v[1][0]+cost*_sgp.state.transform.v[1][1], _sgp.state.transform.v[1][2]}
    }};
    _sgp.state.transform = transform;
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_rotate_at(float theta, float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_translate(x, y);
    sgp_rotate(theta);
    sgp_translate(-x, -y);
}

void sgp_scale(float sx, float sy) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    // multiply by scale matrix:
    //   sx, 0.0f, 0.0f,
    // 0.0f,   sy, 0.0f,
    // 0.0f, 0.0f, 1.0f,
    _sgp.state.transform.v[0][0] *= sx;
    _sgp.state.transform.v[1][0] *= sx;
    _sgp.state.transform.v[0][1] *= sy;
    _sgp.state.transform.v[1][1] *= sy;
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_scale_at(float sx, float sy, float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_translate(x, y);
    sgp_scale(sx, sy);
    sgp_translate(-x, -y);
}

void sgp_set_pipeline(sg_pipeline pipeline) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    _sgp.state.pipeline = pipeline;

    // restore uniform for the default pipeline
    memset(&_sgp.state.uniform, 0, sizeof(sgp_uniform));
    if(pipeline.id == SG_INVALID_ID) {
        _sgp.state.uniform.size = sizeof(sgp_color);
        _sgp.state.uniform.content[0] = _sgp.state.color.r;
        _sgp.state.uniform.content[1] = _sgp.state.color.g;
        _sgp.state.uniform.content[2] = _sgp.state.color.b;
        _sgp.state.uniform.content[3] = _sgp.state.color.a;
    }
}

void sgp_reset_pipeline(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    sg_pipeline pip = {SG_INVALID_ID};
    sgp_set_pipeline(pip);
}

void sgp_set_uniform(const void* data, uint32_t size) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.state.pipeline.id != SG_INVALID_ID);
    SOKOL_ASSERT(size <= sizeof(float) * SGP_UNIFORM_CONTENT_SLOTS);
    if(size > 0) {
        SOKOL_ASSERT(data);
        memcpy(&_sgp.state.uniform.content, data, size);
    }
    if(size < _sgp.state.uniform.size) {
        // zero old uniform data
        memset((uint8_t*)(&_sgp.state.uniform) + size, 0, _sgp.state.uniform.size - size);
    }
    _sgp.state.uniform.size = size;
}

void sgp_reset_uniform(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.state.pipeline.id != SG_INVALID_ID);
    sgp_set_uniform(NULL, 0);
}

void sgp_set_blend_mode(sgp_blend_mode blend_mode) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    _sgp.state.blend_mode = blend_mode;
}

void sgp_reset_blend_mode(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    sgp_set_blend_mode(SGP_BLENDMODE_NONE);
}

void sgp_set_color(float r, float g, float b, float a) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_color color = {r,g,b,a};
    _sgp.state.color = color;

    // update uniform for the default pipeline
    if(_sgp.state.pipeline.id == SG_INVALID_ID) {
        memset(&_sgp.state.uniform, 0, sizeof(sgp_uniform));
        _sgp.state.uniform.size = sizeof(sgp_color);
        _sgp.state.uniform.content[0] = color.r;
        _sgp.state.uniform.content[1] = color.g;
        _sgp.state.uniform.content[2] = color.b;
        _sgp.state.uniform.content[3] = color.a;
    }
}

void sgp_reset_color(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
}

void sgp_set_image(int channel, sg_image image) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    SOKOL_ASSERT(channel >= 0 && channel < SGP_TEXTURE_SLOTS);
    if(_sgp.state.images.images[channel].id == image.id)
        return;

    _sgp.state.images.images[channel] = image;

    // recalculate images count
    int images_count = (int)_sgp.state.images.count;
    for(int i=_sg_max(channel, images_count-1);i>=0;--i) {
        if(_sgp.state.images.images[i].id != SG_INVALID_ID) {
            images_count = i + 1;
            break;
        }
    }
    _sgp.state.images.count = (uint32_t)images_count;
}

void sgp_unset_image(int channel) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    sg_image img = {SG_INVALID_ID};
    sgp_set_image(channel, img);
}

void sgp_reset_image(int channel) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    if(channel == 0) {
        // channel 0 always use white image
        sgp_set_image(channel, _sgp.white_img);
    } else {
        sg_image img = {SG_INVALID_ID};
        sgp_set_image(channel, img);
    }
}

static _sgp_vertex* _sgp_next_vertices(uint32_t count) {
    if(SOKOL_LIKELY(_sgp.cur_vertex + count <= _sgp.num_vertices)) {
        _sgp_vertex *vertices = &_sgp.vertices[_sgp.cur_vertex];
        _sgp.cur_vertex += count;
        return vertices;
    } else {
        _sgp_set_error(SGP_ERROR_VERTICES_FULL);
        return NULL;
    }
}

static sgp_uniform* _sgp_prev_uniform(void) {
    if(SOKOL_LIKELY(_sgp.cur_uniform > 0)) {
        return &_sgp.uniforms[_sgp.cur_uniform-1];
    } else {
        return NULL;
    }
}

static sgp_uniform* _sgp_next_uniform(void) {
    if(SOKOL_LIKELY(_sgp.cur_uniform < _sgp.num_uniforms)) {
        return &_sgp.uniforms[_sgp.cur_uniform++];
    } else {
        _sgp_set_error(SGP_ERROR_UNIFORMS_FULL);
        return NULL;
    }
}

static _sgp_command* _sgp_prev_command(uint32_t count) {
    if(SOKOL_LIKELY((_sgp.cur_command - _sgp.state._base_command) >= count)) {
        return &_sgp.commands[_sgp.cur_command-count];
    } else {
        return NULL;
    }
}

static _sgp_command* _sgp_next_command(void) {
    if(SOKOL_LIKELY(_sgp.cur_command < _sgp.num_commands)) {
        return &_sgp.commands[_sgp.cur_command++];
    } else {
        _sgp_set_error(SGP_ERROR_COMMANDS_FULL);
        return NULL;
    }
}

void sgp_viewport(int x, int y, int w, int h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // skip in case of the same viewport
    if(_sgp.state.viewport.x == x && _sgp.state.viewport.y == y &&
       _sgp.state.viewport.w == w && _sgp.state.viewport.h == h)
        return;

    // try to reuse last command otherwise use the next one
    _sgp_command* cmd = _sgp_prev_command(1);
    if(!cmd || cmd->cmd != SGP_COMMAND_VIEWPORT)
        cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) return;

    sgp_irect viewport = {x, y, w, h};

    memset(cmd, 0, sizeof(_sgp_command));
    cmd->cmd = SGP_COMMAND_VIEWPORT;
    cmd->args.viewport = viewport;

    // adjust current scissor relative offset
    if(!(_sgp.state.scissor.w < 0 && _sgp.state.scissor.h < 0)) {
        _sgp.state.scissor.x += x - _sgp.state.viewport.x;
        _sgp.state.scissor.y += y - _sgp.state.viewport.y;
    }

    _sgp.state.viewport = viewport;
    _sgp.state.proj = _sgp_default_proj(w, h);
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_reset_viewport(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_viewport(0, 0, _sgp.state.frame_size.w, _sgp.state.frame_size.h);
}

void sgp_scissor(int x, int y, int w, int h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // skip in case of the same scissor
    if(_sgp.state.scissor.x == x && _sgp.state.scissor.y == y &&
       _sgp.state.scissor.w == w && _sgp.state.scissor.h == h)
        return;

    // try to reuse last command otherwise use the next one
    _sgp_command* cmd = _sgp_prev_command(1);
    if(!cmd || cmd->cmd != SGP_COMMAND_SCISSOR)
        cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) return;

    // coordinate scissor in viewport subspace
    sgp_irect viewport_scissor = {_sgp.state.viewport.x + x, _sgp.state.viewport.y + y, w, h};

    // reset scissor
    if(w < 0 && h  < 0) {
        viewport_scissor.x = 0; viewport_scissor.y = 0;
        viewport_scissor.w = _sgp.state.frame_size.w; viewport_scissor.h = _sgp.state.frame_size.h;
    }

    memset(cmd, 0, sizeof(_sgp_command));
    cmd->cmd = SGP_COMMAND_SCISSOR;
    cmd->args.scissor = viewport_scissor;

    sgp_irect scissor = {x, y, w, h};
    _sgp.state.scissor = scissor;
}

void sgp_reset_scissor(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_scissor(0, 0, -1, -1);
}

void sgp_reset_state(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_reset_viewport();
    sgp_reset_scissor();
    sgp_reset_project();
    sgp_reset_transform();
    sgp_reset_blend_mode();
    sgp_reset_color();
    sgp_reset_uniform();
    sgp_reset_pipeline();
}

static inline bool _sgp_uniform_equals(sgp_uniform* a, sgp_uniform* b) {
    return memcmp(a, b, sizeof(sgp_uniform)) == 0;
}

static inline bool _sgp_region_overlaps(_sgp_region a, _sgp_region b) {
    return !(a.x2 <= b.x1 || b.x2 <= a.x1  || a.y2 <= b.y1 || b.y2 <= a.y1);
}

static bool _sgp_merge_batch_command(sg_pipeline pip, sgp_images_uniform images, sgp_uniform uniform, _sgp_region region, uint32_t vertex_index, uint32_t num_vertices) {
#if SGP_BATCH_OPTIMIZER_DEPTH > 0
    _sgp_command* prev_cmd = NULL;
    _sgp_command* inter_cmds[SGP_BATCH_OPTIMIZER_DEPTH];
    uint32_t inter_cmd_count = 0;

    // find a command that is a good candidate to batch
    uint32_t lookup_depth = SGP_BATCH_OPTIMIZER_DEPTH;
    for(uint32_t depth=0;depth<lookup_depth;++depth) {
        _sgp_command* cmd = _sgp_prev_command(depth+1);
        // stop on nonexistent command
        if(!cmd)
            break;

        // command was optimized away, search deeper
        if(cmd->cmd == SGP_COMMAND_NONE) {
            lookup_depth++;
            continue;
        }

        // stop on scissor/viewport
        if(cmd->cmd != SGP_COMMAND_DRAW)
            break;

        // can only batch commands with the same bindings and uniforms
        if(cmd->args.draw.pip.id == pip.id &&
            memcmp(&images, &cmd->args.draw.images, sizeof(sgp_images_uniform)) == 0 &&
            memcmp(&uniform, &_sgp.uniforms[cmd->args.draw.uniform_index], sizeof(sgp_uniform)) == 0) {
            prev_cmd = cmd;
            break;
        } else {
            inter_cmds[inter_cmd_count] = cmd;
            inter_cmd_count++;
        }
    }
    if(!prev_cmd)
        return false;

    // allow batching only if the region of the current or previous draw
    // is not touched by intermediate commands
    bool overlaps_next = false;
    bool overlaps_prev = false;
    _sgp_region prev_region = prev_cmd->args.draw.region;
    for(uint32_t i=0;i<inter_cmd_count;++i) {
        _sgp_region inter_region = inter_cmds[i]->args.draw.region;
        if(_sgp_region_overlaps(region, inter_region)) {
            overlaps_next = true;
            if(overlaps_prev) return false;
        }
        if(_sgp_region_overlaps(prev_region, inter_region)) {
            overlaps_prev = true;
            if(overlaps_next) return false;
        }
    }

    if(!overlaps_next) { // batch in the previous draw command
        if(inter_cmd_count > 0) {
            // not enough vertices space, can't do this batch
            if(SOKOL_UNLIKELY(_sgp.cur_vertex + num_vertices > _sgp.num_vertices))
                return false;

            // rearrange vertices memory for the batch
            uint32_t prev_end_vertex = prev_cmd->args.draw.vertex_index + prev_cmd->args.draw.num_vertices;
            uint32_t move_count = _sgp.cur_vertex - prev_end_vertex;
            memmove(&_sgp.vertices[prev_end_vertex + num_vertices], &_sgp.vertices[prev_end_vertex], move_count * sizeof(_sgp_vertex));
            memcpy(&_sgp.vertices[prev_end_vertex], &_sgp.vertices[vertex_index + num_vertices], num_vertices * sizeof(_sgp_vertex));

            // offset vertices of intermediate draw commands
            for(uint32_t i=0;i<inter_cmd_count;++i) {
                inter_cmds[i]->args.draw.vertex_index += num_vertices;
            }
        }

        // update draw region and vertices
        prev_region.x1 = _sg_min(prev_region.x1, region.x1);
        prev_region.y1 = _sg_min(prev_region.y1, region.y1);
        prev_region.x2 = _sg_max(prev_region.x2, region.x2);
        prev_region.y2 = _sg_max(prev_region.y2, region.y2);
        prev_cmd->args.draw.num_vertices += num_vertices;
        prev_cmd->args.draw.region = prev_region;
    } else { // batch in the next draw command
        SOKOL_ASSERT(inter_cmd_count > 0);

        // append new draw command
        _sgp_command* cmd = _sgp_next_command();
        if(SOKOL_UNLIKELY(!cmd))
            return false;

        uint32_t prev_num_vertices = prev_cmd->args.draw.num_vertices;
        // not enough vertices space, can't do this batch
        if(SOKOL_UNLIKELY(_sgp.cur_vertex + prev_num_vertices > _sgp.num_vertices))
            return false;

        // rearrange vertices memory for the batch
        memmove(&_sgp.vertices[vertex_index + prev_num_vertices], &_sgp.vertices[vertex_index], num_vertices * sizeof(_sgp_vertex));
        memcpy(&_sgp.vertices[vertex_index], &_sgp.vertices[prev_cmd->args.draw.vertex_index], prev_num_vertices * sizeof(_sgp_vertex));

        // update draw region and vertices
        prev_region.x1 = _sg_min(prev_region.x1, region.x1);
        prev_region.y1 = _sg_min(prev_region.y1, region.y1);
        prev_region.x2 = _sg_max(prev_region.x2, region.x2);
        prev_region.y2 = _sg_max(prev_region.y2, region.y2);
        _sgp.cur_vertex += prev_num_vertices;
        num_vertices += prev_num_vertices;

        // configure the draw command
        cmd->cmd = SGP_COMMAND_DRAW;
        cmd->args.draw.pip = pip;
        cmd->args.draw.images = images;
        cmd->args.draw.region = prev_region;
        cmd->args.draw.uniform_index = prev_cmd->args.draw.uniform_index;
        cmd->args.draw.vertex_index = vertex_index;
        cmd->args.draw.num_vertices = num_vertices;

        // force skipping the previous draw command
        prev_cmd->cmd = SGP_COMMAND_NONE;
    }
    return true;
#else
    _SOKOL_UNUSED(pip);
    _SOKOL_UNUSED(images);
    _SOKOL_UNUSED(uniform);
    _SOKOL_UNUSED(region);
    _SOKOL_UNUSED(vertex_index);
    _SOKOL_UNUSED(num_vertices);
    return false;
#endif // SGP_BATCH_OPTIMIZER_DEPTH > 0
}

static void _sgp_queue_draw(sg_pipeline pip, _sgp_region region, uint32_t vertex_index, uint32_t num_vertices) {
    // override pipeline
    if(_sgp.state.pipeline.id != SG_INVALID_ID)
        pip = _sgp.state.pipeline;

    // invalid pipeline
    if(pip.id == SG_INVALID_ID) {
        _sgp.cur_vertex -= num_vertices; // rollback allocated vertices
        return;
    }

    // region is out of screen bounds
    if(region.x1 > 1.0f || region.y1 > 1.0f || region.x2 < -1.0f || region.y2 < -1.0f) {
        _sgp.cur_vertex -= num_vertices; // rollback allocated vertices
        return;
    }

    // try to merge on previous command to draw in a batch
    if(_sgp_merge_batch_command(pip, _sgp.state.images, _sgp.state.uniform, region, vertex_index, num_vertices))
        return;

    // setup uniform, try to reuse previous uniform when possible
    sgp_uniform *prev_uniform = _sgp_prev_uniform();
    bool reuse_uniform = prev_uniform && (memcmp(prev_uniform, &_sgp.state.uniform, sizeof(sgp_uniform)) == 0);
    if(!reuse_uniform) {
        // append new uniform
        sgp_uniform *next_uniform = _sgp_next_uniform();
        if(SOKOL_UNLIKELY(!next_uniform)) {
            _sgp.cur_vertex -= num_vertices; // rollback allocated vertices
            return;
        }
        *next_uniform = _sgp.state.uniform;
    }
    uint32_t uniform_index = _sgp.cur_uniform - 1;

    // append new draw command
    _sgp_command* cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) {
        _sgp.cur_vertex -= num_vertices; // rollback allocated vertices
        return;
    }
    cmd->cmd = SGP_COMMAND_DRAW;
    cmd->args.draw.pip = pip;
    cmd->args.draw.images = _sgp.state.images;
    cmd->args.draw.region = region;
    cmd->args.draw.uniform_index = uniform_index;
    cmd->args.draw.vertex_index = vertex_index;
    cmd->args.draw.num_vertices = num_vertices;
}

static inline sgp_vec2 _sgp_mat3_vec2_mul(const sgp_mat2x3* m, const sgp_vec2* v) {
    sgp_vec2 u = {
        m->v[0][0]*v->x + m->v[0][1]*v->y + m->v[0][2],
        m->v[1][0]*v->x + m->v[1][1]*v->y + m->v[1][2]
    };
    return u;
}

static void _sgp_transform_vec2(sgp_mat2x3* matrix, sgp_vec2* dst, const sgp_vec2 *src, uint32_t count) {
    for(uint32_t i=0;i<count;++i) {
        dst[i] = _sgp_mat3_vec2_mul(matrix, &src[i]);
    }
}

static void _sgp_draw_solid_pip(sg_pipeline pip, const sgp_vec2* vertices, uint32_t num_vertices) {
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* transformed_vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    sgp_mat2x3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    _sgp_region region = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
    for(uint32_t i=0;i<num_vertices;++i) {
        sgp_vec2 p = _sgp_mat3_vec2_mul(&mvp, &vertices[i]);
        region.x1 = _sg_min(region.x1, p.x);
        region.y1 = _sg_min(region.y1, p.y);
        region.x2 = _sg_max(region.x2, p.x);
        region.y2 = _sg_max(region.y2, p.y);
        transformed_vertices[i].position = p;
        transformed_vertices[i].texcoord.x = 0.0f;
        transformed_vertices[i].texcoord.y = 0.0f;
    }
    _sgp_queue_draw(pip, region, vertex_index, num_vertices);
}

void sgp_clear(void) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // setup vertices
    uint32_t num_vertices = 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    _sgp_vertex* v = vertices;
    const sgp_vec2 quad[4] = {
        {-1.0f, -1.0f}, // bottom left
        { 1.0f, -1.0f}, // bottom right
        { 1.0f,  1.0f}, // top right
        {-1.0f,  1.0f}, // top left
    };
    const sgp_vec2 texcoord = {0.0f, 0.0f};

    // make a quad composed of 2 triangles
    v[0].position = quad[0]; v[0].texcoord = texcoord;
    v[1].position = quad[1]; v[1].texcoord = texcoord;
    v[2].position = quad[2]; v[2].texcoord = texcoord;
    v[3].position = quad[3]; v[3].texcoord = texcoord;
    v[4].position = quad[0]; v[4].texcoord = texcoord;
    v[5].position = quad[2]; v[5].texcoord = texcoord;

    _sgp_region region = {-1.0f, -1.0f, 1.0f, 1.0f};

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_NONE);
    _sgp_queue_draw(pip, region, vertex_index, num_vertices);
}

void sgp_draw_points(const sgp_point* points, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_POINTS, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, points, count);
}

void sgp_draw_point(float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_point point = {x, y};
    sgp_draw_points(&point, 1);
}

void sgp_draw_lines(const sgp_line* lines, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINES, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, (const sgp_point*)lines, count*2);
}

void sgp_draw_line(float ax, float ay, float bx, float by) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_line line = {{ax,ay},{bx, by}};
    sgp_draw_lines(&line, 1);
}

void sgp_draw_lines_strip(const sgp_point* points, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, points, count);
}

void sgp_draw_filled_triangles(const sgp_triangle* triangles, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, (const sgp_point*)triangles, count*3);
}

void sgp_draw_filled_triangle(float ax, float ay, float bx, float by, float cx, float cy) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_triangle triangle = {{ax,ay},{bx, by},{cx, cy}};
    sgp_draw_filled_triangles(&triangle, 1);
}

void sgp_draw_filled_triangles_strip(const sgp_point* points, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, points, count);
}

void sgp_draw_filled_rects(const sgp_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    _sgp_vertex* v = vertices;
    const sgp_rect* rect = rects;
    sgp_mat2x3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    _sgp_region region = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
    for(uint32_t i=0;i<count;v+=6, rect++, i++) {
        sgp_vec2 quad[4] = {
            {rect->x,           rect->y + rect->h}, // bottom left
            {rect->x + rect->w, rect->y + rect->h}, // bottom right
            {rect->x + rect->w, rect->y}, // top right
            {rect->x,  rect->y}, // top left
        };
        _sgp_transform_vec2(&mvp, quad, quad, 4);

        for(uint32_t j=0;j<4;++j) {
            region.x1 = _sg_min(region.x1, quad[j].x);
            region.y1 = _sg_min(region.y1, quad[j].y);
            region.x2 = _sg_max(region.x2, quad[j].x);
            region.y2 = _sg_max(region.y2, quad[j].y);
        }

        const sgp_vec2 vtexquad[4] = {
            {0.0f, 1.0f}, // bottom left
            {1.0f, 1.0f}, // bottom right
            {1.0f, 0.0f}, // top right
            {0.0f, 0.0f}, // top left
        };

        // make a quad composed of 2 triangles
        v[0].position = quad[0]; v[0].texcoord = vtexquad[0];
        v[1].position = quad[1]; v[1].texcoord = vtexquad[1];
        v[2].position = quad[2]; v[2].texcoord = vtexquad[2];
        v[3].position = quad[3]; v[3].texcoord = vtexquad[3];
        v[4].position = quad[0]; v[4].texcoord = vtexquad[0];
        v[5].position = quad[2]; v[5].texcoord = vtexquad[2];
    }

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_queue_draw(pip, region, vertex_index, num_vertices);
}

void sgp_draw_filled_rect(float x, float y, float w, float h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_rect rect = {x,y,w,h};
    sgp_draw_filled_rects(&rect, 1);
}

void sgp_draw_textured_rects(const sgp_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    sgp_mat2x3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    _sgp_region region = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
    for(uint32_t i=0;i<count;i++) {
        sgp_vec2 quad[4] = {
            {rects[i].x,              rects[i].y + rects[i].h}, // bottom left
            {rects[i].x + rects[i].w, rects[i].y + rects[i].h}, // bottom right
            {rects[i].x + rects[i].w, rects[i].y}, // top right
            {rects[i].x,  rects[i].y}, // top left
        };
        _sgp_transform_vec2(&mvp, quad, quad, 4);

        for(uint32_t j=0;j<4;++j) {
            region.x1 = _sg_min(region.x1, quad[j].x);
            region.y1 = _sg_min(region.y1, quad[j].y);
            region.x2 = _sg_max(region.x2, quad[j].x);
            region.y2 = _sg_max(region.y2, quad[j].y);
        }

        const sgp_vec2 vtexquad[4] = {
            {0.0f, 1.0f}, // bottom left
            {1.0f, 1.0f}, // bottom right
            {1.0f, 0.0f}, // top right
            {0.0f, 0.0f}, // top left
        };

        // make a quad composed of 2 triangles
        _sgp_vertex* v = &vertices[i*6];
        v[0].position = quad[0]; v[0].texcoord = vtexquad[0];
        v[1].position = quad[1]; v[1].texcoord = vtexquad[1];
        v[2].position = quad[2]; v[2].texcoord = vtexquad[2];
        v[3].position = quad[3]; v[3].texcoord = vtexquad[3];
        v[4].position = quad[0]; v[4].texcoord = vtexquad[0];
        v[5].position = quad[2]; v[5].texcoord = vtexquad[2];
    }

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_queue_draw(pip, region, vertex_index, num_vertices);
}

void sgp_draw_textured_rect(float x, float y, float w, float h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_rect rect = {x, y, w, h};
    sgp_draw_textured_rects(&rect, 1);
}

static sgp_isize _sgp_query_image_size(sg_image img_id) {
    const _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    SOKOL_ASSERT(img);
    sgp_isize size = {img ? img->cmn.width : 0, img ? img->cmn.height : 0};
    return size;
}

void sgp_draw_textured_rects_ex(int channel, const sgp_textured_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    SOKOL_ASSERT(channel >= 0 && channel < SGP_TEXTURE_SLOTS);
    sg_image image = _sgp.state.images.images[channel];
    if(SOKOL_UNLIKELY(count == 0 || image.id == SG_INVALID_ID)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute image values used for texture coords transform
    sgp_isize image_size = _sgp_query_image_size(image);
    if(SOKOL_UNLIKELY(image_size.w == 0 || image_size.h == 0)) return;
    float iw = 1.0f/(float)image_size.w, ih = 1.0f/(float)image_size.h;

    // compute vertices
    sgp_mat2x3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    _sgp_region region = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
    for(uint32_t i=0;i<count;i++) {
        sgp_vec2 quad[4] = {
            {rects[i].dst.x,                  rects[i].dst.y + rects[i].dst.h}, // bottom left
            {rects[i].dst.x + rects[i].dst.w, rects[i].dst.y + rects[i].dst.h}, // bottom right
            {rects[i].dst.x + rects[i].dst.w, rects[i].dst.y}, // top right
            {rects[i].dst.x,  rects[i].dst.y}, // top left
        };
        _sgp_transform_vec2(&mvp, quad, quad, 4);

        for(uint32_t j=0;j<4;++j) {
            region.x1 = _sg_min(region.x1, quad[j].x);
            region.y1 = _sg_min(region.y1, quad[j].y);
            region.x2 = _sg_max(region.x2, quad[j].x);
            region.y2 = _sg_max(region.y2, quad[j].y);
        }

        _sgp_vertex* v = &vertices[i*6];
        v[0].position = quad[0];
        v[1].position = quad[1];
        v[2].position = quad[2];
        v[3].position = quad[3];
        v[4].position = quad[0];
        v[5].position = quad[2];
    }

    // compute texture coords
    for(uint32_t i=0;i<count;i++) {
        // compute source rect
        float tl = rects[i].src.x*iw;
        float tt = rects[i].src.y*ih;
        float tr = (rects[i].src.x + rects[i].src.w)*iw;
        float tb = (rects[i].src.y + rects[i].src.h)*ih;
        sgp_vec2 vtexquad[4] = {
            {tl, tb}, // bottom left
            {tr, tb}, // bottom right
            {tr, tt}, // top right
            {tl, tt}, // top left
        };

        // make a quad composed of 2 triangles
        _sgp_vertex* v = &vertices[i*6];
        v[0].texcoord = vtexquad[0];
        v[1].texcoord = vtexquad[1];
        v[2].texcoord = vtexquad[2];
        v[3].texcoord = vtexquad[3];
        v[4].texcoord = vtexquad[0];
        v[5].texcoord = vtexquad[2];
    }

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_queue_draw(pip, region, vertex_index, num_vertices);
}

void sgp_draw_textured_rect_ex(int channel, sgp_rect dest_rect, sgp_rect src_rect) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_textured_rect rect = {dest_rect, src_rect};
    sgp_draw_textured_rects_ex(channel, &rect, 1);
}

sgp_desc sgp_query_desc(void) {
    return _sgp.desc;
}

sgp_state* sgp_query_state(void) {
    return &_sgp.state;
}

#endif // SOKOL_GP_IMPL_INCLUDED
#endif // SOKOL_GP_IMPL

/*
Copyright (c) 2020-2022 Eduardo Bart (https://github.com/edubart/sokol_gp)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
