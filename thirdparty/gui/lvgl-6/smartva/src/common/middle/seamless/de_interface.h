/*
 * =====================================================================================
 *
 *       Filename:  de_interface.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年02月25日 15时59分40秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef __DE_INTERFACE_H__
#define __DE_INTERFACE_H__
#include "seamless.h"
#include <videoOutPort.h>
#include "sunxi_display2.h"

#define layer_info_t struct disp_layer_config2

typedef enum{
    LAYER_MODE_NORMAL,
    LAYER_MODE_SCALER,
}layer_mode_e;

typedef  struct mytest_t{
    int y;
}mytest_t;

typedef struct{
    mytest_t t;
    int x;
    int y;
    int width;
    int height;
}layer_rect_t;



typedef struct{
    int   use_phyaddr;
    void *phyaddr[3];
    int   fd;
    int   width;
    int   height;
    layer_rect_t crop;
    int   format;
}layer_fb_t;



int layer_init(void);
int layer_exit(void);
int layer_format_to_de(int format);
layer_info_t *layer_request_by_mode(int mode);
layer_info_t *layer_request_by_id(int channel_id, int layer_id);
int layer_set_enable(layer_info_t *layer, int enable);
int layer_set_fb(layer_info_t *layer, layer_fb_t *fb);
int layer_set_src_rect(layer_info_t *layer, layer_rect_t *rect);
int layer_set_scn_rect(layer_info_t *layer, layer_rect_t *rect);
int layer_set_cache(int enable);
int layer_release(layer_info_t *layer);

#endif /*endif __DE_INTERFACE_H__*/
