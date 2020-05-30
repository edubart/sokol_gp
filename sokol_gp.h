/*
sokol_gp.h - minimal efficient cross platform 2D graphics painter
https://github.com/edubart/sokol_gp
*/

#ifndef SOKOL_GP_INCLUDED
#define SOKOL_GP_INCLUDED

#ifndef SOKOL_GFX_INCLUDED
#error "Please include sokol_gfx.h before sokol_gp.h"
#endif

#include <stdbool.h>
#include <stdint.h>

#ifndef SOKOL_API_DECL
#if defined(_WIN32) && defined(SOKOL_DLL) && defined(SOKOL_IMPL)
#define SOKOL_API_DECL __declspec(dllexport)
#elif defined(_WIN32) && defined(SOKOL_DLL)
#define SOKOL_API_DECL __declspec(dllimport)
#else
#define SOKOL_API_DECL extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum sgp_error {
    SGP_NO_ERROR = 0,
    SGP_ERROR_SOKOL_INVALID,
    SGP_ERROR_VERTICES_FULL,
    SGP_ERROR_UNIFORMS_FULL,
    SGP_ERROR_COMMANDS_FULL,
    SGP_ERROR_TRANSFORM_STACK_OVERFLOW,
    SGP_ERROR_TRANSFORM_STACK_UNDERFLOW,
    SGP_ERROR_STATE_STACK_OVERFLOW,
    SGP_ERROR_STATE_STACK_UNDERFLOW
} sgp_error;

typedef enum sgp_command_type {
    SGP_COMMAND_DRAW,
    SGP_COMMAND_VIEWPORT,
    SGP_COMMAND_SCISSOR
} sgp_command_type;

typedef struct sgp_isize {
    int w, h;
} sgp_isize;

typedef struct sgp_irect {
    int x, y, w, h;
} sgp_irect;

typedef struct sgp_rect {
    float x, y, w, h;
} sgp_rect;

typedef struct sgp_vec2 {
    float x, y;
} sgp_vec2;

typedef struct sgp_line {
    sgp_vec2 a, b;
} sgp_line;

typedef struct sgp_triangle {
    sgp_vec2 a, b, c;
} sgp_triangle;

typedef struct sgp_mat3 {
    float v[3][3];
} sgp_mat3;

typedef struct sgp_color {
    float r, g, b, a;
} sgp_color;

typedef struct sgp_desc {
    int max_vertices;
    int max_commands;
} sgp_desc;

typedef struct sgp_uniform {
    sgp_color color;
} sgp_uniform;

typedef struct sgp_state {
    sgp_isize frame_size;
    sgp_irect viewport;
    sgp_irect scissor;
    sgp_mat3 proj;
    sgp_mat3 transform;
    sgp_mat3 mvp;
    sgp_uniform uniform;
    unsigned int _base_vertex;
    unsigned int _base_texvertex;
    unsigned int _base_uniform;
    unsigned int _base_command;
} sgp_state;

// setup functions
SOKOL_API_DECL bool sgp_setup(const sgp_desc* desc);
SOKOL_API_DECL void sgp_shutdown();
SOKOL_API_DECL bool sgp_is_valid();
SOKOL_API_DECL sgp_error sgp_get_error_code();
SOKOL_API_DECL const char* sgp_get_error();

// rendering functions
SOKOL_API_DECL void sgp_begin(int width, int height);
SOKOL_API_DECL void sgp_flush();
SOKOL_API_DECL void sgp_end();

// state projection functions
SOKOL_API_DECL void sgp_ortho(float left, float right, float top, float bottom);
SOKOL_API_DECL void sgp_reset_proj();

// state transform functions
SOKOL_API_DECL void sgp_push_transform();
SOKOL_API_DECL void sgp_pop_transform();
SOKOL_API_DECL void sgp_reset_transform();
SOKOL_API_DECL void sgp_translate(float x, float y);
SOKOL_API_DECL void sgp_rotate(float theta);
SOKOL_API_DECL void sgp_rotate_at(float theta, float x, float y);
SOKOL_API_DECL void sgp_scale(float sx, float sy);
SOKOL_API_DECL void sgp_scale_at(float sx, float sy, float x, float y);

// state change functions
SOKOL_API_DECL void sgp_set_color(float r, float g, float b, float a);
SOKOL_API_DECL void sgp_reset_color();
SOKOL_API_DECL void sgp_viewport(int x, int y, int w, int h);
SOKOL_API_DECL void sgp_reset_viewport();
SOKOL_API_DECL void sgp_scissor(int x, int y, int w, int h);
SOKOL_API_DECL void sgp_reset_scissor();
SOKOL_API_DECL void sgp_reset_state();

// drawing functions
SOKOL_API_DECL void sgp_draw_points(const sgp_vec2* points, unsigned int count);
SOKOL_API_DECL void sgp_draw_point(float x, float y);
SOKOL_API_DECL void sgp_draw_lines(const sgp_line* lines, unsigned int count);
SOKOL_API_DECL void sgp_draw_line(float ax, float ay, float bx, float by);
SOKOL_API_DECL void sgp_draw_line_strip(const sgp_vec2* points, unsigned int count);
SOKOL_API_DECL void sgp_draw_filled_triangles(const sgp_triangle* triangles, unsigned int count);
SOKOL_API_DECL void sgp_draw_filled_triangle(float ax, float ay, float bx, float by, float cx, float cy);
SOKOL_API_DECL void sgp_draw_filled_triangle_strip(const sgp_vec2* points, unsigned int count);
SOKOL_API_DECL void sgp_draw_filled_rects(const sgp_rect* rects, unsigned int count);
SOKOL_API_DECL void sgp_draw_filled_rect(float x, float y, float w, float h);
SOKOL_API_DECL void sgp_draw_textured_rects(sg_image image, const sgp_rect* rects, const sgp_rect* src_rects, unsigned int count);
SOKOL_API_DECL void sgp_draw_textured_rect(sg_image image, sgp_rect rect, const sgp_rect* src_rect);

