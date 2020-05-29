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

#ifndef NANOGP_INCLUDED
#define NANOGP_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#ifndef NANOGP_API
#define NANOGP_API extern
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

typedef struct ngp_isize {
    int w, h;
} ngp_isize;

typedef struct ngp_irect {
    int x, y, w, h;
} ngp_irect;

typedef struct ngp_rect {
    float x, y, w, h;
} ngp_rect;

typedef struct ngp_vec2 {
    float x, y;
} ngp_vec2;

typedef struct ngp_line {
    ngp_vec2 a, b;
} ngp_line;

typedef struct ngp_triangle {
    ngp_vec2 a, b, c;
} ngp_triangle;

typedef ngp_vec2 ngp_vertex;

typedef struct ngp_texvertex {
    ngp_vec2 position;
    ngp_vec2 texcoord;
} ngp_texvertex;

typedef struct ngp_mat3 {
    float v[3][3];
} ngp_mat3;

typedef struct ngp_color {
    float r, g, b, a;
} ngp_color;

typedef struct ngp_desc {
    int max_vertices;
    int max_commands;
} ngp_desc;

typedef struct ngp_state {
    ngp_isize frame_size;
    ngp_irect viewport;
    ngp_irect scissor;
    ngp_mat3 proj;
    ngp_mat3 transform;
    ngp_mat3 mvp;
    ngp_color color;
} ngp_state;

// setup functions
NANOGP_API bool ngp_setup(const ngp_desc* desc);
NANOGP_API void ngp_shutdown();
NANOGP_API bool ngp_is_valid();
NANOGP_API ngp_error ngp_get_error_code();
NANOGP_API const char* ngp_get_error();

// rendering functions
NANOGP_API void ngp_begin(int width, int height);
NANOGP_API void ngp_end();
NANOGP_API void ngp_set_clear_color(float r, float g, float b, float a);
NANOGP_API void ngp_reset_clear_color();

// state projection functions
NANOGP_API void ngp_ortho(float left, float right, float top, float bottom);
NANOGP_API void ngp_reset_proj();

// state transform functions
NANOGP_API void ngp_push_transform();
NANOGP_API void ngp_pop_transform();
NANOGP_API void ngp_reset_transform();
NANOGP_API void ngp_translate(float x, float y);
NANOGP_API void ngp_rotate(float theta);
NANOGP_API void ngp_rotate_at(float theta, float x, float y);
NANOGP_API void ngp_scale(float sx, float sy);
NANOGP_API void ngp_scale_at(float sx, float sy, float x, float y);

// state change functions
NANOGP_API void ngp_set_color(float r, float g, float b, float a);
NANOGP_API void ngp_reset_color();
NANOGP_API void ngp_viewport(int x, int y, int w, int h);
NANOGP_API void ngp_reset_viewport();
NANOGP_API void ngp_scissor(int x, int y, int w, int h);
NANOGP_API void ngp_reset_scissor();
NANOGP_API void ngp_reset_state();

// drawing functions
NANOGP_API void ngp_draw_points(const ngp_vec2* points, unsigned int count);
NANOGP_API void ngp_draw_point(float x, float y);
NANOGP_API void ngp_draw_lines(const ngp_line* lines, unsigned int count);
NANOGP_API void ngp_draw_line(float ax, float ay, float bx, float by);
NANOGP_API void ngp_draw_triangles(const ngp_triangle* triangles, unsigned int count);
NANOGP_API void ngp_draw_triangle(float ax, float ay, float bx, float by, float cx, float cy);
NANOGP_API void ngp_draw_rects(const ngp_rect* rects, unsigned int count);
NANOGP_API void ngp_draw_rect(float x, float y, float w, float h);
NANOGP_API void ngp_draw_line_strip(const ngp_vec2* points, unsigned int count);
NANOGP_API void ngp_draw_triangle_strip(const ngp_vec2* points, unsigned int count);

NANOGP_API void ngp_draw_textured_rects(sg_image image, const ngp_rect* rects, const ngp_rect* src_rects, unsigned int count);
NANOGP_API void ngp_draw_textured_rect(sg_image image, ngp_rect rect, const ngp_rect* src_rect);

// querying functions
NANOGP_API ngp_state* ngp_query_state();
NANOGP_API ngp_desc ngp_query_desc();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NANOGP_INCLUDED

#ifdef NANOGP_IMPL
#ifndef NANOGP_IMPL_INCLUDED
#define NANOGP_IMPL_INCLUDED

#include <stdlib.h>
#include <math.h>

#ifndef NANOGP_ASSERT
#include <assert.h>
#define NANOGP_ASSERT(c) assert(c)
#endif
#ifndef NANOGP_MALLOC
#include <stdlib.h>
#define NANOGP_MALLOC(s) malloc(s)
#define NANOGP_FREE(p) free(p)
#endif
#ifndef NDEBUG
#define NANOGP_DEBUG
#endif
#ifndef NANOGP_LOG
#ifdef NANOGP_DEBUG
#include <stdio.h>
#define NANOGP_LOG(s) { NANOGP_ASSERT(s); puts(s); }
#else
#define NANOGP_LOG(s)
#endif
#endif
#ifndef NANOGP_UNREACHABLE
#define NANOGP_UNREACHABLE NANOGP_ASSERT(false)
#endif
#ifdef __GNUC__
#define NANOGP_LIKELY(x) __builtin_expect(x, 1)
#define NANOGP_UNLIKELY(x) __builtin_expect(x, 0)
#else
#define NANOGP_LIKELY(x) (x)
#define NANOGP_UNLIKELY(x) (x)
#endif
#define NANOGP_DEF(val, def) (((val) == 0) ? (def) : (val))

