#include "sample_common.h"

int frame = 0;

void draw_rects(ngp_context* ngp) {
    int width = ngp->viewport.w, height = ngp->viewport.h;
    int size = 64;
    int hsize = size / 2;
    float time = frame / 60.0f;
    float t = (1.0f+sinf(time))/2.0f;

    // left
    ngp_push_transform(ngp);
    ngp_translate(ngp, width*0.25f - hsize, height*0.5f - hsize);
    ngp_translate(ngp, 0.0f, 2*size*t - size);
    ngp_set_color(ngp, t, 0.3f, 1.0f-t, 1.0f);
    ngp_draw_rect(ngp, 0, 0, size, size);
    ngp_pop_transform(ngp);

    // middle
    ngp_push_transform(ngp);
    ngp_translate(ngp, width*0.5f - hsize, height*0.5f - hsize);
    ngp_rotate_at(ngp, time, hsize, hsize);
    ngp_set_color(ngp, t, 1.0f - t, 0.3f, 1.0f);
    ngp_draw_rect(ngp, 0, 0, size, size);
    ngp_pop_transform(ngp);

    // right
    ngp_push_transform(ngp);
    ngp_translate(ngp, width*0.75f - hsize, height*0.5f - hsize);
    ngp_scale_at(ngp, t + 0.25f, t + 0.5f, hsize, hsize);
    ngp_set_color(ngp, 0.3f, t, 1.0f - t, 1.0f);
    ngp_draw_rect(ngp, 0, 0, size, size);
    ngp_pop_transform(ngp);
}

void draw_points(ngp_context* ngp) {
    ngp_set_color(ngp, 1.0f, 1.0f, 1.0f, 1.0f);
    int width = ngp->viewport.w, height = ngp->viewport.h;
    static ngp_vec2 points[4096];
    uint count = 0;
    for(int y=64;y<height-64 && count < 4096;y+=8) {
        for(int x=64;x<width-64 && count < 4096;x+=8) {
            points[count] = (ngp_vec2){x,y};
            count++;
        }
    }
    ngp_draw_points(ngp, points, count);
}

void draw_lines(ngp_context* ngp) {
    ngp_set_color(ngp, 1.0f, 1.0f, 1.0f, 1.0f);
    ngp_vec2 c = {ngp->viewport.w / 2, ngp->viewport.h / 2};
    ngp_vec2 a = c;
    for(float theta = 0.0f; theta <= M_PI*8.0f; theta+=M_PI/10.0f) {
        float r = 10.0f*theta;
        ngp_vec2 b = {c.x + r*cosf(theta), c.y + r*sinf(theta)};
        ngp_draw_line(ngp, a.x, a.y, b.x, b.y);
        a = b;
    }
}

void draw_triangles(ngp_context* ngp) {
    int width = ngp->viewport.w, height = ngp->viewport.h;
    float time = frame / 60.0f;
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float w = height*0.3f;
    float ax = hw - w, ay = hh + w;
    float bx = hw,     by = hh - w;
    float cx = hw + w, cy = hh + w;
    ngp_set_color(ngp, 1.0f, 0.0f, 1.0f, 1.0f);
    ngp_draw_triangle(ngp, ax, ay, bx, by, cx, cy);
}

void render(ng_context* ngctx, ngp_context* ngp) {
    int width, height;
    ngctx_get_drawable_size(ngctx, &width, &height);
    ngp_set_clear_color(ngp, 0.05f, 0.05f, 0.05f, 1.0f);
    ngp_begin(ngp, width, height);
    int hw = width / 2;
    int hh = height / 2;

    // top left
    ngp_viewport(ngp, 0, 0, hw, hh);
    ngp_set_color(ngp, 0.1f, 0.1f, 0.1f, 1.0f);
    ngp_draw_rect(ngp, 0, 0, hw, hh);
    ngp_push_transform(ngp);
    ngp_translate(ngp, 0.0f, -hh / 4.0f);
    draw_rects(ngp);
    ngp_pop_transform(ngp);
    ngp_push_transform(ngp);
    ngp_translate(ngp, 0.0f, hh / 4.0f);
    ngp_scissor(ngp, 0, 0, hw, 3.0f*hh / 4.0f);
    draw_rects(ngp);
    ngp_reset_scissor(ngp);
    ngp_pop_transform(ngp);

    // top right
    ngp_viewport(ngp, hw, 0, hw, hh);
    draw_triangles(ngp);

    // bottom left
    ngp_viewport(ngp, 0, hh, hw, hh);
    draw_points(ngp);

    // bottom right
    ngp_viewport(ngp, hw, hh, hw, hh);
    ngp_set_color(ngp, 0.1f, 0.1f, 0.1f, 1.0f);
    ngp_draw_rect(ngp, 0, 0, hw, hh);
    draw_lines(ngp);

    ngp_end(ngp);
    ngctx_swap(ngctx);

    frame++;
}

int main() {
    return sample_app(render);
}