// querying functions
SOKOL_API_DECL sgp_state* sgp_query_state();
SOKOL_API_DECL sgp_desc sgp_query_desc();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GP_INCLUDED

#ifdef SOKOL_GP_IMPL
#ifndef SOKOL_GP_IMPL_INCLUDED
#define SOKOL_GP_IMPL_INCLUDED

#include <string.h>
#include <math.h>
#include <stddef.h>

#ifndef SOKOL_ASSERT
#include <assert.h>
#define SOKOL_ASSERT(c) assert(c)
#endif
#ifndef SOKOL_MALLOC
#include <stdlib.h>
#define SOKOL_MALLOC(s) malloc(s)
#define SOKOL_FREE(p) free(p)
#endif
#ifndef SOKOL_DEBUG
#ifndef NDEBUG
#define SOKOL_DEBUG
#endif
#endif
#ifndef SOKOL_LOG
#ifdef SOKOL_DEBUG
#include <stdio.h>
#define SOKOL_LOG(s) { SOKOL_ASSERT(s); puts(s); }
#else
#define SOKOL_LOG(s)
#endif
#endif
#ifndef SOKOL_UNREACHABLE
#define SOKOL_UNREACHABLE SOKOL_ASSERT(false)
#endif
#ifdef __GNUC__
#define SOKOL_LIKELY(x) __builtin_expect(x, 1)
#define SOKOL_UNLIKELY(x) __builtin_expect(x, 0)
#else
#define SOKOL_LIKELY(x) (x)
#define SOKOL_UNLIKELY(x) (x)
#endif
#define SOKOL_DEF(val, def) (((val) == 0) ? (def) : (val))

#define _sgp_f2ushortn(x) (x > 1 ? 65535 : (x < 0 ? 0 : x*65535))

enum {
    _SGP_INIT_COOKIE = 0xCAFED0D,
    _SGP_DEFAULT_MAX_VERTICES = 65536,
    _SGP_DEFAULT_MAX_COMMANDS = 16384,
    _SGP_MAX_STACK_DEPTH = 64,
};

typedef struct sgp_vec2s {
    short x, y;
} sgp_vec2s;

typedef struct sgp_vec2us {
    unsigned short x, y;
} sgp_vec2us;

typedef sgp_vec2 sgp_vertex;

typedef struct sgp_texvertex {
    sgp_vec2 position;
    sgp_vec2us texcoord;
} sgp_texvertex;

typedef struct sgp_draw_args {
    sg_pipeline pip;
    sg_image img;
    unsigned int uniform_index;
    unsigned int vertex_index;
    unsigned int num_vertices;
} sgp_draw_args;

typedef union sgp_command_args {
    sgp_draw_args draw;
    sgp_irect viewport;
    sgp_irect scissor;
} sgp_command_args;

typedef struct sgp_command {
    sgp_command_type cmd;
    sgp_command_args args;
} sgp_command;

typedef struct sgp_context {
    uint32_t init_cookie;
    const char *last_error;
    sgp_error last_error_code;
    sgp_desc desc;

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
    sgp_vertex* vertices;
    sgp_texvertex* texvertices;
    sgp_uniform* uniforms;
    sgp_command* commands;

    // state tracking
    sgp_state state;

    // matrix stack
    unsigned int cur_transform;
    unsigned int cur_state;
    sgp_mat3 transform_stack[_SGP_MAX_STACK_DEPTH];
    sgp_state state_stack[_SGP_MAX_STACK_DEPTH];
} sgp_context;

static sgp_context ngp;

static const sgp_mat3 _sgp_mat3_identity = {{
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
}};

static void _sgp_set_error(sgp_error error_code, const char *error) {
    ngp.last_error_code = error_code;
    ngp.last_error = error;
    SOKOL_LOG(error);
}

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
static const char* solid_vs_source =
"float4 main(float2 position: POSITION): SV_Position {\n"
"    return float4(position, 0.0f, 1.0f);\n"
"}\n";
static const char* solid_fs_source =
"uniform float4 color;\n"
"float4 main(): SV_Target0 {\n"
"    return color;\n"
"}\n";
static const char* tex_vs_source =
"struct vs_in {\n"
"    float2 position: POSITION;\n"
"    float2 texcoord: TEXCOORD0;\n"
"};\n"
"struct vs_out {\n"
"    float2 uv: TEXCOORD0;\n"
"    float4 pos: SV_Position;\n"
"};\n"
"vs_out main(vs_in inp) {\n"
"    vs_out outp;\n"
"    outp.pos = float4(inp.position, 0.0f, 1.0f);\n"
"    outp.uv = inp.texcoord;\n"
"    return outp;\n"
"}\n";
static const char* tex_fs_source =
"Texture2D<float4> tex: register(t0);\n"
"SamplerState smp: register(s0);\n"
"uniform float4 color;\n"
"float4 main(float2 uv: TEXCOORD0): SV_Target0 {\n"
"    return tex.Sample(smp, uv) * color;\n"
"}\n";

