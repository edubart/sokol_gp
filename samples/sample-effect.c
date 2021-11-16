#define SGP_UNIFORM_CONTENT_SLOTS 16

#include "sample_app.h"
#define SOKOL_SHDC_IMPL
#include "sample-effect.glsl.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_SIMD
#define STBI_ONLY_PNG
#include "stb_image.h"

static sg_pipeline pip;
static sg_image image;
static sg_image perlin_image;

static void draw(void) {
    float secs = SDL_GetTicks() / 1000.0f;
    sg_image_info info = sg_query_image_info(image);
    float image_ratio = info.width / (float) info.height;
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
    sgp_set_image(SLOT_effect_iChannel0, image);
    sgp_set_image(SLOT_effect_iChannel1, perlin_image);
    sgp_draw_textured_rect(0, 0, app.width, app.width/image_ratio);
    sgp_reset_image(SLOT_effect_iChannel0);
    sgp_reset_image(SLOT_effect_iChannel1);
    sgp_reset_pipeline();
}

static sg_image load_image(const char *filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename, &width, &height, &channels, 4);
    sg_image img = {SG_INVALID_ID};
    if(!data)
        return img;
    sg_image_desc image_desc;
    memset(&image_desc, 0, sizeof(sg_image_desc));
    image_desc.width = width;
    image_desc.height = height;
    image_desc.min_filter = SG_FILTER_LINEAR;
    image_desc.mag_filter = SG_FILTER_LINEAR;
    image_desc.wrap_u = SG_WRAP_REPEAT;
    image_desc.wrap_v = SG_WRAP_REPEAT;
    image_desc.data.subimage[0][0].ptr = data;
    image_desc.data.subimage[0][0].size = (size_t)(width * height * 4);
    img = sg_make_image(&image_desc);
    stbi_image_free(data);
    return img;
}

static bool init(void) {
    image = load_image("images/2d_magic_lands.png");
    perlin_image = load_image("images/perlin.png");
    if(sg_query_image_state(image) != SG_RESOURCESTATE_VALID || sg_query_image_state(perlin_image) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to load images");
        return false;
    }
    sgp_pipeline_desc pip_desc;
    memset(&pip_desc, 0, sizeof(sgp_pipeline_desc));
    pip_desc.shader = *effect_program_shader_desc(sg_query_backend());

    pip = sgp_make_pipeline(&pip_desc);
    if(sg_query_pipeline_state(pip) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to make custom pipeline: %s\n", sgp_get_error());
        return false;
    }
    return true;
}

static void terminate(void) {
    sg_destroy_image(image);
    sg_destroy_image(perlin_image);
    sg_destroy_pipeline(pip);
}

int main(int argc, char *argv[]) {
    sample_app_desc app_desc;
    memset(&app_desc, 0, sizeof(app_desc));
    app_desc.init = init;
    app_desc.terminate = terminate;
    app_desc.draw = draw;
    app_desc.argc = argc;
    app_desc.argv = argv;
    return sample_app_main(&app_desc);
}
