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
* Batch draw calls even when using different color states. **\*NEW\***
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
#include "sokol_log.h"

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
    sg_desc sgdesc = {
        .context = sapp_sgcontext(),
        .logger.func = slog_func
    };
    sg_setup(&sgdesc);
    if (!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol GFX context!\n");
        exit(-1);
    }

    // Initialize Sokol GP, adjust the size of command buffers for your own use.
    sgp_desc sgpdesc = {0};
    sgp_setup(&sgpdesc);
    if (!sgp_is_valid()) {
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
        .window_title = "Rectangle (Sokol GP)",
        .logger.func = slog_func,
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

## Drawing textured primitives

To draw textured rectangles you can use `sgp_set_image(0, img)` and then `sgp_draw_filled_rect()`,
this will draw an entire texture into a rectangle.
You should later reset the image with `sgp_reset_image(0)` to restore the bound image to default white image,
otherwise you will have glitches when drawing a solid color.

In case you want to draw a specific source from the texture,
you should use `sgp_draw_textured_rect()` instead.

By default textures are drawn using a simple nearest filter sampler,
you can change the sampler with `sgp_set_sampler(0, smp)` before drawing a texture,
it's recommended to restore the default sampler using `sgp_reset_sampler(0)`.

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

## Cheat sheet

Here is a quick list of all library functions for quick reference:

```c
/* Initialization and de-initialization. */
void sgp_setup(const sgp_desc* desc);                 /* Initializes the SGP context, and should be called after `sg_setup`. */
void sgp_shutdown(void);                              /* Destroys the SGP context. */
bool sgp_is_valid(void);                              /* Checks if SGP context is valid, should be checked after `sgp_setup`. */

/* Error handling. */
sgp_error sgp_get_last_error(void);                   /* Returns last SGP error. */
const char* sgp_get_error_message(sgp_error error);   /* Returns a message with SGP error description. */

/* Custom pipeline creation. */
sg_pipeline sgp_make_pipeline(const sgp_pipeline_desc* desc); /* Creates a custom shader pipeline to be used with SGP. */

/* Draw command queue management. */
void sgp_begin(int width, int height);    /* Begins a new SGP draw command queue. */
void sgp_flush(void);                     /* Dispatch current Sokol GFX draw commands. */
void sgp_end(void);                       /* End current draw command queue, discarding it. */

/* 2D coordinate space projection */
void sgp_project(float left, float right, float top, float bottom); /* Set the coordinate space boundary in the current viewport. */
void sgp_reset_project(void);                                       /* Resets the coordinate space to default (coordinate of the viewport). */

/* 2D coordinate space transformation. */
void sgp_push_transform(void);                            /* Saves current transform matrix, to be restored later with a pop. */
void sgp_pop_transform(void);                             /* Restore transform matrix to the same value of the last push. */
void sgp_reset_transform(void);                           /* Resets the transform matrix to identity (no transform). */
void sgp_translate(float x, float y);                     /* Translates the 2D coordinate space. */
void sgp_rotate(float theta);                             /* Rotates the 2D coordinate space around the origin. */
void sgp_rotate_at(float theta, float x, float y);        /* Rotates the 2D coordinate space around a point. */
void sgp_scale(float sx, float sy);                       /* Scales the 2D coordinate space around the origin. */
void sgp_scale_at(float sx, float sy, float x, float y);  /* Scales the 2D coordinate space around a point. */

/* State change for custom pipelines. */
void sgp_set_pipeline(sg_pipeline pipeline);              /* Sets current draw pipeline. */
void sgp_reset_pipeline(void);                            /* Resets to the current draw pipeline to default (builtin pipelines). */
void sgp_set_uniform(const void* data, uint32_t size);    /* Sets uniform buffer for a custom pipeline. */
void sgp_reset_uniform(void);                             /* Resets uniform buffer to default (current state color). */

/* State change functions for the common pipelines. */
void sgp_set_blend_mode(sgp_blend_mode blend_mode);       /* Sets current blend mode. */
void sgp_reset_blend_mode(void);                          /* Resets current blend mode to default (no blending). */
void sgp_set_color(float r, float g, float b, float a);   /* Sets current color modulation. */
void sgp_reset_color(void);                               /* Resets current color modulation to default (white). */
void sgp_set_image(int channel, sg_image image);          /* Sets current bound image in a texture channel. */
void sgp_unset_image(int channel);                        /* Remove current bound image in a texture channel (no texture). */
void sgp_reset_image(int channel);                        /* Resets current bound image in a texture channel to default (white texture). */
void sgp_set_sampler(int channel, sg_sampler sampler);    /* Sets current bound sampler in a texture channel. */
void sgp_unset_sampler(int channel);                      /* Remove current bound sampler in a texture channel (no sampler). */
void sgp_reset_sampler(int channel);                      /* Resets current bound sampler in a texture channel to default (nearest sampler). */

/* State change functions for all pipelines. */
void sgp_viewport(int x, int y, int w, int h);            /* Sets the screen area to draw into. */
void sgp_reset_viewport(void);                            /* Reset viewport to default values (0, 0, width, height). */
void sgp_scissor(int x, int y, int w, int h);             /* Set clip rectangle in the viewport. */
void sgp_reset_scissor(void);                             /* Resets clip rectangle to default (viewport bounds). */
void sgp_reset_state(void);                               /* Reset all state to default values. */

/* Drawing functions. */
void sgp_clear(void);                                                                         /* Clears the current viewport using the current state color. */
void sgp_draw(sg_primitive_type primitive_type, const sgp_vertex* vertices, uint32_t count);  /* Low level drawing function, capable of drawing any primitive. */
void sgp_draw_points(const sgp_point* points, uint32_t count);                                /* Draws points in a batch. */
void sgp_draw_point(float x, float y);                                                        /* Draws a single point. */
void sgp_draw_lines(const sgp_line* lines, uint32_t count);                                   /* Draws lines in a batch. */
void sgp_draw_line(float ax, float ay, float bx, float by);                                   /* Draws a single line. */
void sgp_draw_lines_strip(const sgp_point* points, uint32_t count);                           /* Draws a strip of lines. */
void sgp_draw_filled_triangles(const sgp_triangle* triangles, uint32_t count);                /* Draws triangles in a batch. */
void sgp_draw_filled_triangle(float ax, float ay, float bx, float by, float cx, float cy);    /* Draws a single triangle. */
void sgp_draw_filled_triangles_strip(const sgp_point* points, uint32_t count);                /* Draws strip of triangles. */
void sgp_draw_filled_rects(const sgp_rect* rects, uint32_t count);                            /* Draws a batch of rectangles. */
void sgp_draw_filled_rect(float x, float y, float w, float h);                                /* Draws a single rectangle. */
void sgp_draw_textured_rects(int channel, const sgp_textured_rect* rects, uint32_t count);    /* Draws a batch textured rectangle, each from a source region. */
void sgp_draw_textured_rect(int channel, sgp_rect dest_rect, sgp_rect src_rect);              /* Draws a single textured rectangle from a source region. */

/* Querying functions. */
sgp_state* sgp_query_state(void); /* Returns the current draw state. */
sgp_desc sgp_query_desc(void);    /* Returns description of the current SGP context. */
```

## Robustness

This library has been tested since 2020 in private projects,
and has proved to be stable.

## Sponsorships

This library has been originally sponsored by the MMORPG game [Medivia Online](https://medivia.online/),
I would like to thank them for supporting my work.

Thanks @kkukshtel for sponsoring batching draw calls with different colors feature.

## Related projects

Make sure to checkout the excellent [Sokol](https://github.com/floooh/sokol) project by @floooh,
it features many useful single header C libraries made with quality that can be used for game development.

You may also want to check my other single header C library
[Minicoro](https://github.com/edubart/minicoro),
it brings stackful coroutines for C,
very useful for simplifying finite state machines in game devlopment.

## Updates

* **18-Jan-2024**: Fix shader leaking when creating custom SGP pipelines.
* **07-Jan-2024**: Make possible to set custom depth format and sample count in pipelines, batch draw calls even when using different colors, introduce low level primitive drawing function, format source files.
* **31-Oct-2023**: Update to latest Sokol, introduced new WebGPU backend.
* **30-Sep-2023**: Update to latest Sokol, deprecated GLES2 backend, add image sampler APIs, changes in draw textured APIs.
* **31-Dec-2021**: The library was open sourced.
* **05-Aug-2020**: Added support for custom shaders.
* **03-Jun-2020**: Added the batch optimizer.
* **29-May-2020**: The library was created.

## Screenshots

Here are some screenshots of all samples in `samples` directory.
Click on any of the images to view it in your browser.

Primitives sample:

[![sample-primitives](https://raw.githubusercontent.com/edubart/sokol_gp/master/screenshots/sample-primitives.png)](https://edubart.github.io/sokol_gp/sample-primitives.html)

Blending modes samples:

[![sample-blend](https://raw.githubusercontent.com/edubart/sokol_gp/master/screenshots/sample-blend.png)](https://edubart.github.io/sokol_gp/sample-blend.html)

Frame buffer sample:

[![sample-framebuffer](https://raw.githubusercontent.com/edubart/sokol_gp/master/screenshots/sample-framebuffer.png)](https://edubart.github.io/sokol_gp/sample-framebuffer.html)

Rectangle sample:

[![sample-rectangle](https://raw.githubusercontent.com/edubart/sokol_gp/master/screenshots/sample-rectangle.png)](https://edubart.github.io/sokol_gp/sample-rectangle.html)

Effect sample:

[![sample-effect](https://raw.githubusercontent.com/edubart/sokol_gp/master/screenshots/sample-effect.gif)](https://edubart.github.io/sokol_gp/sample-effect.html)

SDF sample:

[![sample-sdf](https://raw.githubusercontent.com/edubart/sokol_gp/master/screenshots/sample-sdf.gif)](https://edubart.github.io/sokol_gp/sample-sdf.html)

## Donation

I'm a full-time open source developer.
Any amount of the donation will be appreciated and could bring me encouragement to keep supporting this and other open source projects.

[![Become a Patron](https://c5.patreon.com/external/logo/become_a_patron_button.png)](https://www.patreon.com/edubart)
