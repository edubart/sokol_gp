/*
This examples shows how to create 2D shaders,
in this case the shader is a SDF (signed distance field) animation.
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

static sg_pipeline pip;
static sg_shader shd;

static void frame(void) {
    // begin draw commands queue
    int width = sapp_width(), height = sapp_height();
    sgp_begin(width, height);

    // draw using the custom shader pipeline
    sgp_set_pipeline(pip);
    sdf_vs_uniforms_t vs_uniform = {0};
    vs_uniform.iResolution.x = width;
    vs_uniform.iResolution.y = height;
    sdf_fs_uniforms_t fs_uniform = {0};
    fs_uniform.iTime = sapp_frame_count() / 60.0f;
    sgp_set_uniform(&vs_uniform, sizeof(sdf_vs_uniforms_t), &fs_uniform, sizeof(sdf_fs_uniforms_t));
    sgp_unset_image(0);
    sgp_draw_filled_rect(0, 0, width, height);
    sgp_reset_image(0);
    sgp_reset_pipeline();

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

    // initialize shader
    shd = sg_make_shader(sdf_program_shader_desc(sg_query_backend()));
    if (sg_query_shader_state(shd) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to make custom pipeline shader\n");
        exit(-1);
    }
    sgp_pipeline_desc pip_desc = {0};
    pip_desc.shader = shd;
    pip_desc.has_vs_color = true;
    pip = sgp_make_pipeline(&pip_desc);
    if (sg_query_pipeline_state(pip) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to make custom pipeline\n");
        exit(-1);
    }
}

static void cleanup(void) {
    sg_destroy_pipeline(pip);
    sg_destroy_shader(shd);
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
        .window_title = "SDF (Sokol GP)",
        .logger.func = slog_func,
    };
}
