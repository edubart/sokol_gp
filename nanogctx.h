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

#ifndef NANOGCTX_API
#define NANOGCTX_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SDL_Window SDL_Window;

typedef enum ngctx_backend {
    NGCTX_BACKEND_INVALID = 0,
    NGCTX_BACKEND_GLCORE33,
    NGCTX_BACKEND_D3D11,
    NGCTX_BACKEND_DUMMY
} ngctx_backend;

typedef struct ngctx_isize {
    int w, h;
} ngctx_isize;

typedef enum ngctx_error {
    NGCTX_NO_ERROR = 0,
    NGCTX_CREATE_CONTEXT_FAILED,
    NGCTX_CREATE_RENDER_TARGET_FAILED,
    NGCTX_ACTIVATE_CONTEXT_FAILED,
    NGCTX_GRAPHICS_API_UNSUPPORTED,
    NGCTX_WMINFO_FAILED,
    NGCTX_INVALID_BACKEND
} ngctx_error;

typedef struct ngctx_desc {
    ngctx_backend backend;
    int sample_count;
    int depth_size;
} ngctx_desc;

typedef struct ngctx_error_desc {
    ngctx_error code;
    const char* message;
} ngctx_error_desc;

#ifdef NANOGCTX_GLCORE33_BACKEND
typedef void* SDL_GLContext;
typedef struct ngctx_gl_context {
    uint32_t init_cookie;
    SDL_Window* window;
    SDL_GLContext context;
} ngctx_gl_context;

NANOGCTX_API void ngctx_gl_prepare_attributes(ngctx_desc* desc);
NANOGCTX_API ngctx_gl_context* ngctx_gl_create(SDL_Window* window, ngctx_desc* desc);
NANOGCTX_API void ngctx_gl_destroy(ngctx_gl_context* ngctx);
NANOGCTX_API bool ngctx_gl_activate(ngctx_gl_context* ngctx);
NANOGCTX_API ngctx_isize ngctx_gl_get_drawable_size(ngctx_gl_context* ngctx);
NANOGCTX_API bool ngctx_gl_set_swap_interval(ngctx_gl_context* ngctx, int interval);
NANOGCTX_API void ngctx_gl_swap(ngctx_gl_context* ngctx);
#endif // NANOGCTX_GLCORE33_BACKEND

#ifdef NANOGCTX_D3D11_BACKEND
#ifndef D3D11_NO_HELPERS
#define D3D11_NO_HELPERS
#endif
#ifndef CINTERFACE
#define CINTERFACE
#endif
#ifndef COBJMACROS
#define COBJMACROS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

typedef struct ngctx_d3d11_context {
    uint32_t init_cookie;
    ngctx_desc desc;
    SDL_Window* window;
    int width, height;
    int swap_interval;
    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    IDXGISwapChain* swap_chain;
    ID3D11Device* device;
    ID3D11DeviceContext* device_context;
    ID3D11Texture2D* render_target;
    ID3D11RenderTargetView* render_target_view;
    ID3D11Texture2D* depth_stencil_buffer;
    ID3D11DepthStencilView* depth_stencil_view;
} ngctx_d3d11_context;

NANOGCTX_API void ngctx_d3d11_prepare_attributes(ngctx_desc* desc);
NANOGCTX_API ngctx_d3d11_context* ngctx_d3d11_create(SDL_Window* window, ngctx_desc* desc);
NANOGCTX_API void ngctx_d3d11_destroy(ngctx_d3d11_context* ngctx);
NANOGCTX_API bool ngctx_d3d11_activate(ngctx_d3d11_context* ngctx);
NANOGCTX_API ngctx_isize ngctx_d3d11_get_drawable_size(ngctx_d3d11_context* ngctx);
NANOGCTX_API bool ngctx_d3d11_set_swap_interval(ngctx_d3d11_context* ngctx, int interval);
NANOGCTX_API void ngctx_d3d11_swap(ngctx_d3d11_context* ngctx);
NANOGCTX_API const void* ngctx_d3d11_render_target_view();
NANOGCTX_API const void* ngctx_d3d11_depth_stencil_view();

#endif // NANOGCTX_D3D11_BACKEND

#ifdef NANOGCTX_DUMMY_BACKEND
typedef struct ngctx_dummy_context{
    uint32_t init_cookie;
    SDL_Window* window;
} ngctx_dummy_context;

