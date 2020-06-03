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

typedef enum sgp_blend_mode {
    SGP_BLENDMODE_NONE = 0, /* no blending
                               dstRGBA = srcRGBA */
    SGP_BLENDMODE_BLEND,    /* alpha blending
                               dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))
                               dstA = srcA + (dstA * (1-srcA)) */
    SGP_BLENDMODE_ADD,      /* additive blending
                               dstRGB = (srcRGB * srcA) + dstRGB
                               dstA = dstA */
    SGP_BLENDMODE_MOD,      /* color modulate
                               dstRGB = srcRGB * dstRGB
                               dstA = dstA */
    SGP_BLENDMODE_MUL,      /* color multiply
                               dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA))
                               dstA = (srcA * dstA) + (dstA * (1-srcA)) */
    _SGP_BLENDMODE_NUM
} sgp_blend_mode;

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

typedef struct sgp_textured_rect {
    sgp_rect dst;
    sgp_rect src;
} sgp_textured_rect;

typedef struct sgp_vec2 {
    float x, y;
} sgp_vec2;

typedef sgp_vec2 sgp_point;

typedef struct sgp_line {
    sgp_point a, b;
} sgp_line;

typedef struct sgp_triangle {
    sgp_point a, b, c;
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

typedef struct sgp_pipeline {
    uint32_t id;
} sgp_pipeline;

typedef struct sgp_state {
    sgp_isize frame_size;
    sgp_irect viewport;
    sgp_irect scissor;
    sgp_mat3 proj;
    sgp_mat3 transform;
    sgp_mat3 mvp;
    sgp_uniform uniform;
    sgp_blend_mode blend_mode;
    uint32_t _base_vertex;
    uint32_t _base_uniform;
    uint32_t _base_command;
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
//SOKOL_API_DECL void sgp_set_pipeline(sg_pipeline pip);
//SOKOL_API_DECL void sgp_reset_pipeline();
SOKOL_API_DECL void sgp_set_blend_mode(sgp_blend_mode blend_mode);
SOKOL_API_DECL void sgp_reset_blend_mode();
SOKOL_API_DECL void sgp_set_color(float r, float g, float b, float a);
SOKOL_API_DECL void sgp_reset_color();
SOKOL_API_DECL void sgp_viewport(int x, int y, int w, int h);
SOKOL_API_DECL void sgp_reset_viewport();
SOKOL_API_DECL void sgp_scissor(int x, int y, int w, int h);
SOKOL_API_DECL void sgp_reset_scissor();
SOKOL_API_DECL void sgp_reset_state();

// drawing functions
SOKOL_API_DECL void sgp_clear();
SOKOL_API_DECL void sgp_draw_points(const sgp_point* points, uint32_t count);
SOKOL_API_DECL void sgp_draw_point(float x, float y);
SOKOL_API_DECL void sgp_draw_lines(const sgp_line* lines, uint32_t count);
SOKOL_API_DECL void sgp_draw_line(float ax, float ay, float bx, float by);
SOKOL_API_DECL void sgp_draw_line_strip(const sgp_point* points, uint32_t count);
SOKOL_API_DECL void sgp_draw_filled_triangles(const sgp_triangle* triangles, uint32_t count);
SOKOL_API_DECL void sgp_draw_filled_triangle(float ax, float ay, float bx, float by, float cx, float cy);
SOKOL_API_DECL void sgp_draw_filled_triangle_strip(const sgp_point* points, uint32_t count);
SOKOL_API_DECL void sgp_draw_filled_rects(const sgp_rect* rects, uint32_t count);
SOKOL_API_DECL void sgp_draw_filled_rect(float x, float y, float w, float h);
SOKOL_API_DECL void sgp_draw_textured_rects(sg_image image, const sgp_rect* rects, uint32_t count);
SOKOL_API_DECL void sgp_draw_textured_rect(sg_image image, float x, float y, float w, float h);
SOKOL_API_DECL void sgp_draw_textured_rects_ex(sg_image image, const sgp_textured_rect* rects, uint32_t count);
SOKOL_API_DECL void sgp_draw_textured_rect_ex(sg_image image, sgp_rect dest_rect, sgp_rect src_rect);

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

#ifndef SOKOL_GFX_IMPL_INCLUDED
#error "Please include sokol_gfx.h implementation before sokol_gp.h implementation"
#endif

#include <string.h>
#include <math.h>
#include <stddef.h>

#ifndef SOKOL_LIKELY
#ifdef __GNUC__
#define SOKOL_LIKELY(x) __builtin_expect(x, 1)
#define SOKOL_UNLIKELY(x) __builtin_expect(x, 0)
#else
#define SOKOL_LIKELY(x) (x)
#define SOKOL_UNLIKELY(x) (x)
#endif
#endif

enum {
    _SGP_INIT_COOKIE = 0xCAFED0D,
    _SGP_DEFAULT_MAX_VERTICES = 65536,
    _SGP_DEFAULT_MAX_COMMANDS = 16384,
    _SGP_MAX_STACK_DEPTH = 64,
};

typedef struct _sgp_vertex {
    sgp_vec2 position;
    sgp_vec2 texcoord;
} _sgp_vertex;

typedef struct _sgp_draw_args {
    sg_pipeline pip;
    sg_image img;
    uint32_t uniform_index;
    uint32_t vertex_index;
    uint32_t num_vertices;
} _sgp_draw_args;

typedef union _sgp_command_args {
    _sgp_draw_args draw;
    sgp_irect viewport;
    sgp_irect scissor;
} _sgp_command_args;

typedef struct _sgp_command {
    sgp_command_type cmd;
    _sgp_command_args args;
} _sgp_command;

typedef struct _sgp_context {
    uint32_t init_cookie;
    const char *last_error;
    sgp_error last_error_code;
    sgp_desc desc;

    // resources
    sg_shader shader;
    sg_buffer vertex_buf;
    sg_image white_img;
    sg_pipeline pipelines[_SG_PRIMITIVETYPE_NUM*_SGP_BLENDMODE_NUM];

    // command queue
    uint32_t cur_vertex;
    uint32_t cur_uniform;
    uint32_t cur_command;
    uint32_t num_vertices;
    uint32_t num_uniforms;
    uint32_t num_commands;
    _sgp_vertex* vertices;
    sgp_uniform* uniforms;
    _sgp_command* commands;

    // state tracking
    sgp_state state;

    // matrix stack
    uint32_t cur_transform;
    uint32_t cur_state;
    sgp_mat3 transform_stack[_SGP_MAX_STACK_DEPTH];
    sgp_state state_stack[_SGP_MAX_STACK_DEPTH];
} _sgp_context;

static _sgp_context _sgp;

static const sgp_mat3 _sgp_mat3_identity = {{
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
}};

static void _sgp_set_error(sgp_error error_code, const char *error) {
    _sgp.last_error_code = error_code;
    _sgp.last_error = error;
    SOKOL_LOG(error);
}

#if defined(SOKOL_GLCORE33)
static const char* _sgp_vs_source =
"#version 330\n"
"layout(location=0) in vec4 coord;\n"
"out vec2 uv;\n"
"void main() {\n"
"    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
"    uv = coord.zw;\n"
"}\n";
static const char* _sgp_fs_source =
"#version 330\n"
"uniform sampler2D tex;\n"
"uniform vec4 color;\n"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
"void main() {\n"
"    frag_color = texture(tex, uv) * color;\n"
"}\n";

#elif defined(SOKOL_D3D11)
static const char* _sgp_vs_source =
"struct vs_out {\n"
"    float2 uv: TEXCOORD0;\n"
"    float4 pos: SV_Position;\n"
"};\n"
"vs_out main(float4 coord: POSITION) {\n"
"    vs_out outp;\n"
"    outp.pos = float4(coord.xy, 0.0f, 1.0f);\n"
"    outp.uv = coord.zw;\n"
"    return outp;\n"
"}\n";
static const char* _sgp_fs_source =
"Texture2D<float4> tex: register(t0);\n"
"SamplerState smp: register(s0);\n"
"uniform float4 color;\n"
"float4 main(float2 uv: TEXCOORD0): SV_Target0 {\n"
"    return tex.Sample(smp, uv) * color;\n"
"}\n";

#elif defined(SOKOL_DUMMY_BACKEND)
static const char _sgp_vs_source[] = "";
static const char _sgp_fs_source[] = "";
#endif

static sg_pipeline _sgp_lookup_pipeline(sg_primitive_type prim_type, sgp_blend_mode blend_mode) {
    uint32_t pip_index = (prim_type * _SGP_BLENDMODE_NUM) +
                         blend_mode;
    if(_sgp.pipelines[pip_index].id != SG_INVALID_ID)
        return _sgp.pipelines[pip_index];

    sg_blend_state blend;
    switch(blend_mode) {
        case SGP_BLENDMODE_NONE:
            blend = (sg_blend_state) {
                .enabled = false,
                .src_factor_rgb = SG_BLENDFACTOR_ONE,
                .dst_factor_rgb = SG_BLENDFACTOR_ZERO,
                .src_factor_alpha = SG_BLENDFACTOR_ONE,
                .dst_factor_alpha = SG_BLENDFACTOR_ZERO,
            };
            break;
        case SGP_BLENDMODE_BLEND:
            blend = (sg_blend_state) {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .src_factor_alpha = SG_BLENDFACTOR_ONE,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            };
            break;
        case SGP_BLENDMODE_ADD:
            blend = (sg_blend_state) {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE,
                .src_factor_alpha = SG_BLENDFACTOR_ZERO,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE,
            };
            break;
        case SGP_BLENDMODE_MOD:
            blend = (sg_blend_state) {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_DST_COLOR,
                .dst_factor_rgb = SG_BLENDFACTOR_ZERO,
                .src_factor_alpha = SG_BLENDFACTOR_ZERO,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE,

            };
            break;
        case SGP_BLENDMODE_MUL:
            blend = (sg_blend_state) {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_DST_COLOR,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .src_factor_alpha = SG_BLENDFACTOR_DST_ALPHA,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            };
            break;
        default:
            blend = (sg_blend_state) {.enabled = false};
            SOKOL_UNREACHABLE;
            break;
    }
    blend.depth_format = SG_PIXELFORMAT_NONE;

    // create pipeline
    sg_pipeline_desc pip_desc = {
        .layout = {
            .buffers = {
                {.stride=sizeof(_sgp_vertex)},
            },
            .attrs = {
                {.offset=0, .format=SG_VERTEXFORMAT_FLOAT4},
            },
        },
        .shader = _sgp.shader,
        .primitive_type = prim_type,
        .blend = blend
    };
    sg_pipeline pip = sg_make_pipeline(&pip_desc);
    SOKOL_ASSERT(pip.id != SG_INVALID_ID);
    _sgp.pipelines[pip_index] = pip;
    return pip;
};

static void _sgp_setup_pipelines() {
    // create shaders
    sg_shader_desc shader_desc = {
        .attrs = {
            {.name="coord", .sem_name="POSITION", .sem_index=0},
        },
        .vs = {.source = _sgp_vs_source},
        .fs = {
            .source = _sgp_fs_source,
            .uniform_blocks = {{
                .size = 4*sizeof(float),
                .uniforms = {{.name="color", .type=SG_UNIFORMTYPE_FLOAT4}},
            }},
            .images = {{.name = "tex", .type=SG_IMAGETYPE_2D}},
        },
    };
    _sgp.shader = sg_make_shader(&shader_desc);
    SOKOL_ASSERT(_sgp.shader.id != SG_INVALID_ID);

    // create common pipelines
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_BLEND);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_POINTS, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_POINTS, SGP_BLENDMODE_BLEND);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINES, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINES, SGP_BLENDMODE_BLEND);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, SGP_BLENDMODE_BLEND);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, SGP_BLENDMODE_BLEND);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_NONE);
    _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_BLEND);
}

