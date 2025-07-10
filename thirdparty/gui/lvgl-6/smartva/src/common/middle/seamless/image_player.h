#ifndef __IMAGE_PLAYER_H__
#define __IMAGE_PLAYER_H__
#include "seamless.h"


int image_player_init(int mode);
int image_player_exit(void);
int image_player_start(image_out_para_t *out);
int image_player_stop(image_out_para_t *out);
int image_player_set_scn_rect(seamless_rect_t *rect);
int image_player_set_src_rect(seamless_rect_t *rect);
int image_load(image_in_para_t *in ,image_out_para_t *out);
int image_unload(image_out_para_t *out);

#endif