NANOGCTX_API void ngctx_dummy_prepare_attributes(ngctx_desc* desc);
NANOGCTX_API ngctx_dummy_context* ngctx_dummy_create(SDL_Window* window, ngctx_desc* desc);
NANOGCTX_API void ngctx_dummy_destroy(ngctx_dummy_context* ngctx);
NANOGCTX_API bool ngctx_dummy_activate(ngctx_dummy_context* ngctx);
NANOGCTX_API ngctx_isize ngctx_dummy_get_drawable_size(ngctx_dummy_context* ngctx);
NANOGCTX_API bool ngctx_dummy_set_swap_interval(ngctx_dummy_context* ngctx, int interval);
NANOGCTX_API void ngctx_dummy_swap(ngctx_dummy_context* ngctx);
#endif // NANOGCTX_DUMMY_BACKEND

typedef struct ngctx_context {
    ngctx_backend backend;
    union {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        ngctx_gl_context* gl;
        #endif
        #ifdef NANOGCTX_D3D11_BACKEND
        ngctx_d3d11_context* d3d11;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        ngctx_dummy_context* dummy;
        #endif
    };
} ngctx_context;

NANOGCTX_API void ngctx_prepare_attributes(ngctx_desc* desc);
NANOGCTX_API bool ngctx_create(ngctx_context* ngctx, SDL_Window* window, ngctx_desc* desc);
NANOGCTX_API void ngctx_destroy(ngctx_context ngctx);
NANOGCTX_API bool ngctx_activate(ngctx_context ngctx);
NANOGCTX_API bool ngctx_is_valid(ngctx_context ngctx);
NANOGCTX_API ngctx_isize ngctx_get_drawable_size(ngctx_context ngctx);
NANOGCTX_API bool ngctx_set_swap_interval(ngctx_context ngctx, int interval);
NANOGCTX_API void ngctx_swap(ngctx_context ngctx);
NANOGCTX_API const char* ngctx_get_error();
NANOGCTX_API ngctx_error ngctx_get_error_code();

// TODO:
// NANOGCTX_GLES2
// NANOGCTX_GLES3
// NANOGCTX_METAL
// NANOGCTX_D3D11

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

#include <string.h>
#include <SDL2/SDL.h>

enum {
    _NGCTX_INIT_COOKIE = 0xCAFED00D
};

static ngctx_error_desc ngctx_last_error = {NGCTX_NO_ERROR, ""};

static void _ngctx_set_error(ngctx_error code, const char *message) {
    ngctx_last_error.code = code;
    ngctx_last_error.message = message;
    NANOGP_LOG(message);
}

const char* ngctx_get_error() {
    return ngctx_last_error.message;
}

ngctx_error ngctx_get_error_code() {
    return ngctx_last_error.code;
}

#ifdef NANOGCTX_GLCORE33_BACKEND

void ngctx_gl_prepare_attributes(ngctx_desc* desc) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, desc->depth_size);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, desc->sample_count > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, desc->sample_count);
}

ngctx_gl_context* ngctx_gl_create(SDL_Window* window, ngctx_desc* desc) {
    NANOGP_ASSERT(window);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if(!context) {
        _ngctx_set_error(NGCTX_CREATE_CONTEXT_FAILED, SDL_GetError());
        return NULL;
    }
    if(SDL_GL_MakeCurrent(window, context) != 0) {
        _ngctx_set_error(NGCTX_ACTIVATE_CONTEXT_FAILED, SDL_GetError());
        SDL_GL_DeleteContext(context);
        return NULL;
    }

    if(!flextInit()) {
        _ngctx_set_error(NGCTX_GRAPHICS_API_UNSUPPORTED, "OpenGL version 3.3 unsupported");
        return NULL;
    }

    ngctx_gl_context* ngctx = (ngctx_gl_context*)NANOGP_MALLOC(sizeof(ngctx_gl_context));
    memset(ngctx, 0, sizeof(ngctx));
    ngctx->init_cookie = _NGCTX_INIT_COOKIE;
    ngctx->context = context;
    ngctx->window = window;
    return ngctx;
}

void ngctx_gl_destroy(ngctx_gl_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    SDL_GL_DeleteContext(ngctx->context);
    NANOGP_FREE(ngctx);
}

