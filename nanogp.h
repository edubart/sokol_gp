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
#include <math.h>
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
    NGP_ERROR_TRANSFORM_STACK_OVERFLOW,
    NGP_ERROR_TRANSFORM_STACK_UNDERFLOW,
} ngp_error;

typedef enum ngp_command_type {
    NGP_COMMAND_DRAW,
    NGP_COMMAND_VIEWPORT,
    NGP_COMMAND_SCISSOR
} ngp_command_type;

typedef struct ngp_rect {
    float x, y, w, h;
} ngp_rect;

typedef struct ngp_irect {
    int x, y, w, h;
} ngp_irect;

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

typedef struct ngp_draw_args {
    sg_pipeline pip;
    int vertex_index;
    int uniform_index;
    int num_vertices;
} ngp_draw_args;

typedef union ngp_command_args {
    ngp_draw_args draw;
    ngp_irect viewport;
    ngp_irect scissor;
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
    int sample_count;
} ngp_desc;

#define _NGP_MAX_STACK_DEPTH 64

typedef struct ngp_context {
    uint32_t init_cookie;
    const char *last_error;
    ngp_error last_error_code;

    // default render pass
    sg_pass_action pass_action;

    // resources
    sg_buffer vbuf;
    sg_bindings bind;
    sg_pipeline fill_triangles_pip;

    // command queue
    int cur_vertex;
    int cur_uniform;
    int cur_command;
    int num_vertices;
    int num_uniforms;
    int num_commands;
    ngp_vertex* vertices;
    ngp_uniform* uniforms;
    ngp_command* commands;

    // state tracking
    ngp_color color;
    ngp_irect viewport;
    ngp_irect scissor;
    ngp_mat3 proj;
    ngp_mat3 transform;
    ngp_mat3 mvp;
    int width;
    int height;
    bool matrix_dirty;

    // matrix stack
    int cur_transform;
    ngp_mat3 transform_stack[_NGP_MAX_STACK_DEPTH];
} ngp_context;

// setup functions
NGP_API bool ngp_create(ngp_context* ngp, ngp_desc* desc);
NGP_API void ngp_destroy(ngp_context* ngp);
NGP_API bool ngp_is_valid(ngp_context* ngp);
NGP_API ngp_error ngp_get_error_code(ngp_context* ngp);
NGP_API const char* ngp_get_error(ngp_context* ngp);

// rendering functions
NGP_API void ngp_begin(ngp_context* ngp, int width, int height);
NGP_API void ngp_end(ngp_context* ngp);
NGP_API void ngp_set_clear_color(ngp_context* ngp, float r, float g, float b, float a);
NGP_API void ngp_reset_clear_color(ngp_context* ngp);

// state transform functions
NGP_API void ngp_push_transform(ngp_context* ngp);
NGP_API void ngp_pop_transform(ngp_context* ngp);
NGP_API void ngp_translate(ngp_context* ngp, float x, float y);
NGP_API void ngp_rotate(ngp_context* ngp, float theta);
NGP_API void ngp_rotate_at(ngp_context* ngp, float theta, float x, float y);
NGP_API void ngp_scale(ngp_context* ngp, float sx, float sy);
NGP_API void ngp_scale_at(ngp_context* ngp, float sx, float sy, float x, float y);
NGP_API void ngp_reset_transform(ngp_context* ngp);

// state changing functions
NGP_API void ngp_set_color(ngp_context* ngp, float r, float g, float b, float a);
NGP_API void ngp_reset_color(ngp_context* ngp);
NGP_API void ngp_viewport(ngp_context* ngp, int x, int y, int w, int h);
NGP_API void ngp_reset_viewport(ngp_context* ngp);
NGP_API void ngp_scissor(ngp_context* ngp, int x, int y, int w, int h);
NGP_API void ngp_reset_scissor(ngp_context* ngp);
NGP_API void ngp_reset_state(ngp_context* ngp);

