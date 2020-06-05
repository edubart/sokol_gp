#include "sample_common.h"

sg_image fb_image;
sg_pass fb_pass;
sg_image fb_captured_image;
sg_image screen_captured_image;
sg_features features;

sg_image capture_fb_image() {
    sgp_lock();
    sg_image_info info = sg_query_image_info(fb_image);
    int num_pixels = info.width * info.height * 4;
    void *pixels = SOKOL_MALLOC(num_pixels);
    assert(pixels);
    sg_query_image_pixels(fb_image, pixels, num_pixels);
    sg_image_desc image_desc = {
        .width = info.width,
        .height = info.height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .content = {.subimage = {{{.ptr = pixels, .size = num_pixels}}}}
    };
    sg_image image = sg_make_image(&image_desc);
    assert(fb_image.id != SG_INVALID_ID);
    SOKOL_FREE(pixels);
    sgp_unlock();
    return image;
}

sg_image capture_screen_image(int x, int y, int w , int h) {
    sgp_lock();
    sg_image_info info = sg_query_image_info(fb_image);
    int num_pixels = w * h * 4;
    void *pixels = SOKOL_MALLOC(num_pixels);
    assert(pixels);
    sg_query_pixels(x, y, w, h, true, pixels, num_pixels);
    sg_image_desc image_desc = {
        .width = info.width,
        .height = info.height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .content = {.subimage = {{{.ptr = pixels, .size = num_pixels}}}}
    };
    sg_image image = sg_make_image(&image_desc);
    assert(fb_image.id != SG_INVALID_ID);
    SOKOL_FREE(pixels);
    sgp_unlock();
    return image;
}

void draw_fbo() {
    sg_pass_action pass_action = {
        .colors = {{.action = SG_ACTION_CLEAR, .val = {0.0f, 0.0f, 0.0f, 0.0f}}},
        .depth = {.action = SG_ACTION_DONTCARE},
        .stencil = {.action = SG_ACTION_DONTCARE},
    };
    sgp_begin_pass(fb_pass, &pass_action);
    if(!features.origin_top_left)
        sgp_ortho(0, 128, 128, 0);
    sgp_set_color(1.0f, 0.0f, 0.0f, 1.0f);
    sgp_draw_filled_triangle(0, 0, 128, 0, 64, 128);
    sgp_end_pass();

    if(fb_captured_image.id != SG_INVALID_ID) {
        sgp_lock();
        sg_destroy_image(fb_captured_image);
        sgp_unlock();
    }
    fb_captured_image = capture_fb_image();
}

void swap(int width, int height) {
    if(screen_captured_image.id != SG_INVALID_ID) {
        sgp_lock();
        sg_destroy_image(screen_captured_image);
        sgp_unlock();
    }
    screen_captured_image = capture_screen_image(width/2-256, height/2-64, 128, 128);
}

void draw(int width, int height) {
    sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
    draw_fbo();

    sgp_draw_textured_rect(fb_captured_image, width/2-256, height/2-64, 128, 128);

    if(screen_captured_image.id != SG_INVALID_ID) {
        if(!features.origin_top_left)
            sgp_ortho(0, width, height, 0);
        sgp_draw_textured_rect(screen_captured_image,  width/2+128, height/2-64, 128, 128);
    }
}

bool init() {
    features = sg_query_features();
    sg_image_desc fb_image_desc = {
        .render_target = true,
        .width = 128,
        .height = 128
    };
    fb_image = sg_make_image(&fb_image_desc);
    if(fb_image.id == SG_INVALID_ID)
        return false;
    sg_pass_desc pass_desc = {
        .color_attachments = {{.image = fb_image}},
    };
    fb_pass = sg_make_pass(&pass_desc);
    if(fb_pass.id == SG_INVALID_ID)
        return false;
    return true;
}

void terminate() {
    sg_destroy_image(fb_image);
    sg_destroy_pass(fb_pass);
}

int main(int argc, char *argv[]) {
    return sample_app((sample_app_desc){
        .init = init,
        .terminate = terminate,
        .draw = draw,
        .swap = swap,
        .argc = argc,
        .argv = argv
    });
}
