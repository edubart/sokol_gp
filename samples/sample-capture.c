#include "sample_app.h"

static sg_image fb_image;
static sg_pass fb_pass;
static sg_image fb_captured_image;
static sg_image fb_captured_image2;
static sg_features features;

static sg_image capture_fb_image(void) {
    sg_image_info info = sg_query_image_info(fb_image);
    size_t num_pixels = (size_t)(info.width * info.height * 4);
    void *pixels = SOKOL_MALLOC(num_pixels);
    assert(pixels);
    sg_query_image_pixels(fb_image, pixels, num_pixels);
    sg_image_desc image_desc;
    memset(&image_desc, 0, sizeof(image_desc));
    image_desc.width = info.width;
    image_desc.height = info.height;
    image_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    image_desc.data.subimage[0][0].ptr = pixels;
    image_desc.data.subimage[0][0].size = num_pixels;
    sg_image image = sg_make_image(&image_desc);
    assert(sg_query_image_state(image) == SG_RESOURCESTATE_VALID);
    SOKOL_FREE(pixels);
    return image;
}

static sg_image capture_screen_image(int x, int y, int w , int h) {
    sg_image_info info = sg_query_image_info(fb_image);
    size_t num_pixels = (size_t)(info.width * info.height * 4);
    void *pixels = SOKOL_MALLOC(num_pixels);
    assert(pixels);
    sg_query_pixels(x, y, w, h, true, pixels, num_pixels);
    sg_image_desc image_desc;
    memset(&image_desc, 0, sizeof(image_desc));
    image_desc.width = info.width;
    image_desc.height = info.height;
    image_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    image_desc.data.subimage[0][0].ptr = pixels;
    image_desc.data.subimage[0][0].size = num_pixels;
    sg_image image = sg_make_image(&image_desc);
    assert(sg_query_image_state(image) == SG_RESOURCESTATE_VALID);
    SOKOL_FREE(pixels);
    return image;

}

static void draw_fbo(void) {
    sgp_begin(128, 128);
    if(!features.origin_top_left)
        sgp_ortho(0, 128, 128, 0);
    sgp_set_color(1.0f, 0.0f, 0.0f, 1.0f);
    sgp_draw_filled_triangle(0, 0, 128, 0, 64, 128);
    sg_pass_action pass_action;
    memset(&pass_action, 0, sizeof(sg_pass_action));
    pass_action.colors[0].action = SG_ACTION_CLEAR;
    pass_action.depth.action = SG_ACTION_DONTCARE;
    pass_action.stencil.action = SG_ACTION_DONTCARE;
    sg_begin_pass(fb_pass, &pass_action);
    sgp_flush();

    sg_destroy_image(fb_captured_image2);
    fb_captured_image2 = capture_screen_image(0, 0, 128, 128);

    sgp_end();
    sg_end_pass();

    sg_destroy_image(fb_captured_image);
    fb_captured_image = capture_fb_image();
}

static void draw(void) {
    sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
    draw_fbo();
    sgp_translate(app.width/2, app.height/2);
    sgp_set_image(0, fb_captured_image);
    sgp_draw_textured_rect(-256, -64, 128, 128);
    sgp_set_image(0, fb_captured_image2);
    sgp_draw_textured_rect(128, -64, 128, 128);
    sgp_reset_image(0);
}

static bool init(void) {
    features = sg_query_features();
    sg_image_desc fb_image_desc;
    memset(&fb_image_desc, 0, sizeof(sg_image_desc));
    fb_image_desc.render_target = true;
    fb_image_desc.width = 128;
    fb_image_desc.height = 128;
    fb_image = sg_make_image(&fb_image_desc);
    if(sg_query_image_state(fb_image) != SG_RESOURCESTATE_VALID)
        return false;
    sg_pass_desc pass_desc;
    memset(&pass_desc, 0, sizeof(pass_desc));
    pass_desc.color_attachments[0].image = fb_image;
    fb_pass = sg_make_pass(&pass_desc);
    if(sg_query_pass_state(fb_pass) != SG_RESOURCESTATE_VALID)
        return false;
    return true;
}

static void terminate(void) {
    sg_destroy_image(fb_image);
    sg_destroy_pass(fb_pass);
}

int main(int argc, char *argv[]) {
    sample_app_desc app_desc;
    memset(&app_desc, 0, sizeof(app_desc));
    app_desc.init = init;
    app_desc.terminate = terminate;
    app_desc.draw = draw;
    app_desc.argc = argc;
    app_desc.argv = argv;
    return sample_app_main(&app_desc);
}
