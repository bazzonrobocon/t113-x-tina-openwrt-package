#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>

#include <tplayer.h>
#include "backplaydemo.h"

#define TOTAL_VIDEO_AUDIO_NUM 100
#define MAX_FILE_NAME_LEN 256
#define FILE_TYPE_NUM 29
#define FILE_TYPE_LEN 10

typedef struct DemoPlayerContext
{
    TPlayer*          mTPlayer;
    int               mSeekable;
    int               mError;
    int               mVideoFrameNum;
    bool              mPreparedFlag;
    bool              mLoopFlag;
    bool              mSetLoop;
    bool              mComplete;
    char              mUrl[512];
    MediaInfo*        mMediaInfo;
    int               mCurPlayIndex;
    int               mRealFileNum;
    sem_t             mPreparedSem;
    int               mQuitFlag;
    int               mModeFlag;
    char              mVideoAudioList[TOTAL_VIDEO_AUDIO_NUM][MAX_FILE_NAME_LEN];
}DemoPlayerContext;

DemoPlayerContext demoPlayer;
DemoPlayerContext gDemoPlayers[5];
int gScreenWidth = 0;
int gScreenHeight = 0;
int QuitFlag = 0;
int gPlayerNum = 0;
int waitErr = 0;

static void terminate(int sig_no)
{
    printf("Got signal %d, exiting ...\n", sig_no);

    if(demoPlayer.mTPlayer != NULL)
    {
        TPlayerDestroy(demoPlayer.mTPlayer);
        demoPlayer.mTPlayer = NULL;
        printf("TPlayerDestroy() successfully\n");
    }

    sem_destroy(&demoPlayer.mPreparedSem);

    int i=0;
    for(i = gPlayerNum-1;i >= 0;i--){
        if(gDemoPlayers[i].mTPlayer != NULL)
        {
            TPlayerDestroy(gDemoPlayers[i].mTPlayer);
            gDemoPlayers[i].mTPlayer = NULL;
            printf("TPlayerDestroy(%d) successfully\n",i);
        }
    }
    printf("destroy tplayer \n");
    printf("tplaydemo exit\n");
    exit(1);
}
static void install_sig_handler(void)
{
    signal(SIGBUS, terminate);
    signal(SIGFPE, terminate);
    signal(SIGHUP, terminate);
    signal(SIGILL, terminate);
    signal(SIGINT, terminate);
    signal(SIGIOT, terminate);
    signal(SIGPIPE, terminate);
    signal(SIGQUIT, terminate);
    signal(SIGSEGV, terminate);
    signal(SIGSYS, terminate);
    signal(SIGTERM, terminate);
    signal(SIGTRAP, terminate);
    signal(SIGUSR1, terminate);
    signal(SIGUSR2, terminate);
}
static int semTimedWait(sem_t* sem, int64_t time_ms)
{
    int err;

    if(time_ms == -1)
    {
        err = sem_wait(sem);
    }
    else
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += time_ms % 1000 * 1000 * 1000;
        ts.tv_sec += time_ms / 1000 + ts.tv_nsec / (1000 * 1000 * 1000);
        ts.tv_nsec = ts.tv_nsec % (1000*1000*1000);

        err = sem_timedwait(sem, &ts);
    }

    return err;
}