bool ngctx_gl_activate(ngctx_gl_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    if(SDL_GL_MakeCurrent(ngctx->window, ngctx->context) != 0) {
        _ngctx_set_error(NGCTX_ACTIVATE_CONTEXT_FAILED, SDL_GetError());
        return false;
    }
    return true;
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

#endif // NANOGCTX_GLCORE33_BACKEND

#ifdef NANOGCTX_D3D11_BACKEND

#include <SDL2/SDL_syswm.h>
#define SAFE_RELEASE(class, obj) if (obj) { class##_Release(obj); obj=0; }

static ngctx_d3d11_context* _ngctx_d3d11_active = NULL;

void ngctx_d3d11_prepare_attributes(ngctx_desc* desc) {
}

bool _ngctx_d3d11_create_default_render_target(ngctx_d3d11_context* ngctx) {
    HRESULT hr;
    hr = IDXGISwapChain_GetBuffer(ngctx->swap_chain, 0, &IID_ID3D11Texture2D, (void**)&ngctx->render_target);
    if(!SUCCEEDED(hr))
        return false;

    NANOGP_ASSERT(ngctx->render_target);
    hr = ID3D11Device_CreateRenderTargetView(ngctx->device, (ID3D11Resource*)ngctx->render_target, NULL, &ngctx->render_target_view);
    if(!SUCCEEDED(hr))
        return false;
    NANOGP_ASSERT(ngctx->render_target_view);

    D3D11_TEXTURE2D_DESC ds_desc = {
        .Width = ngctx->width,
        .Height = ngctx->height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .SampleDesc = ngctx->swap_chain_desc.SampleDesc,
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL,
    };
    hr = ID3D11Device_CreateTexture2D(ngctx->device, &ds_desc, NULL, &ngctx->depth_stencil_buffer);
    if(!SUCCEEDED(hr))
        return false;
    NANOGP_ASSERT(ngctx->depth_stencil_buffer);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
        .Format = ds_desc.Format,
        .ViewDimension = ngctx->desc.sample_count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D
    };
    hr = ID3D11Device_CreateDepthStencilView(ngctx->device, (ID3D11Resource*)ngctx->depth_stencil_buffer, &dsv_desc, &ngctx->depth_stencil_view);
    if(!SUCCEEDED(hr))
        return false;
    NANOGP_ASSERT(ngctx->depth_stencil_view);
    return true;
}

void _ngctx_d3d11_destroy_default_render_target(ngctx_d3d11_context* ngctx) {
    SAFE_RELEASE(ID3D11Texture2D, ngctx->render_target);
    SAFE_RELEASE(ID3D11RenderTargetView, ngctx->render_target_view);
    SAFE_RELEASE(ID3D11Texture2D, ngctx->depth_stencil_buffer);
    SAFE_RELEASE(ID3D11DepthStencilView, ngctx->depth_stencil_view);
}

void _ngctx_d3d11_update_default_render_target(ngctx_d3d11_context* ngctx) {
    if(ngctx->swap_chain) {
        _ngctx_d3d11_destroy_default_render_target(ngctx);
        IDXGISwapChain_ResizeBuffers(ngctx->swap_chain, 1, ngctx->width, ngctx->height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        _ngctx_d3d11_create_default_render_target(ngctx);
    }
}

bool _ngctx_d3d11_create_device(ngctx_d3d11_context* ngctx, HWND hwnd) {
    ngctx->swap_chain_desc = (DXGI_SWAP_CHAIN_DESC) {
        .BufferDesc = {
            .Width = ngctx->width,
            .Height = ngctx->height,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .RefreshRate = {
                .Numerator = 60,
                .Denominator = 1
            }
        },
        .OutputWindow = hwnd,
        .Windowed = true,
        .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
        .BufferCount = 1,
        .SampleDesc = {
            .Count = ngctx->desc.sample_count > 0 ? ngctx->desc.sample_count : 1,
            .Quality = ngctx->desc.sample_count > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT
    };
    int create_flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
    D3D_FEATURE_LEVEL feature_level = {0};
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,                       /* pAdapter (use default) */
        D3D_DRIVER_TYPE_HARDWARE,   /* DriverType */
        NULL,                       /* Software */
        create_flags,               /* Flags */
        NULL,                       /* pFeatureLevels */
        0,                          /* FeatureLevels */
        D3D11_SDK_VERSION,          /* SDKVersion */
        &ngctx->swap_chain_desc,    /* pSwapChainDesc */
        &ngctx->swap_chain,         /* ppSwapChain */
        &ngctx->device,             /* ppDevice */
        &feature_level,             /* pFeatureLevel */
        &ngctx->device_context);    /* ppImmediateContext */
    if(!SUCCEEDED(hr))
        return false;
    NANOGP_ASSERT(ngctx->swap_chain && ngctx->device && ngctx->device_context);
    return true;
}

void _ngctx_d3d11_destroy_device(ngctx_d3d11_context* ngctx) {
    SAFE_RELEASE(IDXGISwapChain, ngctx->swap_chain);
    SAFE_RELEASE(ID3D11DeviceContext, ngctx->device_context);
    SAFE_RELEASE(ID3D11Device, ngctx->device);
}

ngctx_d3d11_context* ngctx_d3d11_create(SDL_Window* window, ngctx_desc* desc) {
    NANOGP_ASSERT(window);

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);

    if(!SDL_GetWindowWMInfo(window, &wminfo)) {
        _ngctx_set_error(NGCTX_WMINFO_FAILED, SDL_GetError());
        return false;
    }

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    ngctx_d3d11_context* ngctx = (ngctx_d3d11_context*)NANOGP_MALLOC(sizeof(ngctx_d3d11_context));
    memset(ngctx, 0, sizeof(ngctx));
    ngctx->init_cookie = _NGCTX_INIT_COOKIE;
    ngctx->desc = *desc;
    ngctx->window = window;
    ngctx->width = width;
    ngctx->height = height;

    // create device and swap chain
    if(!_ngctx_d3d11_create_device(ngctx, wminfo.info.win.window)) {
        _ngctx_set_error(NGCTX_CREATE_CONTEXT_FAILED, "DirectX 11 failed to create device");
        return NULL;
    }

    // create default render target
    if(!_ngctx_d3d11_create_default_render_target(ngctx)) {
        _ngctx_set_error(NGCTX_CREATE_RENDER_TARGET_FAILED, "DirectX 11 failed to create default render target");
        _ngctx_d3d11_destroy_default_render_target(ngctx);
        _ngctx_d3d11_destroy_device(ngctx);
        NANOGP_FREE(ngctx);
        return NULL;
    }

    _ngctx_d3d11_active = ngctx;
    return ngctx;
}

