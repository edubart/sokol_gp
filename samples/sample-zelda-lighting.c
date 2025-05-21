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
#define LIGHT_SIZE 350.0f     // Even bigger lights

static sg_pipeline pip_blend;    // Pipeline for sprites and background
static sg_pipeline pip_mask;     // Pipeline for lights (mask-based approach)
static sg_shader shd;
static sg_image background_image;
static sg_image link_image;
static sg_image lightmap_image;
static sg_sampler linear_sampler;
static sg_image light_buffer;    // Offscreen buffer to accumulate lights
static sg_image depth_buffer;    // Depth buffer for proper depth testing
static sg_attachments light_pass_attachments;  // Attachments for rendering to light buffer

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

static light_source_t lights[] = {
    {50, 50, 1.0f, 1.0f, 0.8f, 1.2f, LIGHT_SIZE},  // Warm white light
    {500, 400, 1.0f, 0.3f, 0.2f, 1.0f, LIGHT_SIZE},  // Red-orange light
    {200, 200, 0.2f, 0.4f, 1.0f, 1.0f, LIGHT_SIZE},  // Blue light
};
static const int num_lights = sizeof(lights)/sizeof(lights[0]);

// Create a 1x1 white image that will be our default texture
static sg_image create_white_texture(void) {
    uint32_t white_pixel = 0xFFFFFFFF;
    sg_image_desc desc = {
        .width = 1,
        .height = 1,
        .data.subimage[0][0].ptr = &white_pixel,
        .data.subimage[0][0].size = sizeof(white_pixel)
    };
    return sg_make_image(&desc);
}

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
    float secs = sapp_frame_count() * sapp_frame_duration();
    
    // 1. First render all lights to an offscreen buffer
    // ---------------------
    // Begin rendering to the light buffer using our attachments
    sg_pass_action pass_action = {
        .colors[0] = { 
            .load_action = SG_LOADACTION_CLEAR, 
            .clear_value = {0.0f, 0.0f, 0.0f, 0.0f} 
        },
        .depth = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = 1.0f
        }
    };
    sg_pass pass = {
        .action = pass_action,
        .attachments = light_pass_attachments
    };
    sg_begin_pass(&pass);
    sgp_begin(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    fs_uniforms_t uniforms = {
        .ambient_light = 0.05f,     // Very dark ambient for more contrast
        .light_intensity = 1.0f,
        .time = secs,
        .is_light = 1.0f           // Set to light pass mode
    };
    
    // Draw each light to the light buffer with ADDITIVE blending
    // This accumulates all light contributions
    sgp_set_pipeline(pip_mask);
    
    for (int i = 0; i < num_lights; i++) {
        // Animate light positions slightly
        lights[i].x += sinf(secs * 1.0f + i * 1.0f) * 0.5f;
        lights[i].y += cosf(secs * 0.8f + i * 2.0f) * 0.5f;
        
        // Update uniforms for this light
        uniforms.light_r = lights[i].r;
        uniforms.light_g = lights[i].g;
        uniforms.light_b = lights[i].b;
        uniforms.light_intensity = lights[i].intensity;
        sgp_set_uniform(NULL, 0, &uniforms, sizeof(fs_uniforms_t));
        
        // Use lightmap image for the light shape
        sgp_set_image(0, lightmap_image);
        sgp_set_sampler(0, linear_sampler);
        
        sgp_draw_filled_rect(
            lights[i].x - lights[i].size/2,
            lights[i].y - lights[i].size/2,
            lights[i].size,
            lights[i].size
        );
    }
    
    // End light buffer rendering
    sgp_flush();
    sgp_end();
    sg_end_pass();
    
    // 2. Now render the scene with the light buffer as a mask
    // ---------------------
    // Define action for main pass
    sg_pass_action main_pass_action = {
        .colors[0] = { 
            .load_action = SG_LOADACTION_CLEAR, 
            .clear_value = {0.0f, 0.0f, 0.0f, 1.0f} 
        },
        .depth = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = 1.0f
        }
    };
    
    // Begin main pass
    sg_pass main_pass = {
        .action = main_pass_action,
        .swapchain = sglue_swapchain()
    };
    sg_begin_pass(&main_pass);
    
    // Begin sokol_gp
    sgp_begin(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Reset uniforms for scene rendering
    uniforms.is_light = 0.0f;
    
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
    
    // Draw background with lighting
    sgp_set_pipeline(pip_blend);
    sgp_set_image(0, background_image);
    sgp_set_image(1, light_buffer);    // Use our accumulated light buffer as mask
    sgp_set_sampler(0, linear_sampler);
    sgp_set_sampler(1, linear_sampler);
    sgp_set_uniform(NULL, 0, &uniforms, sizeof(fs_uniforms_t));
    sgp_draw_filled_rect(x_offset, y_offset, scale_w, scale_h);
    
    // Draw player sprite
    sgp_set_image(0, link_image);
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
    
    // End scene rendering
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

    // Initialize Sokol GP with default settings
    sgp_desc sgpdesc = {0};
    sgpdesc.depth_format = sapp_depth_format();  // Get depth format from sokol_app
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
    
    // Create offscreen render target for light accumulation
    sg_image_desc light_buffer_desc = {
        .width = WINDOW_WIDTH,
        .height = WINDOW_HEIGHT,
        .render_target = true,
        .sample_count = 1
    };
    light_buffer = sg_make_image(&light_buffer_desc);
    if (sg_query_image_state(light_buffer) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create light buffer\n");
        exit(-1);
    }

    // Create depth buffer image
    sg_image_desc depth_buffer_desc = {
        .width = WINDOW_WIDTH,
        .height = WINDOW_HEIGHT,
        .pixel_format = sapp_depth_format(),
        .render_target = true,
        .sample_count = 1
    };
    depth_buffer = sg_make_image(&depth_buffer_desc);
    if (sg_query_image_state(depth_buffer) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create depth buffer\n");
        exit(-1);
    }
    
    // Create attachments for light pass - now with depth attachment
    sg_attachments_desc light_pass_desc = {
        .colors[0].image = light_buffer,
        .depth_stencil.image = depth_buffer,
        .label = "light-pass"
    };
    light_pass_attachments = sg_make_attachments(&light_pass_desc);
    if (sg_query_attachments_state(light_pass_attachments) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create light pass attachments\n");
        exit(-1);
    }
    
    // Initialize shader
    shd = sg_make_shader(zelda_lighting_program_shader_desc(sg_query_backend()));
    if (sg_query_shader_state(shd) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create shader\n");
        exit(-1);
    }

    // Create pipelines using direct Sokol GFX configuration for proper depth testing
    
    // Pipeline for the main scene rendering
    sg_pipeline_desc pip_blend_desc = {0};
    pip_blend_desc.shader = shd;
    pip_blend_desc.layout.buffers[0].stride = sizeof(sgp_vertex);
    pip_blend_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT4;
    pip_blend_desc.layout.attrs[0].offset = offsetof(sgp_vertex, position);
    pip_blend_desc.layout.attrs[1].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_blend_desc.layout.attrs[1].offset = offsetof(sgp_vertex, color);
    pip_blend_desc.depth.pixel_format = sapp_depth_format();
    pip_blend_desc.depth.write_enabled = true;
    pip_blend_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    pip_blend_desc.colors[0].pixel_format = sapp_color_format();
    pip_blend_desc.colors[0].blend.enabled = true;
    pip_blend_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    pip_blend_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pip_blend_desc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
    pip_blend_desc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
    pip_blend_desc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pip_blend_desc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;
    pip_blend_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    pip_blend_desc.sample_count = sapp_sample_count();
    pip_blend = sg_make_pipeline(&pip_blend_desc);
    if (sg_query_pipeline_state(pip_blend) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "Failed to create blend pipeline\n");
        exit(-1);
    }

    // Pipeline for light mask rendering
    sg_pipeline_desc pip_mask_desc = {0};
    pip_mask_desc.shader = shd;
    pip_mask_desc.layout.buffers[0].stride = sizeof(sgp_vertex);
    pip_mask_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT4;
    pip_mask_desc.layout.attrs[0].offset = offsetof(sgp_vertex, position);
    pip_mask_desc.layout.attrs[1].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_mask_desc.layout.attrs[1].offset = offsetof(sgp_vertex, color);
    pip_mask_desc.depth.pixel_format = sapp_depth_format();
    pip_mask_desc.depth.write_enabled = true;
    pip_mask_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    pip_mask_desc.colors[0].pixel_format = sapp_color_format();
    pip_mask_desc.colors[0].blend.enabled = true;
    pip_mask_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    pip_mask_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE;
    pip_mask_desc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
    pip_mask_desc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ZERO;
    pip_mask_desc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ONE;
    pip_mask_desc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;
    pip_mask_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    pip_mask_desc.sample_count = sapp_sample_count();
    pip_mask = sg_make_pipeline(&pip_mask_desc);
}

static void cleanup(void) {
    sg_destroy_image(background_image);
    sg_destroy_image(link_image);
    sg_destroy_image(lightmap_image);
    sg_destroy_image(light_buffer);
    sg_destroy_image(depth_buffer);
    sg_destroy_attachments(light_pass_attachments);
    sg_destroy_pipeline(pip_blend);
    sg_destroy_pipeline(pip_mask);
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