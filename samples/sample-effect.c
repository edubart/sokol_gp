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
float image_ratio = 1.0f;

void draw() {
    sgp_set_pipeline(pip);
    float secs = SDL_GetTicks() / 1000.0f;
    effect_uniform_t uniform = {
        .iResolution = {app.width, app.height},
        .iTime = secs,
        .iLevel = sinf(secs)*0.5f + 0.5f,
    };
    sgp_set_uniform(&uniform, sizeof(effect_uniform_t));
    sgp_draw_textured_rect(image, 0, 0, app.width, app.width/image_ratio);
    sgp_reset_pipeline();
}

sg_image load_image(const char *filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename, &width, &height, &channels, 4);
    if(!data)
        return (sg_image){.id=0};
    image_ratio = width / (float) height;
    sg_image_desc image_desc = {
        .width = width,
        .height = height,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
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
    if(sg_query_image_state(image) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to load image!");
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