bool sgp_setup(const sgp_desc* desc) {
    SOKOL_ASSERT(_sgp.init_cookie == 0);

    // init
    _sgp.init_cookie = _SGP_INIT_COOKIE;
    _sgp.last_error = "";

    // set desc default values
    _sgp.desc = *desc;
    _sgp.desc.max_vertices = _sg_def(desc->max_vertices, _SGP_DEFAULT_MAX_VERTICES);
    _sgp.desc.max_commands = _sg_def(desc->max_commands, _SGP_DEFAULT_MAX_COMMANDS);

    // allocate buffers
    _sgp.num_vertices = _sgp.desc.max_vertices;
    _sgp.num_commands = _sgp.desc.max_commands;
    _sgp.num_uniforms = _sgp.desc.max_commands;
    _sgp.vertices = (_sgp_vertex*) SOKOL_MALLOC(_sgp.num_vertices * sizeof(_sgp_vertex));
    _sgp.uniforms = (sgp_uniform*) SOKOL_MALLOC(_sgp.num_uniforms * sizeof(sgp_uniform));
    _sgp.commands = (_sgp_command*) SOKOL_MALLOC(_sgp.num_commands * sizeof(_sgp_command));
    SOKOL_ASSERT(_sgp.commands && _sgp.uniforms && _sgp.uniforms);
    memset(_sgp.vertices, 0, _sgp.num_vertices * sizeof(_sgp_vertex));
    memset(_sgp.uniforms, 0, _sgp.num_uniforms * sizeof(sgp_uniform));
    memset(_sgp.commands, 0, _sgp.num_commands * sizeof(_sgp_command));

    // create vertex buffer
    sg_buffer_desc vertex_buf_desc = {
        .size = (int)(_sgp.num_vertices * sizeof(_sgp_vertex)),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_STREAM,
    };
    _sgp.vertex_buf = sg_make_buffer(&vertex_buf_desc);
    SOKOL_ASSERT(_sgp.vertex_buf.id != SG_INVALID_ID);

    // create white texture
    uint32_t pixels[4];
    memset(pixels, 0xFF, sizeof(pixels));
    sg_image_desc white_img_desc = {
        .type = SG_IMAGETYPE_2D,
        .width = 2, .height = 2,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .content = {.subimage = {{{.ptr = pixels, .size = sizeof(pixels)}}}},
        .label = "sgp-white-texture",
    };
    _sgp.white_img = sg_make_image(&white_img_desc);
    SOKOL_ASSERT(_sgp.white_img.id != SG_INVALID_ID);

    _sgp_setup_pipelines();
    return true;
}

