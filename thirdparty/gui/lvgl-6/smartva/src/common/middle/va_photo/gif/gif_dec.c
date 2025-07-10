/*
 * =====================================================================================
 *
 *       Filename:  gif.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年03月21日 10时18分06秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  langaojie
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "gif_dec.h"

#define BYTES_PER_PIXEL 4
#define MAX_IMAGE_BYTES (48 * 1024 * 1024)

//内存由外部控制
static void *bitmap_create(int width, int height)
{
    if (((long long)width * (long long)height) > (MAX_IMAGE_BYTES/BYTES_PER_PIXEL))
    {
	    return NULL;
    }
    return (void*)0x1;//返回一个非空的值,
}

static void bitmap_set_opaque(void *bitmap, bool opaque)//del
{
	(void) opaque;  /* unused */
	(void) bitmap;  /* unused */
	assert(bitmap);
}

static bool bitmap_test_opaque(void *bitmap)//del
{
	(void) bitmap;  /* unused */
	assert(bitmap);
	return false;
}

static unsigned char *bitmap_get_buffer(void *bitmap)
{
	assert(bitmap);
	return bitmap;
}

//内存交由外部控制
static void bitmap_destroy(void *bitmap)
{
	assert(bitmap);
	//free(bitmap);
}

static void bitmap_modified(void *bitmap)//del
{
	(void) bitmap;  /* unused */
	assert(bitmap);
	return;
}

static unsigned char *gif_load_file(const char *path, size_t *data_size)
{
    FILE *fp = NULL;
    int ret = 0;
    unsigned char *data = NULL;

    fp = fopen(path, "rb");
    if(fp == NULL)
    {
        printf("read bmp file error, errno(%d)\n", errno);
        return NULL;
    }

    fseek(fp,0,SEEK_END);
    *data_size = ftell(fp);
    rewind(fp);
    data = (unsigned char *)malloc(sizeof(char)*(*data_size));

    if(data == NULL)
    {
        printf("malloc memory fail\n");
        fclose(fp);
        return NULL;
    }

    ret = fread(data,1,*data_size,fp);
    if (ret != *data_size)
    {
        printf("read src file fail\n");
        fclose(fp);
        free(data);
        return NULL;
    }

    if(fp != NULL)
    {
	    fclose(fp);
    }
    return data;
}

/*
*加载并初始化指定gif文件
*/
gif_info_t* gif_dec_from_file(char *filename)
{
    gif_bitmap_callback_vt bitmap_callbacks =
    {
		bitmap_create,
		bitmap_destroy,
		bitmap_get_buffer,
		bitmap_set_opaque,
		bitmap_test_opaque,
		bitmap_modified
	};
    gif_result code;
    size_t size;
	unsigned char *data = NULL;
    gif_info_t *gif_info = NULL;

    gif_info = malloc(sizeof(gif_info_t));
    if(gif_info == NULL)
    {
        goto err;
    }
    memset(gif_info,0,sizeof(gif_info_t));

    gif_create(&gif_info->gif, &bitmap_callbacks);
    data = gif_load_file(filename, &size);
    if(data == NULL)
    {
        goto err;
    }
    do
    {
		code = gif_initialise(&gif_info->gif, size, data);
		if (code != GIF_OK && code != GIF_WORKING)
		{
            goto err;
		}
	} while (code != GIF_OK);

    gif_info->width  = gif_info->gif.width;
    gif_info->height = gif_info->gif.height;
    gif_info->frame_count = gif_info->gif.frame_count;
    return gif_info;

err:
    gif_finalise(&gif_info->gif);
    if(gif_info != NULL)
    {
        free(gif_info);
    }
    if(data != NULL)
    {
        free(data);
    }
    return NULL;
}

/*
*解码指定帧
*/
int gif_dec_frame(gif_info_t *gif_info,int index)
{
    gif_result code;

    code = gif_decode_frame(&gif_info->gif,index);
    if(code != GIF_OK)
    {
        return -1;
    }
    return 0;
}

/*
*解码结束，销毁gif
*/
int gif_dec_finish(gif_info_t *gif_info)
{
    if(gif_info == NULL)
    {
        return 0;
    }
    gif_finalise(&gif_info->gif);
    free(gif_info->gif.gif_data);
    free(gif_info);
    return 0;
}

/*
*获取gif宽高信息
*/
int gif_dec_get_size(gif_info_t *gif_info, unsigned int *width, unsigned int *height)
{
    *width = gif_info->gif.width;
    *height = gif_info->gif.height;
    return 0;
}

/*
*获取指定帧的延迟时间
*/
int gif_dec_get_delay(gif_info_t *gif_info, int index, int *delay)
{
    *delay = gif_info->gif.frames[index].frame_delay*10;
    return 0;
}

/*
*获取gif帧总数
*/
int gif_dec_get_frame_num(gif_info_t *gif_info, int *num)
{
    *num = gif_info->gif.frame_count;
    return 0;
}

/*
*获取解码buffer
*/
int gif_dec_get_outbuf(gif_info_t *gif_info,void **buf)
{
    *buf = gif_info->gif.frame_image;
    return 0;
}

/*
*设置解码buffer
*/
int gif_dec_set_outbuf(gif_info_t *gif_info,void *buf)
{
    gif_info->gif.frame_image = buf;
    return 0;
}