#elif defined(SOKOL_DUMMY_BACKEND)
static const char solid_vs_source[] = "";
static const char solid_fs_source[] = "";
static const char tex_vs_source[] = "";
static const char tex_fs_source[] = "";
#endif

static void _sgp_setup_pipelines() {
    sg_blend_state default_blend = {
        .enabled = true,
        .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
        .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .src_factor_alpha = SG_BLENDFACTOR_ONE,
        .dst_factor_alpha = SG_BLENDFACTOR_ONE,
        .depth_format = SG_PIXELFORMAT_NONE,
    };

    // create shaders
    sg_shader_desc solid_shader_desc = {
        .attrs = {
            {.name="position", .sem_name="POSITION", .sem_index=0},
        },
        .vs = {.source = solid_vs_source},
        .fs = {
            .source = solid_fs_source,
            .uniform_blocks = {{
                .size = 4*sizeof(float),
                .uniforms = {{.name="color", .type=SG_UNIFORMTYPE_FLOAT4}},
            }},
        },
    };
    ngp.solid_shader = sg_make_shader(&solid_shader_desc);
    SOKOL_ASSERT(ngp.solid_shader.id != SG_INVALID_ID);

    sg_shader_desc tex_shader_desc = {
        .attrs = {
            {.name="position", .sem_name="POSITION", .sem_index=0},
            {.name="texcoord", .sem_name="TEXCOORD", .sem_index=0},
        },
        .vs = {.source = tex_vs_source},
        .fs = {
            .source = tex_fs_source,
            .uniform_blocks = {{
                .size = 4*sizeof(float),
                .uniforms = {{.name="color", .type=SG_UNIFORMTYPE_FLOAT4}},
            }},
            .images = {{.name = "tex", .type=SG_IMAGETYPE_2D}},
        },
    };
    ngp.tex_shader = sg_make_shader(&tex_shader_desc);
    SOKOL_ASSERT(ngp.tex_shader.id != SG_INVALID_ID);

    // create pipelines
    sg_pipeline_desc textriangles_pip_desc = {
        .layout = {.attrs = {
            {.offset=0, .format=SG_VERTEXFORMAT_FLOAT2},
            {.offset=offsetof(sgp_texvertex, texcoord), .format=SG_VERTEXFORMAT_USHORT2N},
        }},
        .shader = ngp.tex_shader,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .blend = default_blend,
    };
    ngp.textriangles_pip = sg_make_pipeline(&textriangles_pip_desc);
    SOKOL_ASSERT(ngp.textriangles_pip.id != SG_INVALID_ID);

    sg_pipeline_desc triangles_pip_desc = {
        .layout = {.attrs = {{.offset=0, .format=SG_VERTEXFORMAT_FLOAT2}}},
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .blend = default_blend,
    };
    ngp.triangles_pip = sg_make_pipeline(&triangles_pip_desc);
    SOKOL_ASSERT(ngp.triangles_pip.id != SG_INVALID_ID);

    sg_pipeline_desc points_pip_desc = {
        .layout = {.attrs = {{.offset=0, .format=SG_VERTEXFORMAT_FLOAT2}}},
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_POINTS,
        .blend = default_blend,
    };
    ngp.points_pip = sg_make_pipeline(&points_pip_desc);
    SOKOL_ASSERT(ngp.points_pip.id != SG_INVALID_ID);

    sg_pipeline_desc lines_pip_desc = {
        .layout = {.attrs = {{.offset=0, .format=SG_VERTEXFORMAT_FLOAT2}}},
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_LINES,
        .blend = default_blend,
    };
    ngp.lines_pip = sg_make_pipeline(&lines_pip_desc);
    SOKOL_ASSERT(ngp.lines_pip.id != SG_INVALID_ID);

    sg_pipeline_desc triangle_strip_pip_desc = {
        .layout = {.attrs = {{.offset=0, .format=SG_VERTEXFORMAT_FLOAT2}}},
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
        .blend = default_blend,
    };
    ngp.triangle_strip_pip = sg_make_pipeline(&triangle_strip_pip_desc);
    SOKOL_ASSERT(ngp.triangle_strip_pip.id != SG_INVALID_ID);

    sg_pipeline_desc line_strip_pip_desc = {
        .layout = {.attrs = {{.offset=0, .format=SG_VERTEXFORMAT_FLOAT2}}},
        .shader = ngp.solid_shader,
        .primitive_type = SG_PRIMITIVETYPE_LINE_STRIP,
        .blend = default_blend,
    };
    ngp.line_strip_pip = sg_make_pipeline(&line_strip_pip_desc);
    SOKOL_ASSERT(ngp.line_strip_pip.id != SG_INVALID_ID);
}