void sgp_shutdown() {
    if(_sgp.init_cookie == 0) return; // not initialized
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state == 0);
    SOKOL_FREE(_sgp.vertices);
    SOKOL_FREE(_sgp.uniforms);
    SOKOL_FREE(_sgp.commands);
    for(uint32_t i=0;i<_SG_PRIMITIVETYPE_NUM*_SGP_BLENDMODE_NUM;++i) {
        sg_pipeline pip = _sgp.pipelines[i];
        if(pip.id != SG_INVALID_ID)
            sg_destroy_pipeline(pip);
    }
    sg_destroy_shader(_sgp.shader);
    sg_destroy_buffer(_sgp.vertex_buf);
    sg_destroy_image(_sgp.white_img);
    _sgp = (_sgp_context){.init_cookie=0};
}

bool sgp_is_valid() {
    return _sgp.init_cookie == _SGP_INIT_COOKIE;
}

sgp_error sgp_get_error_code() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    return _sgp.last_error_code;
}

const char* sgp_get_error() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    return _sgp.last_error;
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
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(_sgp.cur_state >= _SGP_MAX_STACK_DEPTH)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_OVERFLOW, "NGP state stack overflow");
        return;
    }

    // first begin, reset last error
    if(_sgp.cur_state == 0) {
        _sgp.last_error = "";
        _sgp.last_error_code = SGP_NO_ERROR;
    }

    // save current state
    _sgp.state_stack[_sgp.cur_state++] = _sgp.state;

    // reset to default state
    _sgp.state.frame_size = (sgp_isize){width, height};
    _sgp.state.viewport = (sgp_irect){0, 0, width, height};
    _sgp.state.scissor = (sgp_irect){0, 0, -1, -1};
    _sgp.state.proj = _sgp_default_proj(width, height);
    _sgp.state.transform = _sgp_mat3_identity;
    _sgp.state.mvp = _sgp.state.proj;
    _sgp.state.uniform.color = (sgp_color){1.0f, 1.0f, 1.0f, 1.0f};
    _sgp.state.blend_mode = SGP_BLENDMODE_NONE;
    _sgp.state._base_vertex = _sgp.cur_vertex;
    _sgp.state._base_uniform = _sgp.cur_uniform;
    _sgp.state._base_command = _sgp.cur_command;
}