enum {
    _NGP_INIT_COOKIE = 0xCAFED00D,
    _NGP_DEFAULT_MAX_VERTICES = 65536,
    _NGP_DEFAULT_MAX_COMMANDS = 16384,
    _NGP_MAX_STACK_DEPTH = 64,
};

typedef struct ngp_draw_args {
    sg_pipeline pip;
    sg_image img;
    unsigned int uniform_index;
    unsigned int vertex_index;
    unsigned int num_vertices;
} ngp_draw_args;

typedef union ngp_command_args {
    ngp_draw_args draw;
    ngp_irect viewport;
    ngp_irect scissor;
} ngp_command_args;

typedef struct ngp_uniform {
    ngp_color color;
} ngp_uniform;

typedef struct ngp_command {
    ngp_command_type cmd;
    ngp_command_args args;
} ngp_command;

typedef struct ngp_context {
    uint32_t init_cookie;
    const char *last_error;
    ngp_error last_error_code;
    ngp_desc desc;

    // default render pass
    sg_pass_action pass_action;

    // resources
    sg_shader solid_shader;
    sg_shader tex_shader;
    sg_buffer vertex_buf;
    sg_buffer texvertex_buf;
    sg_bindings vertex_bind;
    sg_bindings texvertex_bind;
    sg_pipeline textriangles_pip;
    sg_pipeline triangles_pip;
    sg_pipeline points_pip;
    sg_pipeline lines_pip;
    sg_pipeline triangle_strip_pip;
    sg_pipeline line_strip_pip;

    // command queue
    unsigned int cur_vertex;
    unsigned int cur_texvertex;
    unsigned int cur_uniform;
    unsigned int cur_command;
    unsigned int num_vertices;
    unsigned int num_uniforms;
    unsigned int num_commands;
    ngp_vertex* vertices;
    ngp_texvertex* texvertices;
    ngp_uniform* uniforms;
    ngp_command* commands;

    // state tracking
    ngp_state state;

    // matrix stack
    unsigned int cur_transform;
    ngp_mat3 transform_stack[_NGP_MAX_STACK_DEPTH];
} ngp_context;

static ngp_context ngp;

static const ngp_mat3 _ngp_mat3_identity = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
};

static void _ngp_set_error(ngp_error error_code, const char *error) {
    ngp.last_error_code = error_code;
    ngp.last_error = error;
    NANOGP_LOG(error);
}

/*
@vs vs
layout(location=0) in vec2 position;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
@end

@fs fs
out vec4 frag_color;
uniform vec4 color;
void main() {
    frag_color = color;
}
@end

@program shd vs fs
*/
#if defined(SOKOL_GLCORE33)
static const char* solid_vs_source =
"#version 330\n"
"layout(location=0) in vec2 position;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"}\n";
static const char* solid_fs_source =
"#version 330\n"
"uniform vec4 color;\n"
"out vec4 frag_color;\n"
"void main() {\n"
"    frag_color = color;\n"
"}\n";
static const char* tex_vs_source =
"#version 330\n"
"layout(location=0) in vec2 position;\n"
"layout(location=1) in vec2 texcoord;\n"
"out vec2 uv;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"    uv = texcoord;\n"
"}\n";
static const char* tex_fs_source =
"#version 330\n"
"uniform sampler2D tex;\n"
"uniform vec4 color;\n"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
"void main() {\n"
"    frag_color = texture(tex, uv) * color;\n"
"}\n";