bool sgp_setup(const sgp_desc* desc) {
    SOKOL_ASSERT(ngp.init_cookie == 0);

    // init
    ngp.init_cookie = _SGP_INIT_COOKIE;
    ngp.last_error = "";

    // set desc default values
    ngp.desc = *desc;
    ngp.desc.max_vertices = SOKOL_DEF(desc->max_vertices, _SGP_DEFAULT_MAX_VERTICES);
    ngp.desc.max_commands = SOKOL_DEF(desc->max_commands, _SGP_DEFAULT_MAX_COMMANDS);

    // allocate buffers
    ngp.num_vertices = ngp.desc.max_vertices;
    ngp.num_commands = ngp.desc.max_commands;
    ngp.num_uniforms = ngp.desc.max_commands;
    ngp.vertices = (sgp_vertex*) SOKOL_MALLOC(ngp.num_vertices * sizeof(sgp_vertex));
    ngp.texvertices = (sgp_texvertex*) SOKOL_MALLOC(ngp.num_vertices * sizeof(sgp_texvertex));
    ngp.uniforms = (sgp_uniform*) SOKOL_MALLOC(ngp.num_uniforms * sizeof(sgp_uniform));
    ngp.commands = (sgp_command*) SOKOL_MALLOC(ngp.num_commands * sizeof(sgp_command));
    SOKOL_ASSERT(ngp.commands && ngp.uniforms && ngp.uniforms);

    // create vertex buffer
    sg_buffer_desc vertex_buf_desc = {
        .size = (int)(ngp.num_vertices * sizeof(sgp_vertex)),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_STREAM,
    };
    ngp.vertex_buf = sg_make_buffer(&vertex_buf_desc);
    SOKOL_ASSERT(ngp.vertex_buf.id != SG_INVALID_ID);

    sg_buffer_desc texvertex_buf_desc = {
        .size = (int)(ngp.num_vertices * sizeof(sgp_texvertex)),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_STREAM,
    };
    ngp.texvertex_buf = sg_make_buffer(&texvertex_buf_desc);
    SOKOL_ASSERT(ngp.texvertex_buf.id != SG_INVALID_ID);

    // define the resource bindings
    ngp.vertex_bind = (sg_bindings){
        .vertex_buffers = {ngp.vertex_buf},
    };
    ngp.texvertex_bind = (sg_bindings){
        .vertex_buffers = {ngp.texvertex_buf},
    };

    _sgp_setup_pipelines();
    return true;
}

void sgp_shutdown() {
    if(ngp.init_cookie == 0) return; // not initialized
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_FREE(ngp.vertices);
    SOKOL_FREE(ngp.texvertices);
    SOKOL_FREE(ngp.uniforms);
    SOKOL_FREE(ngp.commands);
    sg_destroy_pipeline(ngp.textriangles_pip);
    sg_destroy_pipeline(ngp.triangles_pip);
    sg_destroy_pipeline(ngp.points_pip);
    sg_destroy_pipeline(ngp.lines_pip);
    sg_destroy_pipeline(ngp.triangle_strip_pip);
    sg_destroy_pipeline(ngp.line_strip_pip);
    sg_destroy_shader(ngp.solid_shader);
    sg_destroy_buffer(ngp.vertex_buf);
    sg_destroy_buffer(ngp.texvertex_buf);
    ngp = (sgp_context){.init_cookie=0};
}

bool sgp_is_valid() {
    return ngp.init_cookie == _SGP_INIT_COOKIE;
}

sgp_error sgp_get_error_code() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    return ngp.last_error_code;
}

const char* sgp_get_error() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    return ngp.last_error;
}

static inline sgp_mat3 _sgp_default_proj(int width, int height) {
    // matrix to convert screen coordinate system
    // to the usual the coordinate system used on the backends
    return (sgp_mat3){{
        {2.0f/width,           0.0f, -1.0f},
        {      0.0f,   -2.0f/height,  1.0f},
        {      0.0f,           0.0f,  1.0f}
    }};
}

void sgp_begin(int width, int height) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(ngp.cur_state >= _SGP_MAX_STACK_DEPTH)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_OVERFLOW, "NGP state stack overflow");
        return;
    }

    // first begin
    if(ngp.cur_state == 0) {
        ngp.last_error = "";
        ngp.last_error_code = SGP_NO_ERROR;
    }

    // save current state
    ngp.state_stack[ngp.cur_state++] = ngp.state;

    // reset to default state
    ngp.state.frame_size = (sgp_isize){width, height};
    ngp.state.viewport = (sgp_irect){0, 0, width, height};
    ngp.state.scissor = (sgp_irect){0, 0, -1, -1};
    ngp.state.proj = _sgp_default_proj(width, height);
    ngp.state.transform = _sgp_mat3_identity;
    ngp.state.mvp = ngp.state.proj;
    ngp.state.uniform.color = (sgp_color){1.0f, 1.0f, 1.0f, 1.0f};
    ngp.state._base_vertex = ngp.cur_vertex;
    ngp.state._base_texvertex = ngp.cur_texvertex;
    ngp.state._base_uniform = ngp.cur_uniform;
    ngp.state._base_command = ngp.cur_command;
}

