/*
sokol_gfx_ext.h - extensions for sokol_gfx
https://github.com/edubart/sokol_gp
*/

#ifndef SOKOL_GFX_EXT_INCLUDED
#define SOKOL_GFX_EXT_INCLUDED

#ifndef SOKOL_GFX_INCLUDED
#error "Please include sokol_gfx.h before sokol_gfx_ext.h"
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

SOKOL_API_DECL void sg_query_image_pixels(sg_image img_id, void* pixels, int size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GFX_EXT_INCLUDED

#ifdef SOKOL_GFX_EXT_IMPL
#ifndef SOKOL_GFX_EXT_IMPL_INCLUDED
#define SOKOL_GFX_EXT_IMPL_INCLUDED

#ifndef SOKOL_GFX_IMPL_INCLUDED
#error "Please include sokol_gfx.h implementation before sokol_gp.h implementation"
#endif

#include <SDL2/SDL.h>

#if defined(SOKOL_GLCORE33)
void _sg_gl_query_image_pixels(_sg_image_t* img, void* pixels) {
    SOKOL_ASSERT(img->gl.target == GL_TEXTURE_2D);
    SOKOL_ASSERT(0 != img->gl.tex[img->cmn.active_slot]);
    _sg_gl_store_texture_binding(0);
    _sg_gl_bind_texture(0, img->gl.target, img->gl.tex[img->cmn.active_slot]);
    glGetTexImage(img->gl.target, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    _SG_GL_CHECK_ERROR();
}

#elif defined(SOKOL_D3D11)
static uint32_t _sg_d3d11_dxgi_format_to_sdl_pixel_format(DXGI_FORMAT dxgi_format) {
    switch(dxgi_format) {
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return SDL_PIXELFORMAT_ARGB8888;
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return SDL_PIXELFORMAT_RGB888;
        default:
            return SDL_PIXELFORMAT_UNKNOWN;
    }
}

void _sg_d3d11_query_image_pixels(_sg_image_t* img, void* pixels) {
    SOKOL_ASSERT(_sg.d3d11.ctx);
    SOKOL_ASSERT(img->d3d11.tex2d);
    HRESULT hr;
    _SOKOL_UNUSED(hr);

    // create staging texture
    ID3D11Texture2D* staging_tex = NULL;
    D3D11_TEXTURE2D_DESC staging_desc = {
        .Width = (UINT)img->cmn.width,
        .Height = (UINT)img->cmn.height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = img->d3d11.format,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .Usage = D3D11_USAGE_STAGING,
        .BindFlags = 0,
        .CPUAccessFlags = D3D11_CPU_ACCESS_READ,
        .MiscFlags = 0
    };
    hr = ID3D11Device_CreateTexture2D(_sg.d3d11.dev, &staging_desc, NULL, &staging_tex);
    SOKOL_ASSERT(SUCCEEDED(hr));

    // copy pixels to staging texture
    ID3D11DeviceContext_CopySubresourceRegion(_sg.d3d11.ctx,
        (ID3D11Resource*)staging_tex,
        0, 0, 0, 0,
        (ID3D11Resource*)img->d3d11.tex2d,
        0, NULL);

    // copy pixels from staging texture
    D3D11_MAPPED_SUBRESOURCE msr = {0};
    hr = ID3D11DeviceContext_Map(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0, D3D11_MAP_READ, 0, &msr);
    SOKOL_ASSERT(SUCCEEDED(hr));
    int res = SDL_ConvertPixels(
        img->cmn.width, img->cmn.height,
        _sg_d3d11_dxgi_format_to_sdl_pixel_format(staging_desc.Format),
        msr.pData, msr.RowPitch,
        SDL_PIXELFORMAT_RGBA8888,
        pixels, img->cmn.width * 4);
    SOKOL_ASSERT(res == 0);
    _SOKOL_UNUSED(res);
    ID3D11DeviceContext_Unmap(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0);

    if(staging_tex) ID3D11Texture2D_Release(staging_tex);
}

#endif

void sg_query_image_pixels(sg_image img_id, void* pixels, int size) {
    SOKOL_ASSERT(pixels);
    SOKOL_ASSERT(img_id.id != SG_INVALID_ID);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    SOKOL_ASSERT(img);
    SOKOL_ASSERT(size >= (img->cmn.width * img->cmn.height * 4));
#if defined(_SOKOL_ANY_GL)
    _sg_gl_query_image_pixels(img, pixels);
#elif defined(SOKOL_D3D11)
    _sg_d3d11_query_image_pixels(img, pixels);
#endif
}

#endif // SOKOL_GFX_EXT_IMPL_INCLUDED
#endif // SOKOL_GFX_EXT_IMPL