#elif defined(SOKOL_D3D11)
static const char solid_fs_source[338] = {
    0x75,0x6e,0x69,0x66,0x6f,0x72,0x6d,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x63,
    0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,
    0x6f,0x61,0x74,0x34,0x20,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x3b,
    0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,
    0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x0a,0x7b,0x0a,0x20,0x20,
    0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,
    0x6c,0x6f,0x72,0x20,0x3a,0x20,0x53,0x56,0x5f,0x54,0x61,0x72,0x67,0x65,0x74,0x30,
    0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,0x39,0x20,0x22,0x22,
    0x0a,0x76,0x6f,0x69,0x64,0x20,0x66,0x72,0x61,0x67,0x5f,0x6d,0x61,0x69,0x6e,0x28,
    0x29,0x0a,0x7b,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,0x39,0x20,0x22,0x22,0x0a,0x20,
    0x20,0x20,0x20,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x20,0x3d,0x20,
    0x63,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x7d,0x0a,0x0a,0x53,0x50,0x49,0x52,0x56,0x5f,
    0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x6d,0x61,0x69,
    0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,0x72,0x61,0x67,0x5f,0x6d,
    0x61,0x69,0x6e,0x28,0x29,0x3b,0x0a,0x20,0x20,0x20,0x20,0x53,0x50,0x49,0x52,0x56,
    0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x73,0x74,
    0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x3b,0x0a,0x20,0x20,0x20,0x20,
    0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x2e,0x66,0x72,0x61,
    0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x20,0x3d,0x20,0x66,0x72,0x61,0x67,0x5f,0x63,
    0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x20,0x20,0x20,0x20,0x72,0x65,0x74,0x75,0x72,0x6e,
    0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x3b,0x0a,0x7d,
    0x0a,0x00,
};
static const char solid_vs_source[498] = {
    0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x67,0x6c,
    0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,
    0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,
    0x6e,0x3b,0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,0x56,
    0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,0x74,0x0a,0x7b,0x0a,0x20,
    0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x70,0x6f,0x73,0x69,0x74,0x69,
    0x6f,0x6e,0x20,0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,0x44,0x30,0x3b,0x0a,
    0x7d,0x3b,0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,0x56,
    0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x67,0x6c,0x5f,0x50,0x6f,
    0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3a,0x20,0x53,0x56,0x5f,0x50,0x6f,0x73,0x69,
    0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,
    0x38,0x20,0x22,0x22,0x0a,0x76,0x6f,0x69,0x64,0x20,0x76,0x65,0x72,0x74,0x5f,0x6d,
    0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,0x38,0x20,
    0x22,0x22,0x0a,0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,
    0x6f,0x6e,0x20,0x3d,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x28,0x70,0x6f,0x73,0x69,
    0x74,0x69,0x6f,0x6e,0x2c,0x20,0x30,0x2e,0x30,0x66,0x2c,0x20,0x31,0x2e,0x30,0x66,
    0x29,0x3b,0x0a,0x7d,0x0a,0x0a,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,
    0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x6d,0x61,0x69,0x6e,0x28,0x53,0x50,
    0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,0x74,0x20,
    0x73,0x74,0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x29,0x0a,0x7b,0x0a,0x20,
    0x20,0x20,0x20,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x73,0x74,
    0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x2e,0x70,0x6f,0x73,0x69,0x74,0x69,
    0x6f,0x6e,0x3b,0x0a,0x20,0x20,0x20,0x20,0x76,0x65,0x72,0x74,0x5f,0x6d,0x61,0x69,
    0x6e,0x28,0x29,0x3b,0x0a,0x20,0x20,0x20,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,
    0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x73,0x74,0x61,0x67,
    0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x3b,0x0a,0x20,0x20,0x20,0x20,0x73,0x74,
    0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x2e,0x67,0x6c,0x5f,0x50,0x6f,
    0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,
    0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x20,0x20,0x20,0x20,0x72,0x65,0x74,0x75,0x72,0x6e,
    0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x3b,0x0a,0x7d,
    0x0a,0x00,
};
#elif defined(SOKOL_DUMMY_BACKEND)
static const char solid_vs_source[] = "";
static const char solid_fs_source[] = "";
#endif

static void _ngp_setup_pipelines() {
    // create shaders
    ngp.solid_shader = sg_make_shader(&(sg_shader_desc){
        .vs.source = solid_vs_source,
        .fs.source = solid_fs_source,
        .fs.uniform_blocks[0] = {
            .uniforms[0] = {.name="color", .type=SG_UNIFORMTYPE_FLOAT4},
            .size = 4*sizeof(float),
        },
        .attrs = {
            [0] = {.name="position", .sem_name="TEXCOORD"},
        },
    });
    NANOGP_ASSERT(ngp.solid_shader.id != SG_INVALID_ID);
    ngp.tex_shader = sg_make_shader(&(sg_shader_desc){
        .vs.source = tex_vs_source,
        .fs.source = tex_fs_source,
        .fs.uniform_blocks[0] = {
            .uniforms[0] = {.name="color", .type=SG_UNIFORMTYPE_FLOAT4},
            .size = 4*sizeof(float),
        },
        .fs.images[0] = {.name = "tex", .type=SG_IMAGETYPE_2D},
        .attrs = {
            [0] = {.name="position", .sem_name="TEXCOORD"},
            [1] = {.name="texcoord", .sem_name="TEXCOORD"},
        },
    });
    NANOGP_ASSERT(ngp.tex_shader.id != SG_INVALID_ID);

    // create pipelines
    ngp.textriangles_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = ngp.tex_shader,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .layout.attrs[0] = { .offset=0, .format=SG_VERTEXFORMAT_FLOAT2 },
        .layout.attrs[1] = { .offset=offsetof(ngp_texvertex, texcoord), .format=SG_VERTEXFORMAT_FLOAT2 },
    });
    NANOGP_ASSERT(ngp.textriangles_pip.id != SG_INVALID_ID);
    ngp.triangles_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .layout.attrs[0] = { .offset=0, .format=SG_VERTEXFORMAT_FLOAT2 },
    });
    NANOGP_ASSERT(ngp.triangles_pip.id != SG_INVALID_ID);
    ngp.points_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_POINTS,
        .layout.attrs[0] = { .offset=0, .format=SG_VERTEXFORMAT_FLOAT2 },
    });
    NANOGP_ASSERT(ngp.points_pip.id != SG_INVALID_ID);
    ngp.lines_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_LINES,
        .layout.attrs[0] = { .offset=0, .format=SG_VERTEXFORMAT_FLOAT2 },
    });
    NANOGP_ASSERT(ngp.lines_pip.id != SG_INVALID_ID);
    ngp.triangle_strip_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
        .layout.attrs[0] = { .offset=0, .format=SG_VERTEXFORMAT_FLOAT2 },
    });
    NANOGP_ASSERT(ngp.triangle_strip_pip.id != SG_INVALID_ID);
    ngp.line_strip_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_LINE_STRIP,
        .layout.attrs[0] = { .offset=0, .format=SG_VERTEXFORMAT_FLOAT2 },
    });
    NANOGP_ASSERT(ngp.line_strip_pip.id != SG_INVALID_ID);
}