int CallbackForTPlayer(void* pUserData, int msg, int param0, void* param1)
{
    DemoPlayerContext* pDemoPlayer = (DemoPlayerContext*)pUserData;

    CEDARX_UNUSE(param1);
    switch(msg)
    {
        case TPLAYER_NOTIFY_PREPARED:
        {
            printf("TPLAYER_NOTIFY_PREPARED,has prepared.\n");
            sem_post(&pDemoPlayer->mPreparedSem);
            pDemoPlayer->mPreparedFlag = 1;
            break;
        }

        case TPLAYER_NOTIFY_PLAYBACK_COMPLETE:
        {
            printf("TPLAYER_NOTIFY_PLAYBACK_COMPLETE\n");
            pDemoPlayer->mComplete = 1;
            if (pDemoPlayer->mModeFlag == BACKPLAY_MODE_ONCE)
                pDemoPlayer->mQuitFlag = 1;
            else if(pDemoPlayer->mSetLoop == 1)
                pDemoPlayer->mLoopFlag = 1;
            else
                pDemoPlayer->mLoopFlag = 0;
            //PowerManagerReleaseWakeLock("tplayerdemo");
            break;
        }

        case TPLAYER_NOTIFY_SEEK_COMPLETE:
        {
            printf("TPLAYER_NOTIFY_SEEK_COMPLETE>>>>info: seek ok.\n");
            break;
        }

        case TPLAYER_NOTIFY_MEDIA_ERROR:
        {
            switch (param0)
            {
                case TPLAYER_MEDIA_ERROR_UNKNOWN:
                {
                    printf("erro type:TPLAYER_MEDIA_ERROR_UNKNOWN\n");
                    break;
                }
                case TPLAYER_MEDIA_ERROR_UNSUPPORTED:
                {
                    printf("erro type:TPLAYER_MEDIA_ERROR_UNSUPPORTED\n");
                    break;
                }
                case TPLAYER_MEDIA_ERROR_IO:
                {
                    printf("erro type:TPLAYER_MEDIA_ERROR_IO\n");
                    break;
                }
            }
            printf("TPLAYER_NOTIFY_MEDIA_ERROR\n");
            pDemoPlayer->mError = 1;
            if(pDemoPlayer->mPreparedFlag == 0){
                printf("recive err when preparing\n");
                sem_post(&pDemoPlayer->mPreparedSem);
            }
            if(pDemoPlayer->mSetLoop == 1){
                pDemoPlayer->mLoopFlag = 1;
            }else{
                pDemoPlayer->mLoopFlag = 0;
            }
            printf("error: open media source fail.\n");
            break;
        }

        case TPLAYER_NOTIFY_NOT_SEEKABLE:
        {
            pDemoPlayer->mSeekable = 0;
            printf("info: media source is unseekable.\n");
            break;
        }

        case TPLAYER_NOTIFY_BUFFER_START:
        {
            printf("have no enough data to play\n");
            break;
        }

        case TPLAYER_NOTIFY_BUFFER_END:
        {
            printf("have enough data to play again\n");
            break;
        }

        case TPLAYER_NOTIFY_VIDEO_FRAME:
        {
            //printf("get the decoded video frame\n");
            break;
        }

        case TPLAYER_NOTIFY_AUDIO_FRAME:
        {
            //printf("get the decoded audio frame\n");
            break;
        }

        case TPLAYER_NOTIFY_SUBTITLE_FRAME:
        {
            //printf("get the decoded subtitle frame\n");
            break;
        }
        case TPLAYER_NOTYFY_DECODED_VIDEO_SIZE:
        {
            int w, h;
            w   = ((int*)param1)[0];   //real decoded video width
            h  = ((int*)param1)[1];   //real decoded video height
            printf("*****tplayerdemo:video decoded width = %d,height = %d",w,h);
            //int divider = 1;
            //if(w>400){
            //    divider = w/400+1;
            //}
            //w = w/divider;
            //h = h/divider;
            printf("real set to display rect:w = %d,h = %d\n",w,h);
            //TPlayerSetSrcRect(pDemoPlayer->mTPlayer, 0, 0, w, h);
        }

        default:
        {
            printf("warning: unknown callback from Tinaplayer.\n");
            break;
        }
    }
    return 0;
}

static int parse_dir_media(char *InputUrl)
{
    /*play all audio and video in one folder*/
    char fileType[FILE_TYPE_NUM][FILE_TYPE_LEN] = {".avi",".mkv",".flv",".ts",".mp4",".ts",".webm",".asf",".mpg",".mpeg",".mov",".vob",".3gp",".wmv",".pmp",".f4v",
                                                                               ".mp1",".mp2",".mp3",".ogg",".flac",".ape",".wav",".m4a",".amr",".aac",".omg",".oma",".aa3"};
    char* lastStrPos;
    DIR *dir = opendir(InputUrl);
    if (dir == NULL)
    {
        printf("opendir %s fail\n",InputUrl);
        return -1;
    }

    struct dirent *entry;
    int count= 0;
    while ((entry = readdir(dir)) != NULL)
    {
        printf("file record length = %d,type = %d, name = %s\n",entry->d_reclen,entry->d_type,entry->d_name);
        if(entry->d_type == 8)
        {
            char* strpos;
            if((strpos = strrchr(entry->d_name,'.')) != NULL)
            {
                int i = 0;
                printf("cut down suffix is:%s\n",strpos);
                for(i = 0;i < FILE_TYPE_NUM;i++)
                {
                    if(!strncasecmp(strpos,&(fileType[i]),strlen(&(fileType[i]))))
                    {
                        printf("find the matched type:%s\n",&(fileType[i]));
                        break;
                    }
                }

                if(i < FILE_TYPE_NUM)
                {
                    if(count < TOTAL_VIDEO_AUDIO_NUM)
                    {
                        strncpy(&demoPlayer.mVideoAudioList[count],entry->d_name,strlen(entry->d_name));
                        printf("video file name = %s\n",&demoPlayer.mVideoAudioList[count]);
                        count++;
                    }
                    else
                    {
                        count++;
                        printf("warning:the video file in /mnt/UDISK/ is %d,which is larger than %d,we only support %d\n",count,TOTAL_VIDEO_AUDIO_NUM,TOTAL_VIDEO_AUDIO_NUM);
                    }
                }
            }
        }
    }
    closedir(dir);
    if(count >  TOTAL_VIDEO_AUDIO_NUM)
    {
        demoPlayer.mRealFileNum = TOTAL_VIDEO_AUDIO_NUM;
    }
    else
    {
        demoPlayer.mRealFileNum = count;
    }
    if(demoPlayer.mRealFileNum == 0)
    {
        printf("[BACKPLAY][ERROR]there are no video or audio files in %s,exit(-1)\n",InputUrl);
        exit(-1);
    }

}

