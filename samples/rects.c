#include "sample_common.h"

int frame = 0;

static ngp_vec2 points_buffer[4096];

void draw_rects() {
    ngp_irect viewport = ngp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    int size = 64;
    int hsize = size / 2;
    float time = frame / 60.0f;
    float t = (1.0f+sinf(time))/2.0f;

    // left
    ngp_push_transform();
    ngp_translate(width*0.25f - hsize, height*0.5f - hsize);
    ngp_translate(0.0f, 2*size*t - size);
    ngp_set_color(t, 0.3f, 1.0f-t, 1.0f);
    ngp_draw_rect(0, 0, size, size);
    ngp_pop_transform();

    // middle
    ngp_push_transform();
    ngp_translate(width*0.5f - hsize, height*0.5f - hsize);
    ngp_rotate_at(time, hsize, hsize);
    ngp_set_color(t, 1.0f - t, 0.3f, 1.0f);
    ngp_draw_rect(0, 0, size, size);
    ngp_pop_transform();

    // right
    ngp_push_transform();
    ngp_translate(width*0.75f - hsize, height*0.5f - hsize);
    ngp_scale_at(t + 0.25f, t + 0.5f, hsize, hsize);
    ngp_set_color(0.3f, t, 1.0f - t, 1.0f);
    ngp_draw_rect(0, 0, size, size);
    ngp_pop_transform();
}

void draw_points() {
    ngp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    ngp_irect viewport = ngp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    uint count = 0;
    for(int y=64;y<height-64 && count < 4096;y+=8) {
        for(int x=64;x<width-64 && count < 4096;x+=8) {
            points_buffer[count++] = (ngp_vec2){x,y};
        }
    }
    ngp_draw_points(points_buffer, count);
}

void draw_lines() {
    ngp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    uint count = 0;
    ngp_irect viewport = ngp_query_state()->viewport;
    ngp_vec2 c = {viewport.w / 2, viewport.h / 2};
    points_buffer[count++] = c;
    for(float theta = 0.0f; theta <= M_PI*8.0f; theta+=M_PI/16.0f) {
        float r = 10.0f*theta;
        points_buffer[count++] = (ngp_vec2){c.x + r*cosf(theta), c.y + r*sinf(theta)};
    }
    ngp_draw_line_strip(points_buffer, count);
}

void draw_triangles() {
    ngp_irect viewport = ngp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    float time = frame / 60.0f;
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float w = height*0.2f;
    float ax = hw - w, ay = hh + w;
    float bx = hw,     by = hh - w;
    float cx = hw + w, cy = hh + w;
    ngp_set_color(1.0f, 0.0f, 1.0f, 1.0f);
    ngp_push_transform();
    ngp_translate(-w*1.5f, 0.0f);
    ngp_draw_triangle(ax, ay, bx, by, cx, cy);
    ngp_translate(w*3.0f, 0.0f);
    int count = 0;
    float step = (2.0f*M_PI)/6.0f;
    for(float theta = 0.0f; theta <= 2.0f*M_PI + step*0.5f; theta+=step) {
        points_buffer[count++] = (ngp_vec2){hw + w*cosf(theta), hh - w*sinf(theta)};
        if(count % 3 == 1)
            points_buffer[count++] = (ngp_vec2){hw, hh};
    }
    ngp_set_color(0.0f, 1.0f, 1.0f, 1.0f);
    ngp_draw_triangle_strip(points_buffer, count);
    ngp_pop_transform();
}

void render(ngctx_context* ngctx) {
    ngctx_isize size = ngctx_get_drawable_size(ngctx);
    ngp_set_clear_color(0.05f, 0.05f, 0.05f, 1.0f);
    ngp_begin(size.w, size.h);
    int hw = size.w / 2;
    int hh = size.h / 2;

    // top left
    ngp_viewport(0, 0, hw, hh);
    ngp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
    ngp_draw_rect(0, 0, hw, hh);
    ngp_push_transform();
    ngp_translate(0.0f, -hh / 4.0f);
    draw_rects();
    ngp_pop_transform();
    ngp_push_transform();
    ngp_translate(0.0f, hh / 4.0f);
    ngp_scissor(0, 0, hw, 3.0f*hh / 4.0f);
    draw_rects();
    ngp_reset_scissor();
    ngp_pop_transform();

    // top right
    ngp_viewport(hw, 0, hw, hh);
    draw_triangles();

    // bottom left
    ngp_viewport(0, hh, hw, hh);
    draw_points();

    // bottom right
    ngp_viewport(hw, hh, hw, hh);
    ngp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
    ngp_draw_rect(0, 0, hw, hh);
    draw_lines();

    ngp_end();
    ngctx_swap(ngctx);

    frame++;
}

int main() {
    return sample_app(render);
}
