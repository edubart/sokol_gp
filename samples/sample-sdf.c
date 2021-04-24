#define SGP_UNIFORM_CONTENT_SLOTS 4

#include "sample_app.h"
#define SOKOL_SHDC_IMPL
#include "sample-sdf.glsl.h"

sg_pipeline pip;

void draw() {
    sgp_set_pipeline(pip);
    sdf_uniforms_t uniform = {
        .iResolution = {app.width, app.height},
        .iTime = SDL_GetTicks() / 1000.0f
    };
    sgp_set_uniform(&uniform, sizeof(sdf_uniforms_t));
    sgp_unset_image(0);
    sgp_draw_filled_rect(0, 0, app.width, app.height);
    sgp_reset_image(0);
    sgp_reset_pipeline();
}

bool init() {
    sgp_pipeline_desc pip_desc = {
        .shader = *sdf_program_shader_desc(sg_query_backend())
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
