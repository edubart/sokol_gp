#include "sample_app.h"

sg_image fb_image;
sg_pass fb_pass;

void draw_triangles() {
    const float PI = 3.14159265358979323846264338327f;
    static sgp_vec2 points_buffer[4096];

    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float w = height*0.3f;
    int count = 0;
    float step = (2.0f*PI)/6.0f;
    for(float theta = 0.0f; theta <= 2.0f*PI + step*0.5f; theta+=step) {
        points_buffer[count++] = (sgp_vec2){hw*1.33f + w*cosf(theta), hh*1.33f - w*sinf(theta)};
        if(count % 3 == 1)
            points_buffer[count++] = (sgp_vec2){hw, hh};
    }
    sgp_set_color(1.0f, 0.0f, 1.0f, 1.0f);
    sgp_draw_filled_triangle_strip(points_buffer, count);
}

void draw_fbo() {
    sgp_begin(128, 128);
    sgp_ortho(0, 128, 128, 0);
    draw_triangles();

    sg_pass_action pass_action = {
        .colors = {{.action = SG_ACTION_CLEAR, .value = {1.0f, 1.0f, 1.0f, 0.2f}}},
        .depth = {.action = SG_ACTION_DONTCARE},
        .stencil = {.action = SG_ACTION_DONTCARE},
    };
    sg_begin_pass(fb_pass, &pass_action);
    sgp_flush();
    sgp_end();
    sg_end_pass();
}

void draw() {
    int height = app.height, width = app.width;
    float time = SDL_GetTicks() / 1000.0f;
    sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
    draw_fbo();
    for(int y=0;y<height;y+=192) {
        for(int x=0;x<width;x+=192) {
            sgp_push_transform();
            sgp_rotate_at(time, x+64, y+64);
            sgp_draw_textured_rect(fb_image, x, y, 128, 128);
            sgp_pop_transform();
        }
    }
}

bool init() {
    sg_image_desc fb_image_desc = {
        .render_target = true,
        .width = 128,
        .height = 128,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
    fb_image = sg_make_image(&fb_image_desc);
    if(fb_image.id == SG_INVALID_ID)
        return false;
    sg_pass_desc pass_desc = {
        .color_attachments = {{.image = fb_image}},
    };
    fb_pass = sg_make_pass(&pass_desc);
    if(fb_pass.id == SG_INVALID_ID)
        return false;
    return true;
}

void terminate() {
    sg_destroy_image(fb_image);
    sg_destroy_pass(fb_pass);
}

int main(int argc, char *argv[]) {
    return sample_app_main(&(sample_app_desc){
        .init = init,
        .terminate = terminate,
        .draw = draw,
        .argc = argc,
        .argv = argv
    });
}