void sgp_flush() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);

    if(ngp.last_error_code != SGP_NO_ERROR || ngp.cur_command <= 0)
        return;

    // flush commands
    uint32_t cur_pip_id = SG_INVALID_ID;
    uint32_t cur_img_id = SG_INVALID_ID;
    unsigned int cur_uniform_index = (unsigned int)(-1);
    unsigned int cur_base_vertex = 0;
    unsigned int base_vertex = ngp.state._base_vertex;
    unsigned int base_texvertex = ngp.state._base_texvertex;
    unsigned int num_vertices = (ngp.cur_vertex - base_vertex) * sizeof(sgp_vertex);
    unsigned int num_texvertices = (ngp.cur_texvertex - base_texvertex) * sizeof(sgp_texvertex);
    sg_update_buffer(ngp.vertex_buf, &ngp.vertices[base_vertex], num_vertices);
    sg_update_buffer(ngp.texvertex_buf, &ngp.texvertices[base_texvertex], num_texvertices);
    for(unsigned int i = ngp.state._base_command; i < ngp.cur_command; ++i) {
        sgp_command* cmd = &ngp.commands[i];
        switch(cmd->cmd) {
            case SGP_COMMAND_VIEWPORT: {
                sgp_irect* args = &cmd->args.viewport;
                sg_apply_viewport(args->x, args->y, args->w, args->h, true);
                break;
            }
            case SGP_COMMAND_SCISSOR: {
                sgp_irect* args = &cmd->args.scissor;
                sg_apply_scissor_rect(args->x, args->y, args->w, args->h, true);
                break;
            }
            case SGP_COMMAND_DRAW: {
                sgp_draw_args* args = &cmd->args.draw;
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
                        cur_base_vertex = base_texvertex;
                    } else {
                        sg_apply_bindings(&ngp.vertex_bind);
                        cur_base_vertex = base_vertex;
                    }
                    cur_img_id = args->img.id;
                }
                if(cur_uniform_index != args->uniform_index) {
                    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &ngp.uniforms[args->uniform_index], sizeof(sgp_uniform));
                    cur_uniform_index = args->uniform_index;
                }
                if(args->num_vertices > 0) {
                    sg_draw(args->vertex_index - cur_base_vertex, args->num_vertices, 1);
                }
                break;
            }
            default: {
                SOKOL_UNREACHABLE;
                break;
            }
        }
    }

    // rewind indexes
    ngp.cur_vertex = ngp.state._base_vertex;
    ngp.cur_texvertex = ngp.state._base_texvertex;
    ngp.cur_uniform = ngp.state._base_uniform;
    ngp.cur_command = ngp.state._base_command;
}

void sgp_end() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(ngp.cur_state <= 0)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_UNDERFLOW, "NGP state stack underflow");
        return;
    }

    // restore old state
    ngp.state = ngp.state_stack[--ngp.cur_state];
}

static inline sgp_mat3 _sgp_mat3_mul(const sgp_mat3* a, const sgp_mat3* b) {
    sgp_mat3 p;
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

static inline sgp_mat3 _sgp_mul_proj_transform(sgp_mat3* proj, sgp_mat3* transform) {
    // this actually multiply matrix projection and transform matrix in an optimized way
    // the effect is the same as "return _sgp_mat3_mul(proj, transform);"
    float x = proj->v[0][0], y = proj->v[1][1];
    float tx = proj->v[0][2], ty = proj->v[1][2];
    float a = transform->v[2][0], b = transform->v[2][1], c = transform->v[2][2];
    return (sgp_mat3) {{
        {x*transform->v[0][0]+a*tx, x*transform->v[0][1]+b*tx, x*transform->v[0][2]+c*tx},
        {y*transform->v[1][0]+a*ty, y*transform->v[1][1]+b*ty, y*transform->v[1][2]+c*ty},
        {a, b, c}
    }};
}

void sgp_ortho(float left, float right, float top, float bottom) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    float w = right - left;
    float h = top - bottom;
    ngp.state.proj = (sgp_mat3){{
        {2.0f/w,   0.0f,  -(right+left)/w},
        {0.0f,   2.0f/h,  -(top+bottom)/h},
        {0.0f,     0.0f,             1.0f}
    }};
    ngp.state.mvp = _sgp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void sgp_reset_proj() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    ngp.state.proj = _sgp_default_proj(ngp.state.viewport.w, ngp.state.viewport.h);
    ngp.state.mvp = _sgp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void sgp_push_transform() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(ngp.cur_transform >= _SGP_MAX_STACK_DEPTH)) {
        _sgp_set_error(SGP_ERROR_TRANSFORM_STACK_OVERFLOW, "NGP transform stack overflow");
        return;
    }
    ngp.transform_stack[ngp.cur_transform++] = ngp.state.transform;
}

void sgp_pop_transform() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(ngp.cur_transform <= 0)) {
        _sgp_set_error(SGP_ERROR_TRANSFORM_STACK_UNDERFLOW, "NGP transform stack underflow");
        return;
    }
    ngp.state.transform = ngp.transform_stack[--ngp.cur_transform];
    ngp.state.mvp = _sgp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void sgp_reset_transform() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    ngp.state.transform = _sgp_mat3_identity;
    ngp.state.mvp = _sgp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void sgp_translate(float x, float y) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    // multiply by translate matrix:
    // 1.0f, 0.0f,    x,
    // 0.0f, 1.0f,    y,
    // 0.0f, 0.0f, 1.0f,
    ngp.state.transform.v[0][2] += x*ngp.state.transform.v[0][0] + y*ngp.state.transform.v[0][1];
    ngp.state.transform.v[1][2] += x*ngp.state.transform.v[1][0] + y*ngp.state.transform.v[1][1];
    ngp.state.transform.v[2][2] += x*ngp.state.transform.v[2][0] + y*ngp.state.transform.v[2][1];
    ngp.state.mvp = _sgp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void sgp_rotate(float theta) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    float sint = sinf(theta), cost = cosf(theta);
    // multiply by rotation matrix:
    // sint,  cost, 0.0f,
    // cost, -sint, 0.0f,
    // 0.0f,  0.0f, 1.0f,
    ngp.state.transform = (sgp_mat3){{
       {sint*ngp.state.transform.v[0][0]+cost*ngp.state.transform.v[0][1], cost*ngp.state.transform.v[0][0]-sint*ngp.state.transform.v[0][1], ngp.state.transform.v[0][2]},
       {sint*ngp.state.transform.v[1][0]+cost*ngp.state.transform.v[1][1], cost*ngp.state.transform.v[1][0]-sint*ngp.state.transform.v[1][1], ngp.state.transform.v[1][2]},
       {sint*ngp.state.transform.v[2][0]+cost*ngp.state.transform.v[2][1], cost*ngp.state.transform.v[2][0]-sint*ngp.state.transform.v[2][1], ngp.state.transform.v[2][2]}
    }};
    ngp.state.mvp = _sgp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void sgp_rotate_at(float theta, float x, float y) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_translate(x, y);
    sgp_rotate(theta);
    sgp_translate(-x, -y);
}

