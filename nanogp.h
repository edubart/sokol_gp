/*
nanogp.h - minimal modern efficient cross platform 2D graphics painter library
https://github.com/edubart/nanogp

MIT License

Copyright (c) 2020 Eduardo Bart (https://github.com/edubart)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef NANOGP_H
#define NANOGP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef NANOGP_IMPL
#define SOKOL_IMPL
#endif

#define SOKOL_GLCORE33
#include "sokol_gfx.h"

#ifndef NGP_ASSERT
#include <assert.h>
#define NGP_ASSERT(c) assert(c)
#endif

#define _NGP_INIT_COOKIE 0xCAFED00D
#define _NGP_MAX_VERTICES 65536

typedef struct ngp_rect {
    float x, y, w, h;
} ngp_rect;

typedef struct ngp_vec2 {
    float x, y;
} ngp_vec2;

typedef struct ngp_mat3 {
    float v[3][3];
} ngp_mat3;

typedef struct ngp_color {
    float r, g, b, a;
} ngp_color;

typedef struct ngp_state {
    ngp_color color;
    ngp_mat3 proj;
} ngp_state;

typedef struct ngp_context {
    uint32_t init_cookie;
    sg_pass_action pass_action;
    const char *last_error;
    ngp_state state;
    struct {
        sg_pipeline pip;
        sg_shader shd;
        sg_buffer vbuf;
        sg_bindings bind;
        ngp_vec2 vertices[_NGP_MAX_VERTICES];
        int cur_vertex;
    } filled_rect;
} ngp_context;

typedef enum {
    NGP_COMMAND_DRAW,
    NGP_COMMAND_VIEWPORT,
    NGP_COMMAND_SCISSOR_RECT
} ngp_command_type;

bool ngp_create(ngp_context* ngp);
void ngp_destroy(ngp_context* ngp);
bool ngp_is_valid(ngp_context* ngp);
const char* ngp_get_error(ngp_context* ngp);
void ngp_begin(ngp_context* ngp, int width, int height);
void ngp_end(ngp_context* ngp);
void ngp_set_clear_color(ngp_context* ngp, ngp_color color);
void ngp_unset_clear_color(ngp_context* ngp);
void ngp_set_color(ngp_context* ngp, ngp_color color);
void ngp_filled_rect(ngp_context* ngp, ngp_rect rect);

inline ngp_vec2 ngp_mat3_identity(ngp_mat3* m) {
    *m = (ngp_mat3){
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };
}

inline ngp_vec2 ngp_mat3_vec2_mul(ngp_mat3* m, ngp_vec2 v) {
    return (ngp_vec2){
        .x = m->v[0][0]*v.x + m->v[0][1]*v.y + m->v[0][2],
        .y = m->v[1][0]*v.x + m->v[1][1]*v.y + m->v[1][2]
    };
}

inline void ngp_mat3_proj2d(ngp_mat3* m, int width, int height) {
    float hw = 2.0f/width;
    float hh = 2.0f/height;
    *m = (ngp_mat3){
          hw,  0.0f, -1.0f,
        0.0f,   -hh,  1.0f,
        0.0f,  0.0f,  1.0f
    };
}

#endif // NANOGP_H

#ifdef NANOGP_IMPL
static void _ngp_setup_filled_rect(ngp_context* ngp) {
    // create a shader
    ngp->filled_rect.shd = sg_make_shader(&(sg_shader_desc){
        .vs.source =
            "#version 330\n"
            "layout(location=0) in vec2 position;\n"
            "void main() {\n"
            "  gl_Position.xy = position;\n"
            "}\n",
        .fs.source =
            "#version 330\n"
            "out vec4 frag_color;\n"
            "uniform vec4 color;\n"
            "void main() {\n"
            "  frag_color = color;\n"
            "}\n",
        .fs.uniform_blocks[0] = {
            .uniforms[0] = {.name="color", .type=SG_UNIFORMTYPE_FLOAT4},
            .size = 4*sizeof(float),
        },
        .attrs = {
            [0] = {.name="position", .sem_name="POSITION"},
        },
    });

    // create a pipeline
    ngp->filled_rect.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = ngp->filled_rect.shd,
        .layout.attrs = {
            [0] = {
                .offset=0,
                .format=SG_VERTEXFORMAT_FLOAT2
            },
        },
    });

    // create vertex buffer
    sg_buffer_desc vbuf_desc = {
        .size = 65536 * sizeof(ngp_vec2),
        .usage = SG_USAGE_STREAM,
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
    };
    ngp->filled_rect.vbuf = sg_make_buffer(&vbuf_desc);

    // define the resource bindings
    ngp->filled_rect.bind = (sg_bindings){
        .vertex_buffers[0] = ngp->filled_rect.vbuf,
    };
}

static void _ngp_setup_pipelines(ngp_context* ngp) {
    _ngp_setup_filled_rect(ngp);
}

static void _ngp_setup_clear(ngp_context* ngp) {
    ngp->pass_action = (sg_pass_action){
        .colors[0] = { .action = SG_ACTION_DONTCARE },
        .colors[1] = { .action = SG_ACTION_DONTCARE },
        .colors[2] = { .action = SG_ACTION_DONTCARE },
        .colors[3] = { .action = SG_ACTION_DONTCARE },
        .stencil = {.action = SG_ACTION_DONTCARE },
        .depth = {.action = SG_ACTION_DONTCARE }
    };
}

static void _ngp_setup_state(ngp_context* ngp) {
    ngp->state.color = (ngp_color){1.0f, 1.0f, 1.0f, 1.0f};
    ngp_mat3_identity(&ngp->state.proj);
}

bool ngp_create(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == 0);

    // setup Sokol
    sg_desc desc = {0};
    desc.context.depth_format = SG_PIXELFORMAT_NONE; // depth buffer is not needed
    sg_setup(&desc);
    if(!sg_isvalid()) {
        ngp->last_error = "Sokol setup failed";
        return false;
    }

    ngp->init_cookie = _NGP_INIT_COOKIE;
    ngp->last_error = "";

    _ngp_setup_pipelines(ngp);
    _ngp_setup_clear(ngp);
    _ngp_setup_state(ngp);

    return true;
}

void ngp_destroy(ngp_context* ngp) {
    if(ngp->init_cookie == 0) return; // not initialized
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);

    sg_shutdown();
    *ngp = (ngp_context){0};
}

bool ngp_is_valid(ngp_context* ngp) {
    return ngp->init_cookie == _NGP_INIT_COOKIE;
}

const char* ngp_get_error(ngp_context* ngp) {
    return ngp->last_error;
}

void ngp_begin(ngp_context* ngp, int width, int height) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    sg_begin_default_pass(&ngp->pass_action, width, height);
    sg_apply_viewport(0, 0, width, height, true);
    sg_apply_scissor_rect(0, 0, width, height, true);
    ngp_mat3_proj2d(&ngp->state.proj, width, height);
}

static void _ngp_flush(ngp_context* ngp) {
    if(ngp->filled_rect.cur_vertex > 0) {
        sg_update_buffer(ngp->filled_rect.vbuf, ngp->filled_rect.vertices, ngp->filled_rect.cur_vertex * sizeof(ngp_vec2));
        sg_apply_pipeline(ngp->filled_rect.pip);
        sg_apply_bindings(&ngp->filled_rect.bind);
        sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &ngp->state.color, sizeof(ngp_color));
        sg_draw(0, ngp->filled_rect.cur_vertex, 1);
        ngp->filled_rect.cur_vertex = 0;
    }
}

void ngp_end(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    _ngp_flush(ngp);
    sg_end_pass();
    sg_commit();
}

void ngp_set_clear_color(ngp_context* ngp, ngp_color color) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp->pass_action.colors[0] = (sg_color_attachment_action) {
        .action = SG_ACTION_CLEAR,
        .val = { color.r, color.g, color.b, color.a}
    };
}

void ngp_unset_clear_color(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp->pass_action.colors[0] = (sg_color_attachment_action) {
        .action = SG_ACTION_DONTCARE,
    };
}

void ngp_set_color(ngp_context* ngp, ngp_color color) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp->state.color = color;
}

void ngp_filled_rect(ngp_context* ngp, ngp_rect rect) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp_vec2 tl = ngp_mat3_vec2_mul(&ngp->state.proj, (ngp_vec2){rect.x, rect.y});
    ngp_vec2 br = ngp_mat3_vec2_mul(&ngp->state.proj, (ngp_vec2){rect.x + rect.w, rect.y + rect.h});
    ngp_vec2 tr = (ngp_vec2){br.x, tl.y};
    ngp_vec2 bl = (ngp_vec2){tl.x, br.y};
    ngp->filled_rect.vertices[ngp->filled_rect.cur_vertex++] = bl;
    ngp->filled_rect.vertices[ngp->filled_rect.cur_vertex++] = br;
    ngp->filled_rect.vertices[ngp->filled_rect.cur_vertex++] = tr;
    ngp->filled_rect.vertices[ngp->filled_rect.cur_vertex++] = bl;
    ngp->filled_rect.vertices[ngp->filled_rect.cur_vertex++] = tr;
    ngp->filled_rect.vertices[ngp->filled_rect.cur_vertex++] = tl;
}

#endif // NANOGP_IMPL
