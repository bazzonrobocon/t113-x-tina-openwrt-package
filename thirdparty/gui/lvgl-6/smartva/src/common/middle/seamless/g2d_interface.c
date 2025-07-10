#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
// #include <linux/g2d_driver.h>
#include "g2d_interface.h"
#include "mem_interface.h"
#include "seamless.h"
static int g2d_fd;

int g2d_init(void)
{
	g2d_fd = open("/dev/g2d", O_RDWR);
	if(g2d_fd == -1)
    {
		SEAMLESS_ERR("open g2d device fail!\n");
	}
	return SEAMLESS_OK;
}

int g2d_exit(void)
{
    if(g2d_fd > 0)
    {
        close(g2d_fd);
    }
    g2d_fd = -1;
    return SEAMLESS_OK;
}

/*
*YUV基本颜色
*黑 0x008080
*白 0xFF8080
*红 0x4C55FF
*黄 0xE20095
*蓝 0x1DFF6B
*青 0xB3AB00
*/
/*颜色填充*/
int g2d_ext_fill(__g2d_para_base_t *src,unsigned int color)
{
    g2d_fillrect_h info;
    int ret;

    memset(&info, 0, sizeof(g2d_fillrect_h));

    info.dst_image_h.bbuff  = 1;
    info.dst_image_h.format = src->format;
    info.dst_image_h.color  = color;
    info.dst_image_h.width  = src->width;
    info.dst_image_h.height = src->height;
    info.dst_image_h.clip_rect.x = src->clip_rect.x;
    info.dst_image_h.clip_rect.y = src->clip_rect.y;
    info.dst_image_h.clip_rect.w = src->clip_rect.w;
    info.dst_image_h.clip_rect.h = src->clip_rect.h;
    info.dst_image_h.alpha = src->alpha;
    info.dst_image_h.mode  = G2D_MIXER_ALPHA;//src->alpha_mode;
	info.dst_image_h.gamut = G2D_BT601;

    info.dst_image_h.use_phy_addr = 1;
    info.dst_image_h.laddr[0] = src->phy_addr[0];
    info.dst_image_h.laddr[1] = src->phy_addr[1];
    info.dst_image_h.laddr[2] = src->phy_addr[2];

	mem_flush_cache((void*)src->vir_addr[0], src->length);
	ret = ioctl(g2d_fd , G2D_CMD_FILLRECT_H ,(unsigned long)(&info));
	if(ret != 0)
    {
		printf("[%d][%s][%s]G2D_CMD_FILLRECT_H failure!\n",__LINE__, __FILE__,__FUNCTION__);
		goto err;
	}
	mem_flush_cache((void*)src->vir_addr[0], src->length);
err:
	return ret;

}
/*
blt_cmd：
G2D_BLT_NONE_H         源操作：缩放、格式转换
G2D_BLT_BLACKNESS,     BLACKNESS :使用与物理调色板的索引0相关的色彩来填充目标矩形区域,(对缺省的物理调色板,该颜色为黑色)
G2D_BLT_NOTMERGEPEN,   dst = ~(dst+src) :
G2D_BLT_MASKNOTPEN,    dst =~src&dst
G2D_BLT_NOTCOPYPEN,    dst =~src
G2D_BLT_MASKPENNOT,    dst =src&~dst
G2D_BLT_NOT,           dst =~dst :使目标矩形区域颜色取反
G2D_BLT_XORPEN,        dst =src^dst
G2D_BLT_NOTMASKPEN,    dst =~(src&dst)
G2D_BLT_MASKPEN,       dst =src&dst
G2D_BLT_NOTXORPEN,     dst =~(src^dst)
G2D_BLT_NOP,           dst =dst
G2D_BLT_MERGENOTPEN,   dst =~src+dst
G2D_BLT_COPYPEN,       dst =src
G2D_BLT_MERGEPENNOT,   dst =src+~dst
G2D_BLT_MERGEPEN,      dst =src+dst
G2D_BLT_WHITENESS      WHITENESS :使用与物理调色板中索引1有关的颜色填充目标矩形区域(对于缺省物理调色板来说,这个颜色为白色)
G2D_ROT_90 旋转
G2D_ROT_180
G2D_ROT_270
G2D_ROT_0
G2D_ROT_H
G2D_ROT_V

note:
旋转要求输出buffer宽8对齐，输入没有要求。
旋转和缩放不可同时操作，需要分两步实现。
旋转yuv数据时，将会全部转换成yuv420p输出

*/
int g2d_base_blt(__g2d_para_base_t *src, __g2d_para_base_t *dst, g2d_blt_flags blt_cmd)
{
    g2d_blt_h blt = {0};
    int ret;
    memset(&blt,0,sizeof(g2d_blt_h));

    blt.flag_h = blt_cmd;
    printf("blt_cmd = %d\n",blt_cmd);

    blt.src_image_h.bbuff  = 1;
    blt.src_image_h.format = src->format;
    blt.src_image_h.width  = src->width;
    blt.src_image_h.height = src->height;
    blt.src_image_h.clip_rect.x = src->clip_rect.x;
    blt.src_image_h.clip_rect.y = src->clip_rect.y;
    blt.src_image_h.clip_rect.w = src->clip_rect.w;
    blt.src_image_h.clip_rect.h = src->clip_rect.h;
    blt.src_image_h.alpha = 0xff;//src->alpha;
    blt.src_image_h.mode  = G2D_GLOBAL_ALPHA;//  src->alpha_mode;
    blt.src_image_h.use_phy_addr = 1;
    blt.src_image_h.laddr[0] = src->phy_addr[0];
    blt.src_image_h.laddr[1] = src->phy_addr[1];
    blt.src_image_h.laddr[2] = src->phy_addr[2];
	printf("src w = %d h = %d x = %x y = %d w = %d h = %d\n",src->width,src->height,src->clip_rect.x,src->clip_rect.y,src->clip_rect.w,src->clip_rect.h);
    printf("src->phy_addr[0] = 0x%lx\n",src->phy_addr[0]);

    blt.dst_image_h.bbuff  = 1;
    blt.dst_image_h.format = dst->format;
    blt.dst_image_h.width  = dst->width;
    blt.dst_image_h.height = dst->height;
    blt.dst_image_h.clip_rect.x = dst->clip_rect.x;
    blt.dst_image_h.clip_rect.y = dst->clip_rect.y;
    blt.dst_image_h.clip_rect.w = dst->clip_rect.w;
    blt.dst_image_h.clip_rect.h = dst->clip_rect.h;
    blt.dst_image_h.alpha = 0xff;//dst->alpha;
    blt.dst_image_h.mode  = G2D_GLOBAL_ALPHA;//dst->alpha_mode;
    blt.dst_image_h.use_phy_addr = 1;
    blt.dst_image_h.laddr[0] = dst->phy_addr[0];
    blt.dst_image_h.laddr[1] = dst->phy_addr[1];
    blt.dst_image_h.laddr[2] = dst->phy_addr[2];
	printf("dst w = %d h = %d x = %x y = %d w = %d h = %d\n",dst->width,dst->height,dst->clip_rect.x,dst->clip_rect.y,dst->clip_rect.w,dst->clip_rect.h);
	printf("src->phy_addr[0] = 0x%lx\n",dst->phy_addr[0]);

    if((blt.flag_h >= G2D_ROT_90) && (blt.flag_h <= G2D_ROT_V))
    {
        blt.dst_image_h.align[0] = 8;
	    blt.dst_image_h.align[1] = 8;
	    blt.dst_image_h.align[2] = 8;
        if(blt.dst_image_h.width%8)
        {
            printf("warning:g2d rotate dst width should align to 8!\n");
        }
    }

    // printf("src format = %d %d\n",src->format,dst->format);
    mem_flush_cache((void*)src->vir_addr[0], src->length);
	ret = ioctl(g2d_fd, G2D_CMD_BITBLT_H, (unsigned long)(&blt));
	if(ret != 0)
    {
		printf("[%d][%s][%s]G2D_CMD_BITBLT_H failure!\n",__LINE__, __FILE__,__FUNCTION__);
		goto err;
	}
	mem_flush_cache((void*)dst->vir_addr[0], src->length);
err:
	return ret;
}