void sgp_flush() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    if(_sgp.last_error_code != SGP_NO_ERROR || _sgp.cur_command <= 0)
        return;

    // define the resource bindings
    sg_bindings bind = (sg_bindings){
        .vertex_buffers = {_sgp.vertex_buf},
    };

    // flush commands
    const uint32_t SG_IMPOSSIBLE_ID = 0xffffffffU;
    uint32_t cur_pip_id = SG_IMPOSSIBLE_ID;
    uint32_t cur_img_id = SG_IMPOSSIBLE_ID;
    uint32_t cur_uniform_index = SG_IMPOSSIBLE_ID;
    uint32_t cur_base_vertex = 0;
    uint32_t base_vertex = _sgp.state._base_vertex;
    uint32_t size_vertices = (_sgp.cur_vertex - base_vertex) * sizeof(_sgp_vertex);
    sg_update_buffer(_sgp.vertex_buf, &_sgp.vertices[base_vertex], size_vertices);
    for(uint32_t i = _sgp.state._base_command; i < _sgp.cur_command; ++i) {
        _sgp_command* cmd = &_sgp.commands[i];
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
                _sgp_draw_args* args = &cmd->args.draw;
                if(args->pip.id != cur_pip_id) {
                    // when pipeline changes, also need to re-apply uniforms and bindings
                    cur_img_id = SG_IMPOSSIBLE_ID;
                    cur_uniform_index = SG_IMPOSSIBLE_ID;
                    cur_pip_id = args->pip.id;
                    sg_apply_pipeline(args->pip);
                }
                if(cur_img_id != args->img.id) {
                    cur_base_vertex = base_vertex;
                    cur_img_id = args->img.id;
                    bind.fs_images[0] = args->img;
                    sg_apply_bindings(&bind);
                }
                if(cur_uniform_index != args->uniform_index) {
                    cur_uniform_index = args->uniform_index;
                    sgp_uniform* uniform = &_sgp.uniforms[cur_uniform_index];
                    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, uniform, sizeof(sgp_uniform));
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
    _sgp.cur_vertex = _sgp.state._base_vertex;
    _sgp.cur_uniform = _sgp.state._base_uniform;
    _sgp.cur_command = _sgp.state._base_command;
}

void sgp_end() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    if(SOKOL_UNLIKELY(_sgp.cur_state <= 0)) {
        _sgp_set_error(SGP_ERROR_STATE_STACK_UNDERFLOW, "NGP state stack underflow");
        return;
    }

    // restore old state
    _sgp.state = _sgp.state_stack[--_sgp.cur_state];
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

