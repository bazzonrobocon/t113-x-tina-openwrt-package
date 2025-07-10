/*
 * =====================================================================================
 *
 *       Filename:  image_player.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年04月01日 15时20分14秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "seamless.h"
#include "jpg_decode.h"
#include "png_decode.h"
#include "mem_interface.h"
#include "image_player.h"

#define ALIGN_TO_8B(x)    ((((x) + (1 <<  3) - 1) >>  3) <<  3)

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "de_interface.h"
struct disp_layer_config2 *h_layer;


int image_unload(image_out_para_t *out);

static int image_buf2fb(image_out_para_t *out, layer_fb_t *fb)
{
    if(out->buf_fd != 0)
    {
        fb->use_phyaddr = 0;
        fb->fd = out->buf_fd;
        fb->phyaddr[0] = NULL;
        fb->phyaddr[1] = NULL;
        fb->phyaddr[2] = NULL;
    }
    else
    {
        fb->use_phyaddr = 1;
        fb->fd = out->buf_fd;
        fb->phyaddr[0] = out->buf_addr[0];
        fb->phyaddr[1] = out->buf_addr[1];
        fb->phyaddr[2] = out->buf_addr[2];
    }
    fb->width  = out->buf_width;
    fb->height = out->buf_height;
    fb->crop.x   = 0;
    fb->crop.y   = 0;
    fb->crop.width   = out->width;
    fb->crop.height  = out->height;
    fb->format = layer_format_to_de(out->format);

    return SEAMLESS_OK;
}

//mode = 0 视频图片共用图层
//mode = 1 图片使用UI图层
int image_player_init(int mode)
{
    if(mode == 0)
    {
        h_layer = layer_request_by_id(0, 0);
    }
    else
    {
        h_layer = layer_request_by_mode(1);
    }
    if(h_layer != NULL)
    {
        return SEAMLESS_OK;
    }
    return SEAMLESS_FAIL;
}

int image_player_exit(void)
{
    layer_set_enable(h_layer, 0);
    layer_release(h_layer);
    return SEAMLESS_OK;
}

int image_player_start(image_out_para_t *out)
{
    layer_fb_t fb;
    image_out_para_t out_temp;

    out_temp.buf_fd = h_layer->info.fb.fd;
    image_buf2fb(out, &fb);
    layer_set_fb(h_layer, &fb);
    layer_set_enable(h_layer, 1);
    image_unload(&out_temp);

    return SEAMLESS_OK;
}

int image_player_stop(image_out_para_t *out)
{
    layer_set_enable(h_layer,0);
    return SEAMLESS_OK;
}

int image_player_set_scn_rect(seamless_rect_t *rect)
{
    layer_rect_t scn_rect;
    scn_rect.x = rect->x;
    scn_rect.y = rect->y;
    scn_rect.width  = rect->width;
    scn_rect.height = rect->height;

    return layer_set_scn_rect(h_layer,&scn_rect);
}

int image_player_set_src_rect(seamless_rect_t *rect)
{
    layer_rect_t src_rect;
    src_rect.x = 0;//rect->x;
    src_rect.y = 0;//rect->y;
    src_rect.width  = rect->width;
    src_rect.height = rect->height;

    return layer_set_src_rect(h_layer,&src_rect);
}

static int check_image_type(char const *filename ,char *suffix)
{
	char *buff = strrchr(filename, '.');
	extern int  SLIB_stricmp(const char * p1_str, const char * p2_str);

	if (SLIB_stricmp((buff + 1), suffix) == 0) {
		return 1;
	} else {
		return 0;
	}
}

static int image_format_to_g2d(int format)
{
    int dst_format = -1;
    switch(format)
    {
        case SEAMLESS_THUMB_BGR:
            dst_format = G2D_FORMAT_BGR888;
            break;
        case SEAMLESS_THUMB_RGB:
            dst_format = G2D_FORMAT_RGB888;
            break;
        case SEAMLESS_THUMB_ABGR:
            dst_format = G2D_FORMAT_ABGR8888;
            break;
        case SEAMLESS_THUMB_YUV420:
            dst_format = G2D_FORMAT_YUV420_PLANAR;
            break;
    }
    return dst_format;
}

static int image_rotate_to_g2d(int rotate)
{
    int dst_rotate = -1;
    switch(rotate)
    {
        case SEAMLESS_ROTATE_0:
            dst_rotate = G2D_ROT_0;
            break;
        case SEAMLESS_ROTATE_90:
            dst_rotate = G2D_ROT_90;
            break;
        case SEAMLESS_ROTATE_180:
            dst_rotate = G2D_ROT_180;
            break;
        case SEAMLESS_ROTATE_270:
            dst_rotate = G2D_ROT_270;
            break;
    }
    return dst_rotate;
}

//解码一张图片
//当前只支持jpg png
int image_load(image_in_para_t *in ,image_out_para_t *out)
{
    int scalerdown = 0;
    int width = 0,height = 0;
    int comp = 4;
    unsigned int width_dec = 0,height_dec = 0;
    void *buf = NULL;
    int ret = 0;
    int src_format = SEAMLESS_THUMB_ABGR,dst_format = SEAMLESS_THUMB_ABGR;
    int dst_width = 0,dst_height = 0;

    ret = stbi_info(in->filename, &width, &height, &comp);
    if(ret < 0 || width == 0 || height == 0 || comp == 0)
    {
        SEAMLESS_ERR("image err!\n");
        return SEAMLESS_FAIL;
    }
/*解码图片*/
    comp = 4;
    if(check_image_type(in->filename,"jpg"))
    {
        printf("run to jpg\n");
        src_format = SEAMLESS_THUMB_BGR;
        scalerdown = jpg_get_scalerdown(width,height);
        width  = (width +((1<<scalerdown)-1)) >>scalerdown;
        height = (height+((1<<scalerdown)-1)) >>scalerdown;
        if(width > 2048 || height >2048)
        {
            printf("image w or h out of limit!\n");
            return SEAMLESS_FAIL;
        }

        buf = mem_palloc(width*height*comp);
        if(buf == NULL)
        {
            SEAMLESS_ERR("palloc fail!\n");
        }

        ret = jpg_decode_sw(in->filename,buf,&width_dec,&height_dec,&comp);
        if(ret != 0 || width != width_dec || height != height_dec)
        {
            SEAMLESS_ERR("decode fail!\n");
            mem_pfree(buf,width*height*comp);
            return SEAMLESS_FAIL;
        }
    }
    else if(check_image_type(in->filename,"png"))
    {
        if(width > 2048 || height >2048)
        {
            printf("image w or h out of limit!\n");
            return SEAMLESS_FAIL;
        }
        printf("run to png\n");
        comp = 4;//不确定解码之后是几个通道，多申请为4通道
        buf = mem_palloc(width*height*comp);
        ret = png_decode(in->filename,buf,&width_dec,&height_dec,&comp);
        printf("buf1 w = %d h = %d ret = %d comp = %d\n",width_dec,height_dec,ret,comp);
        if(ret == 0 && width_dec == width && height_dec == height)
        {
            if(comp == 4)
            {
                src_format = SEAMLESS_THUMB_ABGR;
            }
            else
            {
                src_format = SEAMLESS_THUMB_BGR;
            }
        }
        else
        {
            printf("png free ion\n");
            mem_pfree(buf,width*height*4);
            return SEAMLESS_FAIL;
        }
    }
    else
    {
        return SEAMLESS_FAIL;
    }
