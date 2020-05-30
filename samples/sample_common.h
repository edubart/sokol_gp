#define SDL_DISABLE_IMMINTRIN_H
#include <SDL2/SDL.h>
#define FLEXTGL_IMPL
#include "thirdparty/flextgl.h"
#define SOKOL_GCTX_IMPL
#include "sokol_gctx.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
//#define SOKOL_D3D11
//#define SOKOL_DUMMY_BACKEND
#include "sokol_gfx.h"
#define SOKOL_GP_IMPL
#include "sokol_gp.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

typedef struct sample_app_desc {
    bool (*init)();
    void (*terminate)();
    void (*draw)(int, int);
    int argc;
    char **argv;
} sample_app_desc;

int sample_app(sample_app_desc app) {
    // initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // setup context attributes before window and context creation
    sgctx_desc ctx_desc = {
#if defined(SOKOL_GLCORE33)
        .backend = SGCTX_BACKEND_GLCORE33,
#elif defined(SOKOL_D3D11)
        .backend = SGCTX_BACKEND_D3D11,
#elif defined(SOKOL_DUMMY_BACKEND)
        .backend = SGCTX_BACKEND_DUMMY,
#endif
        .sample_count = 0
    };

    if(ctx_desc.backend == SGCTX_BACKEND_GLCORE33)
        sgctx_gl_prepare_attributes(&ctx_desc);

    // create window
    SDL_Window *window = SDL_CreateWindow("NGP Sample",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        512, 512,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(!window) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return 1;
    }

    // create graphics context
    sgctx_context sgctx;
    if(!sgctx_create(&sgctx, window, &ctx_desc)) {
        fprintf(stderr, "Failed to create SGCTX context: %s\n", sgctx_get_error());
        return 1;
    }
    sgctx_set_swap_interval(sgctx, 1);

#if defined(SOKOL_GLCORE33)
    // load opengl api
    if(!flextInit()) {
        fprintf(stderr, "OpenGL version 3.3 unsupported");
        return 1;
    }
#endif

    // setup sokol
    sg_desc desc = {
        .context = {.depth_format = SG_PIXELFORMAT_NONE}
    };
#ifdef SOKOL_D3D11
    if(ctx_desc.backend == SGCTX_BACKEND_D3D11) {
        desc.context.d3d11.device = sgctx.d3d11->device;
        desc.context.d3d11.device_context = sgctx.d3d11->device_context;
        desc.context.d3d11.render_target_view_cb = sgctx_d3d11_render_target_view;
        desc.context.d3d11.depth_stencil_view_cb = sgctx_d3d11_depth_stencil_view;
    }
#endif
    sg_setup(&desc);
    if(!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol context\n");
        return 1;
    }

    // setup sokol gp
    sgp_desc sample_sgp_desc = {
        .max_vertices=262144,
        .max_commands=32768,
    };
    if(!sgp_setup(&sample_sgp_desc)) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error());
        return 1;
    }

    // setup app resources
    if(!app.init()) {
        fprintf(stderr, "Failed to initialize app\n");
        return 1;
    }

    // run loop
    while(!SDL_QuitRequested()) {
        sgctx_isize size = sgctx_get_drawable_size(sgctx);

        // poll events
        SDL_Event event;
        while(SDL_PollEvent(&event)) { }

        sgp_begin(size.w,  size.h);
        app.draw(size.w, size.h);

        sg_pass_action default_pass_action = {
            .colors = {{.action = SG_ACTION_CLEAR, .val = {0.05f, 0.05f, 0.05f, 1.0f}}},
            .depth = {.action = SG_ACTION_DONTCARE},
            .stencil = {.action = SG_ACTION_DONTCARE},
        };
        sg_begin_default_pass(&default_pass_action, size.w, size.h);
        sgp_flush();
        sg_end_pass();
        sgp_end();
        sg_commit();

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
    app.terminate();
    sgp_shutdown();
    sg_shutdown();
    sgctx_destroy(&sgctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