void backplaydemo(char* InputUrl, int mode)
{
    if (mode != BACKPLAY_MODE_LOOP &&
            mode != BACKPLAY_MODE_ONCE &&
            mode != BACKPLAY_MODE_DIR) {
        printf("[%s %d]error:invalid mode:%d\n", __func__, __LINE__, mode);
        return -1;
    }
    if (mode == BACKPLAY_MODE_ONCE && (GetConfigParamterInt("BootPlay",0) == 0)) {
        printf("[%s %d]error: bootplay not support yet, please check the BootPlay in cedarx.conf\n",__func__, __LINE__);
        return -1;
    }
#if 0
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(0);
    }
    if (pid > 0) {
        exit(0);
    }
    setsid();
#else
    int daemonRet = daemon(1,1);
#endif

    install_sig_handler();

    printf("\n");
    printf("******************************************************************************************\n");
    printf("* This program implements a simple player that running in background\n");
    printf("******************************************************************************************\n");
    if(((access("/dev/zero",F_OK)) < 0)||((access("/dev/fb0",F_OK)) < 0)){
        printf("/dev/zero OR /dev/fb0 is not exit\n");
    }else{
        system("dd if=/dev/zero of=/dev/fb0");//clean the framebuffer
    }


    memset(&demoPlayer, 0, sizeof(DemoPlayerContext));
    demoPlayer.mError = 0;
    demoPlayer.mSeekable = 1;
    demoPlayer.mPreparedFlag = 0;
    demoPlayer.mLoopFlag = 0;
    demoPlayer.mSetLoop = 0;
    demoPlayer.mQuitFlag = 0;
    demoPlayer.mModeFlag = mode;
    QuitFlag = 0;
    demoPlayer.mMediaInfo = NULL;

    if (demoPlayer.mModeFlag == BACKPLAY_MODE_DIR)
        parse_dir_media(InputUrl);
    //* simple loop mode.(for BACKPLAY_MODE_ONCE/BACKPLAY_MODE_LOOP)
    //* create a player.
    demoPlayer.mTPlayer= TPlayerCreate(CEDARX_PLAYER);
    if(demoPlayer.mTPlayer == NULL)
    {
        printf("can not create tplayer, quit.\n");
        exit(-1);
    }
        //* set callback to player.
    TPlayerSetNotifyCallback(demoPlayer.mTPlayer,CallbackForTPlayer, (void*)&demoPlayer);
#ifndef ONLY_ENABLE_AUDIO
   //if((gScreenWidth == 0 || gScreenHeight == 0) && demoPlayer.mTPlayer->mLayerCtrl)
   //{
   //    VoutRect tmpRect;
   //    TPlayerGetDisplayRect(demoPlayer.mTPlayer,&tmpRect);
   //    gScreenWidth = tmpRect.width;
   //    gScreenHeight = tmpRect.height;
   //    printf("screen width:%d,screen height:%d\n",gScreenWidth,gScreenHeight);
   //}
#endif
    sem_init(&demoPlayer.mPreparedSem, 0, 0);
    /* set url*/
    memset(demoPlayer.mUrl,0,512);
    strcpy(demoPlayer.mUrl,InputUrl);
    if (demoPlayer.mModeFlag == BACKPLAY_MODE_DIR) {
        if (demoPlayer.mCurPlayIndex >= demoPlayer.mRealFileNum)
            demoPlayer.mCurPlayIndex = 0;
        strcat(demoPlayer.mUrl, &demoPlayer.mVideoAudioList[demoPlayer.mCurPlayIndex]);
        demoPlayer.mCurPlayIndex++;
    }
    printf("demoPlayer.mUrl = %s\n",demoPlayer.mUrl);