bool ngp_setup(const ngp_desc* desc) {
    NANOGP_ASSERT(ngp.init_cookie == 0);

    // init
    ngp.init_cookie = _NGP_INIT_COOKIE;
    ngp.last_error = "";

    // set desc default values
    ngp.desc = *desc;
    ngp.desc.max_vertices = NANOGP_DEF(desc->max_vertices, _NGP_DEFAULT_MAX_VERTICES);
    ngp.desc.max_commands = NANOGP_DEF(desc->max_commands, _NGP_DEFAULT_MAX_COMMANDS);

    // allocate buffers
    ngp.num_vertices = ngp.desc.max_vertices;
    ngp.num_commands = ngp.desc.max_commands;
    ngp.num_uniforms = ngp.desc.max_commands;
    ngp.vertices = (ngp_vertex*) NANOGP_MALLOC(ngp.num_vertices * sizeof(ngp_vertex));
    ngp.texvertices = (ngp_texvertex*) NANOGP_MALLOC(ngp.num_vertices * sizeof(ngp_texvertex));
    ngp.uniforms = (ngp_uniform*) NANOGP_MALLOC(ngp.num_uniforms * sizeof(ngp_uniform));
    ngp.commands = (ngp_command*) NANOGP_MALLOC(ngp.num_commands * sizeof(ngp_command));
    NANOGP_ASSERT(ngp.commands && ngp.uniforms && ngp.uniforms);

    // create vertex buffer
    ngp.vertex_buf = sg_make_buffer(&(sg_buffer_desc){
        .size = 65536 * sizeof(ngp_vertex),
        .usage = SG_USAGE_STREAM,
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
    });
    NANOGP_ASSERT(ngp.vertex_buf.id != SG_INVALID_ID);
    ngp.texvertex_buf = sg_make_buffer(&(sg_buffer_desc){
        .size = 65536 * sizeof(ngp_texvertex),
        .usage = SG_USAGE_STREAM,
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
    });
    NANOGP_ASSERT(ngp.texvertex_buf.id != SG_INVALID_ID);

    // define the resource bindings
    ngp.vertex_bind = (sg_bindings){
        .vertex_buffers[0] = ngp.vertex_buf,
    };
    ngp.texvertex_bind = (sg_bindings){
        .vertex_buffers[0] = ngp.texvertex_buf,
    };

    // disable default render clear color
    ngp.pass_action = (sg_pass_action){
        .stencil = {.action = SG_ACTION_DONTCARE },
        .depth = {.action = SG_ACTION_DONTCARE }
    };
    for(unsigned int i=0;i<SG_MAX_COLOR_ATTACHMENTS;++i) {
        ngp.pass_action.colors[i] = (sg_color_attachment_action) {
            .action = SG_ACTION_DONTCARE,
        };
    }

    _ngp_setup_pipelines();
    return true;
}

void ngp_shutdown() {
    if(ngp.init_cookie == 0) return; // not initialized
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    NANOGP_FREE(ngp.vertices);
    NANOGP_FREE(ngp.texvertices);
    NANOGP_FREE(ngp.uniforms);
    NANOGP_FREE(ngp.commands);
    sg_destroy_pipeline(ngp.textriangles_pip);
    sg_destroy_pipeline(ngp.triangles_pip);
    sg_destroy_pipeline(ngp.points_pip);
    sg_destroy_pipeline(ngp.lines_pip);
    sg_destroy_pipeline(ngp.triangle_strip_pip);
    sg_destroy_pipeline(ngp.line_strip_pip);
    sg_destroy_shader(ngp.solid_shader);
    sg_destroy_buffer(ngp.vertex_buf);
    sg_destroy_buffer(ngp.texvertex_buf);
    ngp = (ngp_context){0};
}

bool ngp_is_valid() {
    return ngp.init_cookie == _NGP_INIT_COOKIE;
}

ngp_error ngp_get_error_code() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    return ngp.last_error_code;
}

const char* ngp_get_error() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    return ngp.last_error;
}

static inline ngp_mat3 _ngp_default_proj(int width, int height) {
    // matrix to convert screen coordinate system
    // to the usual the coordinate system used on the backends
    return (ngp_mat3){
        2.0f/width,           0.0f, -1.0f,
              0.0f,   -2.0f/height,  1.0f,
              0.0f,           0.0f,  1.0f
    };
}

void ngp_begin(int width, int height) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    sg_begin_default_pass(&ngp.pass_action, width, height);

    // default state
    ngp.state.frame_size = (ngp_isize){width, height};
    ngp.state.viewport = (ngp_irect){0, 0, width, height};
    ngp.state.scissor = (ngp_irect){0, 0, -1, -1};
    ngp.state.proj = _ngp_default_proj(width, height);
    ngp.state.transform = _ngp_mat3_identity;
    ngp.state.mvp = ngp.state.proj;
    ngp.state.color = (ngp_color){1.0f, 1.0f, 1.0f, 1.0f};
}

