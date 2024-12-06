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

static sg_image fb_color_image;
static sg_image fb_resolve_image;
static sg_image fb_depth_image;
static sg_attachments fb_attachments;
static sg_sampler linear_sampler;

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

    sg_pass_action pass_action = {0};
    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass_action.colors[0].store_action = SG_STOREACTION_DONTCARE;
    pass_action.colors[0].clear_value.r = 1.0f;
    pass_action.colors[0].clear_value.g = 1.0f;
    pass_action.colors[0].clear_value.b = 1.0f;
    pass_action.colors[0].clear_value.a = 0.2f;
    sg_pass pass = {
        .action = pass_action,
        .attachments = fb_attachments
    };
    sg_begin_pass(&pass);
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
            sgp_set_image(0, fb_resolve_image);
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
    sg_pass pass = {.swapchain = sglue_swapchain()};
    sg_begin_pass(&pass);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();
}

static void init(void) {
    // initialize Sokol GFX
    sg_desc sgdesc = {
        .environment = sglue_environment(),
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

    // create frame buffer color image (multi-sampled)
    sg_image_desc fb_color_image_desc = {
        .render_target = true,
        .width = 128,
        .height = 128,
        .sample_count = sapp_sample_count(),
    };
    fb_color_image = sg_make_image(&fb_color_image_desc);
    if (sg_query_image_state(fb_color_image) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create frame buffer image\n");
        exit(-1);
    }

    // create frame buffer resolve image
    sg_image_desc fb_resolve_image_desc = {
        .render_target = true,
        .width = 128,
        .height = 128,
        .sample_count = 1,
    };
    fb_resolve_image = sg_make_image(&fb_resolve_image_desc);

    // create frame buffer depth stencil
    sg_image_desc fb_depth_image_desc = {
        .render_target = true,
        .width = 128,
        .height = 128,
        .pixel_format = sapp_depth_format(),
        .sample_count = sapp_sample_count(),
    };
    fb_depth_image = sg_make_image(&fb_depth_image_desc);
    if (sg_query_image_state(fb_depth_image) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create frame buffer depth image\n");
        exit(-1);
    }

    // create frame buffer attachments
    sg_attachments_desc fb_attachments_desc = {
        .colors = {
            {.image = fb_color_image}
        },
        .resolves = {
            {.image = fb_resolve_image}
        },
        .depth_stencil = {
            .image = fb_depth_image
        }
    };
    fb_attachments = sg_make_attachments(&fb_attachments_desc);
    if (sg_query_attachments_state(fb_attachments) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create frame buffer attachments\n");
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
}

static void cleanup(void) {
    sg_destroy_attachments(fb_attachments);
    sg_destroy_image(fb_color_image);
    sg_destroy_image(fb_resolve_image);
    sg_destroy_image(fb_depth_image);
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
        .sample_count = 4,
    };
}