void ngctx_d3d11_destroy(ngctx_d3d11_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    _ngctx_d3d11_destroy_default_render_target(ngctx);
    _ngctx_d3d11_destroy_device(ngctx);
    if(_ngctx_d3d11_active == ngctx)
        _ngctx_d3d11_active = NULL;
    NANOGP_FREE(ngctx);
}

bool ngctx_d3d11_activate(ngctx_d3d11_context* ngctx) {
    _ngctx_d3d11_active = ngctx;
    return true;
}

ngctx_isize ngctx_d3d11_get_drawable_size(ngctx_d3d11_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    ngctx_isize size;
    SDL_GetWindowSize(ngctx->window, &size.w, &size.h);
    return size;
}

bool ngctx_d3d11_set_swap_interval(ngctx_d3d11_context* ngctx, int interval) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    ngctx->swap_interval = interval;
    return true;
}

void ngctx_d3d11_swap(ngctx_d3d11_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);

    IDXGISwapChain_Present(ngctx->swap_chain, ngctx->swap_interval, 0);

    // handle window resizing
    int width, height;
    SDL_GetWindowSize(ngctx->window, &width, &height);
    if(ngctx->width != width || ngctx->height != height) {
        ngctx->width = width;
        ngctx->height = height;
        _ngctx_d3d11_update_default_render_target(ngctx);
    }
}

const void* ngctx_d3d11_render_target_view() {
    if(_ngctx_d3d11_active)
        return (const void*) _ngctx_d3d11_active->render_target_view;
    return NULL;
}

const void* ngctx_d3d11_depth_stencil_view() {
    if(_ngctx_d3d11_active)
        return (const void*) _ngctx_d3d11_active->depth_stencil_view;
    return NULL;
}

#endif // NANOGCTX_D3D11_BACKEND

#ifdef NANOGCTX_DUMMY_BACKEND

void ngctx_dummy_prepare_attributes(ngctx_desc* desc) {
}

ngctx_dummy_context* ngctx_dummy_create(SDL_Window* window, ngctx_desc* desc) {
    NANOGP_ASSERT(window);
    ngctx_dummy_context* ngctx = (ngctx_dummy_context*)NANOGP_MALLOC(sizeof(ngctx_dummy_context));
    memset(ngctx, 0, sizeof(ngctx));
    ngctx->init_cookie = _NGCTX_INIT_COOKIE;
    ngctx->window = window;
    return ngctx;
}

void ngctx_dummy_destroy(ngctx_dummy_context* ngctx) {
    NANOGP_ASSERT(ngctx->init_cookie == _NGCTX_INIT_COOKIE);
    NANOGP_FREE(ngctx);
}