int g2d_base_bld(__g2d_para_base_t *src, __g2d_para_base_t *dst, int src_num, g2d_bld_cmd_flag g2d_bld_cmd)
{
	g2d_bld info;
	int i, ret;
	memset(&info, 0, sizeof(info));
	info.bld_cmd = g2d_bld_cmd;

	for (i = 0; i < src_num; i++)
    {
		info.src_image[i].laddr[0] = (unsigned long)src[i].phy_addr[0];
		info.src_image[i].use_phy_addr = 1;
		info.src_image[i].format = src[i].format;
		info.src_image[i].width = src[i].width;
		info.src_image[i].height = src[i].height;
		info.src_image[i].clip_rect.x = src[i].clip_rect.x;
		info.src_image[i].clip_rect.y = src[i].clip_rect.y;
		info.src_image[i].clip_rect.w = src[i].clip_rect.w;
		info.src_image[i].clip_rect.h = src[i].clip_rect.h;
		info.src_image[i].mode = src[i].alpha_mode;
		info.src_image[i].alpha = src[i].alpha;
	}

	info.dst_image.format = dst->format;
	info.dst_image.width = dst->width;
	info.dst_image.height = dst->height;
	info.dst_image.clip_rect.x = dst->clip_rect.x;
	info.dst_image.clip_rect.y = dst->clip_rect.y;
	info.dst_image.clip_rect.w = dst->clip_rect.w;
	info.dst_image.clip_rect.h = dst->clip_rect.h;
	info.dst_image.mode  = dst->alpha_mode;
	info.dst_image.alpha = dst->alpha;
	info.dst_image.laddr[0] = (unsigned long)dst->phy_addr[0];
	info.dst_image.use_phy_addr = 1;

	info.src_image[0].align[0] = 0;
	info.src_image[0].align[1] = 0;
	info.src_image[0].align[2] = 0;
	info.src_image[1].align[0] = 0;
	info.src_image[1].align[1] = 0;
	info.src_image[1].align[2] = 0;
	info.dst_image.align[0] = 0;
	info.dst_image.align[1] = 0;
	info.dst_image.align[2] = 0;

	mem_flush_cache((void*)src->vir_addr[0], src->length);

	ret = ioctl(g2d_fd, G2D_CMD_BLD_H, (unsigned long)(&info));

	mem_flush_cache((void*)dst->vir_addr[0], dst->length);

	return ret;
}