// drawing functions
NGP_API void ngp_fill_rect(ngp_context* ngp, float x, float y, float w, float h);

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
    ngp->fill_triangles_pip = sg_make_pipeline(&(sg_pipeline_desc){
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
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
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
        .context.depth_format = SG_PIXELFORMAT_NONE, // depth buffer is not needed
        .context.sample_count = desc->sample_count,
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
        .stencil = {.action = SG_ACTION_DONTCARE },
        .depth = {.action = SG_ACTION_DONTCARE }
    };
    for(int i=0;i<SG_MAX_COLOR_ATTACHMENTS;++i) {
        ngp->pass_action.colors[i] = (sg_color_attachment_action) {
            .action = SG_ACTION_DONTCARE,
        };
    }

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

void ngp_begin(ngp_context* ngp, int width, int height) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    sg_begin_default_pass(&ngp->pass_action, width, height);

    // default state
    ngp->color = (ngp_color){1.0f, 1.0f, 1.0f, 1.0f};
    ngp->viewport = (ngp_irect){0, 0, width, height};
    ngp->scissor = (ngp_irect){0, 0, -1, -1};
    ngp->proj = ngp_mat3_proj2d(width, height);
    ngp->transform = ngp_mat3_identity();
    ngp->mvp = ngp_mat3_mul(&ngp->proj, &ngp->transform);
    ngp->width = width;
    ngp->height = height;
    ngp->matrix_dirty = false;
}

static void _ngp_rewind(ngp_context* ngp) {
    ngp->last_error = "";
    ngp->last_error_code = NGP_NO_ERROR;
    ngp->cur_vertex = 0;
    ngp->cur_uniform = 0;
    ngp->cur_command = 0;
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
                ngp_irect* args = &cmd->args.viewport;
                sg_apply_viewport(args->x, args->y, args->w, args->h, true);
                break;
            }
            case NGP_COMMAND_SCISSOR: {
                ngp_irect* args = &cmd->args.scissor;
                sg_apply_scissor_rect(args->x, args->y, args->w, args->h, true);
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

void ngp_set_clear_color(ngp_context* ngp, float r, float g, float b, float a) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    for(int i=0;i<SG_MAX_COLOR_ATTACHMENTS;++i) {
        ngp->pass_action.colors[i] = (sg_color_attachment_action) {
            .action = SG_ACTION_CLEAR,
            .val = {r,g,b,a}
        };
    }
}

void ngp_reset_clear_color(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    for(int i=0;i<SG_MAX_COLOR_ATTACHMENTS;++i) {
        ngp->pass_action.colors[i] = (sg_color_attachment_action) {
            .action = SG_ACTION_DONTCARE,
        };
    }
}

void ngp_push_transform(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    if(ngp->cur_transform >= _NGP_MAX_STACK_DEPTH) {
        _ngp_set_error(ngp, NGP_ERROR_TRANSFORM_STACK_OVERFLOW, "NGP transform stack overflow");
        return;
    }
    ngp->transform_stack[ngp->cur_transform++] = ngp->transform;
}

void ngp_pop_transform(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    if(ngp->cur_transform <= 0) {
        _ngp_set_error(ngp, NGP_ERROR_TRANSFORM_STACK_UNDERFLOW, "NGP transform stack underflow");
        return;
    }
    ngp->transform = ngp->transform_stack[--ngp->cur_transform];
    ngp->matrix_dirty = true;
}

void ngp_translate(ngp_context* ngp, float x, float y) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp_mat3 matrix = {
       1.0f, 0.0f,    x,
       0.0f, 1.0f,    y,
       0.0f, 0.0f, 1.0f,
    };
    ngp->transform = ngp_mat3_mul(&ngp->transform, &matrix);
    ngp->matrix_dirty = true;
}

void ngp_rotate(ngp_context* ngp, float theta) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    float sint = sinf(theta), cost = cosf(theta);
    ngp_mat3 matrix = {
       sint,  cost, 0.0f,
       cost, -sint, 0.0f,
       0.0f,  0.0f, 1.0f,
    };
    ngp->transform = ngp_mat3_mul(&ngp->transform, &matrix);
    ngp->matrix_dirty = true;
}

void ngp_rotate_at(ngp_context* ngp, float theta, float x, float y) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp_translate(ngp, x, y);
    ngp_rotate(ngp, theta);
    ngp_translate(ngp, -x, -y);
}

void ngp_scale(ngp_context* ngp, float sx, float sy) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp_mat3 matrix = {
        sx,  0.0f, 0.0f,
       0.0f,   sy, 0.0f,
       0.0f, 0.0f, 1.0f,
    };
    ngp->transform = ngp_mat3_mul(&ngp->transform, &matrix);
    ngp->matrix_dirty = true;
}

void ngp_scale_at(ngp_context* ngp, float sx, float sy, float x, float y) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp_translate(ngp, x, y);
    ngp_scale(ngp, sx, sy);
    ngp_translate(ngp, -x, -y);
}