/*修正参数*/
    if(in->format != SEAMLESS_THUMB_AUTO && in->format != src_format)
    {
        dst_format = in->format;
    }
    else if(in->format == SEAMLESS_THUMB_AUTO)
    {
        dst_format = SEAMLESS_THUMB_YUV420;//SEAMLESS_THUMB_BGR;//src_format;
    }
    if(in->rotate == SEAMLESS_ROTATE_0)
    {
        dst_width  = in->width;
        dst_height = in->height;
    }
    else if(in->rotate == SEAMLESS_ROTATE_90 || in->rotate == SEAMLESS_ROTATE_270)
    {
        dst_width  = ALIGN_TO_8B(in->height);//硬件要求8对齐,暂时用缩放满足8对齐，后续考虑用de裁剪8对齐引起的黑边
        dst_height = ALIGN_TO_8B(in->width);    //400 240
    }
    else if(in->rotate == SEAMLESS_ROTATE_180)
    {
        dst_width  = ALIGN_TO_8B(in->width);//硬件要求8对齐,暂时用缩放满足8对齐，后续考虑用de裁剪8对齐引起的黑边
        dst_height = ALIGN_TO_8B(in->height);
    }

/*检查宽高以及格式并转换*/
    if(width != dst_width || height != dst_height || dst_format != src_format)
    {
        __g2d_para_base_t src = {0};
        __g2d_para_base_t dst = {0};
        int temp_comp = comp;

        src.clip_rect.w = width;
        src.clip_rect.h = height;
        src.format = image_format_to_g2d(src_format);
        src.vir_addr[0] = (unsigned long)buf;
        src.phy_addr[0] = (unsigned long)mem_va2pa(buf);
        src.width = width;
        src.height = height;
        src.length = width * height * comp;

        comp = 4;//暂时以ARGB大小申请,保证所有格式都能支持
        dst.clip_rect.w = dst_width;
        dst.clip_rect.h = dst_height;
        dst.format = image_format_to_g2d(dst_format);
        if(dst_format == SEAMLESS_THUMB_YUV420)
        {
            dst.vir_addr[0] = (unsigned long)mem_palloc(dst_width * dst_height * 3/2);
            dst.phy_addr[0] = (unsigned long)mem_va2pa((void*)dst.vir_addr[0]);
            dst.phy_addr[2] = (unsigned long)((char*)(dst.phy_addr[0])+(dst_width * dst_height));
            dst.phy_addr[1] = (unsigned long)((char*)(dst.phy_addr[0])+(dst_width * dst_height*5/4));
        }
        else
        {
            dst.vir_addr[0] = (unsigned long)mem_palloc(dst_width * dst_height * 4);//暂时以ARGB大小申请,保证所有格式都能支持
            dst.phy_addr[0] = (unsigned long)mem_va2pa((void*)dst.vir_addr[0]);
        }
        dst.width = dst_width;
        dst.height = dst_height;
        dst.length = dst_width * dst_height * 3/2;
        g2d_format_conver(&src, &dst);
        mem_pfree(buf,width*height*temp_comp);//释放解码图片的内存
        buf = (void*)dst.vir_addr[0];
    }

