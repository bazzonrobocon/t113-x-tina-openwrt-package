/*
 * =====================================================================================
 *
 *       Filename:  g2d_interface.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年02月25日 16时00分30秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef __G2D_INTERFACE_H__
#define __G2D_INTERFACE_H__
#include "g2d_driver_enh.h"

typedef enum{
	G2D_ROTATE_0,
	G2D_ROTATE_90,
	G2D_ROTATE_180,
	G2D_ROTATE_270,
	G2D_ROTATE_H,
	G2D_ROTATE_V,
	G2D_ROTATE_MAX,
}__g2d_rotate_mode;

typedef struct {
	int x;
	int y;
	unsigned int w;
	unsigned int h;
} g2d_rect_t;

typedef struct
{
    int alpha;
    int alpha_mode;
	int format;
	unsigned int width;
	unsigned int height;
    unsigned int length;
	g2d_rect_t clip_rect;
    unsigned long vir_addr[3];
    unsigned long phy_addr[3];
} __g2d_para_base_t;

int g2d_init(void);
int g2d_exit(void);
int g2d_ext_fill(__g2d_para_base_t *src,unsigned int color);
int g2d_base_blt(__g2d_para_base_t *src,__g2d_para_base_t *dst,g2d_blt_flags blt_cmd);
int g2d_base_bld(__g2d_para_base_t *src, __g2d_para_base_t *dst, int src_num, g2d_bld_cmd_flag g2d_bld_cmd);
int g2d_ext_bld_alpha(__g2d_para_base_t *src, __g2d_para_base_t *dst, int src_num);
int g2d_ext_rotate(__g2d_para_base_t *src,__g2d_para_base_t *dst,int angle);
int g2d_ext_blt_conver(__g2d_para_base_t *src,__g2d_para_base_t *dst);
int g2d_format_conver(__g2d_para_base_t *src, __g2d_para_base_t *dst);


#endif /*__G2D_INTERFACE_H__*/
