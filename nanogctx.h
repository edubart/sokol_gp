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

#ifndef NANOGCTX_INCLUDED
#define NANOGCTX_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#ifndef NANOGCTX_API
#define NANOGCTX_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ngctx_backend {
    NGCTX_BACKEND_DUMMY = 0,
    NGCTX_BACKEND_GLCORE33,
    NGCTX_BACKEND_WGPU
} ngctx_backend;

typedef struct ngctx_isize {
    int w, h;
} ngctx_isize;

typedef enum ngctx_error {
    NGCTX_NO_ERROR = 0,
    NGCTX_CREATE_CONTEXT_FAILED,
    NGCTX_ACTIVATE_CONTEXT_FAILED,
    NGCTX_GRAPHICS_API_UNSUPPORTED,
    NGCTX_INVALID_BACKEND
} ngctx_error;

typedef struct ngctx_desc {
    int sample_count;
    int depth_size;
} ngctx_desc;

#ifdef NANOGCTX_GLCORE33_BACKEND
typedef struct ngctx_gl_context {
    uint32_t init_cookie;
    const char* last_error;
    ngctx_error last_error_code;
    SDL_Window* window;
    SDL_GLContext context;
} ngctx_gl_context;

NANOGCTX_API void ngctx_gl_prepare_attributes(ngctx_desc* desc);
NANOGCTX_API bool ngctx_gl_create(ngctx_gl_context* ngctx, SDL_Window* window);
NANOGCTX_API void ngctx_gl_destroy(ngctx_gl_context* ngctx);
NANOGCTX_API bool ngctx_gl_activate(ngctx_gl_context* ngctx);
NANOGCTX_API bool ngctx_gl_is_valid(ngctx_gl_context* ngctx);
NANOGCTX_API ngctx_isize ngctx_gl_get_drawable_size(ngctx_gl_context* ngctx);
NANOGCTX_API bool ngctx_gl_set_swap_interval(ngctx_gl_context* ngctx, int interval);
NANOGCTX_API void ngctx_gl_swap(ngctx_gl_context* ngctx);
NANOGCTX_API const char* ngctx_gl_get_error(ngctx_gl_context* ngctx);
NANOGCTX_API ngctx_error ngctx_gl_get_error_code(ngctx_gl_context* ngctx);
#endif // NANOGCTX_GLCORE33_BACKEND

#ifdef NANOGCTX_WGPU_BACKEND
typedef struct ngctx_wgpu_context {
    uint32_t init_cookie;
    SDL_Window* window;
} ngctx_wgpu_context;

NANOGCTX_API void ngctx_wgpu_prepare_attributes(ngctx_desc* desc);
NANOGCTX_API bool ngctx_wgpu_create(ngctx_wgpu_context* ngctx, SDL_Window* window);
NANOGCTX_API void ngctx_wgpu_destroy(ngctx_wgpu_context* ngctx);
NANOGCTX_API bool ngctx_wgpu_activate(ngctx_wgpu_context* ngctx);
NANOGCTX_API bool ngctx_wgpu_is_valid(ngctx_wgpu_context* ngctx);
NANOGCTX_API ngctx_isize ngctx_wgpu_get_drawable_size(ngctx_wgpu_context* ngctx);
NANOGCTX_API bool ngctx_wgpu_set_swap_interval(ngctx_wgpu_context* ngctx, int interval);
NANOGCTX_API void ngctx_wgpu_swap(ngctx_wgpu_context* ngctx);
NANOGCTX_API const char* ngctx_wgpu_get_error(ngctx_wgpu_context* ngctx);
NANOGCTX_API ngctx_error ngctx_wgpu_get_error_code(ngctx_wgpu_context* ngctx);
#endif // NANOGCTX_WGPU_BACKEND

#ifdef NANOGCTX_DUMMY_BACKEND
typedef struct ngctx_dummy_context {
    uint32_t init_cookie;
    SDL_Window* window;
} ngctx_dummy_context;

