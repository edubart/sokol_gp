/*
This examples shows all primitives shapes and transformation functions.
*/

#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include <stdio.h>
#include <stdlib.h>

static const float PI = 3.14159265358979323846264338327f;

static void draw_rects(void) {
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    int size = 64;
    int hsize = size / 2;
    float time = sapp_frame_count() / 60.0f;
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

static void draw_points(void) {
    // point grid
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    static sgp_vec2 points_buffer[4096];
    unsigned int count = 0;
    for (int y=64;y<height-64 && count < 4096;y+=8) {
        for (int x=64;x<width-64 && count < 4096;x+=8) {
            sgp_vec2 v = {(float)x,(float)y};
            points_buffer[count++] = v;
        }
    }
    sgp_draw_points(points_buffer, count);
}

static void draw_lines(void) {
    // spiral
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    sgp_irect viewport = sgp_query_state()->viewport;
    sgp_vec2 c = {viewport.w / 2.0f, viewport.h / 2.0f};
    static sgp_vec2 points_buffer[4096];
    unsigned int count = 0;
    points_buffer[count++] = c;
    for (float theta = 0.0f; theta <= PI*8.0f; theta+=PI/16.0f) {
        float r = 10.0f*theta;
        sgp_vec2 v = {c.x + r*cosf(theta), c.y + r*sinf(theta)};
        points_buffer[count++] = v;
    }
    sgp_draw_lines_strip(points_buffer, count);

    // x
    sgp_push_transform();
    sgp_translate(viewport.w/2, viewport.h/2);
    int x_size = 32;
    sgp_line x_line[2] = {
        {{-x_size, -x_size}, { x_size, x_size}},
        {{ x_size, -x_size}, {-x_size, x_size}},
    };
    sgp_draw_lines(x_line, 2);
    sgp_pop_transform();
}

static void draw_triangles(void) {
    float time = sapp_frame_count() / 60.0f;
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;

    // triangle
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

    // hexagon
    sgp_translate(w*3.0f, -hh*0.5f);
    sgp_set_color(0.0f, 1.0f, 1.0f, 1.0f);
    {
        float step = (2.0f*PI)/6.0f;
        unsigned int count = 0;
        static sgp_vec2 points_buffer[4096];
        for (float theta = 0.0f; theta <= 2.0f*PI + step*0.5f; theta+=step) {
            sgp_vec2 v = {hw + w*cosf(theta), hh - w*sinf(theta)};
            points_buffer[count++] = v;
            if (count % 3 == 1) {
                sgp_vec2 u = {hw, hh};
                points_buffer[count++] = u;
            }
        }
        sgp_draw_filled_triangles_strip(points_buffer, count);
    }

    // color wheel
    sgp_translate(0.0f, hh);
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    {
        float step = (2.0f*PI)/64.0f;
        unsigned int count = 0;
        static sgp_vertex vertex_buffer[4096];
        for (float theta = 0.0f; theta <= 2.0f*PI + step*0.5f; theta+=step) {
            sgp_vec2 v = {hw + w*cosf(theta), hh - w*sinf(theta)};
            vertex_buffer[count].position = v;
            vertex_buffer[count].color = (sgp_color_ub4){(sinf(theta + time*1) + 1.0f) * 0.5f * 255.0f,
                                                         (sinf(theta + time*2) + 1.0f) * 0.5f * 255.0f,
                                                         (sinf(theta + time*4) + 1.0f) * 0.5f * 255.0f, 255};
            count++;
            if (count % 3 == 1) {
                sgp_vec2 u = {hw, hh};
                vertex_buffer[count].position = u;
                vertex_buffer[count].color = (sgp_color_ub4){255, 255, 255, 255};
                count++;
            }
        }
        sgp_draw(SG_PRIMITIVETYPE_TRIANGLE_STRIP, vertex_buffer, count);
    }
    sgp_pop_transform();
}

static void frame(void) {
    // begin draw commands queue
    int width = sapp_width(), height = sapp_height();
    sgp_begin(width, height);

    int hw = width / 2;
    int hh = height / 2;

    // draw background
    sgp_set_color(0.05f, 0.05f, 0.05f, 1.0f);
    sgp_clear();
    sgp_reset_color();

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

    // dispatch draw commands
    sg_pass_action pass_action = {0};
    sg_begin_default_pass(&pass_action, width, height);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();
}

static void init(void) {
    // initialize Sokol GFX
    sg_desc sgdesc = {
        .context = sapp_sgcontext(),
        .logger.func = slog_func
    };
    sg_setup(&sgdesc);
    if (!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol GFX context!\n");
        exit(-1);
    }

    // initialize Sokol GP
    sgp_desc sgpdesc = {0};
    sgp_setup(&sgpdesc);
    if (!sgp_is_valid()) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error_message(sgp_get_last_error()));
        exit(-1);
    }
}

static void cleanup(void) {
    sgp_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .window_title = "Primitives (Sokol GP)",
        .logger.func = slog_func,
    };
}
