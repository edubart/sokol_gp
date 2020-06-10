#include "sample_app.h"

static sgp_vec2 points_buffer[4096];
const float PI = 3.14159265358979323846264338327f;

void draw_rects() {
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    int size = 64;
    int hsize = size / 2;
    float time = app.frame / 60.0f;
    float t = (1.0f+sinf(time))/2.0f;

    // left
    sgp_push_transform();
    sgp_translate(width*0.25f - hsize, height*0.5f - hsize);
    sgp_translate(0.0f, 2*size*t - size);
    sgp_set_color(t, 0.3f, 1.0f-t, 1.0f);
    sgp_draw_filled_rect(0, 0, size, size);
    sgp_pop_transform();

    // middle
    sgp_push_transform();
    sgp_translate(width*0.5f - hsize, height*0.5f - hsize);
    sgp_rotate_at(time, hsize, hsize);
    sgp_set_color(t, 1.0f - t, 0.3f, 1.0f);
    sgp_draw_filled_rect(0, 0, size, size);
    sgp_pop_transform();

    // right
    sgp_push_transform();
    sgp_translate(width*0.75f - hsize, height*0.5f - hsize);
    sgp_scale_at(t + 0.25f, t + 0.5f, hsize, hsize);
    sgp_set_color(0.3f, t, 1.0f - t, 1.0f);
    sgp_draw_filled_rect(0, 0, size, size);
    sgp_pop_transform();
}

void draw_points() {
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    unsigned int count = 0;
    for(int y=64;y<height-64 && count < 4096;y+=8) {
        for(int x=64;x<width-64 && count < 4096;x+=8) {
            points_buffer[count++] = (sgp_vec2){(float)x,(float)y};
        }
    }
    sgp_draw_points(points_buffer, count);
}

void draw_lines() {
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    unsigned int count = 0;
    sgp_irect viewport = sgp_query_state()->viewport;
    sgp_vec2 c = {viewport.w / 2.0f, viewport.h / 2.0f};
    points_buffer[count++] = c;
    for(float theta = 0.0f; theta <= PI*8.0f; theta+=PI/16.0f) {
        float r = 10.0f*theta;
        points_buffer[count++] = (sgp_vec2){c.x + r*cosf(theta), c.y + r*sinf(theta)};
    }
    sgp_draw_line_strip(points_buffer, count);
}

void draw_triangles() {
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float w = height*0.2f;
    float ax = hw - w, ay = hh + w;
    float bx = hw,     by = hh - w;
    float cx = hw + w, cy = hh + w;
    sgp_set_color(1.0f, 0.0f, 1.0f, 1.0f);
    sgp_push_transform();
    sgp_translate(-w*1.5f, 0.0f);
    sgp_draw_filled_triangle(ax, ay, bx, by, cx, cy);
    sgp_translate(w*3.0f, 0.0f);
    int count = 0;
    float step = (2.0f*PI)/6.0f;
    for(float theta = 0.0f; theta <= 2.0f*PI + step*0.5f; theta+=step) {
        points_buffer[count++] = (sgp_vec2){hw + w*cosf(theta), hh - w*sinf(theta)};
        if(count % 3 == 1)
            points_buffer[count++] = (sgp_vec2){hw, hh};
    }
    sgp_set_color(0.0f, 1.0f, 1.0f, 1.0f);
    sgp_draw_filled_triangle_strip(points_buffer, count);
    sgp_pop_transform();
}

void draw() {
    int hw = app.width / 2;
    int hh = app.height / 2;

    // top left
    sgp_viewport(0, 0, hw, hh);
    sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
    sgp_clear();
    sgp_reset_color();
    sgp_push_transform();
    sgp_translate(0.0f, -hh / 4.0f);
    draw_rects();
    sgp_pop_transform();
    sgp_push_transform();
    sgp_translate(0.0f, hh / 4.0f);
    sgp_scissor(0, 0, hw, 3.0f*hh / 4.0f);
    draw_rects();
    sgp_reset_scissor();
    sgp_pop_transform();

    // top right
    sgp_viewport(hw, 0, hw, hh);
    draw_triangles();

    // bottom left
    sgp_viewport(0, hh, hw, hh);
    draw_points();

    // bottom right
    sgp_viewport(hw, hh, hw, hh);
    sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
    sgp_clear();
    sgp_reset_color();
    draw_lines();
}

bool init() {
    return true;
}

void terminate() {
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