static sgp_mat3 _sgp_mul_proj_transform(sgp_mat3* proj, sgp_mat3* transform) {
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
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    float w = right - left;
    float h = top - bottom;
    _sgp.state.proj = (sgp_mat3){{
        {2.0f/w,   0.0f,  -(right+left)/w},
        {0.0f,   2.0f/h,  -(top+bottom)/h},
        {0.0f,     0.0f,             1.0f}
    }};
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_reset_proj() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.proj = _sgp_default_proj(_sgp.state.viewport.w, _sgp.state.viewport.h);
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_push_transform() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(_sgp.cur_transform >= _SGP_MAX_STACK_DEPTH)) {
        _sgp_set_error(SGP_ERROR_TRANSFORM_STACK_OVERFLOW, "NGP transform stack overflow");
        return;
    }
    _sgp.transform_stack[_sgp.cur_transform++] = _sgp.state.transform;
}

void sgp_pop_transform() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(_sgp.cur_transform <= 0)) {
        _sgp_set_error(SGP_ERROR_TRANSFORM_STACK_UNDERFLOW, "NGP transform stack underflow");
        return;
    }
    _sgp.state.transform = _sgp.transform_stack[--_sgp.cur_transform];
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_reset_transform() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.transform = _sgp_mat3_identity;
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_translate(float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    // multiply by translate matrix:
    // 1.0f, 0.0f,    x,
    // 0.0f, 1.0f,    y,
    // 0.0f, 0.0f, 1.0f,
    _sgp.state.transform.v[0][2] += x*_sgp.state.transform.v[0][0] + y*_sgp.state.transform.v[0][1];
    _sgp.state.transform.v[1][2] += x*_sgp.state.transform.v[1][0] + y*_sgp.state.transform.v[1][1];
    _sgp.state.transform.v[2][2] += x*_sgp.state.transform.v[2][0] + y*_sgp.state.transform.v[2][1];
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_rotate(float theta) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    float sint = sinf(theta), cost = cosf(theta);
    // multiply by rotation matrix:
    // sint,  cost, 0.0f,
    // cost, -sint, 0.0f,
    // 0.0f,  0.0f, 1.0f,
    _sgp.state.transform = (sgp_mat3){{
       {sint*_sgp.state.transform.v[0][0]+cost*_sgp.state.transform.v[0][1], cost*_sgp.state.transform.v[0][0]-sint*_sgp.state.transform.v[0][1], _sgp.state.transform.v[0][2]},
       {sint*_sgp.state.transform.v[1][0]+cost*_sgp.state.transform.v[1][1], cost*_sgp.state.transform.v[1][0]-sint*_sgp.state.transform.v[1][1], _sgp.state.transform.v[1][2]},
       {sint*_sgp.state.transform.v[2][0]+cost*_sgp.state.transform.v[2][1], cost*_sgp.state.transform.v[2][0]-sint*_sgp.state.transform.v[2][1], _sgp.state.transform.v[2][2]}
    }};
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_rotate_at(float theta, float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_translate(x, y);
    sgp_rotate(theta);
    sgp_translate(-x, -y);
}

void sgp_scale(float sx, float sy) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    // multiply by scale matrix:
    //   sx, 0.0f, 0.0f,
    // 0.0f,   sy, 0.0f,
    // 0.0f, 0.0f, 1.0f,
    _sgp.state.transform.v[0][0] *= sx;
    _sgp.state.transform.v[1][0] *= sx;
    _sgp.state.transform.v[2][0] *= sx;
    _sgp.state.transform.v[0][1] *= sy;
    _sgp.state.transform.v[1][1] *= sy;
    _sgp.state.transform.v[2][1] *= sy;
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_scale_at(float sx, float sy, float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_translate(x, y);
    sgp_scale(sx, sy);
    sgp_translate(-x, -y);
}

void sgp_set_blend_mode(sgp_blend_mode blend_mode) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    _sgp.state.blend_mode = blend_mode;
}

void sgp_reset_blend_mode() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    sgp_set_blend_mode(SGP_BLENDMODE_NONE);
}

void sgp_set_color(float r, float g, float b, float a) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.uniform.color = (sgp_color){r,g,b,a};
}

void sgp_reset_color() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    _sgp.state.uniform.color = (sgp_color){1.0f, 1.0f, 1.0f, 1.0f};
}

static _sgp_vertex* _sgp_next_vertices(uint32_t count) {
    if(SOKOL_LIKELY(_sgp.cur_vertex + count <= _sgp.num_vertices)) {
        _sgp_vertex *vertices = &_sgp.vertices[_sgp.cur_vertex];
        _sgp.cur_vertex += count;
        return vertices;
    } else {
        _sgp_set_error(SGP_ERROR_VERTICES_FULL, "NGP vertices buffer is full");
        return NULL;
    }
}

static sgp_uniform* _sgp_prev_uniform() {
    if(SOKOL_LIKELY(_sgp.cur_uniform > 0)) {
        return &_sgp.uniforms[_sgp.cur_uniform-1];
    } else {
        return NULL;
    }
}

static sgp_uniform* _sgp_next_uniform() {
    if(SOKOL_LIKELY(_sgp.cur_uniform < _sgp.num_uniforms)) {
        return &_sgp.uniforms[_sgp.cur_uniform++];
    } else {
        _sgp_set_error(SGP_ERROR_UNIFORMS_FULL, "NGP uniform buffer is full");
        return NULL;
    }
}

