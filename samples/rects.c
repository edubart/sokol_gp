#include "sample_common.h"

void draw(ng_context* ngctx, ngp_context* ngp) {
    int width, height;
    ngctx_get_drawable_size(ngctx, &width, &height);
    ngp_set_clear_color(ngp, (ngp_color){0, 0, 0, 1});
    ngp_set_color(ngp, (ngp_color){1, 0, 0, 1});
    ngp_begin(ngp, width, height);
    for(int y=64;y<height-128; y+=128)
        for(int x=64;x<width-128; x+=128)
            ngp_filled_rect(ngp, (ngp_rect){x, y, 64, 64});
    ngp_end(ngp);
    ngctx_swap(ngctx);
}

int main() {
    return sample_app(draw);
}
