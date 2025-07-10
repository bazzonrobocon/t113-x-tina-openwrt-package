/*
 * =====================================================================================
 *
 *       Filename:  de_interface.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年02月25日 15时59分34秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "de_interface.h"
#include "bs_widget.h"
#include "image.h"
#include "ion_mem_alloc.h"

#include <videoOutPort.h>

extern image_display_t *get_display_opr(void);
extern image_process_t *get_process_opr(void);

// static image_display_t *disp_opr;
// static image_process_t *proc_opr;
static int disp_fd;


int do_disp_cmd(int cmd, void *pinfo)
{
    unsigned long args[4] = {0};
	unsigned int ret = 0;

	args[0] = 0;
	args[1] = (unsigned long)pinfo;
	args[2] = 1;
	ret = ioctl(disp_fd, cmd, args);
	if (ret != 0)
    {
		SEAMLESS_LOG("fail to get para\n");
		ret = -1;
	}
    return ret;
}
//初始化显示驱动
int layer_init(void)
{
    disp_fd = open("/dev/disp",O_RDWR);
    return SEAMLESS_OK;
}

int layer_exit(void)
{
    close(disp_fd);
    return SEAMLESS_OK;
}

int layer_format_to_de(int format)
{
	int disp_fmt;

	switch (format)
    {
		case SEAMLESS_THUMB_ARGB:
			disp_fmt = DISP_FORMAT_ARGB_8888;
			break;
        case SEAMLESS_THUMB_BGR:
            disp_fmt = DISP_FORMAT_BGR_888;
            break;
        case SEAMLESS_THUMB_ABGR:
            disp_fmt = DISP_FORMAT_ABGR_8888;
            break;
        case SEAMLESS_THUMB_YUV420:
            disp_fmt = DISP_FORMAT_YUV420_P;
            break;
		default:
			disp_fmt = DISP_FORMAT_ARGB_8888;
			break;
	}
	return disp_fmt;
}

layer_info_t *layer_request_by_mode(int mode)
{
    struct disp_layer_config2 config;
    int i;
    layer_info_t *info = NULL;

    info = malloc(sizeof(layer_info_t));
    if(info == NULL)
    {
        return NULL;
    }
    memset(info,0,sizeof(struct disp_layer_config2));

    if(mode == LAYER_MODE_NORMAL)
    {
        for(i=0;i<4;i++)
        {
            memset(&config,0,sizeof(struct disp_layer_config2));
            config.channel  = 1;
            config.layer_id = 0;
            config.enable   = -1;
            do_disp_cmd(DISP_LAYER_GET_CONFIG2,&config);
            if(config.enable == 0)
            {
                break;
            }
        }
        if(i == 4)
        {
            free(info);
            return NULL;
        }
        // info->layer   = i;
        // info->channel = 0;
    }
    else if(mode == LAYER_MODE_SCALER)
    {
        for(i=0;i<4;i++)
        {
            memset(&config,0,sizeof(struct disp_layer_config2));
            config.channel  = 0;
            config.layer_id = i;
            config.enable   = -1;
            do_disp_cmd(DISP_LAYER_GET_CONFIG2,&config);
            if(config.enable == 0)
            {
                break;
            }
            printf("langaojie i = %d\n",i);
        }
        if(i == 4)
        {
            printf("LAYER_MODE_SCALER\n");
            free(info);
            return NULL;
        }
        // info->layer   = i;
        // info->channel = 0;
    }

	// config.info.screen_win.x = 0;
	// config.info.screen_win.y = 0;
	// config.info.screen_win.width  = width;
	// config.info.screen_win.height = height;
	// config.info.mode           = LAYER_MODE_BUFFER;
	// config.info.alpha_mode     = 0;
	// config.info.alpha_value    = 0x80;
	// config.info.fb.flags       = DISP_BF_NORMAL;
	// config.info.fb.scan        = DISP_SCAN_PROGRESSIVE;
	// config.info.fb.color_space = (rect->height < 720) ? DISP_BT601 : DISP_BT709;
	// config.info.zorder         = info->channel*4 + info->layer;
    // info->config = config;

    return info;
}

layer_info_t *layer_request_by_id(int channel_id, int layer_id)
{
    layer_info_t *info = NULL;

    info = malloc(sizeof(layer_info_t));
    if(info == NULL)
    {
        return NULL;
    }
    memset(info,0,sizeof(struct disp_layer_config2));
    info->channel  = channel_id;
    info->layer_id = layer_id;
    do_disp_cmd(DISP_LAYER_GET_CONFIG2, info);
    // info->channel = channel_id;
    // info->layer   = layer_id;

    return info;
}

int layer_set_enable(layer_info_t *layer, int enable)
{
    // if(enable == layer->enable)
    // {
    //     return SEAMLESS_OK;
    // }
    layer->enable = enable;
    do_disp_cmd(DISP_LAYER_SET_CONFIG2,layer);
    if(enable == 0)
    {
        usleep(17000);
        do_disp_cmd(DISP_LAYER_SET_CONFIG2,layer);
    }
    return 0;
}

int layer_set_fb(layer_info_t *layer, layer_fb_t *fb)
{
    layer->info.fb.fd             = fb->fd;
    layer->info.fb.size[0].width  = fb->width;
    layer->info.fb.size[0].height = fb->height;
    layer->info.fb.size[1].width  = fb->width;
    layer->info.fb.size[1].height = fb->height;
    layer->info.fb.size[2].width  = fb->width;
    layer->info.fb.size[2].height = fb->height;
    layer->info.fb.format         = fb->format;
    if(layer->info.fb.format == DISP_FORMAT_YUV420_P)
    {
        layer->info.fb.size[1].width  = fb->width/2;
        layer->info.fb.size[1].height = fb->height/2;
        layer->info.fb.size[2].width  = fb->width/2;
        layer->info.fb.size[2].height = fb->height/2;
    }
    return do_disp_cmd(DISP_LAYER_SET_CONFIG2,layer);
}

int layer_set_src_rect(layer_info_t *layer, layer_rect_t *rect)
{
    layer->info.fb.crop.x = rect->x;
	layer->info.fb.crop.y = rect->y;
	layer->info.fb.crop.width  = rect->width;
	layer->info.fb.crop.height = rect->height;
    layer->info.fb.crop.x = layer->info.fb.crop.x << 32;
	layer->info.fb.crop.y = layer->info.fb.crop.y << 32;
	layer->info.fb.crop.width  = layer->info.fb.crop.width  << 32;
	layer->info.fb.crop.height = layer->info.fb.crop.height << 32;

    return do_disp_cmd(DISP_LAYER_SET_CONFIG2, layer);
}

int layer_set_scn_rect(layer_info_t *layer, layer_rect_t *rect)
{
    layer->info.screen_win.x = rect->x;
	layer->info.screen_win.y = rect->y;
	layer->info.screen_win.width = rect->width;
	layer->info.screen_win.height = rect->height;

    return do_disp_cmd(DISP_LAYER_SET_CONFIG2, layer);
}

int layer_set_cache(int enable)
{
    unsigned long temp = enable;
    return do_disp_cmd(DISP_SHADOW_PROTECT, (void*)temp);
}

int layer_release(layer_info_t *layer)
{
    layer->enable = 0;
    do_disp_cmd(DISP_LAYER_SET_CONFIG2, layer);
    free(layer);
    return 0;
}