int g2d_ext_bld_alpha(__g2d_para_base_t *src, __g2d_para_base_t *dst, int src_num)
{
    g2d_base_bld(src,dst,src_num,G2D_BLD_SRCOVER);
	return 0;
}

int g2d_ext_rotate(__g2d_para_base_t *src,__g2d_para_base_t *dst,int angle)
{
    // if(angle == G2D_ROT_90 || angle == G2D_ROT_270){
    //     dst->width  = src->height;
    //     dst->height = src->width;
    //     dst->clip_rect.w = dst->width;
    //     dst->clip_rect.h = dst->height;

    // }
    // if(src->format >= G2D_FORMAT_IYUV422_V0Y1U0Y0){
    //     dst->format = G2D_FORMAT_YUV420_PLANAR;
    // }
    g2d_base_blt(src,dst,angle);
	return 0;
}

int g2d_ext_blt_conver(__g2d_para_base_t *src,__g2d_para_base_t *dst)
{
    // if(src->format == dst->format){
    //     return 0;
    // }
    g2d_base_blt(src,dst,G2D_BLT_NONE_H);
	return 0;
}

 int g2d_format_conver(__g2d_para_base_t *src, __g2d_para_base_t *dst)
{
	int ret = 0;
	unsigned long arg[3];

	struct mixer_para para;
	memset(&para, 0, sizeof(struct mixer_para));
	para.flag_h  = G2D_BLT_NONE_H;
	para.op_flag = OP_BITBLT;

	para.src_image_h.use_phy_addr = 1;
	para.src_image_h.laddr[0] = (unsigned long)src->phy_addr[0];
    para.src_image_h.laddr[1] = (unsigned long)src->phy_addr[1];
    para.src_image_h.laddr[2] = (unsigned long)src->phy_addr[2];
	para.src_image_h.format = src->format;
	para.src_image_h.width = src->width;
	para.src_image_h.height = src->height;
	para.src_image_h.clip_rect.x = src->clip_rect.x;
	para.src_image_h.clip_rect.y = src->clip_rect.y;
	para.src_image_h.clip_rect.w = src->clip_rect.w;
	para.src_image_h.clip_rect.h = src->clip_rect.h;
	para.src_image_h.alpha = src->alpha;
	para.src_image_h.mode = src->alpha_mode;


	para.dst_image_h.use_phy_addr = 1;
	para.dst_image_h.laddr[0] = (unsigned long)dst->phy_addr[0];
    para.dst_image_h.laddr[1] = (unsigned long)dst->phy_addr[1];
    para.dst_image_h.laddr[2] = (unsigned long)dst->phy_addr[2];
	para.dst_image_h.format = dst->format;
	para.dst_image_h.width = dst->width;
	para.dst_image_h.height = dst->height;
	para.dst_image_h.clip_rect.x = dst->clip_rect.x;
	para.dst_image_h.clip_rect.y = dst->clip_rect.y;
	para.dst_image_h.clip_rect.w = dst->clip_rect.w;
	para.dst_image_h.clip_rect.h = dst->clip_rect.h;
	para.dst_image_h.alpha = src->alpha;
	para.dst_image_h.mode = src->alpha_mode;


	arg[0] = (unsigned long)(&para);
	arg[1] = 1;
	mem_flush_cache((void*)src->vir_addr[0], src->length);
	ret = ioctl(g2d_fd, G2D_CMD_MIXER_TASK,(void *)arg);
	if (ret)
	{
		printf("[%d][%s][%s]G2D_CMD_MIXER_TASK failure!\n", __LINE__, __FILE__, __FUNCTION__);
		ret = -1;
		goto exit;
	}
	mem_flush_cache((void*)dst->vir_addr[0], dst->length);
exit:
	return ret;
}