void ngp_reset_transform(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp->transform = ngp_mat3_identity();
    ngp->matrix_dirty = true;
}

void ngp_set_color(ngp_context* ngp, float r, float g, float b, float a) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp->color = (ngp_color){r,g,b,a};
}

void ngp_reset_color(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp->color = (ngp_color){1.0f, 1.0f, 1.0f, 1.0f};
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

void ngp_viewport(ngp_context* ngp, int x, int y, int w, int h) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);

    // skip in case of the same viewport
    if(ngp->viewport.x == x && ngp->viewport.y == y &&
       ngp->viewport.w == ngp->viewport.w && ngp->viewport.h == h)
        return;

    ngp_command* cmd = _ngp_next_command(ngp);
    if(!cmd) return;
    *cmd = (ngp_command) {
        .cmd = NGP_COMMAND_VIEWPORT,
        .args.viewport = {x, y, w, h},
    };
    ngp->viewport = (ngp_irect){x, y, w, h};
    ngp->proj = ngp_mat3_proj2d(w, h);
    ngp->matrix_dirty = true;
}

void ngp_reset_viewport(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp_viewport(ngp, 0, 0, ngp->width, ngp->height);
}

void ngp_scissor(ngp_context* ngp, int x, int y, int w, int h) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);

    // skip in case of the same scissor
    if(ngp->scissor.x == x && ngp->scissor.y == y &&
       ngp->scissor.w == ngp->scissor.w && ngp->scissor.h == h)
        return;

    ngp_command* cmd = _ngp_next_command(ngp);
    if(!cmd) return;

    // coordinate scissor in viewport subspace
    ngp_irect viewport_scissor = {ngp->viewport.x + x, ngp->viewport.y + y, w, h};

    // reset scissor
    if(w == -1 && h == -1 && x == 0 && y == 0)
        viewport_scissor = (ngp_irect){0, 0, ngp->width, ngp->height};

    *cmd = (ngp_command) {
        .cmd = NGP_COMMAND_SCISSOR,
        .args.scissor = viewport_scissor,
    };
    ngp->scissor = (ngp_irect){x, y, w, h};
}

void ngp_reset_scissor(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp_scissor(ngp, 0, 0, -1, -1);
}

void ngp_reset_state(ngp_context* ngp) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);
    ngp_reset_viewport(ngp);
    ngp_reset_scissor(ngp);
    ngp_reset_transform(ngp);
    ngp_reset_color(ngp);
}

static void _ngp_update_mvp(ngp_context* ngp) {
    if(!ngp->matrix_dirty)
        return;
    ngp->mvp = ngp_mat3_mul(&ngp->proj, &ngp->transform);
    ngp->matrix_dirty = false;
}

void ngp_fill_rect(ngp_context* ngp, float x, float y, float w, float h) {
    NGP_ASSERT(ngp->init_cookie == _NGP_INIT_COOKIE);

    //TODO: skip in case outside of viewport/scissor boundings?

    // compute vertexes
    float r = x + w, b = y + h;
    ngp_vec2 tl = (ngp_vec2){x, y};
    ngp_vec2 br = (ngp_vec2){r, b};
    ngp_vec2 tr = (ngp_vec2){r, y};
    ngp_vec2 bl = (ngp_vec2){x, b};

    // apply transform
    _ngp_update_mvp(ngp);
    tl = ngp_mat3_vec2_mul(&ngp->mvp, tl);
    br = ngp_mat3_vec2_mul(&ngp->mvp, br);
    tr = ngp_mat3_vec2_mul(&ngp->mvp, tr);
    bl = ngp_mat3_vec2_mul(&ngp->mvp, bl);

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
                         memcmp(&prev_uniform->color, &ngp->color, sizeof(ngp_color)) == 0;
    if(!reuse_uniform) {
        // append new uniform
        ngp_uniform *uniform = _ngp_next_uniform(ngp);
        if(!uniform) return;
        uniform->color = ngp->color;
    }
    int uniform_index = ngp->cur_uniform - 1;

    ngp_command* prev_cmd = _ngp_prev_command(ngp);
    bool merge_cmd = prev_cmd &&
                     prev_cmd->cmd == NGP_COMMAND_DRAW &&
                     prev_cmd->args.draw.pip.id == ngp->fill_triangles_pip.id &&
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
                .pip = ngp->fill_triangles_pip,
                .vertex_index = vertex_index,
                .uniform_index = uniform_index,
                .num_vertices = 6,
            },
        };
    }
}

#endif // NANOGP_IMPL
