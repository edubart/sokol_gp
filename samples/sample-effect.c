/*
This sample showcases how to create 2D shader effects using multiple textures.
*/

#define SGP_UNIFORM_CONTENT_SLOTS 16
#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_SIMD
#define STBI_ONLY_PNG
#include "stb_image.h"

#define SOKOL_SHDC_IMPL
#include "sample-effect.glsl.h"

static sg_pipeline pip;
static sg_shader shd;
static sg_image image;
static sg_sampler linear_sampler;
static sg_image perlin_image;

static void frame(void) {
    // begin draw commands queue
    int window_width = sapp_width(), window_height = sapp_height();
    sgp_begin(window_width, window_height);

    float secs = sapp_frame_count() * sapp_frame_duration();
    sg_image_desc image_desc = sg_query_image_desc(image);
    float window_ratio = window_width / (float)window_height;
    float image_ratio = image_desc.width / (float)image_desc.height;
    effect_uniforms_t uniforms;
    memset(&uniforms, 0, sizeof(effect_uniforms_t));
    uniforms.iVelocity.x = 0.02f;
    uniforms.iVelocity.y = 0.01f;
    uniforms.iPressure = 0.3f;
    uniforms.iTime = secs;
    uniforms.iWarpiness = 0.2f;
    uniforms.iRatio = image_ratio;
    uniforms.iZoom = 0.4f;
    uniforms.iLevel = 1.0f;
    sgp_set_pipeline(pip);
    sgp_set_uniform(&uniforms, sizeof(effect_uniforms_t));
    sgp_set_image(SLOT_effect_iTexChannel0, image);
    sgp_set_image(SLOT_effect_iTexChannel1, perlin_image);
    sgp_set_sampler(SLOT_effect_iTexChannel0, linear_sampler);
    sgp_set_sampler(SLOT_effect_iTexChannel1, linear_sampler);
    float width = (window_ratio >= image_ratio) ? window_width : image_ratio*window_height;
    float height = (window_ratio >= image_ratio) ? window_width/image_ratio : window_height;
    sgp_draw_filled_rect(0, 0, width, height);
    sgp_reset_image(SLOT_effect_iTexChannel0);
    sgp_reset_image(SLOT_effect_iTexChannel1);
    sgp_reset_sampler(SLOT_effect_iTexChannel1);
    sgp_reset_sampler(SLOT_effect_iTexChannel0);
    sgp_reset_pipeline();

    // dispatch draw commands
    sg_pass_action pass_action = {0};
    sg_begin_default_pass(&pass_action, window_width, window_height);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();
}

static sg_image load_image(const char *filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename, &width, &height, &channels, 4);
    sg_image img = {SG_INVALID_ID};
    if (!data) {
        return img;
    }
    sg_image_desc image_desc;
    memset(&image_desc, 0, sizeof(sg_image_desc));
    image_desc.width = width;
    image_desc.height = height;
    image_desc.data.subimage[0][0].ptr = data;
    image_desc.data.subimage[0][0].size = (size_t)(width * height * 4);
    img = sg_make_image(&image_desc);
    stbi_image_free(data);
    return img;
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

    // load image
    image = load_image("images/lpc_winter_preview.png");
    perlin_image = load_image("images/perlin.png");
    if (sg_query_image_state(image) != SG_RESOURCESTATE_VALID || sg_query_image_state(perlin_image) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to load images");
        exit(-1);
    }

    // create linear sampler
    sg_sampler_desc linear_sampler_desc = {
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
    };
    linear_sampler = sg_make_sampler(&linear_sampler_desc);
    if (sg_query_sampler_state(linear_sampler) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to create linear sampler");
        exit(-1);
    }

    // initialize shader
    shd = sg_make_shader(effect_program_shader_desc(sg_query_backend()));
    if (sg_query_shader_state(shd) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to make custom pipeline shader\n");
        exit(-1);
    }
    sgp_pipeline_desc pip_desc;
    memset(&pip_desc, 0, sizeof(sgp_pipeline_desc));
    pip_desc.shader = shd;
    pip_desc.has_vs_color = true;
    pip = sgp_make_pipeline(&pip_desc);
    if (sg_query_pipeline_state(pip) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to make custom pipeline\n");
        exit(-1);
    }
}

static void cleanup(void) {
    sg_destroy_image(image);
    sg_destroy_image(perlin_image);
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