void sgp_scale(float sx, float sy) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    // multiply by scale matrix:
    //   sx, 0.0f, 0.0f,
    // 0.0f,   sy, 0.0f,
    // 0.0f, 0.0f, 1.0f,
    ngp.state.transform.v[0][0] *= sx;
    ngp.state.transform.v[1][1] *= sy;
    ngp.state.mvp = _sgp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void sgp_scale_at(float sx, float sy, float x, float y) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_translate(x, y);
    sgp_scale(sx, sy);
    sgp_translate(-x, -y);
}

void sgp_set_color(float r, float g, float b, float a) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    ngp.state.uniform.color = (sgp_color){r,g,b,a};
}

void sgp_reset_color() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    ngp.state.uniform.color = (sgp_color){1.0f, 1.0f, 1.0f, 1.0f};
}

static sgp_vertex* _sgp_next_vertices(unsigned int count) {
    if(SOKOL_LIKELY(ngp.cur_vertex + count <= ngp.num_vertices)) {
        sgp_vertex *vertices = &ngp.vertices[ngp.cur_vertex];
        ngp.cur_vertex += count;
        return vertices;
    } else {
        _sgp_set_error(SGP_ERROR_VERTICES_FULL, "NGP vertices buffer is full");
        return NULL;
    }
}

static sgp_uniform* _sgp_prev_uniform() {
    if(SOKOL_LIKELY(ngp.cur_uniform > 0)) {
        return &ngp.uniforms[ngp.cur_uniform-1];
    } else {
        return NULL;
    }
}

static sgp_uniform* _sgp_next_uniform() {
    if(SOKOL_LIKELY(ngp.cur_uniform < ngp.num_uniforms)) {
        return &ngp.uniforms[ngp.cur_uniform++];
    } else {
        _sgp_set_error(SGP_ERROR_UNIFORMS_FULL, "NGP uniform buffer is full");
        return NULL;
    }
}

static sgp_command* _sgp_prev_command() {
    if(SOKOL_LIKELY(ngp.cur_command > 0)) {
        return &ngp.commands[ngp.cur_command-1];
    } else {
        return NULL;
    }
}

static sgp_command* _sgp_next_command() {
    if(SOKOL_LIKELY(ngp.cur_command < ngp.num_commands)) {
        return &ngp.commands[ngp.cur_command++];
    } else {
        _sgp_set_error(SGP_ERROR_COMMANDS_FULL, "NGP command buffer is full");
        return NULL;
    }
}

void sgp_viewport(int x, int y, int w, int h) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);

    // skip in case of the same viewport
    if(ngp.state.viewport.x == x && ngp.state.viewport.y == y &&
       ngp.state.viewport.w == w && ngp.state.viewport.h == h)
        return;

    sgp_command* cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) return;
    *cmd = (sgp_command) {
        .cmd = SGP_COMMAND_VIEWPORT,
        .args = {.viewport = {x, y, w, h}},
    };

    // adjust current scissor relative offset
    if(!(ngp.state.scissor.w == -1 && ngp.state.scissor.h == -1 && ngp.state.scissor.x == 0 && ngp.state.scissor.y == 0)) {
        ngp.state.scissor.x += x - ngp.state.viewport.x;
        ngp.state.scissor.y += y - ngp.state.viewport.y;
    }

    ngp.state.viewport = (sgp_irect){x, y, w, h};
    ngp.state.proj = _sgp_default_proj(w, h);
    ngp.state.mvp = _sgp_mul_proj_transform(&ngp.state.proj, &ngp.state.transform);
}

void sgp_reset_viewport() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_viewport(0, 0, ngp.state.frame_size.w, ngp.state.frame_size.h);
}

void sgp_scissor(int x, int y, int w, int h) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);

    // skip in case of the same scissor
    if(ngp.state.scissor.x == x && ngp.state.scissor.y == y &&
       ngp.state.scissor.w == w && ngp.state.scissor.h == h)
        return;

    sgp_command* cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) return;

    // coordinate scissor in viewport subspace
    sgp_irect viewport_scissor = {ngp.state.viewport.x + x, ngp.state.viewport.y + y, w, h};

    // reset scissor
    if(w == -1 && h == -1 && x == 0 && y == 0)
        viewport_scissor = (sgp_irect){0, 0, ngp.state.frame_size.w, ngp.state.frame_size.h};

    *cmd = (sgp_command) {
        .cmd = SGP_COMMAND_SCISSOR,
        .args = {.scissor = viewport_scissor},
    };
    ngp.state.scissor = (sgp_irect){x, y, w, h};
}

void sgp_reset_scissor() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_scissor(0, 0, -1, -1);
}