#if 1
    if(TPlayerSetDataSource(demoPlayer.mTPlayer,(const char*)demoPlayer.mUrl,NULL) != 0)
    {
        printf("TPlayerSetDataSource() return fail.\n");
        goto err_BPlayer;
    }
    else
    {
        printf("setDataSource end\n");
    }

    /* prepare */
    demoPlayer.mPreparedFlag = 0;
    if(TPlayerPrepareAsync(demoPlayer.mTPlayer) != 0)
    {
        printf("TPlayerPrepareAsync() return fail.\n");
    }
    else
    {
        printf("prepare\n");
    }
    waitErr = semTimedWait(&demoPlayer.mPreparedSem,300*1000);
    if(waitErr == -1)
    {
        printf("prepare fail,has wait 300s\n");
        goto err_BPlayer;
    }
    else if(demoPlayer.mError == 1)
    {
        printf("prepare fail\n");
        goto err_BPlayer;
    }
    printf("prepared ok\n");
    if(TPlayerStart(demoPlayer.mTPlayer) != 0)
    {
        printf("TPlayerStart() return fail.\n");
        goto err_BPlayer;
    }
    else
    {
        printf("started.\n");
    }
    demoPlayer.mSetLoop = 1;
    while(!demoPlayer.mQuitFlag) {
        sleep(1);
        if(demoPlayer.mLoopFlag) {
            demoPlayer.mLoopFlag = 0;
            printf("TPlayerReset begin\n");
            if(TPlayerReset(demoPlayer.mTPlayer) != 0)
            {
                printf("TPlayerReset return fail.\n");
            }
            else
            {
                printf("reset the player ok.\n");
                if(demoPlayer.mError == 1)
                {
                    demoPlayer.mError = 0;
                }
                //PowerManagerReleaseWakeLock("tplayerdemo");
            }
            demoPlayer.mSeekable = 1;   //* if the media source is not seekable, this flag will be
                                                                //* clear at the TINA_NOTIFY_NOT_SEEKABLE callback.
            if (demoPlayer.mModeFlag == BACKPLAY_MODE_DIR) {
               if(demoPlayer.mCurPlayIndex >= demoPlayer.mRealFileNum)
                   demoPlayer.mCurPlayIndex = 0;
               strcpy(demoPlayer.mUrl, InputUrl);
               strcat(demoPlayer.mUrl, &demoPlayer.mVideoAudioList[demoPlayer.mCurPlayIndex]);
               printf("demoPlayer.mUrl = %s\n", demoPlayer.mUrl);
               demoPlayer.mCurPlayIndex++;
            }
            //* set url to the tplayer.
            if(TPlayerSetDataSource(demoPlayer.mTPlayer,(const char*)demoPlayer.mUrl,NULL) != 0)
            {
                printf("TPlayerSetDataSource() return fail.\n");
            }
            else
            {
                printf("TPlayerSetDataSource() end\n");
            }
            demoPlayer.mPreparedFlag = 0;
            if(TPlayerPrepareAsync(demoPlayer.mTPlayer) != 0)
            {
                printf("TPlayerPrepareAsync() return fail.\n");
            }
            else
            {
                printf("preparing...\n");
            }
            waitErr = semTimedWait(&demoPlayer.mPreparedSem,300*1000);
            if(waitErr == -1)
            {
                printf("prepare fail,has wait 300s\n");
                break;
            }
            else if(demoPlayer.mError == 1)
            {
                printf("prepare fail\n");
                break;
            }
            printf("prepare ok\n");
            printf("start play\n");

            /*set display rect for tlayer optimalization*/
//            TPlayerSetDisplayRect(demoPlayer.mTPlayer, 0, 0, gScreenWidth,gScreenHeight);

            //TPlayerSetLooping(demoPlayer.mTPlayer,1);//let the player into looping mode
            //* start the playback
            if(TPlayerStart(demoPlayer.mTPlayer) != 0)
            {
                printf("TPlayerStart() return fail.\n");
            }
            else
            {
                printf("started.\n");
                //PowerManagerAcquireWakeLock("tplayerdemo");
            }
       }

    }
err_BPlayer:
    if(demoPlayer.mTPlayer != NULL)
    {
        TPlayerDestroy(demoPlayer.mTPlayer);
        demoPlayer.mTPlayer = NULL;
        printf("TPlayerDestroy() successfully\n");
    }

    printf("destroy tplayer \n");

    sem_destroy(&demoPlayer.mPreparedSem);
#endif
exit_player:
    printf("exit the player\n");

}
