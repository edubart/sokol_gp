#include "sample_common.h"

void render(ng_context* ngctx, ngp_context* ngp) {
    float time = SDL_GetTicks() / 1000.0f;
    int width, height;
    ngctx_get_drawable_size(ngctx, &width, &height);
    ngp_set_clear_color(ngp, (ngp_color){0.0f, 0.0f, 0.0f, 1.0f});
    ngp_begin(ngp, width, height);

    int size = 128;
    int hsize = size / 2;
    float t = (1.0f+sinf(time))/2.0f;
    ngp_rect rect = {0, 0, size, size};

    // top left
    ngp_push_transform(ngp);
    ngp_translate(ngp, width*0.25f - hsize, height*0.25f - hsize);
    ngp_translate(ngp, 2*size*t - size, 0.0f);
    ngp_set_color(ngp, (ngp_color){t, 0.5f, 0.5f, 1.0f});
    ngp_filled_rect(ngp, rect);
    ngp_pop_transform(ngp);

    // top right
    ngp_push_transform(ngp);
    ngp_translate(ngp, width*0.75f - hsize, height*0.25f - hsize);
    ngp_rotate_at(ngp, time, hsize, hsize);
    ngp_set_color(ngp, (ngp_color){0.5f, t, 0.5f, 1.0f});
    ngp_filled_rect(ngp, rect);
    ngp_pop_transform(ngp);

    // bot left
    ngp_push_transform(ngp);
    ngp_translate(ngp, width*0.25f - hsize, height*0.75f - hsize);
    ngp_scale_at(ngp, t + 0.5f, t + 0.25f, hsize, hsize);
    ngp_set_color(ngp, (ngp_color){0.5f, 0.5f, t, 1.0f});
    ngp_filled_rect(ngp, rect);
    ngp_pop_transform(ngp);

    ngp_end(ngp);
    ngctx_swap(ngctx);
}

int main() {
    return sample_app(render);
}
