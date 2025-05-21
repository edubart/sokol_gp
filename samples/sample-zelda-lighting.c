#define SGP_UNIFORM_CONTENT_SLOTS 16
#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_SIMD
#define STBI_ONLY_PNG
#include "stb_image.h"

#define SOKOL_SHDC_IMPL
#include "sample-zelda-lighting.glsl.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define LIGHT_SIZE 150.0f

static sg_pipeline pip_blend;    // Pipeline for sprites (alpha blending)
static sg_pipeline pip_add;      // Pipeline for lights (additive blending)
static sg_shader shd;
static sg_image background_image;
static sg_image link_image;
static sg_image lightmap_image;
static sg_sampler linear_sampler;

// Structure to match shader uniforms
typedef struct {
    float ambient_light;
    float light_intensity;
    float time;
    float is_light;          // Whether this is a light pass
    float light_r;           // Light color components
    float light_g;
    float light_b;
    float _pad;             // For alignment
} fs_uniforms_t;

// Player sprite position and dimensions
static struct {
    float x, y;          // Position
    float w, h;          // Dimensions
    float scale;         // Scale factor
} player = {
    .x = WINDOW_WIDTH/2,  // Center X position
    .y = WINDOW_HEIGHT/2, // Center Y position
    .w = 112,            // Original sprite width
    .h = 112,            // Original sprite height
    .scale = 1.0f        // No additional scaling needed
};

// Light source definition
typedef struct {
    float x, y;          // Position
    float r, g, b;       // Color
    float intensity;     // Light intensity
    float size;         // Light radius
} light_source_t;

#define MAX_LIGHTS 8
static light_source_t lights[] = {
    {300, 300, 1.0f, 1.0f, 0.8f, 1.0f, LIGHT_SIZE},  // Warm white light
    {500, 400, 1.0f, 0.3f, 0.2f, 1.0f, LIGHT_SIZE},  // Red-orange light
    {200, 200, 0.2f, 0.4f, 1.0f, 1.0f, LIGHT_SIZE},  // Blue light
};
static const int num_lights = sizeof(lights)/sizeof(lights[0]);

static sg_image load_image(const char *filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename, &width, &height, &channels, 4);
    sg_image img = {SG_INVALID_ID};
    if (!data) {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        return img;
    }
    sg_image_desc image_desc = {0};
    image_desc.width = width;
    image_desc.height = height;
    image_desc.data.subimage[0][0].ptr = data;
    image_desc.data.subimage[0][0].size = (size_t)(width * height * 4);
    img = sg_make_image(&image_desc);
    stbi_image_free(data);
    return img;
}

static void frame(void) {
    sgp_begin(WINDOW_WIDTH, WINDOW_HEIGHT);

    float secs = sapp_frame_count() * sapp_frame_duration();
    fs_uniforms_t uniforms = {
        .ambient_light = 0.05f,     // Very dark ambient for more contrast
        .light_intensity = 1.0f,    // Base light intensity
        .time = secs,
        .is_light = 0.0f           // Start with regular texture mode
    };

    // Set up constant bindings
    sgp_set_image(0, background_image);
    sgp_set_image(1, lightmap_image);
    sgp_set_sampler(0, linear_sampler);
    sgp_set_sampler(1, linear_sampler);

    // Calculate background scaling to maintain aspect ratio and fill screen
    float bg_aspect = 870.0f/674.0f;
    float screen_aspect = (float)WINDOW_WIDTH/WINDOW_HEIGHT;
    float scale_w = WINDOW_WIDTH;
    float scale_h = WINDOW_WIDTH/bg_aspect;
    if (scale_h < WINDOW_HEIGHT) {
        scale_h = WINDOW_HEIGHT;
        scale_w = WINDOW_HEIGHT * bg_aspect;
    }
    float x_offset = (WINDOW_WIDTH - scale_w) * 0.5f;
    float y_offset = (WINDOW_HEIGHT - scale_h) * 0.5f;
    
    // Draw background with alpha blend
    sgp_set_pipeline(pip_blend);
    sgp_set_uniform(NULL, 0, &uniforms, sizeof(fs_uniforms_t));
    sgp_draw_filled_rect(x_offset, y_offset, scale_w, scale_h);

    // Draw lights with additive blend
    sgp_set_pipeline(pip_add);
    uniforms.is_light = 1.0f;  // Switch to light mode
    for (int i = 0; i < num_lights; i++) {
        // Animate light positions slightly
        lights[i].x += sinf(secs * 2.0f + i * 1.0f) * 0.5f;
        lights[i].y += cosf(secs * 1.5f + i * 2.0f) * 0.5f;
        
        // Update uniforms for this light
        uniforms.light_r = lights[i].r;
        uniforms.light_g = lights[i].g;
        uniforms.light_b = lights[i].b;
        uniforms.light_intensity = lights[i].intensity;
        sgp_set_uniform(NULL, 0, &uniforms, sizeof(fs_uniforms_t));
        
        sgp_draw_filled_rect(
            lights[i].x - lights[i].size/2,
            lights[i].y - lights[i].size/2,
            lights[i].size,
            lights[i].size
        );
    }

    // Draw player sprite with alpha blend
    sgp_set_pipeline(pip_blend);
    uniforms.is_light = 0.0f;  // Back to regular texture mode
    sgp_set_image(0, link_image);  // Only change the main texture
    sgp_set_uniform(NULL, 0, &uniforms, sizeof(fs_uniforms_t));
    
    sgp_draw_filled_rect(
        player.x - player.w/2,
        player.y - player.h/2, 
        player.w,
        player.h
    );

    // Reset state
    sgp_reset_image(0);
    sgp_reset_image(1);
    sgp_reset_sampler(0);
    sgp_reset_sampler(1);
    sgp_reset_pipeline();

    // Dispatch draw commands
    sg_pass pass = {.swapchain = sglue_swapchain()};
    sg_begin_pass(&pass);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();
}