/*检查旋转并执行*/
    if(in->rotate != SEAMLESS_ROTATE_0)//400 240 -> 240 400
    {
        __g2d_para_base_t src = {0};
        __g2d_para_base_t dst = {0};

        src.width  = dst_width;
        src.height = dst_height;
        src.clip_rect.w = dst_width;
        src.clip_rect.h = dst_height;
        src.format = image_format_to_g2d(dst_format);

        if(dst_format == SEAMLESS_THUMB_YUV420)
        {
            src.vir_addr[0] = (unsigned long)buf;
            src.phy_addr[0] = (unsigned long)mem_va2pa(buf);
            src.phy_addr[1] = (unsigned long)((char*)(src.phy_addr[0])+(dst_width * dst_height));
            src.phy_addr[2] = (unsigned long)((char*)(src.phy_addr[0])+(dst_width * dst_height*5/4));
            src.length = dst_width * dst_height * 3/2;
        }
        else
        {
            src.vir_addr[0] = (unsigned long)buf;
            src.phy_addr[0] = (unsigned long)mem_va2pa(buf);
            src.length = dst_width * dst_height * 4;
        }

        dst.format = image_format_to_g2d(dst_format);
        if(dst_format == SEAMLESS_THUMB_YUV420)
        {
            dst.vir_addr[0] = (unsigned long)mem_palloc(dst_width * dst_height * 3/2);
            dst.phy_addr[0] = (unsigned long)mem_va2pa((void*)dst.vir_addr[0]);
            dst.phy_addr[1] = (unsigned long)((char*)(dst.phy_addr[0])+(dst_width * dst_height));
            dst.phy_addr[2] = (unsigned long)((char*)(dst.phy_addr[0])+(dst_width * dst_height*5/4));
            dst.length = dst_width * dst_height * 3/2;
        }
        else
        {
            dst.vir_addr[0] = (unsigned long)mem_palloc(dst_width * dst_height * 4);
            dst.phy_addr[0] = (unsigned long)mem_va2pa((void*)dst.vir_addr[0]);
        }

        if(in->rotate == SEAMLESS_ROTATE_90 || in->rotate == SEAMLESS_ROTATE_270)
        {
            dst.width  = dst_height;
            dst.height = dst_width;
            dst.clip_rect.w = dst_height;
            dst.clip_rect.h = dst_width;
            dst_width = dst.width;
            dst_height = dst.height;
        }
        else
        {
            dst.width  = dst_width;
            dst.height = dst_height;
            dst.clip_rect.w = dst_width;
            dst.clip_rect.h = dst_height;
        }

        g2d_ext_rotate(&src,&dst,image_rotate_to_g2d(in->rotate));
        if(dst_format == SEAMLESS_THUMB_YUV420)
        {
            mem_pfree(buf,width*height*3/2);//释放缩放图片的内存
        }
        else
        {
            mem_pfree(buf,width*height*4);//释放缩放图片的内存
        }

        buf = (void*)dst.vir_addr[0];

    }

/*修正输出参数*/
    out->buf_fd      = mem_va2fd(buf);
    out->buf_addr[0] = (void *)mem_va2pa((void*)buf);//buf;
    out->buf_addr[1] = (void *)((char*)(out->buf_addr[0])+(dst_width * dst_height));//NULL;
    out->buf_addr[2] = (void *)((char*)(out->buf_addr[0])+(dst_width * dst_height*5/4));;//NULL;
    if(dst_format == SEAMLESS_THUMB_YUV420)
    {
        out->buf_size    = dst_width * dst_height * 3/2;
    }
    else
    {
        out->buf_size    = dst_width * dst_height * 4;
    }
    out->buf_width   = dst_width;
    out->buf_height  = dst_height;
    out->width       = dst_width;
    out->height      = dst_height;
    out->format      = dst_format;//in->format;
    return SEAMLESS_OK;
}

int image_unload(image_out_para_t *out)
{
    void* viraddr = NULL;

    if(out->buf_fd != 0)
    {
        viraddr = mem_fd2va(out->buf_fd);
        if(viraddr != NULL)
        {
            mem_pfree(viraddr, out->buf_size);
            return SEAMLESS_OK;
        }
        SEAMLESS_ERR("image free mem fail!\n");
        return SEAMLESS_FAIL;
    }

    if(out->buf_addr[0] != NULL)
    {
        mem_pfree(viraddr, out->buf_size);
        return SEAMLESS_OK;
    }
    SEAMLESS_ERR("image free mem fail!\n");
    return SEAMLESS_FAIL;
}
