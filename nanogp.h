/*
nanogp.h - minimal efficient cross platform 2D graphics painter
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
#include <stdlib.h>
#include <string.h>
#include "sokol_gfx.h"

#ifndef NGP_API
#define NGP_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ngp_error {
    NGP_NO_ERROR = 0,
    NGP_ERROR_SOKOL_INVALID,
    NGP_ERROR_VERTICES_FULL,
    NGP_ERROR_UNIFORMS_FULL,
    NGP_ERROR_COMMANDS_FULL,
} ngp_error;

typedef enum ngp_command_type {
    NGP_COMMAND_DRAW,
    NGP_COMMAND_VIEWPORT,
    NGP_COMMAND_SCISSOR_RECT
} ngp_command_type;

typedef struct ngp_rect {
    float x, y, w, h;
} ngp_rect;

typedef struct ngp_vec2 {
    float x, y;
} ngp_vec2;

typedef ngp_vec2 ngp_vertex;

typedef struct ngp_mat3 {
    float v[3][3];
} ngp_mat3;

typedef struct ngp_color {
    float r, g, b, a;
} ngp_color;

typedef struct ngp_state {
    ngp_color color;
    ngp_mat3 proj;
    ngp_mat3 modelview;
    ngp_mat3 mvp;
    int width;
    int height;
    bool matrix_dirty;
} ngp_state;

typedef struct ngp_draw_args {
    sg_pipeline pip;
    int vertex_index;
    int uniform_index;
    int num_vertices;
} ngp_draw_args;

typedef struct {
    int x, y, w, h;
} ngp_viewport_args;

typedef struct {
    int x, y, w, h;
} ngp_scissor_rect_args;

typedef union ngp_command_args {
    ngp_draw_args draw;
    ngp_viewport_args viewport;
    ngp_scissor_rect_args scissor_rect;
} ngp_command_args;

typedef struct ngp_command {
    ngp_command_type cmd;
    ngp_command_args args;
} ngp_command;

typedef struct ngp_uniform {
    ngp_color color;
} ngp_uniform;

typedef struct ngp_desc {
    int max_vertices;
    int max_commands;
} ngp_desc;

typedef struct ngp_context {
    uint32_t init_cookie;
    sg_pass_action pass_action;
    const char *last_error;
    ngp_error last_error_code;
    ngp_state state;
    sg_pipeline filled_rect_pip;
    int cur_vertex;
    int cur_uniform;
    int cur_command;
    int num_vertices;
    int num_uniforms;
    int num_commands;
    sg_shader shd;
    sg_buffer vbuf;
    sg_bindings bind;
    ngp_vertex* vertices;
    ngp_uniform* uniforms;
    ngp_command* commands;
} ngp_context;

NGP_API bool ngp_create(ngp_context* ngp, ngp_desc* desc);
NGP_API void ngp_destroy(ngp_context* ngp);
NGP_API bool ngp_is_valid(ngp_context* ngp);
NGP_API ngp_error ngp_get_error_code(ngp_context* ngp);
NGP_API const char* ngp_get_error(ngp_context* ngp);
NGP_API void ngp_reset_state(ngp_context* ngp);
NGP_API void ngp_begin(ngp_context* ngp, int width, int height);
NGP_API void ngp_end(ngp_context* ngp);
NGP_API void ngp_set_clear_color(ngp_context* ngp, ngp_color color);
NGP_API void ngp_unset_clear_color(ngp_context* ngp);
NGP_API void ngp_set_color(ngp_context* ngp, ngp_color color);
NGP_API void ngp_filled_rect(ngp_context* ngp, ngp_rect rect);

static inline ngp_mat3 ngp_mat3_identity() {
    return (ngp_mat3){
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };
}

static inline ngp_vec2 ngp_mat3_vec2_mul(const ngp_mat3* m, ngp_vec2 v) {
    return (ngp_vec2){
        .x = m->v[0][0]*v.x + m->v[0][1]*v.y + m->v[0][2],
        .y = m->v[1][0]*v.x + m->v[1][1]*v.y + m->v[1][2]
    };
}

static inline ngp_mat3 ngp_mat3_mul(const ngp_mat3* a, const ngp_mat3* b) {
    ngp_mat3 p;
    p.v[0][0] = a->v[0][0]*b->v[0][0] + a->v[0][1]*b->v[1][0] + a->v[0][2]*b->v[2][0];
    p.v[0][1] = a->v[0][0]*b->v[0][1] + a->v[0][1]*b->v[1][1] + a->v[0][2]*b->v[2][1];
    p.v[0][2] = a->v[0][0]*b->v[0][2] + a->v[0][1]*b->v[1][2] + a->v[0][2]*b->v[2][2];
    p.v[1][0] = a->v[1][0]*b->v[0][0] + a->v[1][1]*b->v[1][0] + a->v[1][2]*b->v[2][0];
    p.v[1][1] = a->v[1][0]*b->v[0][1] + a->v[1][1]*b->v[1][1] + a->v[1][2]*b->v[2][1];
    p.v[1][2] = a->v[1][0]*b->v[0][2] + a->v[1][1]*b->v[1][2] + a->v[1][2]*b->v[2][2];
    p.v[2][0] = a->v[2][0]*b->v[0][0] + a->v[2][1]*b->v[1][0] + a->v[2][2]*b->v[2][0];
    p.v[2][1] = a->v[2][0]*b->v[0][1] + a->v[2][1]*b->v[1][1] + a->v[2][2]*b->v[2][1];
    p.v[2][2] = a->v[2][0]*b->v[0][2] + a->v[2][1]*b->v[1][2] + a->v[2][2]*b->v[2][2];
    return p;
}

static inline ngp_mat3 ngp_mat3_proj2d(int width, int height) {
    return (ngp_mat3){
        2.0f/width,           0.0f, -1.0f,
              0.0f,   -2.0f/height,  1.0f,
              0.0f,           0.0f,  1.0f
    };
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NANOGP_H

#ifdef NANOGP_IMPL

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_gfx.h"

#ifndef NGP_ASSERT
#include <assert.h>
#define NGP_ASSERT(c) assert(c)
#endif
#ifndef NGP_MALLOC
#include <stdlib.h>
#define NGP_MALLOC(s) malloc(s)
#define NGP_FREE(p) free(p)
#endif
#ifndef NDEBUG
#define NGP_DEBUG
#endif
#ifndef NGP_LOG
#ifdef NGP_DEBUG
#include <stdio.h>
#define NGP_LOG(s) { NGP_ASSERT(s); puts(s); }
#else
#define NGP_LOG(s)
#endif
#endif
#ifndef NGP_UNREACHABLE
#define NGP_UNREACHABLE NGP_ASSERT(false)
#endif

#define _NGP_INIT_COOKIE 0xCAFED00D
#define _NGP_DEFAULT_MAX_VERTICES 65536
#define _NGP_DEFAULT_MAX_COMMANDS 16384

#define ngp_def(val, def) (((val) == 0) ? (def) : (val))

static void _ngp_set_error(ngp_context* ngp, ngp_error error_code, const char *error) {
    ngp->last_error_code = error_code;
    ngp->last_error = error;
    NGP_LOG(error);
}

static void _ngp_setup_pipelines(ngp_context* ngp) {
    // create a pipeline
    ngp->filled_rect_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = sg_make_shader(&(sg_shader_desc){
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
        }),
        .layout.attrs = {
            [0] = {
                .offset=0,
                .format=SG_VERTEXFORMAT_FLOAT2
            },
        },
    });
}

bool ngp_create(ngp_context* ngp, ngp_desc* desc) {
    NGP_ASSERT(ngp->init_cookie == 0);

    // setup sokol
    sg_setup(&(sg_desc){
        .context.depth_format = SG_PIXELFORMAT_NONE // depth buffer is not needed
    });
    if(!sg_isvalid()) {
        _ngp_set_error(ngp, NGP_ERROR_SOKOL_INVALID, "sokol_gfx initialization failed");
        return false;
    }

    ngp->init_cookie = _NGP_INIT_COOKIE;
    ngp->last_error = "";
    ngp->num_vertices = ngp_def(desc->max_vertices, _NGP_DEFAULT_MAX_VERTICES);
    ngp->num_commands = ngp_def(desc->max_commands, _NGP_DEFAULT_MAX_COMMANDS);
    ngp->num_uniforms = ngp_def(desc->max_commands, _NGP_DEFAULT_MAX_COMMANDS);
    ngp->vertices = (ngp_vertex*) NGP_MALLOC(ngp->num_vertices * sizeof(ngp_vertex));
    ngp->uniforms = (ngp_uniform*) NGP_MALLOC(ngp->num_uniforms * sizeof(ngp_uniform));
    ngp->commands = (ngp_command*) NGP_MALLOC(ngp->num_commands * sizeof(ngp_command));
    NGP_ASSERT(ngp->commands && ngp->uniforms && ngp->uniforms);

    // create vertex buffer
    ngp->vbuf = sg_make_buffer(&(sg_buffer_desc){
        .size = 65536 * sizeof(ngp_vec2),
        .usage = SG_USAGE_STREAM,
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
    });

    // define the resource bindings
    ngp->bind = (sg_bindings){
        .vertex_buffers[0] = ngp->vbuf,
    };

    // disable default render clear color
    ngp->pass_action = (sg_pass_action){
        .colors[0] = { .action = SG_ACTION_DONTCARE },
        .colors[1] = { .action = SG_ACTION_DONTCARE },
        .colors[2] = { .action = SG_ACTION_DONTCARE },
        .colors[3] = { .action = SG_ACTION_DONTCARE },
        .stencil = {.action = SG_ACTION_DONTCARE },
        .depth = {.action = SG_ACTION_DONTCARE }
    };

    _ngp_setup_pipelines(ngp);
    return true;
}

void ngp_destroy(ngp_context* ngp) {
    if(ngp->init_cookie == 0) return; // not initialized
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    NGP_FREE(ngp->vertices);
    NGP_FREE(ngp->uniforms);
    NGP_FREE(ngp->commands);
    sg_shutdown();
    memset(ngp, 0, sizeof(ngp_context));
}

bool ngp_is_valid(ngp_context* ngp) {
    return ngp->init_cookie == _NGP_INIT_COOKIE;
}

ngp_error ngp_get_error_code(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    return ngp->last_error_code;
}

const char* ngp_get_error(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    return ngp->last_error;
}

static void _ngp_update_mvp(ngp_context* ngp) {
    if(!ngp->state.matrix_dirty)
        return;
    ngp->state.mvp = ngp_mat3_mul(&ngp->state.proj, &ngp->state.modelview);
    ngp->state.matrix_dirty = false;
}

void ngp_reset_state(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp->state.color = (ngp_color){1.0f, 1.0f, 1.0f, 1.0f};
    ngp->state.modelview = ngp_mat3_identity();
    ngp->state.proj = ngp_mat3_proj2d(ngp->state.width, ngp->state.height);
    ngp->state.matrix_dirty = true;
    _ngp_update_mvp(ngp);
}

void ngp_begin(ngp_context* ngp, int width, int height) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    sg_begin_default_pass(&ngp->pass_action, width, height);
    ngp->state.width = width;
    ngp->state.height = height;
    ngp_reset_state(ngp);
}

static void _ngp_rewind(ngp_context* ngp) {
    ngp->cur_command = 0;
    ngp->cur_uniform = 0;
    ngp->cur_vertex = 0;
    ngp->last_error_code = NGP_NO_ERROR;
    ngp->last_error = "";
}

static void _ngp_flush_commands(ngp_context* ngp) {
    if(ngp->last_error_code != NGP_NO_ERROR || ngp->cur_command <= 0)
        return;

    uint32_t cur_pip_id = SG_INVALID_ID;
    int cur_uniform_index = -1;
    sg_update_buffer(ngp->vbuf, ngp->vertices, ngp->cur_vertex * sizeof(ngp_vertex));
    for(int i = 0; i < ngp->cur_command; ++i) {
        ngp_command* cmd = &ngp->commands[i];
        switch(cmd->cmd) {
            case NGP_COMMAND_VIEWPORT: {
                break;
            }
            case NGP_COMMAND_SCISSOR_RECT: {
                break;
            }
            case NGP_COMMAND_DRAW: {
                ngp_draw_args* args = &cmd->args.draw;
                bool pip_changed = false;
                if(args->pip.id != cur_pip_id) {
                    sg_apply_pipeline(args->pip);
                    cur_pip_id = args->pip.id;
                    // when pipeline changes, also need to re-apply uniforms and bindings
                    cur_uniform_index = -1;
                    pip_changed = true;
                }
                if(pip_changed) {
                    sg_apply_bindings(&ngp->bind);
                }
                if(cur_uniform_index != args->uniform_index) {
                    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &ngp->uniforms[args->uniform_index], sizeof(ngp_uniform));
                    cur_uniform_index = args->uniform_index;
                }
                sg_draw(args->vertex_index, args->num_vertices, 1);
                break;
            }
            default: {
                NGP_UNREACHABLE;
                break;
            }
        }
    }
    _ngp_rewind(ngp);
}

void ngp_end(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    _ngp_flush_commands(ngp);
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

static ngp_vertex* _ngp_next_vertices(ngp_context* ngp, int count) {
    if(ngp->cur_vertex + count <= ngp->num_vertices) {
        ngp_vertex *vertices = &ngp->vertices[ngp->cur_vertex];
        ngp->cur_vertex += count;
        return vertices;
    } else {
        _ngp_set_error(ngp, NGP_ERROR_VERTICES_FULL, "NGP vertices buffer is full");
        return NULL;
    }
}
static ngp_uniform* _ngp_prev_uniform(ngp_context* ngp) {
    if(ngp->cur_uniform > 0) {
        return &ngp->uniforms[ngp->cur_uniform-1];
    } else {
        return NULL;
    }
}

static ngp_uniform* _ngp_next_uniform(ngp_context* ngp) {
    if(ngp->cur_uniform < ngp->num_uniforms) {
        return &ngp->uniforms[ngp->cur_uniform++];
    } else {
        _ngp_set_error(ngp, NGP_ERROR_UNIFORMS_FULL, "NGP uniform buffer is full");
        return NULL;
    }
}

static ngp_command* _ngp_prev_command(ngp_context* ngp) {
    if(ngp->cur_command > 0) {
        return &ngp->commands[ngp->cur_command-1];
    } else {
        return NULL;
    }
}

static ngp_command* _ngp_next_command(ngp_context* ngp) {
    if(ngp->cur_command < ngp->num_commands) {
        return &ngp->commands[ngp->cur_command++];
    } else {
        _ngp_set_error(ngp, NGP_ERROR_COMMANDS_FULL, "NGP command buffer is full");
        return NULL;
    }
}

void ngp_translate(ngp_context* ngp, float x, float y) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp_mat3 translate = {
       1.0f, 0.0f,    x,
       0.0f, 1.0f,    y,
       0.0f, 0.0f, 1.0f,
    };
    ngp->state.modelview = ngp_mat3_mul(&ngp->state.modelview, &translate);
    ngp->state.matrix_dirty = true;
}

void ngp_filled_rect(ngp_context* ngp, ngp_rect rect) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);

    // compute rect vertexes
    _ngp_update_mvp(ngp);
    ngp_vec2 tl = ngp_mat3_vec2_mul(&ngp->state.mvp, (ngp_vec2){rect.x, rect.y});
    ngp_vec2 br = ngp_mat3_vec2_mul(&ngp->state.mvp, (ngp_vec2){rect.x + rect.w, rect.y + rect.h});
    ngp_vec2 tr = (ngp_vec2){br.x, tl.y};
    ngp_vec2 bl = (ngp_vec2){tl.x, br.y};

    // setup vertex
    int vertex_index = ngp->cur_vertex;
    ngp_vertex* vertices = _ngp_next_vertices(ngp, 6);
    if(!vertices) return;

    vertices[0] = bl;
    vertices[1] = br;
    vertices[2] = tr;
    vertices[3] = bl;
    vertices[4] = tr;
    vertices[5] = tl;

    // setup uniform, try to reuse previous uniform when possible
    ngp_uniform *prev_uniform = _ngp_prev_uniform(ngp);
    bool reuse_uniform = prev_uniform &&
                         memcmp(&prev_uniform->color, &ngp->state.color, sizeof(ngp_color)) == 0;
    if(!reuse_uniform) {
        // append new uniform
        ngp_uniform *uniform = _ngp_next_uniform(ngp);
        if(!uniform) return;
        uniform->color = ngp->state.color;
    }
    int uniform_index = ngp->cur_uniform - 1;

    ngp_command* prev_cmd = _ngp_prev_command(ngp);
    bool merge_cmd = prev_cmd &&
                     prev_cmd->cmd == NGP_COMMAND_DRAW &&
                     prev_cmd->args.draw.pip.id == ngp->filled_rect_pip.id &&
                     prev_cmd->args.draw.uniform_index == uniform_index;
    if(merge_cmd) {
        // merge command for batched rendering
        prev_cmd->args.draw.num_vertices += 6;
    } else {
        // append new draw command
        ngp_command* cmd = _ngp_next_command(ngp);
        if(!cmd) return;
        *cmd = (ngp_command) {
            .cmd = NGP_COMMAND_DRAW,
            .args.draw = {
                .pip = ngp->filled_rect_pip,
                .vertex_index = vertex_index,
                .uniform_index = uniform_index,
                .num_vertices = 6,
            },
        };
    }
}

#endif // NANOGP_IMPL