static void _ngp_rewind() {
    ngp.last_error = "";
    ngp.last_error_code = NGP_NO_ERROR;
    ngp.cur_vertex = 0;
    ngp.cur_texvertex = 0;
    ngp.cur_uniform = 0;
    ngp.cur_command = 0;
}

static void _ngp_flush_commands() {
    if(ngp.last_error_code != NGP_NO_ERROR || ngp.cur_command <= 0)
        return;

    uint32_t cur_pip_id = SG_INVALID_ID;
    uint32_t cur_img_id = SG_INVALID_ID;
    int cur_uniform_index = -1;
    sg_update_buffer(ngp.vertex_buf, ngp.vertices, ngp.cur_vertex * sizeof(ngp_vertex));
    sg_update_buffer(ngp.texvertex_buf, ngp.texvertices, ngp.cur_texvertex * sizeof(ngp_texvertex));
    for(unsigned int i = 0; i < ngp.cur_command; ++i) {
        ngp_command* cmd = &ngp.commands[i];
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
                    cur_img_id = SG_INVALID_ID;
                    cur_uniform_index = -1;
                    pip_changed = true;
                }
                if(pip_changed || cur_img_id != args->img.id) {
                    if(args->img.id != SG_INVALID_ID) {
                        ngp.texvertex_bind.fs_images[0] = args->img;
                        sg_apply_bindings(&ngp.texvertex_bind);
                    } else {
                        sg_apply_bindings(&ngp.vertex_bind);
                    }
                    cur_img_id = args->img.id;
                }
                if(cur_uniform_index != args->uniform_index) {
                    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &ngp.uniforms[args->uniform_index], sizeof(ngp_uniform));
                    cur_uniform_index = args->uniform_index;
                }
                if(args->num_vertices > 0) {
                    sg_draw(args->vertex_index, args->num_vertices, 1);
                }
                break;
            }
            default: {
                NANOGP_UNREACHABLE;
                break;
            }
        }
    }
    _ngp_rewind();
}

void ngp_end() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    _ngp_flush_commands();
    sg_end_pass();
    sg_commit();
}

void ngp_set_clear_color(float r, float g, float b, float a) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    for(unsigned int i=0;i<SG_MAX_COLOR_ATTACHMENTS;++i) {
        ngp.pass_action.colors[i] = (sg_color_attachment_action) {
            .action = SG_ACTION_CLEAR,
            .val = {r,g,b,a}
        };
    }
}

void ngp_reset_clear_color() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    for(unsigned int i=0;i<SG_MAX_COLOR_ATTACHMENTS;++i) {
        ngp.pass_action.colors[i] = (sg_color_attachment_action) {
            .action = SG_ACTION_DONTCARE,
        };
    }
}

static inline ngp_mat3 _ngp_mat3_mul(const ngp_mat3* a, const ngp_mat3* b) {
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

static inline ngp_mat3 _ngp_mul_proj_transform(ngp_mat3* proj, ngp_mat3* transform) {
    // this actually multiply matrix projection and transform matrix in an optimized way
    // the effect is the same as "return _ngp_mat3_mul(proj, transform);"
    float x = proj->v[0][0], y = proj->v[1][1];
    float tx = proj->v[0][2], ty = proj->v[1][2];
    float a = transform->v[2][0], b = transform->v[2][1], c = transform->v[2][2];
    return (ngp_mat3) {
        x*transform->v[0][0]+a*tx, x*transform->v[0][1]+b*tx, x*transform->v[0][2]+c*tx,
        y*transform->v[1][0]+a*ty, y*transform->v[1][1]+b*ty, y*transform->v[1][2]+c*ty,
        a, b, c
    };
};

void ngp_ortho(float left, float right, float top, float bottom) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    float w = right - left;
    float h = top - bottom;
    ngp.state.proj = (ngp_mat3){
        2.0f/w,   0.0f,  -(right+left)/w,
        0.0f,   2.0f/h,  -(top+bottom)/h,
        0.0f,     0.0f,             1.0f
    };
    ngp.state.mvp = _ngp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void ngp_reset_proj() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp.state.proj = _ngp_default_proj(ngp.state.viewport.w, ngp.state.viewport.h);
    ngp.state.mvp = _ngp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void ngp_push_transform() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    if(NANOGP_UNLIKELY(ngp.cur_transform >= _NGP_MAX_STACK_DEPTH)) {
        _ngp_set_error(NGP_ERROR_TRANSFORM_STACK_OVERFLOW, "NGP transform stack overflow");
        return;
    }
    ngp.transform_stack[ngp.cur_transform++] = ngp.state.transform;
}

