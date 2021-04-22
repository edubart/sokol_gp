#define SGP_UNIFORM_CONTENT_SLOTS 16

#include "sample_app.h"
#define SOKOL_SHDC_IMPL
#include "sample-effect.glsl.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STBI_ONLY_PNG
#include "stb_image.h"

sg_pipeline pip;
sg_image image;
sg_image perlin_image;

void draw() {
    sgp_set_pipeline(pip);
    float secs = SDL_GetTicks() / 1000.0f;
    sg_image_info info = sg_query_image_info(image);
    float image_ratio = info.width / (float) info.height;
    effect_uniform_t uniform = {
        .iPressure = 0.3f,
        .iVelocity = {0.02f, 0.01f},
        .iRatio = image_ratio,
        .iTime = secs,
        .iWarpiness = 0.2f,
        .iZoom = 0.4f,
        .iLevel = 1.0f,
    };
    sgp_set_uniform(&uniform, sizeof(effect_uniform_t));
    sgp_set_image(SLOT_iChannel0, image);
    sgp_set_image(SLOT_iChannel1, perlin_image);
    sgp_draw_textured_rect(0, 0, app.width, app.width/image_ratio);
    sgp_reset_image(1);
    sgp_reset_image(0);
    sgp_reset_pipeline();
}

sg_image load_image(const char *filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename, &width, &height, &channels, 4);
    if(!data)
        return (sg_image){.id=0};
    sg_image_desc image_desc = {
        .width = width,
        .height = height,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .data = {.subimage = {{{.ptr = data, .size = width * height * 4}}}},
    };
    sg_image image = sg_make_image(&image_desc);
    stbi_image_free(data);
    return image;
}

bool init() {
    image = load_image("images/2d_magic_lands.png");
    perlin_image = load_image("images/perlin.png");
    if(sg_query_image_state(image) != SG_RESOURCESTATE_VALID || sg_query_image_state(perlin_image) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to load images");
        return false;
    }
    sgp_pipeline_desc pip_desc = {
        .fs = effect_shader_desc(sg_query_backend())->fs
    };
    pip = sgp_make_pipeline(&pip_desc);
    if(sg_query_pipeline_state(pip) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to make custom pipeline: %s\n", sgp_get_error());
        return false;
    }
    return true;
}

void terminate() {
    sg_destroy_image(image);
    sg_destroy_image(perlin_image);
    sg_destroy_pipeline(pip);
}

int main(int argc, char *argv[]) {
    return sample_app_main(&(sample_app_desc){
        .init = init,
        .terminate = terminate,
        .draw = draw,
        .width=512, .height=512,
        .argc = argc,
        .argv = argv
    });
}
