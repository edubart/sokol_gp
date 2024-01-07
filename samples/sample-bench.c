/*
This sample is used to benchmark, it intentionally draws lots of primitives
with interleaved textures or pipelines, to measure then gains of the batch optimizer.

In my latest tests on my computer (edubart) with batch optimizer active
there is a 2.2x in FPS gains.
*/

// Uncomment this to disable the batch optimizer.
// #define SGP_BATCH_OPTIMIZER_DEPTH 0

#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_time.h"
#include "sokol_log.h"

#include <stdio.h>
#include <stdlib.h>

static sg_image image1;
static sg_image image2;
static float image_ratio;

static const int count = 48;
static const int rect_count = 4;

static void bench_repeated_textured(void) {
    sgp_reset_color();
    sgp_set_image(0, image1);
    for (int y=0;y<count;++y) {
        for (int x=0;x<count;++x) {
            sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
    sgp_reset_image(0);
}

static void bench_multiple_textured(void) {
    sgp_reset_color();
    for (int y=0;y<count;++y) {
        for (int x=0;x<count;++x) {
            sgp_set_image(0, x % 2 == 0 ? image1 : image2);
            sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
    sgp_reset_image(0);
}

static void bench_colored_textured(void) {
    sgp_reset_color();
    sgp_set_image(0, image1);
    for (int y=0;y<count;++y) {
        for (int x=0;x<count;++x) {
            if (x % 3 == 0) {
                sgp_set_color(1.0f, 0, 0, 1.0f);
            } else if (x % 3 == 1) {
                sgp_set_color(0, 1.0f, 0, 1.0f);
            } else {
                sgp_set_color(0, 0, 1.0f, 1.0f);
            }
            sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
    sgp_reset_image(0);
}

static void bench_repeated_filled(void) {
    sgp_reset_color();
    for (int y=0;y<count;++y) {
        for (int x=0;x<count;++x) {
            sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

static void bench_colored_filled(void) {
    sgp_reset_color();
    for (int y=0;y<count;++y) {
        for (int x=0;x<count;++x) {
            if (x % 3 == 0) {
                sgp_set_color(1.0f, 0, 0, 1.0f);
            } else if (x % 3 == 1) {
                sgp_set_color(0, 1.0f, 0, 1.0f);
            } else {
                sgp_set_color(0, 0, 1.0f, 1.0f);
            }
            sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
        }
    }
}

static void bench_mixed(void) {
    for (int diagonal = 0; diagonal < 2*count - 1; ++diagonal) {
        int advance = _sg_max(diagonal - count + 1, 0);
        for (int y = diagonal - advance, x = advance; y >= 0 && x < count; --y, ++x) {
            if (x % 3 == 0) {
                sgp_set_color(1.0f, 0, 0, 1.0f);
            } else if (x % 3 == 1) {
                sgp_set_color(0, 1.0f, 0, 1.0f);
            } else {
                sgp_set_color(0, 0, 1.0f, 1.0f);
            }
            if ((x+y) % 2 == 0) {
                sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
            } else {
                sgp_set_image(0, image1);
                sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
                sgp_reset_image(0);
            }
        }
    }
}

static void bench_sync_mixed(void) {
    sgp_set_image(0, image1);
    sgp_reset_color();
    for (int y=0;y<count;++y) {
        for (int x=0;x<count;++x) {
            if ((x+y) % 2 == 0) {
                sgp_set_color(1.0f, 0, 0, 1.0f);
                sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
            } else {
                sgp_set_color(0, 1.0f, 0, 1.0f);
                sgp_draw_filled_rect(x*rect_count*2, y*rect_count*2, rect_count, rect_count);
            }
        }
    }
    sgp_reset_image(0);
}

static void draw_cat(void) {
    sgp_reset_color();
    sgp_set_image(0, image1);
    sgp_draw_filled_rect(0, 0, rect_count*count*2, rect_count*count*2);
    sgp_reset_image(0);
}

static void draw_rect(void) {
    sgp_reset_color();
    sgp_draw_filled_rect(0, 0, rect_count*count*2, rect_count*count*2);
}

static void frame(void) {
    // begin draw commands queue
    int width = sapp_width(), height = sapp_height();
    sgp_begin(width, height);

    // draw background
    sgp_set_color(0.05f, 0.05f, 0.05f, 1.0f);
    sgp_clear();
    sgp_reset_color();

    int off = count*rect_count*2;
    bench_repeated_textured();

    sgp_translate(off, 0);
    bench_multiple_textured();

    sgp_translate(off, 0);
    bench_colored_textured();

    sgp_translate(-2*off, off);
    bench_repeated_filled();

    sgp_translate(off, 0);
    bench_mixed();

    sgp_translate(off, 0);
    bench_colored_filled();

    sgp_translate(-2*off, off);
    draw_cat();

    sgp_translate(off, 0);
    draw_rect();

    sgp_translate(off, 0);
    bench_sync_mixed();

    // dispatch draw commands
    sg_pass_action pass_action = {0};
    sg_begin_default_pass(&pass_action, width, height);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();


    // count frames
    static int fps = 0;
    static uint64_t last = 0;
    uint64_t now = stm_now();
    fps++;
    if (stm_sec(now - last) >= 1.0) {
        printf("FPS: %d\n", fps);
        last = now;
        fps = 0;
    }
}

static sg_image create_image(int width, int height) {
    size_t num_pixels = (size_t)(width * height * 4);
    unsigned char* data = (unsigned char*)malloc(num_pixels);
    assert(data);
    for (int y=0;y<height;++y) {
        for (int x=0;x<width;++x) {
            data[y*width*4+x*4+0] = (x*255) / width;
            data[y*width*4+x*4+1] = (y*255) / height;
            data[y*width*4+x*4+2] = 255 - (x*y*255)/ (width*height);
            data[y*width*4+x*4+3] = 255;
        }
    }
    sg_image_desc image_desc;
    memset(&image_desc, 0, sizeof(sg_image_desc));
    image_desc.width = width;
    image_desc.height = height;
    image_desc.data.subimage[0][0].ptr = data;
    image_desc.data.subimage[0][0].size = num_pixels;
    sg_image image = sg_make_image(&image_desc);
    free(data);
    assert(sg_query_image_state(image) == SG_RESOURCESTATE_VALID);
    return image;
}

static void init(void) {
    stm_setup();

    // initialize Sokol GFX
    sg_desc sgdesc = {
        .context = sapp_sgcontext(),
        .logger.func = slog_func
    };
    sg_setup(&sgdesc);
    if (!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol GFX context!\n");
        exit(-1);
    }

    // initialize Sokol GP
    sgp_desc sgpdesc = {
        .max_vertices = 262144,
        .max_commands = 32768
    };
    sgp_setup(&sgpdesc);
    if (!sgp_is_valid()) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error_message(sgp_get_last_error()));
        exit(-1);
    }

#if defined(_SAPP_LINUX) && defined(SOKOL_GLCORE33)
    /* Disable swap interval */
    _sapp_glx_swapinterval(0);
#endif

    image1 = create_image(128, 128);
    image2 = create_image(128, 128);
    image_ratio = 1.0f;
}

static void cleanup(void) {
    sg_destroy_image(image1);
    sg_destroy_image(image2);
    sgp_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .window_title = "Bench (Sokol GP)",
        .width = 1280,
        .height = 1280,
        .logger.func = slog_func,
    };
}