void ngp_pop_transform() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    if(NANOGP_UNLIKELY(ngp.cur_transform <= 0)) {
        _ngp_set_error(NGP_ERROR_TRANSFORM_STACK_UNDERFLOW, "NGP transform stack underflow");
        return;
    }
    ngp.state.transform = ngp.transform_stack[--ngp.cur_transform];
    ngp.state.mvp = _ngp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void ngp_reset_transform() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp.state.transform = _ngp_mat3_identity;
    ngp.state.mvp = _ngp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void ngp_translate(float x, float y) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    // multiply by translate matrix:
    // 1.0f, 0.0f,    x,
    // 0.0f, 1.0f,    y,
    // 0.0f, 0.0f, 1.0f,
    ngp.state.transform.v[0][2] += x*ngp.state.transform.v[0][0] + y*ngp.state.transform.v[0][1];
    ngp.state.transform.v[1][2] += x*ngp.state.transform.v[1][0] + y*ngp.state.transform.v[1][1];
    ngp.state.transform.v[2][2] += x*ngp.state.transform.v[2][0] + y*ngp.state.transform.v[2][1];
    ngp.state.mvp = _ngp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void ngp_rotate(float theta) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    float sint = sinf(theta), cost = cosf(theta);
    // multiply by rotation matrix:
    // sint,  cost, 0.0f,
    // cost, -sint, 0.0f,
    // 0.0f,  0.0f, 1.0f,
    ngp.state.transform = (ngp_mat3){
       sint*ngp.state.transform.v[0][0]+cost*ngp.state.transform.v[0][1], cost*ngp.state.transform.v[0][0]-sint*ngp.state.transform.v[0][1], ngp.state.transform.v[0][2],
       sint*ngp.state.transform.v[1][0]+cost*ngp.state.transform.v[1][1], cost*ngp.state.transform.v[1][0]-sint*ngp.state.transform.v[1][1], ngp.state.transform.v[1][2],
       sint*ngp.state.transform.v[2][0]+cost*ngp.state.transform.v[2][1], cost*ngp.state.transform.v[2][0]-sint*ngp.state.transform.v[2][1], ngp.state.transform.v[2][2],
    };
    ngp.state.mvp = _ngp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void ngp_rotate_at(float theta, float x, float y) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_translate(x, y);
    ngp_rotate(theta);
    ngp_translate(-x, -y);
}

void ngp_scale(float sx, float sy) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    // multiply by scale matrix:
    //   sx, 0.0f, 0.0f,
    // 0.0f,   sy, 0.0f,
    // 0.0f, 0.0f, 1.0f,
    ngp.state.transform.v[0][0] *= sx;
    ngp.state.transform.v[1][1] *= sy;
    ngp.state.mvp = _ngp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void ngp_scale_at(float sx, float sy, float x, float y) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_translate(x, y);
    ngp_scale(sx, sy);
    ngp_translate(-x, -y);
}

void ngp_set_color(float r, float g, float b, float a) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp.state.color = (ngp_color){r,g,b,a};
}

void ngp_reset_color() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp.state.color = (ngp_color){1.0f, 1.0f, 1.0f, 1.0f};
}

static ngp_vertex* _ngp_next_vertices(unsigned int count) {
    if(NANOGP_LIKELY(ngp.cur_vertex + count <= ngp.num_vertices)) {
        ngp_vertex *vertices = &ngp.vertices[ngp.cur_vertex];
        ngp.cur_vertex += count;
        return vertices;
    } else {
        _ngp_set_error(NGP_ERROR_VERTICES_FULL, "NGP vertices buffer is full");
        return NULL;
    }
}

static ngp_uniform* _ngp_prev_uniform() {
    if(NANOGP_LIKELY(ngp.cur_uniform > 0)) {
        return &ngp.uniforms[ngp.cur_uniform-1];
    } else {
        return NULL;
    }
}

static ngp_uniform* _ngp_next_uniform() {
    if(NANOGP_LIKELY(ngp.cur_uniform < ngp.num_uniforms)) {
        return &ngp.uniforms[ngp.cur_uniform++];
    } else {
        _ngp_set_error(NGP_ERROR_UNIFORMS_FULL, "NGP uniform buffer is full");
        return NULL;
    }
}

static ngp_command* _ngp_prev_command() {
    if(NANOGP_LIKELY(ngp.cur_command > 0)) {
        return &ngp.commands[ngp.cur_command-1];
    } else {
        return NULL;
    }
}

static ngp_command* _ngp_next_command() {
    if(NANOGP_LIKELY(ngp.cur_command < ngp.num_commands)) {
        return &ngp.commands[ngp.cur_command++];
    } else {
        _ngp_set_error(NGP_ERROR_COMMANDS_FULL, "NGP command buffer is full");
        return NULL;
    }
}

void ngp_viewport(int x, int y, int w, int h) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);

    // skip in case of the same viewport
    if(ngp.state.viewport.x == x && ngp.state.viewport.y == y &&
       ngp.state.viewport.w == w && ngp.state.viewport.h == h)
        return;

    ngp_command* cmd = _ngp_next_command();
    if(NANOGP_UNLIKELY(!cmd)) return;
    *cmd = (ngp_command) {
        .cmd = NGP_COMMAND_VIEWPORT,
        .args.viewport = {x, y, w, h},
    };

    // adjust current scissor relative offset
    if(!(ngp.state.scissor.w == -1 && ngp.state.scissor.h == -1 && ngp.state.scissor.x == 0 && ngp.state.scissor.y == 0)) {
        ngp.state.scissor.x += x - ngp.state.viewport.x;
        ngp.state.scissor.y += y - ngp.state.viewport.y;
    }

    ngp.state.viewport = (ngp_irect){x, y, w, h};
    ngp.state.proj = _ngp_default_proj(w, h);
    ngp.state.mvp = _ngp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void ngp_reset_viewport() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_viewport(0, 0, ngp.state.frame_size.w, ngp.state.frame_size.h);
}

