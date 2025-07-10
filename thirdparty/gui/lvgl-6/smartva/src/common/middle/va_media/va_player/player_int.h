#ifndef _PLAYER_INT_H_
#define _PLAYER_INT_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>
#include <allwinner/tplayer.h>
#include "dbList.h"

typedef enum
{
	DEFAULAT_CMD    = 0,
	INIT_CMD        = 1,
	EXIT_CMD        = 2,
	PREPARE_CMD     = 3,
	PLAY_CMD        = 4,
	PAUSE_CMD       = 5,
	STOP_CMD        = 6,
	SEEK_TO         = 7,
	SETTING         = 8,
}playerCmd;

enum playerSetParam
{
	SET_VOLUME      = 0,
	SET_LOOP        = 1,
	SET_SCALE       = 2,
	SET_DISPLAY     = 3,
	SET_SPEED       = 4,
	SET_ROTATE      = 5,
	SET_AUDIO       = 6,
	SET_SOUND_CHANNEL = 7,
	SET_AUDIO_EQ    = 8,
};

typedef enum
{
	EXIT_STATUS      = 0,
	INIT_STATUS      = 1,
	STOP_STATUS      = 2,
	ERROR_STATUS     = 3,
	PREPARING_STATUS = 4,
	PREPARED_STATUS  = 5,
	PAUSE_STATUS     = 6,
	PLAY_STATUS      = 7,
	SEEKTO_STATUS    = 8,
	SETTING_STATUS   = 9,
	COMPLETE_STATUS  = 10,
	PRE_COMPLETE_STATUS = 11,
	FIRST_VIDEO_FRAME_SHOW_STATUS = 12,
	FIRST_AUDIO_FRAME_PLAY_STATUS = 13,
}playerStatus;

typedef enum
{
    RESOLUTION_360P_PIXEL     = 230400, //640*360
    RESOLUTION_480P_PIXEL     = 345600, //720*480
    RESOLUTION_720P_PIXEL     = 921600, //1280*720
    RESOLUTION_1080P_PIXEL    = 2073600,//1920*1080
}resolutionPixel;

#define QUEUE_MAX_SIZE    10

typedef struct PLAYER_T
{
    TPlayer*          mTPlayer;
    int               mSeekable;
    int               mStopping;
    int               mError;
    int               mVideoFrameNum;
    int               mExceedVideoLimit;
    MediaInfo*		  mMediaInfo;
	playerStatus	  mpstatus;
	pthread_t		  id;
    sem_t             mPreparedSem;
	TPlayerNotifyCallback callback;

	db_list_t*		  queue_head;
}player_t;


struct player_param{
	playerCmd cmd;
	long int param[2];
};

int tplayer_init(player_t *tplayer,int player_type);
int tplayer_exit(player_t *tplayer);
int tplayer_play_url(player_t *tplayer, const char *path);
int tplayer_play(player_t *tplayer);
int tplayer_pause(player_t *tplayer);
int tplayer_seekto(player_t *tplayer, int nSeekTimeSec);
int tplayer_stop(player_t *tplayer);
int tplayer_volume(player_t *tplayer, int volume);
int tplayer_switch_audio(player_t *tplayer, int audio);
int tplayer_set_speed(player_t *tplayer, TplayerPlaySpeedType nSpeed);
int tplayer_set_looping(player_t *tplayer, bool bLoop);
int tplayer_set_rotate(player_t *tplayer, TplayerVideoRotateType rotateDegree);
int tplayer_set_audio_eq(player_t *tplayer, AudioEqType type);
int tplayer_get_duration(player_t *tplayer, int* sec);
int tplayer_get_current_pos(player_t *tplayer, int* sec);
int tplayer_set_displayrect(player_t *tplayer, int x, int y, unsigned int width, unsigned int height);
player_t *tplayer_pthread_create(void);
void tplayer_pthread_destory(player_t *tplayer);
void tplayer_set_callback(player_t *tplayer, TPlayerNotifyCallback callback);
MediaInfo* tplayer_get_media_Info(player_t *tplayer);
playerStatus tplayer_get_status(player_t *tplayer);
int tplayer_switch_soundchannel(player_t *tplayer, int soundchannelmode);


#endif
