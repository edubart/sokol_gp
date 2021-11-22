#define SGP_UNIFORM_CONTENT_SLOTS 4

#include "sample_app.h"
#define SOKOL_SHDC_IMPL
#include "sample-sdf.glsl.h"

static sg_pipeline pip;

static void draw(void) {
    sgp_set_pipeline(pip);
    sdf_uniforms_t uniform;
    memset(&uniform, 0, sizeof(sdf_uniforms_t));
    uniform.iResolution.x = app.width;
    uniform.iResolution.y = app.height;
    uniform.iTime = SDL_GetTicks() / 1000.0f;
    sgp_set_uniform(&uniform, sizeof(sdf_uniforms_t));
    sgp_unset_image(0);
    sgp_draw_filled_rect(0, 0, app.width, app.height);
    sgp_reset_image(0);
    sgp_reset_pipeline();
}

static bool init(void) {
    sgp_pipeline_desc pip_desc;
    memset(&pip_desc, 0, sizeof(sgp_pipeline_desc));
    pip_desc.shader = *sdf_program_shader_desc(sg_query_backend());
    pip = sgp_make_pipeline(&pip_desc);
    if(sg_query_pipeline_state(pip) != SG_RESOURCESTATE_VALID) {
        fprintf(stderr, "failed to make custom pipeline\n");
        return false;
    }
    return true;
}

static void terminate(void) {
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
