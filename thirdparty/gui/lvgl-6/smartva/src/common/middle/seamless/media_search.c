/*
 * =====================================================================================
 *
 *       Filename:  media_search.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年02月25日 15时58分38秒
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
#include "rat.h"
#include "rat_npl.h"
#include "image_player.h"

//检查媒体类型
static int media_check_type(char*filename)
{
	int type;
	switch(rat_get_file_type(filename))
	{
		case RAT_MEDIA_TYPE_PIC:
			type = SEAMLESS_MEDIA_IMAGE;
			break;
		case RAT_MEDIA_TYPE_AUDIO:
			type = SEAMLESS_MEDIA_AUDIO;
			break;
		case RAT_MEDIA_TYPE_VIDEO:
			type = SEAMLESS_MEDIA_VIDEO;
			break;
		default:
			type = SEAMLESS_MEDIA_UNKNOW;
	}
	return type;
}

//搜索某种媒体
static int media_search(seamless_rat_ctrl_t *h_rat)
{
    h_rat->handle = rat_open(h_rat->path, h_rat->type, 0);
    if(h_rat->handle == 0)
    {
        return SEAMLESS_FAIL;
    }
	// strcmp(NULL,"langaojie");
    rat_set_file_for_play(h_rat->handle,"");
    rat_move_cursor_to_first(h_rat->handle);
    h_rat->total = rat_get_cur_scan_cnt(h_rat->handle);
    if(h_rat->total == 0)
    {
        if(h_rat->handle)
        {
            rat_close(h_rat->handle);
        }
		SEAMLESS_ERR("search err\n");
        return SEAMLESS_FAIL;
    }
    h_rat->npl = rat_npl_open(h_rat->type);
    rat_npl_set_play_mode(h_rat->npl,RAT_PLAY_MODE_ROTATE_ALL);
    rat_npl_set_cur(h_rat->npl,h_rat->total);
	return SEAMLESS_OK;
}

//尝试探测指定媒体
int media_probe(seamless_ctrl_t *h_seamless)
{
    int show_mode;
	int ret = SEAMLESS_OK;
    show_mode = h_seamless->init_para.show_mode;

	if(show_mode == SEAMLESS_MODE_IMAGE_ONLY)
	{
        /*搜索图片文件，并加入搜索链表*/
        h_seamless->rat_image.path = h_seamless->init_para.source_path;
        h_seamless->rat_image.type = RAT_MEDIA_TYPE_PIC;
        ret = media_search(&h_seamless->rat_image);
	}
	else if(show_mode == SEAMLESS_MODE_VIDEO_ONLY)
	{
	    /*搜索视频文件，并加入搜索链表*/
        h_seamless->rat_video.path = h_seamless->init_para.source_path;
        h_seamless->rat_video.type = RAT_MEDIA_TYPE_VIDEO;
        ret = media_search(&h_seamless->rat_video);
	}
	else if(show_mode == SEAMLESS_MODE_MIXED)
	{
	    /*搜索视频和图片文件，并加入搜索链表*/
        h_seamless->rat_mixed.path = h_seamless->init_para.source_path;
        h_seamless->rat_mixed.type = RAT_MEDIA_TYPE_VIDEO_AND_PIC;
        ret = media_search(&h_seamless->rat_mixed);
	}
	else if(show_mode == SEAMLESS_MODE_SPLIT || show_mode == SEAMLESS_MODE_SPLIT_SYNC)
	{
        int ret_image;
        int ret_video;
		/*搜索图片文件，并加入搜索链表*/
        h_seamless->rat_image.path = h_seamless->init_para.source_path;
        h_seamless->rat_image.type = RAT_MEDIA_TYPE_PIC;
        ret_image = media_search(&h_seamless->rat_image);

        /*搜索视频文件，并加入搜索链表*/
        h_seamless->rat_video.path = h_seamless->init_para.source_path;
        h_seamless->rat_video.type = RAT_MEDIA_TYPE_VIDEO;
        ret_video = media_search(&h_seamless->rat_video);

		/*根据搜索结果，改变播放器模式*/
		if(ret_video == SEAMLESS_FAIL && ret_image == SEAMLESS_FAIL)
	    {
	        ret = SEAMLESS_FAIL;
	    }
		else if(ret_video == SEAMLESS_OK && ret_image == SEAMLESS_FAIL)
		{
			h_seamless->init_para.show_mode = SEAMLESS_MODE_VIDEO_ONLY;
		}
		else if(ret_video == SEAMLESS_FAIL && ret_image == SEAMLESS_OK)
		{
			h_seamless->init_para.show_mode = SEAMLESS_MODE_IMAGE_ONLY;
		}
	}

    SEAMLESS_LOG("show_mode = %d",h_seamless->init_para.show_mode);
    SEAMLESS_LOG("video num = %d",h_seamless->rat_video.total);
    SEAMLESS_LOG("photo num = %d",h_seamless->rat_image.total);
    SEAMLESS_LOG("mixed num = %d",h_seamless->rat_mixed.total);
    return ret;
}