bool ngctx_dummy_activate(ngctx_dummy_context* ngctx) {
    return true;
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

void ngctx_prepare_attributes(ngctx_desc* desc) {
    switch(desc->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            ngctx_gl_prepare_attributes(desc); return;
        #endif
        #ifdef NANOGCTX_D3D11_BACKEND
        case NGCTX_BACKEND_D3D11:
            ngctx_d3d11_prepare_attributes(desc); return;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            ngctx_dummy_prepare_attributes(desc); return;
        #endif
        default:
            NANOGP_UNREACHABLE; return;
    }
}

bool ngctx_create(ngctx_context* ngctx, SDL_Window* window, ngctx_desc* desc) {
    bool ok = false;
    switch(desc->backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            ngctx->gl = ngctx_gl_create(window, desc);
            ok = ngctx->gl != NULL;
            break;
        #endif
        #ifdef NANOGCTX_D3D11_BACKEND
        case NGCTX_BACKEND_D3D11:
            ngctx->d3d11 = ngctx_d3d11_create(window, desc);
            ok = ngctx->d3d11 != NULL;
            break;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            ngctx->dummy = ngctx_dummy_create(window, desc);
            ok = ngctx->dummy != NULL;
            break;
        #endif
        default:
            _ngctx_set_error(NGCTX_INVALID_BACKEND, "invalid backend");
            break;
    }
    if(ok)
        ngctx->backend = desc->backend;
    return ok;
}

void ngctx_destroy(ngctx_context ngctx) {
    switch(ngctx.backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            ngctx_gl_destroy(ngctx.gl); break;
        #endif
        #ifdef NANOGCTX_D3D11_BACKEND
        case NGCTX_BACKEND_D3D11:
            ngctx_d3d11_destroy(ngctx.d3d11); break;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            ngctx_dummy_destroy(ngctx.dummy); break;
        #endif
        default:
            NANOGP_UNREACHABLE; break;
    }
}

bool ngctx_activate(ngctx_context ngctx) {
    switch(ngctx.backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            return ngctx_gl_activate(ngctx.gl);
        #endif
        #ifdef NANOGCTX_D3D11_BACKEND
        case NGCTX_BACKEND_D3D11:
            return ngctx_d3d11_activate(ngctx.d3d11);
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            return ngctx_dummy_activate(ngctx.dummy);
        #endif
        default:
            NANOGP_UNREACHABLE; return false;
    }
}

bool ngctx_is_valid(ngctx_context ngctx) {
    return ngctx.backend != NGCTX_BACKEND_INVALID;
}

ngctx_isize ngctx_get_drawable_size(ngctx_context ngctx) {
    switch(ngctx.backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            return ngctx_gl_get_drawable_size(ngctx.gl);
        #endif
        #ifdef NANOGCTX_D3D11_BACKEND
        case NGCTX_BACKEND_D3D11:
            return ngctx_d3d11_get_drawable_size(ngctx.d3d11);
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            return ngctx_dummy_get_drawable_size(ngctx.dummy);
        #endif
        default:
            NANOGP_UNREACHABLE; return (ngctx_isize){0};
    }
}

bool ngctx_set_swap_interval(ngctx_context ngctx, int interval) {
    switch(ngctx.backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            return ngctx_gl_set_swap_interval(ngctx.gl, interval);
        #endif
        #ifdef NANOGCTX_D3D11_BACKEND
        case NGCTX_BACKEND_D3D11:
            return ngctx_d3d11_set_swap_interval(ngctx.d3d11, interval);
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            return ngctx_dummy_set_swap_interval(ngctx.dummy, interval);
        #endif
        default:
            NANOGP_UNREACHABLE; return false;
    }
}

void ngctx_swap(ngctx_context ngctx) {
    switch(ngctx.backend) {
        #ifdef NANOGCTX_GLCORE33_BACKEND
        case NGCTX_BACKEND_GLCORE33:
            ngctx_gl_swap(ngctx.gl); return;
        #endif
        #ifdef NANOGCTX_D3D11_BACKEND
        case NGCTX_BACKEND_D3D11:
            ngctx_d3d11_swap(ngctx.d3d11); return;
        #endif
        #ifdef NANOGCTX_DUMMY_BACKEND
        case NGCTX_BACKEND_DUMMY:
            ngctx_dummy_swap(ngctx.dummy); return;
        #endif
        default:
            NANOGP_UNREACHABLE; return;
    }
}

#endif // NANOGCTX_IMPL_INCLUDED
#endif // NANOGCTX_IMPL