void sgp_reset_state() {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_reset_viewport();
    sgp_reset_scissor();
    sgp_reset_transform();
    sgp_reset_color();
}

static inline bool sgp_color_eq(sgp_color a, sgp_color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static void _sgp_queue_draw(sg_pipeline pip, unsigned int vertex_index, unsigned int num_vertices, sg_image img) {
    // setup uniform, try to reuse previous uniform when possible
    sgp_uniform *prev_uniform = _sgp_prev_uniform();
    bool reuse_uniform = prev_uniform && memcmp(prev_uniform, &ngp.state.uniform, sizeof(sgp_uniform)) == 0;
    if(!reuse_uniform) {
        // append new uniform
        sgp_uniform *uniform = _sgp_next_uniform();
        if(SOKOL_UNLIKELY(!uniform)) return;
        *uniform = ngp.state.uniform;
    }
    unsigned int uniform_index = ngp.cur_uniform - 1;

    sgp_command* prev_cmd = _sgp_prev_command();
    bool merge_cmd = prev_cmd &&
                     prev_cmd->cmd == SGP_COMMAND_DRAW &&
                     prev_cmd->args.draw.pip.id == pip.id &&
                     prev_cmd->args.draw.img.id == img.id &&
                     prev_cmd->args.draw.uniform_index == uniform_index;
    if(merge_cmd) {
        // merge command for batched rendering
        prev_cmd->args.draw.num_vertices += num_vertices;
    } else {
        // append new draw command
        sgp_command* cmd = _sgp_next_command();
        if(SOKOL_UNLIKELY(!cmd)) return;
        cmd->cmd = SGP_COMMAND_DRAW,
        cmd->args.draw = (sgp_draw_args){
            .pip = pip,
            .img = img,
            .uniform_index = uniform_index,
            .vertex_index = vertex_index,
            .num_vertices = num_vertices,
        };
    }
}

static inline sgp_vec2 sgp_mat3_vec2_mul(const sgp_mat3* m, const sgp_vec2* v) {
    return (sgp_vec2){
        .x = m->v[0][0]*v->x + m->v[0][1]*v->y + m->v[0][2],
        .y = m->v[1][0]*v->x + m->v[1][1]*v->y + m->v[1][2]
    };
}

static inline void _sgp_transform_vertices(sgp_mat3* matrix, sgp_vertex* dest, const sgp_vec2 *src, unsigned int count) {
    for(unsigned int i=0;i<count;++i)
        dest[i] = sgp_mat3_vec2_mul(matrix, &src[i]);
}

static void _sgp_draw_solid_pip(sg_pipeline pip, const sgp_vec2* vertices, unsigned int count) {
    unsigned int vertex_index = ngp.cur_vertex;
    sgp_vertex* transformed_vertices = _sgp_next_vertices(count);
    if(SOKOL_UNLIKELY(!vertices)) return;

    sgp_mat3 mvp = ngp.state.mvp; // copy to stack for more efficiency
    _sgp_transform_vertices(&mvp, transformed_vertices, vertices, count);
    _sgp_queue_draw(pip, vertex_index, count, (sg_image){0});
}

void sgp_draw_points(const sgp_vec2* points, unsigned int count) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(count == 0) return;
    _sgp_draw_solid_pip(ngp.points_pip, points, count);
}

void sgp_draw_point(float x, float y) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_vec2 point = {x, y};
    sgp_draw_points(&point, 1);
}

void sgp_draw_lines(const sgp_line* lines, unsigned int count) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(count == 0) return;
    _sgp_draw_solid_pip(ngp.lines_pip, (const sgp_vec2*)lines, count*2);
}

void sgp_draw_line(float ax, float ay, float bx, float by) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_line line = {{ax,ay},{bx, by}};
    sgp_draw_lines(&line, 1);
}

void sgp_draw_line_strip(const sgp_vec2* points, unsigned int count) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(count == 0) return;
    _sgp_draw_solid_pip(ngp.line_strip_pip, points, count);
}

void sgp_draw_filled_triangles(const sgp_triangle* triangles, unsigned int count) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(count == 0) return;
    _sgp_draw_solid_pip(ngp.triangles_pip, (const sgp_vec2*)triangles, count*3);
}

void sgp_draw_filled_triangle(float ax, float ay, float bx, float by, float cx, float cy) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_triangle triangle = {{ax,ay},{bx, by},{cx, cy}};
    sgp_draw_filled_triangles(&triangle, 1);
}

void sgp_draw_filled_triangle_strip(const sgp_vec2* points, unsigned int count) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(count == 0) return;
    _sgp_draw_solid_pip(ngp.triangle_strip_pip, points, count);
}

void sgp_draw_filled_rects(const sgp_rect* rects, unsigned int count) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(count == 0) return;

    // setup vertices
    unsigned int num_vertices = count * 6;
    unsigned int vertex_index = ngp.cur_vertex;
    sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    sgp_vertex* v = vertices;
    const sgp_rect* rect = rects;
    sgp_mat3 mvp = ngp.state.mvp; // copy to stack for more efficiency
    for(unsigned int i=0;i<count;v+=6, i++, rect++) {
        float l = rect->x, t = rect->y;
        float r = l + rect->w, b = t + rect->h;
        sgp_vec2 quad[4] = {
            {l, b}, // bottom left
            {r, b}, // bottom right
            {r, t}, // top right
            {l, t}, // top left
        };
        _sgp_transform_vertices(&mvp, quad, quad, 4);

        // make a quad composed of 2 triangles
        v[0] = quad[0];
        v[1] = quad[1];
        v[2] = quad[2];
        v[3] = quad[3];
        v[4] = quad[0];
        v[5] = quad[2];
    }

    _sgp_queue_draw(ngp.triangles_pip, vertex_index, num_vertices, (sg_image){0});
}