static _sgp_command* _sgp_prev_command() {
    if(SOKOL_LIKELY(_sgp.cur_command > 0)) {
        return &_sgp.commands[_sgp.cur_command-1];
    } else {
        return NULL;
    }
}

static _sgp_command* _sgp_next_command() {
    if(SOKOL_LIKELY(_sgp.cur_command < _sgp.num_commands)) {
        return &_sgp.commands[_sgp.cur_command++];
    } else {
        _sgp_set_error(SGP_ERROR_COMMANDS_FULL, "NGP command buffer is full");
        return NULL;
    }
}

void sgp_viewport(int x, int y, int w, int h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // skip in case of the same viewport
    if(_sgp.state.viewport.x == x && _sgp.state.viewport.y == y &&
       _sgp.state.viewport.w == w && _sgp.state.viewport.h == h)
        return;

    _sgp_command* cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) return;
    *cmd = (_sgp_command) {
        .cmd = SGP_COMMAND_VIEWPORT,
        .args = {.viewport = {x, y, w, h}},
    };

    // adjust current scissor relative offset
    if(!(_sgp.state.scissor.w < 0 && _sgp.state.scissor.h < 0)) {
        _sgp.state.scissor.x += x - _sgp.state.viewport.x;
        _sgp.state.scissor.y += y - _sgp.state.viewport.y;
    }

    _sgp.state.viewport = (sgp_irect){x, y, w, h};
    _sgp.state.proj = _sgp_default_proj(w, h);
    _sgp.state.mvp = _sgp_mul_proj_transform(&_sgp.state.proj, &_sgp.state.transform);
}

void sgp_reset_viewport() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_viewport(0, 0, _sgp.state.frame_size.w, _sgp.state.frame_size.h);
}

void sgp_scissor(int x, int y, int w, int h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // skip in case of the same scissor
    if(_sgp.state.scissor.x == x && _sgp.state.scissor.y == y &&
       _sgp.state.scissor.w == w && _sgp.state.scissor.h == h)
        return;

    _sgp_command* cmd = _sgp_next_command();
    if(SOKOL_UNLIKELY(!cmd)) return;

    // coordinate scissor in viewport subspace
    sgp_irect viewport_scissor = {_sgp.state.viewport.x + x, _sgp.state.viewport.y + y, w, h};

    // reset scissor
    if(w < 0 && h  < 0)
        viewport_scissor = (sgp_irect){0, 0, _sgp.state.frame_size.w, _sgp.state.frame_size.h};

    *cmd = (_sgp_command) {
        .cmd = SGP_COMMAND_SCISSOR,
        .args = {.scissor = viewport_scissor},
    };
    _sgp.state.scissor = (sgp_irect){x, y, w, h};
}

void sgp_reset_scissor() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_scissor(0, 0, -1, -1);
}

void sgp_reset_state() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_reset_viewport();
    sgp_reset_scissor();
    sgp_reset_proj();
    sgp_reset_transform();
    sgp_reset_blend_mode();
    sgp_reset_color();
}

static void _sgp_queue_draw(sg_pipeline pip, uint32_t vertex_index, uint32_t num_vertices, sg_image img) {
    // setup uniform, try to reuse previous uniform when possible
    sgp_uniform *prev_uniform = _sgp_prev_uniform();
    bool reuse_uniform = prev_uniform && memcmp(prev_uniform, &_sgp.state.uniform, sizeof(sgp_uniform)) == 0;
    if(!reuse_uniform) {
        // append new uniform
        sgp_uniform *uniform = _sgp_next_uniform();
        if(SOKOL_UNLIKELY(!uniform)) return;
        *uniform = _sgp.state.uniform;
    }
    uint32_t uniform_index = _sgp.cur_uniform - 1;

    _sgp_command* prev_cmd = _sgp_prev_command();
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
        _sgp_command* cmd = _sgp_next_command();
        if(SOKOL_UNLIKELY(!cmd)) return;
        cmd->cmd = SGP_COMMAND_DRAW,
        cmd->args.draw = (_sgp_draw_args){
            .pip = pip,
            .img = img,
            .uniform_index = uniform_index,
            .vertex_index = vertex_index,
            .num_vertices = num_vertices,
        };
    }
}

static inline sgp_vec2 _sgp_mat3_vec2_mul(const sgp_mat3* m, const sgp_vec2* v) {
    return (sgp_vec2){
        .x = m->v[0][0]*v->x + m->v[0][1]*v->y + m->v[0][2],
        .y = m->v[1][0]*v->x + m->v[1][1]*v->y + m->v[1][2]
    };
}

static void _sgp_transform_vec2(sgp_mat3* matrix, sgp_vec2* dst, const sgp_vec2 *src, uint32_t count) {
    for(uint32_t i=0;i<count;++i)
        dst[i] = _sgp_mat3_vec2_mul(matrix, &src[i]);
}

