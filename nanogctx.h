/*
nanogctx.h - graphics context creation utility for SDL
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

#ifndef NANOGCTX_H
#define NANOGCTX_H

#include <stdbool.h>
#include <stdint.h>

#ifndef NGP_ASSERT
#include <assert.h>
#define NGP_ASSERT(c) assert(c)
#endif

#define _NANOGCTX_INIT_COOKIE 0xCAFED00D

#include <SDL2/SDL.h>

typedef struct ng_context_desc {
    int sample_count;
    int depth_size;
} ng_context_desc;

typedef struct ng_context {
    uint32_t init_cookie;
    SDL_Window* window;
    SDL_GLContext context;
    const char* last_error;
} ng_context;

typedef struct ng_isize {
    int w, h;
} ng_isize;

void ngctx_prepare_attributes();
bool ngctx_create(ng_context* ctx, SDL_Window* window);
void ngctx_destroy(ng_context* ctx);
bool ngctx_is_valid(ng_context* ctx);
ng_isize ngctx_get_drawable_size(ng_context* ctx);
bool ngctx_set_swap_interval(ng_context* ctx, int interval);
void ngctx_swap(ng_context* ctx);
const char* ngctx_get_error(ng_context* ctx);

#endif // NANOGCTX_H

#ifdef NANOGCTX_IMPL

void ngctx_prepare_attributes(ng_context_desc* desc) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, desc->depth_size);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, desc->sample_count > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, desc->sample_count);
}

bool ngctx_create(ng_context* ctx, SDL_Window* window) {
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if(!context) {
        ctx->last_error = SDL_GetError();
        return false;
    }
    if(SDL_GL_MakeCurrent(window, context) != 0) {
        ctx->last_error = SDL_GetError();
        SDL_GL_DeleteContext(context);
        return false;
    }

    if(!flextInit()) {
        ctx->last_error = "OpenGL version 3.3 not supported";
        return false;
    }

    ctx->init_cookie = _NANOGCTX_INIT_COOKIE;
    ctx->context = context;
    ctx->window = window;
    ctx->last_error = "";
    return true;
}

void ngctx_destroy(ng_context* ctx) {
    if(ctx->init_cookie == 0) return; // not initialized
    SDL_GL_MakeCurrent(NULL, NULL);
    SDL_GL_DeleteContext(ctx->context);
    *ctx = (ng_context){0};
}

bool ngctx_is_valid(ng_context* ctx) {
    return ctx->init_cookie == _NANOGCTX_INIT_COOKIE;
}

ng_isize ngctx_get_drawable_size(ng_context* ctx) {
    ng_isize size;
    NGP_ASSERT(ctx->init_cookie == _NANOGCTX_INIT_COOKIE);
    SDL_GL_GetDrawableSize(ctx->window, &size.w, &size.h);
    return size;
}

bool ngctx_set_swap_interval(ng_context* ctx, int interval) {
    NGP_ASSERT(ctx->init_cookie == _NANOGCTX_INIT_COOKIE);
    return SDL_GL_SetSwapInterval(interval) == 0;
}

void ngctx_swap(ng_context* ctx) {
    NGP_ASSERT(ctx->init_cookie == _NANOGCTX_INIT_COOKIE);
    SDL_GL_SwapWindow(ctx->window);
}

const char* ngctx_get_error(ng_context* ctx) {
    return ctx->last_error;
}

#endif // NANOGCTX_IMPL