NANOGCTX_API void ngctx_dummy_prepare_attributes(ngctx_desc* desc);
NANOGCTX_API bool ngctx_dummy_create(ngctx_dummy_context* ngctx, SDL_Window* window);
NANOGCTX_API void ngctx_dummy_destroy(ngctx_dummy_context* ngctx);
NANOGCTX_API bool ngctx_dummy_activate(ngctx_dummy_context* ngctx);
NANOGCTX_API bool ngctx_dummy_is_valid(ngctx_dummy_context* ngctx);
NANOGCTX_API ngctx_isize ngctx_dummy_get_drawable_size(ngctx_dummy_context* ngctx);
NANOGCTX_API bool ngctx_dummy_set_swap_interval(ngctx_dummy_context* ngctx, int interval);
NANOGCTX_API void ngctx_dummy_swap(ngctx_dummy_context* ngctx);
NANOGCTX_API const char* ngctx_dummy_get_error(ngctx_dummy_context* ngctx);
NANOGCTX_API ngctx_error ngctx_dummy_get_error_code(ngctx_dummy_context* ngctx);
#endif // NANOGCTX_DUMMY_BACKEND

typedef struct ngctx_context {
    union {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        ngctx_gl_context gl;
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        ngctx_wgpu_context wgpu;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        ngctx_dummy_context dummy;
        #endif
    };
    ngctx_backend backend;
} ngctx_context;

NANOGCTX_API void ngctx_prepare_attributes(ngctx_desc* desc, ngctx_backend backend);
NANOGCTX_API bool ngctx_create(ngctx_context* ngctx, SDL_Window* window, ngctx_backend backend);
NANOGCTX_API void ngctx_destroy(ngctx_context* ngctx);
NANOGCTX_API bool ngctx_activate(ngctx_context* ngctx);
NANOGCTX_API bool ngctx_is_valid(ngctx_context* ngctx);
NANOGCTX_API ngctx_isize ngctx_get_drawable_size(ngctx_context* ngctx);
NANOGCTX_API bool ngctx_set_swap_interval(ngctx_context* ngctx, int interval);
NANOGCTX_API void ngctx_swap(ngctx_context* ngctx);
NANOGCTX_API const char* ngctx_get_error(ngctx_context* ngctx);

// TODO:
// NANOGCTX_GLES2
// NANOGCTX_GLES3
// NANOGCTX_D3D11
// NANOGCTX_METAL
// NANOGCTX_WGPU

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NANOGCTX_INCLUDED

#ifdef NANOGCTX_IMPL
#ifndef NANOGCTX_IMPL_INCLUDED
#define NANOGCTX_IMPL_INCLUDED

#ifndef NANOGP_ASSERT
#include <assert.h>
#define NANOGP_ASSERT(c) assert(c)
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

enum {
    _NGCTX_INIT_COOKIE = 0xCAFED00D
};

#ifdef NANOGCTX_GLCORE33_BACKEND

static void _ngctx_gl_set_error(ngctx_gl_context* ngctx, ngctx_error error_code, const char *error) {
    ngctx->last_error_code = error_code;
    ngctx->last_error = error;
    NANOGP_LOG(error);
}

void ngctx_gl_prepare_attributes(ngctx_desc* desc) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, desc->depth_size);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, desc->sample_count > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, desc->sample_count);
}

bool ngctx_gl_create(ngctx_gl_context* ngctx, SDL_Window* window) {
    NANOGP_ASSERT(ngctx->init_cookie == 0);
    NANOGP_ASSERT(window);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if(!context) {
        _ngctx_gl_set_error(ngctx, NGCTX_CREATE_CONTEXT_FAILED, SDL_GetError());
        return false;
    }
    if(SDL_GL_MakeCurrent(window, context) != 0) {
        _ngctx_gl_set_error(ngctx, NGCTX_ACTIVATE_CONTEXT_FAILED, SDL_GetError());
        SDL_GL_DeleteContext(context);
        return false;
    }

    if(!flextInit()) {
        _ngctx_gl_set_error(ngctx, NGCTX_GRAPHICS_API_UNSUPPORTED, "OpenGL version 3.3 unsupported");
        return false;
    }

    ngctx->init_cookie = _NGCTX_INIT_COOKIE;
    ngctx->context = context;
    ngctx->window = window;
    ngctx->last_error = "";
    return true;
}

void ngctx_gl_destroy(ngctx_gl_context* ngctx) {
    if(ngctx->init_cookie == 0) return; // not initialized
    SDL_GL_MakeCurrent(NULL, NULL);
    SDL_GL_DeleteContext(ngctx->context);
    *ngctx = (ngctx_gl_context){0};
}

