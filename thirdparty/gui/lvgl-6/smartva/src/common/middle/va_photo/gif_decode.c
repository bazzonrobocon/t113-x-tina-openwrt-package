/*
 * =====================================================================================
 *
 *       Filename:  gif_decode.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年03月21日 09时55分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  langaojie
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "bs_widget.h"
#include "gif_decode.h"
#include "gif_dec.h"
#include "image.h"
#include "ion_mem_alloc.h"

#define GIF_PLAY_TIME_STEP 10 //播放检测间隔
#define GIF_PLAY_DELAY  (GIF_PLAY_TIME_STEP*1000)

#define GIF_ALIGN_SIZE 8
#define GIF_ALIGN(x) (((x) + ((GIF_ALIGN_SIZE) - 1)) & ~((GIF_ALIGN_SIZE) - 1))

static pthread_t       gif_id;
static pthread_cond_t  gif_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gif_wait_mutex;
static pthread_mutex_t gif_status_mutex;
static pthread_attr_t  attr;
static gif_ctrl_t      gif_ctrl;

#if 1//以下内容可以放置到另一个文件中，移植用。

extern image_display_t *get_display_opr(void);
extern image_process_t *get_process_opr(void);

image_display_t *disp_opr;
image_process_t *proc_opr;
static struct SunxiMemOpsS *ionHdle;

int priv_port_init(void)
{
    int ret;
	ionHdle = GetMemAdapterOpsS();
	ret = ionHdle->open();
	if (ret != 0)
    {
		printf("open ION error: %d\n", ret);
		return -1;
	}
    disp_opr = get_display_opr();
    proc_opr = get_process_opr();
    return 0;
}

int priv_port_exit(void)
{
    if(ionHdle != NULL)
    {
        ionHdle->close();
    }
    return 0;
}

void* priv_port_request_buf(int size)
{
    return ionHdle->palloc(size);
}

void priv_port_free_buf(void *viraddr)
{
    ionHdle->pfree(viraddr);
}

int priv_port_get_disp_para(gif_rect_t *rect, int *scn_w, int *scn_h)
{
    image_disp_info_t disp_info;
    disp_opr->disp_info_get(&disp_info);
    rect->x = 0;
    rect->y = 0;
    rect->w = disp_info.screen_width;
    rect->h = disp_info.screen_height;
    *scn_w = disp_info.screen_width;
    *scn_h = disp_info.screen_height;
    return 0;
}


static int priv_port_scaler_frame(void *src_buf,int src_width,int src_height,\
                                  void *dst_buf,int dst_width,int dst_height)
{
    image_enh_t src_enh,dst_enh;

    // printf("src_buf = %p w = %d h = %d\n",src_buf,src_width,src_height);
    // printf("dst_buf = %p w = %d h = %d\n",dst_buf,dst_width,dst_height);
    memset(&src_enh,0,sizeof(image_enh_t));
    memset(&dst_enh,0,sizeof(image_enh_t));

    src_enh.buf.vir_addr = src_buf;
	src_enh.buf.phy_addr = ionHdle->cpu_get_phyaddr(src_enh.buf.vir_addr);
	src_enh.buf.fd       = ionHdle->get_bufferFd(src_enh.buf.vir_addr);
	src_enh.buf.comp     = 4;
	src_enh.buf.width    = src_width;
	src_enh.buf.height   = src_height;
	src_enh.buf.length   = src_enh.buf.width * src_enh.buf.height * src_enh.buf.comp;
	src_enh.buf.fmt      = IMAGE_FORMAT_ABGR8888;
    src_enh.clip_rect.w  = src_width;
	src_enh.clip_rect.h  = src_height;

    dst_enh.buf.vir_addr = dst_buf;
	dst_enh.buf.phy_addr = ionHdle->cpu_get_phyaddr(dst_enh.buf.vir_addr);
	dst_enh.buf.fd       = ionHdle->get_bufferFd(dst_enh.buf.vir_addr);
	dst_enh.buf.comp     = 4;
	dst_enh.buf.width    = dst_width;
	dst_enh.buf.height   = dst_height;
	dst_enh.buf.length   = dst_enh.buf.width * dst_enh.buf.height * dst_enh.buf.comp;
	dst_enh.buf.fmt      = IMAGE_FORMAT_ARGB8888;
    dst_enh.clip_rect.w  = dst_width;
	dst_enh.clip_rect.h  = dst_height;

    proc_opr->scaler(&src_enh, &dst_enh);
    return 0;
}

static int priv_port_rotate_frame(void *src_buf,int src_width,int src_height,int rotate,void *dst_buf)
{
    return 0;
}

static int priv_port_display_frame(void *src_buf,int src_width,int src_height)
{
    image_rect_t crop_rect;
    image_buffer_t buf;
    memset(&buf,0,sizeof(image_buffer_t));

    buf.vir_addr = src_buf;
	buf.phy_addr = ionHdle->cpu_get_phyaddr(buf.vir_addr);
	buf.fd       = ionHdle->get_bufferFd(buf.vir_addr);
	buf.comp     = 4;
	buf.width    = src_width;
	buf.height   = src_height;
	buf.length   = buf.width * buf.height * buf.comp;
	buf.fmt      = IMAGE_FORMAT_ARGB8888;

    crop_rect.x = 0;
    crop_rect.y = 0;
    crop_rect.w = src_width;
    crop_rect.h = src_height;
    disp_opr->display(&buf, &crop_rect);//设置显示区域和显示数据

    return 0;
}

//////////////////////////////////////////////////////////////////

#endif


static void gif_play_phread_down(void)
{
	pthread_mutex_lock(&gif_wait_mutex);
	pthread_cond_wait(&gif_wait_cond, &gif_wait_mutex);
	pthread_mutex_unlock(&gif_wait_mutex);
}

static void gif_play_thread_up(void)
{
	pthread_mutex_lock(&gif_wait_mutex);
	pthread_cond_signal(&gif_wait_cond);
	pthread_mutex_unlock(&gif_wait_mutex);
}

static void* gif_play_pthread(void *arg)
{
	int delay_cnt;
    int delay;
    int index = 0;
    gif_info_t *gif_info = NULL;

	while(1)
	{
		if(gif_ctrl.status == GIF_STATUS_PLAY)
		{
            gif_dec_get_delay(gif_info,index,&delay);
            gif_dec_frame(gif_info,index);
            priv_port_scaler_frame(gif_info->buffer,gif_info->width,gif_info->height,\
                                   gif_ctrl.next_buf.addr,gif_ctrl.next_buf.width,gif_ctrl.next_buf.height);
            // priv_port_rotate_frame(gif_ctrl.next_buf.addr,gif_ctrl.next_buf.width,gif_ctrl.next_buf.height,);
            priv_port_display_frame(gif_ctrl.next_buf.addr,gif_ctrl.next_buf.width,gif_ctrl.next_buf.height);
			delay_cnt = delay / GIF_PLAY_TIME_STEP;

			while (delay_cnt)
			{
				usleep(GIF_PLAY_DELAY);

				if (gif_ctrl.status != GIF_STATUS_PLAY)
				{
					break ;
				}
				else
				{
					delay_cnt-- ;
				}
			}
            gif_ctrl.is_first_frame_finish = 1;
            {
                gif_buf_t temp;
                memcpy(&temp, &gif_ctrl.next_buf,sizeof(gif_buf_t));
                memcpy(&gif_ctrl.next_buf, &gif_ctrl.prev_buf, sizeof(gif_buf_t));
                memcpy(&gif_ctrl.prev_buf, &temp, sizeof(gif_buf_t));
            }

            index++;
            if(index > (gif_info->frame_count-1))
            {
                index = 0;

                if(gif_ctrl.para.mode == GIF_MODE_ONCE)
                {
                    pthread_mutex_lock(&gif_status_mutex);
                    if(gif_ctrl.gif_info->buffer != NULL)
                    {
                        gif_ctrl.cb.free_buf(gif_ctrl.gif_info->buffer);
                    }
                    if(gif_ctrl.next_buf.addr != NULL)
                    {
                        gif_ctrl.cb.free_buf(gif_ctrl.next_buf.addr);
                    }
                    if(gif_ctrl.prev_buf.addr != NULL)
                    {
                        gif_ctrl.cb.free_buf(gif_ctrl.prev_buf.addr);
                    }

                    gif_dec_finish(gif_ctrl.gif_info);
                    gif_ctrl.gif_info = NULL;

                    memset(&gif_ctrl.next_buf,0,sizeof(gif_buf_t));
                    memset(&gif_ctrl.prev_buf,0,sizeof(gif_buf_t));
                    gif_ctrl.is_finish = 1;
                    gif_ctrl.status = GIF_STATUS_STOP;
                    gif_ctrl.is_busy = 0;
                    gif_ctrl.is_first_frame_finish = 0;
                    printf("gif down2!!!!\n");
                    pthread_mutex_unlock(&gif_status_mutex);
                    gif_play_phread_down();
                    gif_ctrl.is_busy = 1;
                    gif_ctrl.is_finish = 0;
                    gif_info = gif_ctrl.gif_info;
                    index = 0;
                    printf("gif up2!!!!\n");

                }
                else
                {
                    gif_ctrl.is_finish = 1;
                }
            }
		}

		if(gif_ctrl.status == GIF_STATUS_STOP)
		{
			printf("gif down!!!\n");
			gif_ctrl.is_busy = 0;
            gif_ctrl.is_first_frame_finish = 0;
			gif_play_phread_down();
            gif_ctrl.is_busy = 1;
            gif_ctrl.is_finish = 0;
            gif_info = gif_ctrl.gif_info;
            index = 0;
			printf("gif up!!!!\n");
		}
		if(gif_ctrl.status == GIF_STATUS_STOP)
		{
			pthread_exit(0);
		}
	}
}

int gif_play_set_callback(gif_cb_vector_t *cb)
{
    gif_ctrl.cb = *cb;
    return 0;
}

int gif_play_init(void)
{
    int ret;
    gif_cb_vector_t cb =
    {
        priv_port_display_frame,
        priv_port_rotate_frame,
        priv_port_scaler_frame,
        priv_port_get_disp_para,
        priv_port_request_buf,
        priv_port_free_buf,
    };

    memset(&gif_ctrl, 0, sizeof(gif_ctrl_t));

    pthread_mutex_init(&gif_wait_mutex, NULL);
    pthread_mutex_init(&gif_status_mutex, NULL);
    pthread_cond_init(&gif_wait_cond, NULL);

    pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x4000);
    ret = pthread_create(&gif_id, &attr, gif_play_pthread, NULL);
	if(ret < 0)
	{
		com_err("create thread auto play err!\n");
	}
    priv_port_init();          //此函数应在外部调用，移植用
    gif_play_set_callback(&cb);//此函数应在外部调用，移植用
    gif_ctrl.cb.get_disp_para(&gif_ctrl.para.disp_area, &gif_ctrl.para.screen_width, &gif_ctrl.para.screen_height);
    return 0;
}

int gif_play_exit(void)
{
    printf("gif_play_exit\n");
    gif_play_stop();
    gif_play_thread_up();
    pthread_join(gif_id, NULL);
	pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&gif_wait_mutex);
    pthread_mutex_destroy(&gif_status_mutex);
    pthread_cond_destroy(&gif_wait_cond);
    priv_port_exit();          //此函数应在外部调用，移植用
    return 0;
}

int gif_play_start(char *filename, int mode)
{
    gif_info_t *gif_info = NULL;

    if(gif_ctrl.status == GIF_STATUS_PLAY)
    {
        gif_play_stop();
    }
    pthread_mutex_lock(&gif_status_mutex);
    gif_info = gif_dec_from_file(filename);
    if(gif_info == NULL)
    {
        return -1;
    }

    gif_ctrl.gif_info = gif_info;
    gif_ctrl.status = GIF_STATUS_PLAY;
    gif_ctrl.para.mode = mode;
    gif_ctrl.next_buf.addr   = gif_ctrl.cb.requst_buf(gif_ctrl.para.disp_area.w * gif_ctrl.para.disp_area.h * 4);
    gif_ctrl.next_buf.width  = gif_ctrl.para.disp_area.w;
    gif_ctrl.next_buf.height = gif_ctrl.para.disp_area.h;
    gif_ctrl.prev_buf.addr   = gif_ctrl.cb.requst_buf(gif_ctrl.para.disp_area.w * gif_ctrl.para.disp_area.h * 4);
    gif_ctrl.prev_buf.width  = gif_ctrl.para.disp_area.w;
    gif_ctrl.prev_buf.height = gif_ctrl.para.disp_area.h;

    gif_info->buffer = gif_ctrl.cb.requst_buf(gif_info->width * gif_info->height * 4);  //获取解码buffer
    gif_dec_set_outbuf(gif_info, gif_info->buffer);//设置解码buffer到解码器

    gif_play_thread_up();
    pthread_mutex_unlock(&gif_status_mutex);
    return 0;
}

//停止播放，释放当前gif的资源
int gif_play_stop(void)
{
    pthread_mutex_lock(&gif_status_mutex);
    if(gif_ctrl.status == GIF_STATUS_PLAY)
    {
        gif_ctrl.status = GIF_STATUS_STOP;
        while(1)
        {
            if(gif_ctrl.is_busy == 0)
            {
                break;
            }
            usleep(1000);
        }
        if(gif_ctrl.gif_info->buffer != NULL)
        {
            gif_ctrl.cb.free_buf(gif_ctrl.gif_info->buffer);
        }
        if(gif_ctrl.next_buf.addr != NULL)
        {
            gif_ctrl.cb.free_buf(gif_ctrl.next_buf.addr);
        }
        if(gif_ctrl.prev_buf.addr != NULL)
        {
            gif_ctrl.cb.free_buf(gif_ctrl.prev_buf.addr);
        }

        gif_dec_finish(gif_ctrl.gif_info);
        gif_ctrl.gif_info = NULL;

        memset(&gif_ctrl.next_buf, 0, sizeof(gif_buf_t));
        memset(&gif_ctrl.prev_buf, 0, sizeof(gif_buf_t));
    }
    pthread_mutex_unlock(&gif_status_mutex);
    return 0;
}

int gif_play_is_finish(void)
{
    return gif_ctrl.is_finish;
}

int gif_wait_first_frame(void)
{
    int cnt = 1000;
    while(1)
    {
        if(gif_ctrl.is_first_frame_finish == 1 || gif_ctrl.is_finish == 1)
        {
            break;
        }
        usleep(1000);
        cnt --;
        if(cnt == 0)
        {
            printf("gif_wait_first_frame timeout!\n");
            break;
        }
    }
    return 0;
}

int gif_decode_sw(char *filename, void *output_buf, unsigned int *width, unsigned int *height, int *comp)
{
    gif_info_t *gif_info = NULL;
    int ret;

    gif_info = gif_dec_from_file(filename);
    if(gif_info == NULL)
    {
        return -1;
    }
    gif_dec_set_outbuf(gif_info, output_buf);
    gif_dec_get_size(gif_info, width, height);
    ret = gif_dec_frame(gif_info, 0);
    gif_dec_finish(gif_info);
    *comp = 4;
    return ret;
}
