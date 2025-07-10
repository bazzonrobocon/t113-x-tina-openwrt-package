#ifndef __TPLAYER_DEMO_H__
#define __TPLAYER_DEMO_H__


#define tp_err(fmt, args...) printf("[%s, %d]error: "fmt"\r\n", __func__, __LINE__, ##args)
#define tp_info(fmt, args...) printf("[%s, %d]info: "fmt"\r\n", __func__, __LINE__, ##args)

#define TPLAYERDEMO_DEBUG 0
#if TPLAYERDEMO_DEBUG
#define tp_dbg(fmt, args...) printf("[%s, %d]debug "fmt"\r\n", __func__, __LINE__, ##args)
#else
#define tp_dbg(fmt, args...)
#endif


#define TOTAL_VIDEO_AUDIO_NUM 100
#define MAX_FILE_NAME_LEN 256
#define FILE_TYPE_NUM 31
#define FILE_TYPE_LEN 10
#define VIDEO_TYPE_NUM 11
#define VIDEO_TYPE_LEN 10
#define USE_REPEAT_RESET_MODE 1
#define LOOP_PLAY_FLAG 1

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
    char              mVideoAudioList[TOTAL_VIDEO_AUDIO_NUM][MAX_FILE_NAME_LEN];
    int               mCurPlayIndex;
    int               mRealFileNum;
    sem_t             mPreparedSem;
}DemoPlayerContext;

//* define commands for user control.
typedef struct Command
{
    const char* strCommand;
    int         nCommandId;
    const char* strHelpMsg;
}Command;

#define COMMAND_HELP            0x1     //* show help message.
#define COMMAND_QUIT            0x2     //* quit this program.

#define COMMAND_SET_SOURCE      0x101   //* set url of media file.
#define COMMAND_PREPARE         0x102   //* prepare the media file.
#define COMMAND_PLAY            0x103   //* start playback.
#define COMMAND_PAUSE           0x104   //* pause the playback.
#define COMMAND_STOP            0x105   //* stop the playback.
#define COMMAND_SEEKTO          0x106   //* seek to posion, in unit of second.
#define COMMAND_RESET           0x107   //* reset the player
#define COMMAND_SHOW_MEDIAINFO  0x108   //* show media information.
#define COMMAND_SHOW_DURATION   0x109   //* show media duration, in unit of second.
#define COMMAND_SHOW_POSITION   0x110   //* show current play position, in unit of second.
#define COMMAND_SWITCH_AUDIO    0x111   //* switch autio track.
#define COMMAND_PLAY_URL        0x112   //set url and prepare and play
#define COMMAND_SET_VOLUME      0x113   //set the software volume
#define COMMAND_GET_VOLUME      0x114   //get the software volume
#define COMMAND_SET_LOOP        0x115   //set loop play flag,1 means loop play,0 means not loop play
#define COMMAND_SET_SCALEDOWN   0x116   //set video scale down ratio,valid value is:2,4,8 .  2 means 1/2 scaledown,4 means 1/4 scaledown,8 means 1/8 scaledown
#define COMMAND_FAST_FORWARD    0x117   //fast forward,valid value is:2,4,8,16, 2 means 2 times fast forward,4 means 4 times fast forward,8 means 8 times fast forward,16 means 16 times fast forward
#define COMMAND_FAST_BACKWARD   0x118   //fast backward,valid value is:2,4,8,16,2 means 2 times fast backward,4 means 4 times fast backward,8 means 8 times fast backward,16 means 16 times fast backward
#define COMMAND_SET_SRC_RECT    0x119  //set display source crop rect
#define COMMAND_SET_OUTPUT_RECT 0x120  //set display output display rect
#define COMMAND_GET_DISP_FRAMERATE   0x121   //* show the real display framerate

#if TPLAYERDEMO_SUSPEND_TEST
#define COMMAND_START_SUSPEND_TEST 0X122 //* run the loop of suspend
#define COMMAND_STOP_SUSPEND_TEST 0X123 //* stop the loop of suspend
#endif

#endif
