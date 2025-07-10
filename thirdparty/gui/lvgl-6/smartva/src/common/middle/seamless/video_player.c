
#include <stdlib.h>
#include "seamless.h"
#include "media_mixture.h"
#include "app_config_param.h"
#include <allwinner/tplayer.h>

static seamless_rect_t video_rect = {0,0,800,480};
static int video_rotate = TPLAYER_VIDEO_ROTATE_DEGREE_0;


//播放器主回调
static void video_player_callback(void *ui_player, media_event_t event, void *param)
{
	return;
}

static void video_player_status_callback(void *ui_player, void *param)
{
	return;
}
//初始化视频播放器
int video_player_init(void)
{
    media_func_register(MOVIE_SCENE, video_player_callback, video_player_status_callback);
	media_player_state_waiting_for(INIT_STATUS);

    return SEAMLESS_OK;
}

//退出视频播放器
int video_player_exit(void)
{
	media_func_unregister(MOVIE_SCENE, 0);
	return SEAMLESS_OK;
}

//获取播放器状态
int video_player_get_status(void)
{
	player_ui_t *player_ui = media_get_player_data();
	int status = tplayer_get_status(player_ui->tplayer);
	// printf("status = %d\n",status);
    if(status == PLAY_STATUS)
	{
		return VIDEO_STATUS_PLAY;
	}
	else if(status == STOP_STATUS || status == COMPLETE_STATUS || status == INIT_STATUS)
	{
		printf("VIDEO_STATUS_STOP\n");
		return VIDEO_STATUS_STOP;
	}
    return VIDEO_STATUS_OTHER;
}


//开始播放
int video_player_start(char *filename)
{
	player_ui_t *player_ui = media_get_player_data();

	if(tplayer_play_url(player_ui->tplayer, filename) < 0)
	{
		return SEAMLESS_FAIL;
	}
	tplayer_set_rotate(player_ui->tplayer, video_rotate);
	tplayer_play(player_ui->tplayer);
	tplayer_set_displayrect(player_ui->tplayer,video_rect.x,video_rect.y,video_rect.width,video_rect.height);

	media_player_state_waiting_for(PLAY_STATUS);

    return SEAMLESS_OK;
}

//停止播放
int video_player_stop(void)
{
	player_ui_t *player_ui = media_get_player_data();
	tplayer_stop(player_ui->tplayer);
	media_player_state_waiting_for(STOP_STATUS);
    return SEAMLESS_OK;
}


//设置显示区域
int video_player_set_win_rect(seamless_rect_t *rect)
{
	video_rect.x = rect->x;
	video_rect.y = rect->y;
	video_rect.width  = rect->width;
	video_rect.height = rect->height;
    return SEAMLESS_OK;
}

//设置缩放方式
int video_set_win_zoom(void)
{
	return SEAMLESS_OK;
}

int video_set_rotate(int rotate)
{
	int temp = TPLAYER_VIDEO_ROTATE_DEGREE_0;
	switch(rotate)
	{
		case SEAMLESS_ROTATE_0:
			temp = TPLAYER_VIDEO_ROTATE_DEGREE_0;
			break;
		case SEAMLESS_ROTATE_90:
			temp = TPLAYER_VIDEO_ROTATE_DEGREE_90;
			break;
		case SEAMLESS_ROTATE_180:
			temp = TPLAYER_VIDEO_ROTATE_DEGREE_180;
			break;
		case SEAMLESS_ROTATE_270:
			temp = TPLAYER_VIDEO_ROTATE_DEGREE_270;
			break;
	}
	video_rotate = temp;
	return SEAMLESS_OK;
}

int video_player_set_status_cb(void *cb)
{
	media_set_player_status_callback((media_player_status_cb)cb);
	return SEAMLESS_OK;
}