//加载媒体
int media_preload(seamless_ctrl_t *h_seamless)
{
	char* filename;
	HRATNPL rat_npl;
	int try_cnt = 0;
    int index,index_backup;
    int show_mode;
	int ret;
    seamless_media_type_e media_type,media_type_temp;

    show_mode = h_seamless->init_para.show_mode;
    filename = h_seamless->loading.filename;

	switch(show_mode)//获取播放列表
	{
		case SEAMLESS_MODE_IMAGE_ONLY:
			rat_npl = h_seamless->rat_image.npl;
			break;
		case SEAMLESS_MODE_VIDEO_ONLY:
			rat_npl = h_seamless->rat_video.npl;
			break;
		case SEAMLESS_MODE_MIXED:
			rat_npl = h_seamless->rat_mixed.npl;
			break;
		case SEAMLESS_MODE_SPLIT:
		case SEAMLESS_MODE_SPLIT_SYNC:
			rat_npl = h_seamless->rat_image.npl;//在分屏模式下，视频部分资源准备由视频线程完成。
			break;
		default:
			goto out;
	}
try_again:
	index = rat_npl_get_next(rat_npl);
	if(index < 0)
	{
		goto out;
	}
	rat_npl_index2file(rat_npl,index,filename);
	media_type = media_check_type(filename);
	rat_npl_set_cur(rat_npl,index);
	SEAMLESS_LOG("media_type = %d name = %s\n",media_type,filename);

	if(media_type == SEAMLESS_MEDIA_IMAGE)
	{
		image_in_para_t in;
		image_out_para_t out;
		memset(&in,0,sizeof(image_in_para_t));
		memset(&out,0,sizeof(image_out_para_t));

		in.filename = filename;
		in.format = SEAMLESS_THUMB_AUTO;
		in.width  = h_seamless->init_para.image_rect.width;
		in.height = h_seamless->init_para.image_rect.height;
		in.rotate = h_seamless->init_para.screen_rotate;
		ret = image_load(&in,&out);
		if(ret == SEAMLESS_FAIL)
		{
			if(try_cnt++ < rat_npl_get_total_count(rat_npl))
			{
				printf("try again!\n");
				goto try_again;
			}
			else
			{
				return SEAMLESS_FAIL;
			}
		}

		h_seamless->loading.image.buf_width     = out.buf_width;
		h_seamless->loading.image.buf_height    = out.buf_height;
		h_seamless->loading.image.width         = out.width;
		h_seamless->loading.image.height        = out.height;
        h_seamless->loading.image.buf_addr[0]   = out.buf_addr[0];
        h_seamless->loading.image.buf_addr[1]   = (NULL);
        h_seamless->loading.image.buf_addr[2]   = (NULL);
		h_seamless->loading.image.buf_fd        = out.buf_fd;
		h_seamless->loading.image.format        = out.format;

#if 1//加载时间适应。视频加载需要一些时间。
		index_backup = index;//
		index = rat_npl_get_next(rat_npl);
		if(index < 0)
		{
			goto out;
		}
		rat_npl_index2file(rat_npl,index,filename);
		media_type_temp = media_check_type(filename);
		rat_npl_set_cur(rat_npl,index_backup);
		rat_npl_index2file(rat_npl,index_backup,filename);

		if(media_type_temp == SEAMLESS_MEDIA_VIDEO)
		{
			h_seamless->loading.prepare_time = 70;
		}
		else
		{
			h_seamless->loading.prepare_time = 0;
		}
#endif
	}
	else if(media_type == SEAMLESS_MEDIA_VIDEO)
	{
		/*视频预加载动作目前只是解析文件名，根据需要在此处添加更多操作。*/
#if 0
		__log("filename = %s",filename);
		__log("test = %s %s","\\","\\WPSettings.dat");

		if(eLIBs_strcmp(eLIBs_strchrlast(filename,'\\'),"\\WPSettings.dat"))
		{
			__log("ignore WPSettings.dat");
			if(rat_npl_get_total_count(rat_npl)>1)
			{
				__log("ignore WPSettings.dat");
				goto try_again;
			}
			else
			{
				__log("something err!");
				media_set_status(SEAMLESS_STATUS_STOP);
			}
		}
#endif
	}
	else if(media_type == SEAMLESS_MEDIA_UNKNOW)
	{
		SEAMLESS_ERR("preload fail\n");
		// media_set_status(SEAMLESS_STATUS_STOP);
	}


	h_seamless->loading.media_type = media_type;
	//g_seamless_ctrl->loading.used		= 0;
	printf("preload ok\n");
out:
	return EPDK_OK;
}

//释放媒体
int media_unload(seamless_ctrl_t *h_seamless)
{
	if(h_seamless->rat_image.handle)
	{
		rat_close(h_seamless->rat_image.handle);
	}
	if(h_seamless->rat_video.handle)
	{
		rat_close(h_seamless->rat_video.handle);
	}
	if(h_seamless->rat_mixed.handle)
	{
		rat_close(h_seamless->rat_mixed.handle);
	}

	return SEAMLESS_OK;
}
