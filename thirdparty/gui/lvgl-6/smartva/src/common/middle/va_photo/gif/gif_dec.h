/*
 * =====================================================================================
 *
 *       Filename:  gif.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年03月21日 10时18分12秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef __GIF_DEC_H__
#define __GIF_DEC_H__

#include "libnsgif.h"

typedef struct
{
    gif_animation gif;
    int width;
    int height;
    int frame_count;
    void *buffer;
}gif_info_t;

// int gif_dec_init(void);
// int gif_dec_exit(void);
gif_info_t* gif_dec_from_file(char *filename);
int gif_dec_frame(gif_info_t *gif_info,int index);
int gif_dec_finish(gif_info_t *gif_info);
int gif_dec_get_size(gif_info_t *gif_info,unsigned int *width,unsigned int *height);
int gif_dec_get_delay(gif_info_t *gif_info,int index,int *delay);
int gif_dec_get_outbuf(gif_info_t *gif_info,void **buf);
int gif_dec_set_outbuf(gif_info_t *gif_info,void *buf);
int gif_dec_get_frame_num(gif_info_t *gif_info,int *num);
#endif
