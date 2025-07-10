/*
 * =====================================================================================
 *
 *       Filename:  seamless.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年02月25日 15时38分12秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  langaojie
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef __SEAMLESS_H__
#define __SEAMLESS_H__

#include "rat.h"
#include "rat_npl.h"
#include "rat_common.h"
#include "seamless_conf.h"

typedef enum
{
	SEAMLESS_ROTATE_0         = 0x0,//无旋转
	SEAMLESS_ROTATE_90        = 0x1,//顺时针旋转90
	SEAMLESS_ROTATE_180       = 0x2,//顺时针旋转180
	SEAMLESS_ROTATE_270       = 0x3,//顺时针旋转270
}seamless_rotate_e;

typedef enum
{
	SEAMLESS_MODE_VIDEO_ONLY  = 0x0,//单独播放视频
	SEAMLESS_MODE_IMAGE_ONLY  = 0x1,//单独播放图片
	SEAMLESS_MODE_MIXED		  = 0x2,//混合播放视频图片
	SEAMLESS_MODE_SPLIT	  = 0x3,//同时播放视频图片,如果只有视频或者图片，将转换为only模式
	SEAMLESS_MODE_SPLIT_SYNC  = 0x4,//同步播放视频图片,一个视频对应一张图片。如果只有视频或者图片，将转换为only模式
}seamless_show_mode_e;

typedef enum
{
	SEAMLESS_MEDIA_VIDEO      = 0x0,//视频
	SEAMLESS_MEDIA_IMAGE      = 0x1,//图片
	SEAMLESS_MEDIA_AUDIO      = 0x2,//音频
	SEAMLESS_MEDIA_UNKNOW     = 0x3,//未知媒体
}seamless_media_type_e;


typedef enum
{
	SEAMLESS_STATUS_STOP	  = 0x0,//播放
	SEAMLESS_STATUS_PLAY	  = 0x1,//停止
}seamless_status_e;

typedef enum
{
	SEAMLESS_ORDER_RANDOM     = 0x0,//随机播放
	SEAMLESS_ORDER_CIRCLE	  = 0x1,//顺序循环播放
}seamless_play_order_e;

typedef enum
{
	SEAMLESS_THUMB_YUV420     = 0x0,//设置缩略图格式为yuv（暂未支持）
	SEAMLESS_THUMB_ARGB	  = 0x1,//设置缩略图格式为argb
	SEAMLESS_THUMB_ABGR	  = 0x2,//设置缩略图格式为argb
	SEAMLESS_THUMB_RGB		  = 0x3,//设置缩略图格式为rgb（暂未支持）
	SEAMLESS_THUMB_BGR	      = 0x4,//设置缩略图格式为bgr
	SEAMLESS_THUMB_AUTO       = 0x5,//由解码器设置格式（推荐使用）
}seamless_thumb_format_e;

typedef struct
{
    char *filename;
    int   width;
    int   height;
    int   format;
    int   rotate;
}image_in_para_t;

typedef struct
{
	int   buf_fd;
    void *buf_addr[3];
    int   buf_size;
    int   buf_width;
    int   buf_height;
    int   width;
    int   height;
    int   format;
}image_out_para_t;

typedef struct{
	char						filename[256];		//文件名，可以换成rat index，减少内存使用
	//int						media_index;		//媒体id，使用rat转换为filename
	seamless_media_type_e	    media_type;			//媒体类型
	int						    prepare_time;	    //媒体切换时，提前加载时间
	image_out_para_t            image;
}seamless_media_info_t;

typedef struct
{
    char *path;
    int   type;
    HRAT  handle;               //文件搜索句柄
    int   index;                  //当前文件索引号
    int   total;                  //文件总数
    HRATNPL npl;                //播放列表
} seamless_rat_ctrl_t;

typedef struct{
    int width;
    int height;
}seamless_size_t;

typedef struct{
    int x;
    int y;
    int width;
    int height;
}seamless_rect_t;


typedef struct {
    char					   *source_path;        //资源文件搜索路径。
	seamless_rect_t				video_rect;		//视频逻辑显示区域。
	seamless_rect_t			    image_rect;			//视频逻辑显示区域。
	seamless_show_mode_e        show_mode;			//显示模式，video only、image+video等
	seamless_play_order_e		play_order;		//播放模式，circle、random
	seamless_rotate_e			screen_rotate;		//屏幕旋转角度。旋转屏需要设置本参数。请勿随意设置，否则可能花屏
	seamless_size_t             screen_size;

}seamless_init_para_t;

typedef struct {
	seamless_status_e		    status;				//播放状态
    seamless_rat_ctrl_t		    rat_video;			//视频搜索结果
    seamless_rat_ctrl_t		    rat_image;			//图片搜索结果
    seamless_rat_ctrl_t		    rat_mixed;			//视频+图片搜索结果
	seamless_media_info_t		loading;			//下一媒体信息
	seamless_media_info_t		playing;			//当前媒体信息
    seamless_init_para_t        init_para;          //初始化信息
} seamless_ctrl_t ;

#include "de_interface.h"
#include "mem_interface.h"
#include "g2d_interface.h"
#include "image_player.h"
#include "video_player.h"
#include "media_search.h"

int SeamlessInit(seamless_init_para_t *para);
int SeamlessStart(void);
int SeamlessStop(void);
int SeamlessExit(void);
int SeamlessGetStatus(void);


#endif