void sgp_draw_filled_rect(float x, float y, float w, float h) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_rect rect = {x,y,w,h};
    sgp_draw_filled_rects(&rect, 1);
}

static sgp_texvertex* _sgp_next_texvertices(unsigned int count) {
    if(SOKOL_LIKELY(ngp.cur_texvertex + count <= ngp.num_vertices)) {
        sgp_texvertex *texvertices = &ngp.texvertices[ngp.cur_texvertex];
        ngp.cur_texvertex += count;
        return texvertices;
    } else {
        _sgp_set_error(SGP_ERROR_VERTICES_FULL, "NGP vertices buffer is full");
        return NULL;
    }
}

void sgp_draw_textured_rects(sg_image image, const sgp_rect* rects, const sgp_rect* src_rects, unsigned int count) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    if(count == 0 || image.id == SG_INVALID_ID) return;

    // setup vertices
    unsigned int num_vertices = count * 6;
    unsigned int vertex_index = ngp.cur_texvertex;
    sgp_texvertex* texvertices = _sgp_next_texvertices(num_vertices);
    if(SOKOL_UNLIKELY(!texvertices)) return;

    // compute vertices
    sgp_texvertex* v = texvertices;
    const sgp_rect* rect = rects;
    const sgp_rect* src_rect = src_rects;
    sgp_mat3 mvp = ngp.state.mvp; // copy to stack for more efficiency

    // split in two branches outside the loop so the compiler can vectorize better
    if(!src_rect) {
        for(unsigned int i=0;i<count;v+=6, i++, rect++) {
            float l = rect->x, t = rect->y;
            float r = l + rect->w, b = t + rect->h;
            sgp_vec2 quad[4] = {
                {l, b}, // bottom left
                {r, b}, // bottom right
                {r, t}, // top right
                {l, t}, // top left
            };
            _sgp_transform_vertices(&mvp, quad, quad, 4);

            // make a quad composed of 2 triangles
            const sgp_vec2us vtexquad[4] = {
                {    0, 65535}, // bottom left
                {65535, 65535}, // bottom right
                {65535,     0}, // top right
                {    0,     0}, // top left
            };
            v[0] = (sgp_texvertex){quad[0], vtexquad[0]};
            v[1] = (sgp_texvertex){quad[1], vtexquad[1]};
            v[2] = (sgp_texvertex){quad[2], vtexquad[2]};
            v[3] = (sgp_texvertex){quad[3], vtexquad[3]};
            v[4] = (sgp_texvertex){quad[0], vtexquad[0]};
            v[5] = (sgp_texvertex){quad[2], vtexquad[2]};
        }
    } else {
        //TODO: benchmark this query and optimize
        float iw = 0.0f, ih = 0.0f;
        sg_image_info info = sg_query_image_info(image);
        if(SOKOL_LIKELY(info.width != 0 && info.height != 0)) {
            iw = 1.0f/info.width;
            ih = 1.0f/info.height;
        }

        for(unsigned int i=0;i<count;v+=6, i++, rect++, src_rect++) {
            float l = rect->x, t = rect->y;
            float r = l + rect->w, b = t + rect->h;
            sgp_vec2 quad[4] = {
                {l, b}, // bottom left
                {r, b}, // bottom right
                {r, t}, // top right
                {l, t}, // top left
            };
            _sgp_transform_vertices(&mvp, quad, quad, 4);

            // make a quad composed of 2 triangles
            unsigned short tl = _sgp_f2ushortn(src_rect->x*iw), tt = _sgp_f2ushortn(src_rect->y*ih);
            unsigned short tr = _sgp_f2ushortn(l + src_rect->w*iw), tb = _sgp_f2ushortn(t + src_rect->h*ih);
            sgp_vec2us vtexquad[4] = {
                {tl, tb}, // bottom left
                {tr, tb}, // bottom right
                {tr, tt}, // top right
                {tl, tt}, // top left
            };
            v[0] = (sgp_texvertex){quad[0], vtexquad[0]};
            v[1] = (sgp_texvertex){quad[1], vtexquad[1]};
            v[2] = (sgp_texvertex){quad[2], vtexquad[2]};
            v[3] = (sgp_texvertex){quad[3], vtexquad[3]};
            v[4] = (sgp_texvertex){quad[0], vtexquad[0]};
            v[5] = (sgp_texvertex){quad[2], vtexquad[2]};
        }
    }

    _sgp_queue_draw(ngp.textriangles_pip, vertex_index, num_vertices, image);
}

void sgp_draw_textured_rect(sg_image image, sgp_rect rect, const sgp_rect* src_rect) {
    SOKOL_ASSERT(ngp.init_cookie == _SGP_INIT_COOKIE);
    sgp_draw_textured_rects(image, &rect, src_rect, 1);
}

sgp_desc sgp_query_desc() {
    return ngp.desc;
}

sgp_state* sgp_query_state() {
    return &ngp.state;
}

#endif // SOKOL_GP_IMPL_INCLUDED
#endif // SOKOL_GP_IMPL