static void _sgp_transform_vertices(sgp_mat3* matrix, _sgp_vertex* dst, const sgp_vec2 *src, uint32_t count) {
    for(uint32_t i=0;i<count;++i) {
        dst[i] = (_sgp_vertex) {
            .position = _sgp_mat3_vec2_mul(matrix, &src[i]),
            .texcoord = {0.0f, 0.0f}
        };
    }
}

static void _sgp_draw_solid_pip(sg_pipeline pip, const sgp_vec2* vertices, uint32_t count) {
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* transformed_vertices = _sgp_next_vertices(count);
    if(SOKOL_UNLIKELY(!vertices)) return;

    sgp_mat3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    _sgp_transform_vertices(&mvp, transformed_vertices, vertices, count);
    _sgp_queue_draw(pip, vertex_index, count, _sgp.white_img);
}

void sgp_clear() {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);

    // setup vertices
    uint32_t num_vertices = 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    _sgp_vertex* v = vertices;
    const sgp_vec2 quad[4] = {
        {-1.0f, -1.0f}, // bottom left
        { 1.0f, -1.0f}, // bottom right
        { 1.0f,  1.0f}, // top right
        {-1.0f,  1.0f}, // top left
    };
    const sgp_vec2 texcoord = {0.0f, 0.0f};

    // make a quad composed of 2 triangles
    v[0].position = quad[0]; v[0].texcoord = texcoord;
    v[1].position = quad[1]; v[1].texcoord = texcoord;
    v[2].position = quad[2]; v[2].texcoord = texcoord;
    v[3].position = quad[3]; v[3].texcoord = texcoord;
    v[4].position = quad[0]; v[4].texcoord = texcoord;
    v[5].position = quad[2]; v[5].texcoord = texcoord;

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, SGP_BLENDMODE_NONE);
    _sgp_queue_draw(pip, vertex_index, num_vertices, _sgp.white_img);
}

void sgp_draw_points(const sgp_point* points, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_POINTS, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, points, count);
}

void sgp_draw_point(float x, float y) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_point point = {x, y};
    sgp_draw_points(&point, 1);
}

void sgp_draw_lines(const sgp_line* lines, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINES, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, (const sgp_point*)lines, count*2);
}

void sgp_draw_line(float ax, float ay, float bx, float by) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_line line = {{ax,ay},{bx, by}};
    sgp_draw_lines(&line, 1);
}

void sgp_draw_line_strip(const sgp_point* points, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_LINE_STRIP, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, points, count);
}

void sgp_draw_filled_triangles(const sgp_triangle* triangles, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, (const sgp_point*)triangles, count*3);
}

void sgp_draw_filled_triangle(float ax, float ay, float bx, float by, float cx, float cy) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_triangle triangle = {{ax,ay},{bx, by},{cx, cy}};
    sgp_draw_filled_triangles(&triangle, 1);
}

void sgp_draw_filled_triangle_strip(const sgp_point* points, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;
    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLE_STRIP, _sgp.state.blend_mode);
    _sgp_draw_solid_pip(pip, points, count);
}

void sgp_draw_filled_rects(const sgp_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    _sgp_vertex* v = vertices;
    const sgp_rect* rect = rects;
    sgp_mat3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    for(uint32_t i=0;i<count;v+=6, rect++, i++) {
        sgp_vec2 quad[4] = {
            {rect->x,           rect->y + rect->h}, // bottom left
            {rect->x + rect->w, rect->y + rect->h}, // bottom right
            {rect->x + rect->w, rect->y}, // top right
            {rect->x,  rect->y}, // top left
        };
        _sgp_transform_vec2(&mvp, quad, quad, 4);
        const sgp_vec2 texcoord = {0.0f, 0.0f};

        // make a quad composed of 2 triangles
        v[0].position = quad[0]; v[0].texcoord = texcoord;
        v[1].position = quad[1]; v[1].texcoord = texcoord;
        v[2].position = quad[2]; v[2].texcoord = texcoord;
        v[3].position = quad[3]; v[3].texcoord = texcoord;
        v[4].position = quad[0]; v[4].texcoord = texcoord;
        v[5].position = quad[2]; v[5].texcoord = texcoord;
    }

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_queue_draw(pip, vertex_index, num_vertices, _sgp.white_img);
}

void sgp_draw_filled_rect(float x, float y, float w, float h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_rect rect = {x,y,w,h};
    sgp_draw_filled_rects(&rect, 1);
}

