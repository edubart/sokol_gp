#include "sample_app.h"

static sg_image fb_image;
static sg_pass fb_pass;

static void draw_triangles(void) {
    const float PI = 3.14159265358979323846264338327f;
    static sgp_vec2 points_buffer[4096];

    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float w = height*0.3f;
    unsigned int count = 0;
    float step = (2.0f*PI)/6.0f;
    for(float theta = 0.0f; theta <= 2.0f*PI + step*0.5f; theta+=step) {
        sgp_vec2 v = {hw*1.33f + w*cosf(theta), hh*1.33f - w*sinf(theta)};
        points_buffer[count++] = v;
        if(count % 3 == 1) {
            sgp_vec2 u = {hw, hh};
            points_buffer[count++] = u;
        }
    }
    sgp_set_color(1.0f, 0.0f, 1.0f, 1.0f);
    sgp_draw_filled_triangle_strip(points_buffer, count);
}

static void draw_fbo(void) {
    sgp_begin(128, 128);
    sgp_ortho(0, 128, 128, 0);
    draw_triangles();

    sg_pass_action pass_action;
    memset(&pass_action, 0, sizeof(sg_pass_action));
    pass_action.colors[0].action = SG_ACTION_CLEAR;
    pass_action.colors[0].value.r = 1.0f;
    pass_action.colors[0].value.g = 1.0f;
    pass_action.colors[0].value.b = 1.0f;
    pass_action.colors[0].value.a = 0.2f;
    pass_action.depth.action = SG_ACTION_DONTCARE;
    pass_action.stencil.action = SG_ACTION_DONTCARE;
    sg_begin_pass(fb_pass, &pass_action);
    sgp_flush();
    sgp_end();
    sg_end_pass();
}

static void draw(void) {
    int height = app.height, width = app.width;
    float time = SDL_GetTicks() / 1000.0f;
    sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
    draw_fbo();
    for(int y=0;y<height;y+=192) {
        for(int x=0;x<width;x+=192) {
            sgp_push_transform();
            sgp_rotate_at(time, x+64, y+64);
            sgp_set_image(0, fb_image);
            sgp_draw_textured_rect(x, y, 128, 128);
            sgp_reset_image(0);
            sgp_pop_transform();
        }
    }
}

static bool init(void) {
    sg_image_desc fb_image_desc;
    memset(&fb_image_desc, 0, sizeof(sg_image_desc));
    fb_image_desc.render_target = true;
    fb_image_desc.width = 128;
    fb_image_desc.height = 128;
    fb_image_desc.min_filter = SG_FILTER_LINEAR;
    fb_image_desc.mag_filter = SG_FILTER_LINEAR;
    fb_image_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    fb_image_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    fb_image = sg_make_image(&fb_image_desc);
    if(sg_query_image_state(fb_image) != SG_RESOURCESTATE_VALID)
        return false;
    sg_pass_desc pass_desc;
    memset(&pass_desc, 0, sizeof(sg_pass_desc));
    pass_desc.color_attachments[0].image = fb_image;
    fb_pass = sg_make_pass(&pass_desc);
    if(sg_query_pass_state(fb_pass) != SG_RESOURCESTATE_VALID)
        return false;
    return true;
}

static void terminate(void) {
    sg_destroy_image(fb_image);
    sg_destroy_pass(fb_pass);
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
