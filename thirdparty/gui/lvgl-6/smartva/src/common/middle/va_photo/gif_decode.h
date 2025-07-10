/*
 * =====================================================================================
 *
 *       Filename:  gif_decode.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年03月21日 09时55分48秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  langaojie
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef __GIF_DECODE_H__
#define __GIF_DECODE_H__
#include "gif_dec.h"

typedef enum
{
    GIF_STATUS_STOP,
    GIF_STATUS_PLAY,
}gif_status_e;

typedef enum
{
    GIF_MODE_CIRCLE,
    GIF_MODE_ONCE,
}gif_mode_e;

typedef struct
{
    void *addr;
    int width;
    int height;
}gif_buf_t;

typedef struct
{
    int x;
    int y;
    int w;
    int h;
}gif_rect_t;

typedef struct
{
    int (*display_frame)(void *src_buf, int src_width, int src_height);
    int (*rotate_frame)(void *src_buf, int src_width, int src_height, int rotate, void *dst_buf);
    int (*scaler_frame)(void *src_buf, int src_width, int src_height,\
                        void *dst_buf, int dst_width, int dst_height);
    int (*get_disp_para)(gif_rect_t *rect, int *scn_w, int *scn_h);
    void* (*requst_buf)(int size);
    void  (*free_buf)(void *viraddr);
}gif_cb_vector_t;



typedef struct
{
    int mode;
    int scaler;
    int rotate;
    int screen_width;
    int screen_height;
    gif_rect_t disp_area;
}gif_start_para_t;

typedef struct
{
    gif_start_para_t para;  //启动参数
    int status;             //播放状态
    int is_busy;            //线程状态
    int is_finish;          //播放一次完毕
    int is_first_frame_finish;  //播放完成第一帧
    gif_buf_t prev_buf;     //显示buffer
    gif_buf_t next_buf;     //预加载buffer
    gif_info_t *gif_info;   //当前gif信息
    gif_cb_vector_t cb;
}gif_ctrl_t;


//解码gif，用于动态显示
int gif_play_init(void);            //初始化gif播放器
int gif_play_exit(void);            //退出gif播放器
int gif_play_start(char *filename,int mode); //播放指定gif图片，循环播放
int gif_play_stop(void);            //停止播放
int gif_play_set_callback(gif_cb_vector_t *cb);    //设置回调，实现显示、旋转、缩放、内存操作
int gif_play_is_finish(void);       //是否播放完成一次
int gif_wait_first_frame(void);

//解码gif第一帧,用于静态显示
int gif_decode_sw(char *filename, void *output_buf, unsigned int *width, unsigned int *height, int *comp);


#endif