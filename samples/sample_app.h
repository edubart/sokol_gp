#define SDL_DISABLE_IMMINTRIN_H
#include <SDL2/SDL.h>

#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_gctx.h"
#include "sokol_gp.h"
#include <stdio.h>
#include <math.h>

typedef struct sample_app_desc {
    bool (*init)(void);
    void (*terminate)(void);
    void (*draw)(void);
    int width;
    int height;
    int argc;
    char **argv;
} sample_app_desc;

typedef struct sample_app {
    sample_app_desc desc;
    int width;
    int height;
    int frame;
} sample_app;

static sample_app app;

static int sample_app_main(const sample_app_desc* app_desc) {
    app.desc = *app_desc;

    // initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // setup context attributes before window and context creation
    sgctx_desc ctx_desc;
    memset(&ctx_desc, 0, sizeof(sgctx_desc));
    ctx_desc.sample_count = 0;

#if defined(SOKOL_GLCORE33) || defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
    sgctx_gl_prepare_attributes(&ctx_desc);
#endif

    // create window
    SDL_Window *window = SDL_CreateWindow("Sokol GP Sample",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        app.width, app.height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(!window) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return 1;
    }

    // create graphics context
    sgctx_context sgctx = sgctx_create(window, &ctx_desc);
    if(!sgctx_is_valid(sgctx)) {
        fprintf(stderr, "Failed to create SGCTX context: %s\n", sgctx_get_error());
        return 1;
    }

    // set swap interval
    bool vsync = true;
    for(int i=1;i<app.desc.argc;++i) {
        if(strcmp(app.desc.argv[i], "-no-vsync") == 0)
            vsync = false;
    }
    sgctx_set_swap_interval(sgctx, vsync ? 1 : 0);

    // setup sokol
    sg_desc desc;
    memset(&desc, 0, sizeof(sg_desc));
    desc.context.depth_format = SG_PIXELFORMAT_NONE;
#if defined(SOKOL_D3D11)
    desc.context.d3d11.device = sgctx.d3d11->device;
    desc.context.d3d11.device_context = sgctx.d3d11->device_context;
    desc.context.d3d11.render_target_view_cb = sgctx_d3d11_render_target_view;
    desc.context.d3d11.depth_stencil_view_cb = sgctx_d3d11_depth_stencil_view;
#endif
    sg_setup(&desc);
    if(!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol context\n");
        return 1;
    }

    // setup sokol gp
    sgp_desc sample_sgp_desc;
    memset(&sample_sgp_desc, 0, sizeof(sgp_desc));
    sample_sgp_desc.max_vertices = 262144;
    sample_sgp_desc.max_commands = 32768;

    if(!sgp_setup(&sample_sgp_desc)) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error());
        return 1;
    }

    // setup app.desc resources
    if(!app.desc.init()) {
        fprintf(stderr, "Failed to initialize app.desc\n");
        return 1;
    }

    // run loop
    while(!SDL_QuitRequested()) {
        sgctx_isize size = sgctx_get_drawable_size(sgctx);
        app.width = size.w;
        app.height = size.h;

        // poll events
        SDL_Event event;
        while(SDL_PollEvent(&event)) { }

        sgp_begin(size.w,  size.h);
        app.desc.draw();

        sg_pass_action default_pass_action;
        memset(&default_pass_action, 0, sizeof(sg_pass_action));
        default_pass_action.colors[0].action = SG_ACTION_CLEAR;
        default_pass_action.colors[0].value.r = 0.05f;
        default_pass_action.colors[0].value.g = 0.05f;
        default_pass_action.colors[0].value.b = 0.05f;
        default_pass_action.colors[0].value.a = 1.0f;
        default_pass_action.depth.action = SG_ACTION_DONTCARE;
        default_pass_action.stencil.action = SG_ACTION_DONTCARE;
        sg_begin_default_pass(&default_pass_action, size.w, size.h);
        sgp_flush();
        sg_end_pass();
        sgp_end();
        sg_commit();
        app.frame++;

        if(!sgctx_swap(sgctx)) {
            fprintf(stderr, "Failed to swap window buffers: %s\n", sgctx_get_error());
            return 1;
        }

        // print FPS
        static uint32_t fps = 0;
        static uint32_t last = 0;
        uint32_t now = SDL_GetTicks();
        fps++;
        if(now >= last + 1000) {
            printf("FPS: %d\n", fps);
            last = now;
            fps = 0;
        }
    }

    // destroy
    app.desc.terminate();
    sgp_shutdown();
    sg_shutdown();
    sgctx_destroy(sgctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