static void init(void) {
    // Initialize Sokol GFX
    sg_desc sgdesc = {
        .environment = sglue_environment(),
        .logger.func = slog_func
    };
    sg_setup(&sgdesc);
    if (!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol GFX context!\n");
        exit(-1);
    }

    // Initialize Sokol GP
    sgp_desc sgpdesc = {0};
    sgp_setup(&sgpdesc);
    if (!sgp_is_valid()) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error_message(sgp_get_last_error()));
        exit(-1);
    }

    // Load images
    background_image = load_image("images/background.png");
    link_image = load_image("images/link.png");
    lightmap_image = load_image("images/simple_white_alpha.png");
    
    if (sg_query_image_state(background_image) != SG_RESOURCESTATE_VALID ||
        sg_query_image_state(link_image) != SG_RESOURCESTATE_VALID ||
        sg_query_image_state(lightmap_image) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to load one or more images\n");
        exit(-1);
    }

    // Create linear sampler
    sg_sampler_desc linear_sampler_desc = {
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
    linear_sampler = sg_make_sampler(&linear_sampler_desc);
    if (sg_query_sampler_state(linear_sampler) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create linear sampler\n");
        exit(-1);
    }

    // Initialize shader and pipelines
    shd = sg_make_shader(zelda_lighting_program_shader_desc(sg_query_backend()));
    if (sg_query_shader_state(shd) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create shader\n");
        exit(-1);
    }

    // Create alpha blend pipeline for sprites
    sgp_pipeline_desc pip_blend_desc = {0};
    pip_blend_desc.shader = shd;
    pip_blend_desc.has_vs_color = true;
    pip_blend_desc.blend_mode = SGP_BLENDMODE_BLEND;
    pip_blend = sgp_make_pipeline(&pip_blend_desc);
    if (sg_query_pipeline_state(pip_blend) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create blend pipeline\n");
        exit(-1);
    }

    // Create additive blend pipeline for lights
    sgp_pipeline_desc pip_add_desc = {0};
    pip_add_desc.shader = shd;
    pip_add_desc.has_vs_color = true;
    pip_add_desc.blend_mode = SGP_BLENDMODE_ADD;
    pip_add = sgp_make_pipeline(&pip_add_desc);
    if (sg_query_pipeline_state(pip_add) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create additive pipeline\n");
        exit(-1);
    }
}

static void cleanup(void) {
    sg_destroy_image(background_image);
    sg_destroy_image(link_image);
    sg_destroy_image(lightmap_image);
    sg_destroy_pipeline(pip_blend);
    sg_destroy_pipeline(pip_add);
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
        .window_title = "Zelda Lighting (Sokol GP)",
        .width = WINDOW_WIDTH,
        .height = WINDOW_HEIGHT,
        .logger.func = slog_func,
    };
} 