bool ngctx_gl_activate(ngctx_gl_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    if(SDL_GL_MakeCurrent(ngctx->window, ngctx->context) != 0) {
        _ngctx_gl_set_error(ngctx, NGCTX_ACTIVATE_CONTEXT_FAILED, SDL_GetError());
        return false;
    }
    return true;
}

bool ngctx_gl_is_valid(ngctx_gl_context* ngctx) {
    return ngctx->init_cookie == _NGCTX_INIT_COOKIE;
}

ngctx_isize ngctx_gl_get_drawable_size(ngctx_gl_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    ngctx_isize size;
    SDL_GL_GetDrawableSize(ngctx->window, &size.w, &size.h);
    return size;
}

bool ngctx_gl_set_swap_interval(ngctx_gl_context* ngctx, int interval) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    return SDL_GL_SetSwapInterval(interval) == 0;
}

void ngctx_gl_swap(ngctx_gl_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    SDL_GL_SwapWindow(ngctx->window);
}

const char* ngctx_gl_get_error(ngctx_gl_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    return ngctx->last_error;
}

ngctx_error ngctx_gl_get_error_code(ngctx_gl_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    return ngctx->last_error_code;
}

#endif // NANOGCTX_GLCORE33_BACKEND

#ifdef NANOGCTX_WGPU_BACKEND

void ngctx_wgpu_prepare_attributes(ngctx_desc* desc) {
}

bool ngctx_wgpu_create(ngctx_wgpu_context* ngctx, SDL_Window* window) {
    NANOGP_ASSERT(ngctx->init_cookie == 0);
    NANOGP_ASSERT(window);
    ngctx->window = window;
    return true;
}

void ngctx_wgpu_destroy(ngctx_wgpu_context* ngctx) {
    if(ngctx->init_cookie == 0) return; // not initialized
    *ngctx = (ngctx_wgpu_context){0};
}

bool ngctx_wgpu_activate(ngctx_wgpu_context* ngctx) {
    return true;
}

bool ngctx_wgpu_is_valid(ngctx_wgpu_context* ngctx) {
    return ngctx->init_cookie == _NGCTX_INIT_COOKIE;
}

ngctx_isize ngctx_wgpu_get_drawable_size(ngctx_wgpu_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    ngctx_isize size;
    SDL_GetWindowSize(ngctx->window, &size.w, &size.h);
    return size;
}

bool ngctx_wgpu_set_swap_interval(ngctx_wgpu_context* ngctx, int interval) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    return true;
}

void ngctx_wgpu_swap(ngctx_wgpu_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
}

const char* ngctx_wgpu_get_error(ngctx_wgpu_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    return "";
}

ngctx_error ngctx_wgpu_get_error_code(ngctx_wgpu_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    return NGCTX_NO_ERROR;
}

#endif // NANOGCTX_WGPU_BACKEND

#ifdef NANOGCTX_DUMMY_BACKEND

void ngctx_dummy_prepare_attributes(ngctx_desc* desc) {
}

bool ngctx_dummy_create(ngctx_dummy_context* ngctx, SDL_Window* window) {
    NANOGP_ASSERT(ngctx->init_cookie == 0);
    NANOGP_ASSERT(window);
    ngctx->init_cookie = _NGCTX_INIT_COOKIE;
    ngctx->window = window;
    return true;
}

void ngctx_dummy_destroy(ngctx_dummy_context* ngctx) {
    if(ngctx->init_cookie == 0) return; // not initialized
    *ngctx = (ngctx_dummy_context){0};
}

bool ngctx_dummy_activate(ngctx_dummy_context* ngctx) {
    return true;
}

bool ngctx_dummy_is_valid(ngctx_dummy_context* ngctx) {
    return ngctx->init_cookie == _NGCTX_INIT_COOKIE;
}

ngctx_isize ngctx_dummy_get_drawable_size(ngctx_dummy_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    ngctx_isize size;
    SDL_GetWindowSize(ngctx->window, &size.w, &size.h);
    return size;
}

bool ngctx_dummy_set_swap_interval(ngctx_dummy_context* ngctx, int interval) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    return true;
}

void ngctx_dummy_swap(ngctx_dummy_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
}

const char* ngctx_dummy_get_error(ngctx_dummy_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    return "";
}

ngctx_error ngctx_dummy_get_error_code(ngctx_dummy_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    return NGCTX_NO_ERROR;
}