void ngp_scissor(int x, int y, int w, int h) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);

    // skip in case of the same scissor
    if(ngp.state.scissor.x == x && ngp.state.scissor.y == y &&
       ngp.state.scissor.w == w && ngp.state.scissor.h == h)
        return;

    ngp_command* cmd = _ngp_next_command();
    if(NANOGP_UNLIKELY(!cmd)) return;

    // coordinate scissor in viewport subspace
    ngp_irect viewport_scissor = {ngp.state.viewport.x + x, ngp.state.viewport.y + y, w, h};

    // reset scissor
    if(w == -1 && h == -1 && x == 0 && y == 0)
        viewport_scissor = (ngp_irect){0, 0, ngp.state.frame_size.w, ngp.state.frame_size.h};

    *cmd = (ngp_command) {
        .cmd = NGP_COMMAND_SCISSOR,
        .args.scissor = viewport_scissor,
    };
    ngp.state.scissor = (ngp_irect){x, y, w, h};
}

void ngp_reset_scissor() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_scissor(0, 0, -1, -1);
}

void ngp_reset_state() {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_reset_viewport();
    ngp_reset_scissor();
    ngp_reset_transform();
    ngp_reset_color();
}

static inline bool ngp_color_eq(ngp_color a, ngp_color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static void _ngp_queue_draw(sg_pipeline pip, unsigned int vertex_index, unsigned int num_vertices, sg_image img) {
    // setup uniform, try to reuse previous uniform when possible
    ngp_uniform *prev_uniform = _ngp_prev_uniform();
    bool reuse_uniform = prev_uniform && ngp_color_eq(prev_uniform->color, ngp.state.color);
    if(!reuse_uniform) {
        // append new uniform
        ngp_uniform *uniform = _ngp_next_uniform();
        if(NANOGP_UNLIKELY(!uniform)) return;
        uniform->color = ngp.state.color;
    }
    unsigned int uniform_index = ngp.cur_uniform - 1;

    ngp_command* prev_cmd = _ngp_prev_command();
    bool merge_cmd = prev_cmd &&
                     prev_cmd->cmd == NGP_COMMAND_DRAW &&
                     prev_cmd->args.draw.pip.id == pip.id &&
                     prev_cmd->args.draw.img.id == img.id &&
                     prev_cmd->args.draw.uniform_index == uniform_index;
    if(merge_cmd) {
        // merge command for batched rendering
        prev_cmd->args.draw.num_vertices += num_vertices;
    } else {
        // append new draw command
        ngp_command* cmd = _ngp_next_command();
        if(NANOGP_UNLIKELY(!cmd)) return;
        cmd->cmd = NGP_COMMAND_DRAW,
        cmd->args.draw = (ngp_draw_args){
            .pip = pip,
            .img = img,
            .uniform_index = uniform_index,
            .vertex_index = vertex_index,
            .num_vertices = num_vertices,
        };
    }
}

static inline ngp_vec2 ngp_mat3_vec2_mul(const ngp_mat3* m, const ngp_vec2* v) {
    return (ngp_vec2){
        .x = m->v[0][0]*v->x + m->v[0][1]*v->y + m->v[0][2],
        .y = m->v[1][0]*v->x + m->v[1][1]*v->y + m->v[1][2]
    };
}

static void _ngp_transform_vertices(ngp_mat3 matrix, ngp_vec2* dest, const ngp_vec2 *src, unsigned int count) {
    for(unsigned int i=0;i<count;++i)
        dest[i] = ngp_mat3_vec2_mul(&matrix, &src[i]);
}

static void _ngp_draw_solid_pip(sg_pipeline pip, const ngp_vec2* vertices, unsigned int count) {
    unsigned int vertex_index = ngp.cur_vertex;
    ngp_vertex* transformed_vertices = _ngp_next_vertices(count);
    if(NANOGP_UNLIKELY(!vertices)) return;

    _ngp_transform_vertices(ngp.state.mvp, transformed_vertices, vertices, count);
    _ngp_queue_draw(pip, vertex_index, count, (sg_image){0});
}

void ngp_draw_triangles(const ngp_triangle* triangles, unsigned int count) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    _ngp_draw_solid_pip(ngp.triangles_pip, (const ngp_vec2*)triangles, count*3);
}

void ngp_draw_triangle(float ax, float ay, float bx, float by, float cx, float cy) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_draw_triangles(&(ngp_triangle){{ax,ay},{bx, by},{cx, cy}}, 1);
}

void ngp_draw_rects(const ngp_rect* rects, unsigned int count) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);

    // setup vertices
    unsigned int num_vertices = count * 6;
    unsigned int vertex_index = ngp.cur_vertex;
    ngp_vertex* vertices = _ngp_next_vertices(num_vertices);
    if(NANOGP_UNLIKELY(!vertices)) return;

    // compute vertices
    ngp_vertex* v = vertices;
    const ngp_rect* rect = rects;
    for(unsigned int i=0;i<count;v+=6, i++, rect++) {
        float l = rect->x, t = rect->y;
        float r = l + rect->w, b = t + rect->h;
        ngp_vec2 quad[4] = {
            {l, b}, // bottom left
            {r, b}, // bottom right
            {r, t}, // top right
            {l, t}, // top left
        };
        _ngp_transform_vertices(ngp.state.mvp, quad, quad, 4);

        // make a quad composed of 2 triangles
        v[0] = quad[0];
        v[1] = quad[1];
        v[2] = quad[2];
        v[3] = quad[3];
        v[4] = quad[0];
        v[5] = quad[2];
    }

    _ngp_queue_draw(ngp.triangles_pip, vertex_index, num_vertices, (sg_image){0});
}

