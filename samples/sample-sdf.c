#define SGP_UNIFORM_CONTENT_SLOTS 4

#include "sample_app.h"
#include "sample-sdf.glsl.h"

sg_pipeline pip;

void draw() {
    sgp_set_pipeline(pip);
    sdf_uniform_t uniform = {
        .iResolution = {app.width, app.height},
        .iTime = SDL_GetTicks() / 1000.0f
    };
    sgp_set_uniform(&uniform, sizeof(sdf_uniform_t));
    sgp_draw_filled_rect(0, 0, app.width, app.height);
    sgp_reset_pipeline();
}

bool init() {
    sgp_pipeline_desc pip_desc = {
        .fs = sdf_shader_desc(SG_BACKEND_GLCORE33)->fs
    };
    pip = sgp_make_pipeline(&pip_desc);
    if(pip.id == SG_INVALID_ID) {
        fprintf(stderr, "failed to make custom pipeline %s\n", sgp_get_error());
        return false;
    }
    return true;
}

void terminate() {
    sg_destroy_pipeline(pip);
}

int main(int argc, char *argv[]) {
    return sample_app_main(&(sample_app_desc){
        .width=512, .height=512,
        .init = init,
        .terminate = terminate,
        .draw = draw,
        .argc = argc,
        .argv = argv
    });
}
