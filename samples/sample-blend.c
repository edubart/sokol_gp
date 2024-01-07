/*
This sample tests all blending modes that Sokol GP provides.
*/

#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define SOKOL_SHDC_IMPL
#include "sample-sdf.glsl.h"

#include <stdio.h>
#include <stdlib.h>

static void draw_3rects(float brightness, float alpha) {
    sgp_set_color(brightness, 0.0f, 0.0f, alpha);
    sgp_draw_filled_rect(0, 0, 10, 10);
    sgp_set_color(0.0f, brightness, 0.0f, alpha);
    sgp_draw_filled_rect(0, 5, 10, 10);
    sgp_set_color(0.0f, 0.0f, brightness, alpha);
    sgp_draw_filled_rect(5, 2.5f, 10, 10);
}

static void draw_rects(float ratio) {
    sgp_project(0, 60*ratio, 0, 60);
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);

    // none
    sgp_set_blend_mode(SGP_BLENDMODE_NONE);
    sgp_push_transform();
    sgp_translate(10,10);
    draw_3rects(1.0f, 1.0f);
    sgp_pop_transform();

    // blend
    sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
    sgp_push_transform();
    sgp_translate(30,10);
    draw_3rects(1.0f, 0.5f);
    sgp_pop_transform();

    // add
    sgp_set_blend_mode(SGP_BLENDMODE_ADD);
    sgp_push_transform();
    sgp_translate(50,10);
    draw_3rects(1.0f, 1.0f);
    sgp_pop_transform();

    // mod
    sgp_set_blend_mode(SGP_BLENDMODE_MOD);
    sgp_push_transform();
    sgp_translate(10,30);
    draw_3rects(1.0f, 1.0f);
    sgp_pop_transform();

    // mod
    sgp_set_blend_mode(SGP_BLENDMODE_MUL);
    sgp_push_transform();
    sgp_translate(30,30);
    draw_3rects(1.0f, 1.0f);
    sgp_pop_transform();
}

static void draw_checkboard(int width, int height) {
    sgp_set_color(0.2f, 0.2f, 0.2f, 1.0f);
    sgp_clear();
    sgp_set_color(0.4f, 0.4f, 0.4f, 1.0f);
    for (int y=0;y<height/32+1;y++) {
        for (int x=0;x<width/32+1;x++) {
            if ((x+y) % 2 == 0) {
                sgp_draw_filled_rect(x*32,y*32,32,32);
            }
        }
    }
    sgp_reset_color();

}

static void frame(void) {
    // begin draw commands queue
    int width = sapp_width(), height = sapp_height();
    sgp_begin(width, height);

    draw_checkboard(width, height);
    draw_rects(width/(float)height);

    // dispatch draw commands
    sg_pass_action pass_action = {0};
    sg_begin_default_pass(&pass_action, width, height);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();
}

static void init(void) {
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
    sgp_desc sgpdesc = {0};
    sgp_setup(&sgpdesc);
    if (!sgp_is_valid()) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error_message(sgp_get_last_error()));
        exit(-1);
    }
}

static void cleanup(void) {
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
        .window_title = "Blend (Sokol GP)",
        .logger.func = slog_func,
    };
}