void ngp_draw_rect(float x, float y, float w, float h) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_draw_rects(&(ngp_rect){x,y,w,h}, 1);
}

void ngp_draw_points(const ngp_vec2* points, unsigned int count) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    _ngp_draw_solid_pip(ngp.points_pip, points, count);
}

void ngp_draw_point(float x, float y) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_draw_points(&(ngp_vec2){x, y}, 1);
}

void ngp_draw_lines(const ngp_line* lines, unsigned int count) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    _ngp_draw_solid_pip(ngp.lines_pip, (const ngp_vec2*)lines, count*2);
}

void ngp_draw_line(float ax, float ay, float bx, float by) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_draw_lines(&(ngp_line){{ax,ay},{bx, by}}, 1);
}

void ngp_draw_line_strip(const ngp_vec2* points, unsigned int count) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    _ngp_draw_solid_pip(ngp.line_strip_pip, points, count);
}

void ngp_draw_triangle_strip(const ngp_vec2* points, unsigned int count) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    _ngp_draw_solid_pip(ngp.triangle_strip_pip, points, count);
}

static ngp_texvertex* _ngp_next_texvertices(unsigned int count) {
    if(NANOGP_LIKELY(ngp.cur_texvertex + count <= ngp.num_vertices)) {
        ngp_texvertex *texvertices = &ngp.texvertices[ngp.cur_texvertex];
        ngp.cur_texvertex += count;
        return texvertices;
    } else {
        _ngp_set_error(NGP_ERROR_VERTICES_FULL, "NGP vertices buffer is full");
        return NULL;
    }
}

void ngp_draw_textured_rects(sg_image image, const ngp_rect* rects, const ngp_rect* src_rects, unsigned int count) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    NANOGP_ASSERT(image.id != SG_INVALID_ID);

    // setup vertices
    unsigned int num_vertices = count * 6;
    unsigned int vertex_index = ngp.cur_texvertex;
    ngp_texvertex* texvertices = _ngp_next_texvertices(num_vertices);
    if(NANOGP_UNLIKELY(!texvertices)) return;

    float iw = 0.0f, ih = 0.0f;
    if(src_rects) {
        //TODO: benchmark this query and optimize
        sg_image_info info = sg_query_image_info(image);
        if(NANOGP_LIKELY(info.width != 0 && info.height != 0)) {
            iw = 1.0f/info.width;
            ih = 1.0f/info.height;
        }
    }

    // compute vertices
    ngp_texvertex* v = texvertices;
    const ngp_rect* rect = rects;
    const ngp_rect* src_rect = src_rects;
    for(unsigned int i=0;i<count;v+=6, i++, rect++) {
        float l = rect->x, t = rect->y;
        float r = l + rect->w, b = t + rect->h;
        ngp_vec2 quad[4] = {
            {l, b}, // bottom left
            {r, b}, // bottom right
            {r, t}, // top right
            {l, t}, // top left
        };
        _ngp_transform_vertices(ngp.state.mvp, quad, quad, 4);

        ngp_vec2 texquad[4];
        if(src_rect) {
            float tl = src_rect->x*iw, tt = src_rect->y*ih;
            float tr = l + src_rect->w*iw, tb = t + src_rect->h*ih;
            texquad[0] = (ngp_vec2){tl, tb}; // bottom left
            texquad[1] = (ngp_vec2){tr, tb}; // bottom right
            texquad[2] = (ngp_vec2){tr, tt}; // top right
            texquad[3] = (ngp_vec2){tl, tt}; // top left
            src_rect++;
        } else {
            texquad[0] = (ngp_vec2){0.0f, 1.0f}; // bottom left
            texquad[1] = (ngp_vec2){1.0f, 1.0f}; // bottom right
            texquad[2] = (ngp_vec2){1.0f, 0.0f}; // top right
            texquad[3] = (ngp_vec2){0.0f, 0.0f}; // top left
        }

        // make a quad composed of 2 triangles
        v[0] = (ngp_texvertex){quad[0], texquad[0]};
        v[1] = (ngp_texvertex){quad[1], texquad[1]};
        v[2] = (ngp_texvertex){quad[2], texquad[2]};
        v[3] = (ngp_texvertex){quad[3], texquad[3]};
        v[4] = (ngp_texvertex){quad[0], texquad[0]};
        v[5] = (ngp_texvertex){quad[2], texquad[2]};
    }

    _ngp_queue_draw(ngp.textriangles_pip, vertex_index, num_vertices, image);
}

void ngp_draw_textured_rect(sg_image image, ngp_rect rect, const ngp_rect* src_rect) {
    NANOGP_ASSERT(ngp.init_cookie == _NGP_INIT_COOKIE);
    ngp_draw_textured_rects(image, &rect, src_rect, 1);
}

ngp_desc ngp_query_desc() {
    return ngp.desc;
}

ngp_state* ngp_query_state() {
    return &ngp.state;
}

#endif // NANOGP_IMPL_INCLUDED
#endif // NANOGP_IMPL