void sgp_draw_textured_rects(sg_image image, const sgp_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0 || image.id == SG_INVALID_ID)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute vertices
    sgp_mat3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    for(uint32_t i=0;i<count;i++) {
        sgp_vec2 quad[4] = {
            {rects[i].x,              rects[i].y + rects[i].h}, // bottom left
            {rects[i].x + rects[i].w, rects[i].y + rects[i].h}, // bottom right
            {rects[i].x + rects[i].w, rects[i].y}, // top right
            {rects[i].x,  rects[i].y}, // top left
        };
        _sgp_transform_vec2(&mvp, quad, quad, 4);

        const sgp_vec2 vtexquad[4] = {
            {0.0f, 1.0f}, // bottom left
            {1.0f, 1.0f}, // bottom right
            {1.0f, 0.0f}, // top right
            {0.0f, 0.0f}, // top left
        };

        // make a quad composed of 2 triangles
        _sgp_vertex* v = &vertices[i*6];
        v[0].position = quad[0]; v[0].texcoord = vtexquad[0];
        v[1].position = quad[1]; v[1].texcoord = vtexquad[1];
        v[2].position = quad[2]; v[2].texcoord = vtexquad[2];
        v[3].position = quad[3]; v[3].texcoord = vtexquad[3];
        v[4].position = quad[0]; v[4].texcoord = vtexquad[0];
        v[5].position = quad[2]; v[5].texcoord = vtexquad[2];
    }

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_queue_draw(pip, vertex_index, num_vertices, image);
}

void sgp_draw_textured_rect(sg_image image, float x, float y, float w, float h) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_rect rect = {x, y, w, h};
    sgp_draw_textured_rects(image, &rect, 1);
}

static sgp_isize _sgp_query_image_size(sg_image img_id) {
    const _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    return (sgp_isize){img->cmn.width, img->cmn.height};
}

void sgp_draw_textured_rects_ex(sg_image image, const sgp_textured_rect* rects, uint32_t count) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    if(SOKOL_UNLIKELY(count == 0 || image.id == SG_INVALID_ID)) return;

    // setup vertices
    uint32_t num_vertices = count * 6;
    uint32_t vertex_index = _sgp.cur_vertex;
    _sgp_vertex* vertices = _sgp_next_vertices(num_vertices);
    if(SOKOL_UNLIKELY(!vertices)) return;

    // compute image values used for texture coords transform
    sgp_isize image_size = _sgp_query_image_size(image);
    if(SOKOL_UNLIKELY(image_size.w == 0 || image_size.h == 0)) return;
    float iw = 1.0f/image_size.w, ih = 1.0f/image_size.h;

    // compute vertices
    sgp_mat3 mvp = _sgp.state.mvp; // copy to stack for more efficiency
    for(uint32_t i=0;i<count;i++) {
        sgp_vec2 quad[4] = {
            {rects[i].dst.x,                   rects[i].dst.y + rects[i].dst.h}, // bottom left
            {rects[i].dst.x + rects[i].dst.w, rects[i].dst.y + rects[i].dst.h}, // bottom right
            {rects[i].dst.x + rects[i].dst.w, rects[i].dst.y}, // top right
            {rects[i].dst.x,  rects[i].dst.y}, // top left
        };
        _sgp_transform_vec2(&mvp, quad, quad, 4);
        _sgp_vertex* v = &vertices[i*6];
        v[0].position = quad[0];
        v[1].position = quad[1];
        v[2].position = quad[2];
        v[3].position = quad[3];
        v[4].position = quad[0];
        v[5].position = quad[2];
    }

    // compute texture coords
    for(uint32_t i=0;i<count;i++) {
        // compute source rect
        float tl = rects[i].src.x*iw;
        float tt = rects[i].src.y*ih;
        float tr = (rects[i].src.x + rects[i].src.w)*iw;
        float tb = (rects[i].src.y + rects[i].src.h)*ih;
        sgp_vec2 vtexquad[4] = {
            {tl, tb}, // bottom left
            {tr, tb}, // bottom right
            {tr, tt}, // top right
            {tl, tt}, // top left
        };

        // make a quad composed of 2 triangles
        _sgp_vertex* v = &vertices[i*6];
        v[0].texcoord = vtexquad[0];
        v[1].texcoord = vtexquad[1];
        v[2].texcoord = vtexquad[2];
        v[3].texcoord = vtexquad[3];
        v[4].texcoord = vtexquad[0];
        v[5].texcoord = vtexquad[2];
    }

    sg_pipeline pip = _sgp_lookup_pipeline(SG_PRIMITIVETYPE_TRIANGLES, _sgp.state.blend_mode);
    _sgp_queue_draw(pip, vertex_index, num_vertices, image);
}

void sgp_draw_textured_rect_ex(sg_image image, sgp_rect dest_rect, sgp_rect src_rect) {
    SOKOL_ASSERT(_sgp.init_cookie == _SGP_INIT_COOKIE);
    SOKOL_ASSERT(_sgp.cur_state > 0);
    sgp_textured_rect rect = {dest_rect, src_rect};
    sgp_draw_textured_rects_ex(image, &rect, 1);
}

sgp_desc sgp_query_desc() {
    return _sgp.desc;
}

sgp_state* sgp_query_state() {
    return &_sgp.state;
}

#endif // SOKOL_GP_IMPL_INCLUDED
#endif // SOKOL_GP_IMPL
