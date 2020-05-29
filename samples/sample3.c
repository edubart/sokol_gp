#include "sample_common.h"

sg_image fbo_image;
sg_pass fbo_pass;

static ngp_vec2 points_buffer[4096];

void draw_triangles() {
    ngp_irect viewport = ngp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float w = height*0.3f;
    int count = 0;
    float step = (2.0f*M_PI)/6.0f;
    for(float theta = 0.0f; theta <= 2.0f*M_PI + step*0.5f; theta+=step) {
        points_buffer[count++] = (ngp_vec2){hw + w*cosf(theta), hh - w*sinf(theta)};
        if(count % 3 == 1)
            points_buffer[count++] = (ngp_vec2){hw, hh};
    }
    ngp_set_color(1.0f, 0.0f, 1.0f, 1.0f);
    ngp_draw_triangle_strip(points_buffer, count);
}

void draw_fbo() {
    ngp_begin(128, 128);
    draw_triangles();

    sg_begin_pass(fbo_pass, &(sg_pass_action){
        .stencil.action = SG_ACTION_DONTCARE,
        .depth.action = SG_ACTION_DONTCARE,
        .colors[0] = {.action = SG_ACTION_CLEAR, .val = {0.0f, 0.0f, 0.0f, 0.0f}}
    });
    ngp_flush();
    sg_end_pass();
    ngp_end();
}

void draw(int width, int height) {
    float time = SDL_GetTicks() / 1000.0f;
    draw_fbo();
    for(int y=0;y<height;y+=128) {
        for(int x=0;x<width;x+=128) {
            ngp_push_transform();
            ngp_rotate_at(time, x+64, y+64);
            ngp_draw_textured_rect(fbo_image, (ngp_rect){x, y, 128, 128}, NULL);
            ngp_pop_transform();
        }
    }
}

bool init() {
    fbo_image = sg_make_image(&(sg_image_desc){
        .width = 128,
        .height = 128,
        .render_target = true,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE
    });
    if(fbo_image.id == SG_INVALID_ID)
        return false;
    fbo_pass = sg_make_pass(&(sg_pass_desc){
        .color_attachments[0].image = fbo_image
    });
    if(fbo_pass.id == SG_INVALID_ID)
        return false;
    return true;
}

void terminate() {
    sg_destroy_image(fbo_image);
    sg_pass_desc(fbo_pass);
}

int main(int argc, char *argv[]) {
    return sample_app((sample_app_desc){
        .init = init,
        .terminate = terminate,
        .draw = draw
    });
}
