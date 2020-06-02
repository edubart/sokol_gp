#include "sample_common.h"

int frame = 0;

void draw_3rects(float brightness, float alpha) {
    sgp_set_color(brightness, 0.0f, 0.0f, alpha);
    sgp_draw_filled_rect(0, 0, 10, 10);
    sgp_set_color(0.0f, brightness, 0.0f, alpha);
    sgp_draw_filled_rect(0, 5, 10, 10);
    sgp_set_color(0.0f, 0.0f, brightness, alpha);
    sgp_draw_filled_rect(5, 2.5f, 10, 10);
}

void draw_rects(float ratio) {
    sgp_ortho(0, 100*ratio, 0, 100);
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);

    // none
    sgp_set_blend_mode(SGP_BLENDMODE_NONE);
    sgp_push_transform();
    sgp_translate(10,10);
    draw_3rects(1.0f, 1.0f);
    sgp_pop_transform();

    // blend
    sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
    sgp_push_transform();
    sgp_translate(30,10);
    draw_3rects(1.0f, 0.5f);
    sgp_pop_transform();

    // add
    sgp_set_blend_mode(SGP_BLENDMODE_ADD);
    sgp_push_transform();
    sgp_translate(50,10);
    draw_3rects(1.0f, 1.0f);
    sgp_pop_transform();

    // mod
    sgp_set_blend_mode(SGP_BLENDMODE_MOD);
    sgp_push_transform();
    sgp_translate(10,30);
    draw_3rects(1.0f, 1.0f);
    sgp_pop_transform();

    // mod
    sgp_set_blend_mode(SGP_BLENDMODE_MUL);
    sgp_push_transform();
    sgp_translate(30,30);
    draw_3rects(1.0f, 1.0f);
    sgp_pop_transform();
}

void draw_checkboard(int width, int height) {
    sgp_set_color(0.2f, 0.2f, 0.2f, 1.0f);
    sgp_clear();
    sgp_set_color(0.4f, 0.4f, 0.4f, 1.0f);
    for(int y=0;y<height/32+1;y++)
        for(int x=0;x<width/32+1;x++)
            if((x+y) % 2 == 0)
                sgp_draw_filled_rect(x*32,y*32,32,32);
    sgp_reset_color();

}
void draw(int width, int height) {
    draw_checkboard(width, height);
    draw_rects(width/(float)height);

    frame++;
}

bool init() {
    return true;
}

void terminate() {
}

int main(int argc, char *argv[]) {
    return sample_app((sample_app_desc){
        .init = init,
        .terminate = terminate,
        .draw = draw,
        .argc = argc,
        .argv = argv
    });
}
