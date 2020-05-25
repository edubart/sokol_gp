#include "sample_common.h"

void draw(ng_context* ngctx, ngp_context* ngp) {
    int width, height;
    ngctx_get_drawable_size(ngctx, &width, &height);
    ngp_set_clear_color(ngp, (ngp_color){0.0f, 0.0f, 0.0f, 1.0f});
    ngp_begin(ngp, width, height);
    ngp_translate(ngp, width / 2.0f, height / 2.0f);
    for(int y=64;y<height-128; y+=128) {
        for(int x=64;x<width-128; x+=128) {
            ngp_set_color(ngp, (ngp_color){x / (float)width, y / (float)height, 0.0f, 1.0f});
            ngp_filled_rect(ngp, (ngp_rect){x, y, 64, 64});
        }
    }
    ngp_end(ngp);
    ngctx_swap(ngctx);
}

int main() {
    return sample_app(draw);
}
