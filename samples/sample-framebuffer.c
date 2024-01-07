/*
This sample showcases how to use Sokol GP to draw inside frame buffers (render targets).
*/

#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include <stdio.h>
#include <stdlib.h>

static sg_image fb_image;
static sg_image fb_depth_image;
static sg_sampler linear_sampler;
static sg_pass fb_pass;

static void draw_triangles(void) {
    const float PI = 3.14159265f;
    static sgp_vec2 points_buffer[4096];

    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float w = height*0.3f;
    unsigned int count = 0;
    float step = (2.0f*PI)/6.0f;
    for (float theta = 0.0f; theta <= 2.0f*PI + step*0.5f; theta+=step) {
        sgp_vec2 v = {hw*1.33f + w*cosf(theta), hh*1.33f - w*sinf(theta)};
        points_buffer[count++] = v;
        if (count % 3 == 1) {
            sgp_vec2 u = {hw, hh};
            points_buffer[count++] = u;
        }
    }
    sgp_set_color(1.0f, 0.0f, 1.0f, 1.0f);
    sgp_draw_filled_triangles_strip(points_buffer, count);
}

static void draw_fbo(void) {
    sgp_begin(128, 128);
    sgp_project(0, 128, 128, 0);
    draw_triangles();

    sg_pass_action pass_action;
    memset(&pass_action, 0, sizeof(sg_pass_action));
    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass_action.colors[0].clear_value.r = 1.0f;
    pass_action.colors[0].clear_value.g = 1.0f;
    pass_action.colors[0].clear_value.b = 1.0f;
    pass_action.colors[0].clear_value.a = 0.2f;
    sg_begin_pass(fb_pass, &pass_action);
    sgp_flush();
    sgp_end();
    sg_end_pass();
}

static void frame(void) {
    // begin draw commands queue
    int width = sapp_width(), height = sapp_height();
    sgp_begin(width, height);

    // draw background
    sgp_set_color(0.05f, 0.05f, 0.05f, 1.0f);
    sgp_clear();
    sgp_reset_color();

    float time = sapp_frame_count() / 60.0f;
    sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
    draw_fbo();
    int i = 0;
    for (int y=0;y<height;y+=192) {
        for (int x=0;x<width;x+=192) {
            sgp_push_transform();
            sgp_rotate_at(time, x+64, y+64);
            sgp_set_image(0, fb_image);
            sgp_set_sampler(0, linear_sampler);
            if (i % 2 == 0) {
                sgp_draw_filled_rect(x, y, 128, 128);
            } else {
                sgp_rect dest_rect = {x, y, 128, 128};
                sgp_rect src_rect = {0, 0, 128, 128};
                sgp_draw_textured_rect(0, dest_rect, src_rect);
            }
            sgp_reset_image(0);
            sgp_reset_sampler(0);
            sgp_pop_transform();
            i++;
        }
    }

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

    // create frame buffer image
    sg_image_desc fb_image_desc;
    memset(&fb_image_desc, 0, sizeof(sg_image_desc));
    fb_image_desc.render_target = true;
    fb_image_desc.width = 128;
    fb_image_desc.height = 128;
    fb_image = sg_make_image(&fb_image_desc);
    if (sg_query_image_state(fb_image) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create frame buffer image\n");
        exit(-1);
    }

    // create frame buffer depth stencil
    sg_image_desc fb_depth_image_desc;
    memset(&fb_depth_image_desc, 0, sizeof(sg_image_desc));
    fb_depth_image_desc.render_target = true;
    fb_depth_image_desc.width = 128;
    fb_depth_image_desc.height = 128;
    fb_depth_image_desc.pixel_format = sapp_depth_format();
    fb_depth_image = sg_make_image(&fb_depth_image_desc);
    if (sg_query_image_state(fb_depth_image) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create frame buffer depth image\n");
        exit(-1);
    }

    // create linear sampler
    sg_sampler_desc linear_sampler_desc = {
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
    linear_sampler = sg_make_sampler(&linear_sampler_desc);
    if (sg_query_sampler_state(linear_sampler) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to create linear sampler");
        exit(-1);
    }

    // create frame buffer pass
    sg_pass_desc pass_desc;
    memset(&pass_desc, 0, sizeof(sg_pass_desc));
    pass_desc.color_attachments[0].image = fb_image;
    pass_desc.depth_stencil_attachment.image = fb_depth_image;
    fb_pass = sg_make_pass(&pass_desc);
    if (sg_query_pass_state(fb_pass) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create frame buffer pass\n");
        exit(-1);
    }
}

static void cleanup(void) {
    sg_destroy_image(fb_image);
    sg_destroy_image(fb_depth_image);
    sg_destroy_pass(fb_pass);
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
        .window_title = "Frame buffer (Sokol GP)",
        .logger.func = slog_func,
    };
}
