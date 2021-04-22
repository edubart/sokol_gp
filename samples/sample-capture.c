#include "sample_app.h"

sg_image fb_image;
sg_pass fb_pass;
sg_image fb_captured_image;
sg_image fb_captured_image2;
sg_features features;

sg_image capture_fb_image() {
    sg_image_info info = sg_query_image_info(fb_image);
    int num_pixels = info.width * info.height * 4;
    void *pixels = SOKOL_MALLOC(num_pixels);
    assert(pixels);
    sg_query_image_pixels(fb_image, pixels, num_pixels);
    sg_image_desc image_desc = {
        .width = info.width,
        .height = info.height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data = {.subimage = {{{.ptr = pixels, .size = num_pixels}}}}
    };
    sg_image image = sg_make_image(&image_desc);
    assert(sg_query_image_state(image) == SG_RESOURCESTATE_VALID);
    SOKOL_FREE(pixels);
    return image;
}

sg_image capture_screen_image(int x, int y, int w , int h) {
    sg_image_info info = sg_query_image_info(fb_image);
    int num_pixels = w * h * 4;
    void *pixels = SOKOL_MALLOC(num_pixels);
    assert(pixels);
    sg_query_pixels(x, y, w, h, true, pixels, num_pixels);
    sg_image_desc image_desc = {
        .width = info.width,
        .height = info.height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data = {.subimage = {{{.ptr = pixels, .size = num_pixels}}}}
    };
    sg_image image = sg_make_image(&image_desc);
    assert(sg_query_image_state(image) == SG_RESOURCESTATE_VALID);
    SOKOL_FREE(pixels);
    return image;

}

void draw_fbo() {
    sgp_begin(128, 128);
    if(!features.origin_top_left)
        sgp_ortho(0, 128, 128, 0);
    sgp_set_color(1.0f, 0.0f, 0.0f, 1.0f);
    sgp_draw_filled_triangle(0, 0, 128, 0, 64, 128);
    sg_pass_action pass_action = {
        .colors = {{.action = SG_ACTION_CLEAR, .value = {0.0f, 0.0f, 0.0f, 0.0f}}},
        .depth = {.action = SG_ACTION_DONTCARE},
        .stencil = {.action = SG_ACTION_DONTCARE},
    };
    sg_begin_pass(fb_pass, &pass_action);
    sgp_flush();

    sg_destroy_image(fb_captured_image2);
    fb_captured_image2 = capture_screen_image(0, 0, 128, 128);

    sgp_end();
    sg_end_pass();

    sg_destroy_image(fb_captured_image);
    fb_captured_image = capture_fb_image();
}

void draw() {
    sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
    draw_fbo();
    sgp_translate(app.width/2, app.height/2);
    sgp_draw_textured_rect(fb_captured_image, -256, -64, 128, 128);
    sgp_draw_textured_rect(fb_captured_image2,  128, -64, 128, 128);
}

bool init() {
    features = sg_query_features();
    sg_image_desc fb_image_desc = {
        .render_target = true,
        .width = 128,
        .height = 128
    };
    fb_image = sg_make_image(&fb_image_desc);
    if(sg_query_image_state(fb_image) != SG_RESOURCESTATE_VALID)
        return false;
    sg_pass_desc pass_desc = {
        .color_attachments = {{.image = fb_image}},
    };
    fb_pass = sg_make_pass(&pass_desc);
    if(sg_query_pass_state(fb_pass) != SG_RESOURCESTATE_VALID)
        return false;
    return true;
}

void terminate() {
    sg_destroy_image(fb_image);
    sg_destroy_pass(fb_pass);
}

int main(int argc, char *argv[]) {
    return sample_app_main(&(sample_app_desc){
        .init = init,
        .terminate = terminate,
        .draw = draw,
        .argc = argc,
        .argv = argv
    });
}