#endif // NANOGCTX_DUMMY_BACKEND

void ngctx_prepare_attributes(ngctx_desc* desc, ngctx_backend backend) {
    switch(backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            ngctx_gl_prepare_attributes(desc); return;
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            ngctx_wgpu_prepare_attributes(desc); return;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            ngctx_dummy_prepare_attributes(desc); return;
        #endif
        default:
            NANOGP_UNREACHABLE; return;
    }
}

bool ngctx_create(ngctx_context* ngctx, SDL_Window* window, ngctx_backend backend) {
    bool ok = false;
    switch(backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            ok = ngctx_gl_create(&ngctx->gl, window); break;
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            ok = ngctx_wgpu_create(&ngctx->wgpu, window); break;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            ok = ngctx_dummy_create(&ngctx->dummy, window); break;
        #endif
    }
    if(ok) ngctx->backend = backend;
    return ok;
}

void ngctx_destroy(ngctx_context* ngctx) {
    switch(ngctx->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            ngctx_gl_destroy(&ngctx->gl); break;
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            ngctx_wgpu_destroy(&ngctx->wgpu); break;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            ngctx_dummy_destroy(&ngctx->dummy); break;
        #endif
        default:
            NANOGP_UNREACHABLE; break;
    }
}

bool ngctx_activate(ngctx_context* ngctx) {
    switch(ngctx->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            return ngctx_gl_activate(&ngctx->gl);
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            return ngctx_wgpu_activate(&ngctx->wgpu);
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            return ngctx_dummy_activate(&ngctx->dummy);
        #endif
        default:
            NANOGP_UNREACHABLE; return false;
    }
}

bool ngctx_is_valid(ngctx_context* ngctx) {
    switch(ngctx->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            return ngctx_gl_is_valid(&ngctx->gl);
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            return ngctx_wgpu_is_valid(&ngctx->wgpu);
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            return ngctx_dummy_is_valid(&ngctx->dummy);
        #endif
        default:
            NANOGP_UNREACHABLE; return false;
    }
}

ngctx_isize ngctx_get_drawable_size(ngctx_context* ngctx) {
    switch(ngctx->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            return ngctx_gl_get_drawable_size(&ngctx->gl);
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            return ngctx_wgpu_get_drawable_size(&ngctx->wgpu);
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            return ngctx_dummy_get_drawable_size(&ngctx->dummy);
        #endif
        default:
            NANOGP_UNREACHABLE; return (ngctx_isize){0};
    }
}

bool ngctx_set_swap_interval(ngctx_context* ngctx, int interval) {
    switch(ngctx->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            return ngctx_gl_set_swap_interval(&ngctx->gl, interval);
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            return ngctx_wgpu_set_swap_interval(&ngctx->wgpu, interval);
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            return ngctx_dummy_set_swap_interval(&ngctx->dummy, interval);
        #endif
        default:
            NANOGP_UNREACHABLE; return false;
    }
}

void ngctx_swap(ngctx_context* ngctx) {
    switch(ngctx->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            ngctx_gl_swap(&ngctx->gl); return;
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            ngctx_wgpu_swap(&ngctx->wgpu); return;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            ngctx_dummy_swap(&ngctx->dummy); return;
        #endif
        default:
            NANOGP_UNREACHABLE; return;
    }
}

const char* ngctx_get_error(ngctx_context* ngctx) {
    switch(ngctx->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            return ngctx_gl_get_error(&ngctx->gl);
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            return ngctx_wgpu_get_error(&ngctx->wgpu);
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            return ngctx_dummy_get_error(&ngctx->dummy);
        #endif
        default:
            return "invalid backend";
    }
}

ngctx_error ngctx_get_error_code(ngctx_context* ngctx) {
    switch(ngctx->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            return ngctx_gl_get_error_code(&ngctx->gl);
        #endif
        #ifdef NANOGCTX_WGPU_BACKEND
        case NGCTX_BACKEND_WGPU:
            return ngctx_wgpu_get_error_code(&ngctx->wgpu);
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            return ngctx_dummy_get_error_code(&ngctx->dummy);
        #endif
        default:
            return NGCTX_INVALID_BACKEND;
    }
}

#endif // NANOGCTX_IMPL_INCLUDED
#endif // NANOGCTX_IMPL
