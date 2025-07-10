#include "seamless.h"
#include "player_int.h"
#include <semaphore.h>
#include <pthread.h>

#define SEAMLESS_SOURCE_PATH "/mnt/exUDISK"			//资源扫描路径
#define SEAMLESS_THUMB_PATH  "/mnt/exUDISK/.thumb"	//缩略图存放路径
#define SEAMLESS_THUMB_CONFIG_FILE  "/mnt/exUDISK/.thumb/thumb.cfg"	//缩略图配置文件
#define IMAGE_PLAY_TIME      1			//图片播放时间间隔，为检测间隔的整数倍 单位：s,后续可以放在全局配置中动态改变
#define IMAGE_PLAY_TIME_STEP 50				//图片播放检测间隔，单位：ms
#define IMAGE_PLAY_TIME_PRE  100		//图片播放结束前，预留准备下一资源时间，为检测间隔的整数倍 单位： ms
#define ALIGN_TO_8B(x)    ((((x) + (1 <<  3) - 1) >>  3) <<  3)
#define ALIGN_TO_16B(x)   ((((x) + (1 <<  4) - 1) >>  4) <<  4)
#define ALIGN_TO_32B(x)   ((((x) + (1 <<  5) - 1) >>  5) <<  5)


static seamless_ctrl_t *g_seamless = NULL;//全局结构体
static pthread_mutex_t status_mutex;
static sem_t preload_finish_sem;
static sem_t play_finish_sem;
static sem_t play_sync_sem;
// static sem_t play_start_sem;

static pthread_cond_t  preload_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t preload_wait_mutex;

static pthread_cond_t  image_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t image_wait_mutex;

static pthread_t  video_id;
static pthread_t  image_id;
static pthread_t  mixed_id;
// static pthread_t  split_id;
static pthread_t  image_slave_id;
static pthread_t  preload_id;

static int player_get_status(void);
static int player_set_status(int status);
static int player_start(void);


//逻辑坐标映射至物理坐标
static void logic_coordinates_mapping(seamless_init_para_t *para)
{
	int width_phy = 0,height_phy = 0;
    int rotate = 0;
    seamless_rect_t video_rect = {0};
    seamless_rect_t image_rect = {0};

    width_phy  = para->screen_size.width;
    height_phy = para->screen_size.height;
    rotate     = para->screen_rotate;

	if(rotate == SEAMLESS_ROTATE_0)
	{
		video_rect.x	   = para->video_rect.x;
		video_rect.y	   = para->video_rect.y;
		video_rect.width   = para->video_rect.width;
		video_rect.height  = para->video_rect.height;
		image_rect.x	   = para->image_rect.x;
		image_rect.y	   = para->image_rect.y;
		image_rect.width   = para->image_rect.width;
		image_rect.height  = para->image_rect.height;
	}
	else if(rotate == SEAMLESS_ROTATE_90)
	{
		video_rect.x		= para->video_rect.y;
		video_rect.y		= height_phy - para->video_rect.width - para->video_rect.x;
		video_rect.width    = para->video_rect.height;
		video_rect.height   = para->video_rect.width;
		image_rect.x		= para->image_rect.y;
		image_rect.y		= height_phy - para->image_rect.width - para->image_rect.x;
		image_rect.width    = para->image_rect.height;
		image_rect.height   = para->image_rect.width;
	}
	else if(rotate == SEAMLESS_ROTATE_180)
	{
		video_rect.x		= width_phy - para->video_rect.width - para->video_rect.x;
		video_rect.y		= height_phy - para->video_rect.height - para->video_rect.y;
		video_rect.width    = para->video_rect.width;
		video_rect.height   = para->video_rect.height;
		image_rect.x		= width_phy - para->image_rect.width - para->image_rect.x;
		image_rect.y		= height_phy - para->image_rect.height - para->image_rect.y;
		image_rect.width    = para->image_rect.width;
		image_rect.height   = para->image_rect.height;
	}
	else if(rotate == SEAMLESS_ROTATE_270)
	{
		video_rect.x		= width_phy-para->video_rect.y-para->video_rect.height;
		video_rect.y		= para->video_rect.x;
		video_rect.width    = para->video_rect.height;
		video_rect.height   = para->video_rect.width;
		image_rect.x		= width_phy-para->image_rect.y-para->image_rect.height;
		image_rect.y		= para->image_rect.x;
		image_rect.width    = para->image_rect.height;
		image_rect.height   = para->image_rect.width;
	}

    para->video_rect.x = video_rect.x;
	para->video_rect.y = video_rect.y;
	para->video_rect.width = video_rect.width;
	para->video_rect.height = video_rect.height;
    para->image_rect.x = image_rect.x;
	para->image_rect.y = image_rect.y;
	para->image_rect.width = image_rect.width;
	para->image_rect.height = image_rect.height;

	SEAMLESS_LOG("video area %d %d %d %d",video_rect.x,video_rect.y,video_rect.width,video_rect.height);
	SEAMLESS_LOG("image area %d %d %d %d",image_rect.x,image_rect.y,image_rect.width,image_rect.height);
}

//检查初始化参数
//合法性、对齐、映射
static int check_init_para(seamless_init_para_t *para)
{
    if(para == NULL)
    {
        return SEAMLESS_FAIL;
    }

    logic_coordinates_mapping(para);
	para->screen_rotate= (4-para->screen_rotate)%4;
    return SEAMLESS_OK;
}

//休眠指定线程
static void thread_down(pthread_mutex_t *mutex, pthread_cond_t *cond)
{
	pthread_mutex_lock(mutex);
	pthread_cond_wait(cond, mutex);
	pthread_mutex_unlock(mutex);
}

//唤醒指定线程
static void thread_up(pthread_mutex_t *mutex, pthread_cond_t *cond)
{
	pthread_mutex_lock(mutex);
	pthread_cond_signal(cond);
	pthread_mutex_unlock(mutex);
}

//唤醒预加载线程
static void wakeup_proload_thread(void)
{
	thread_up(&preload_wait_mutex,&preload_wait_cond);
}

//等待预加载完成
static void wait_preload_finish(void)
{
	sem_wait(&preload_finish_sem);
}

//报告预加载完成
static void report_preload_finish(void)
{
	sem_post(&preload_finish_sem);
}

//唤醒图片播放子线程
static void wakeup_playing_thread(void)
{
	thread_up(&image_wait_mutex,&image_wait_cond);
}

//等待图片播放子线程完成播放
static void wait_playing_finish(void)
{
	sem_wait(&play_finish_sem);
}

//图片播放子线程报告播放完毕
static void report_playing_finish(void)
{
	sem_post(&play_finish_sem);
}

//报告视频开始同步
//mixed模式时，释放最后一张图片
//split模式时，通知图片线程播放图片
static int report_playing_sync(int mode)
{
	int value = 0;
	int try_cnt = 0;
	do
	{
		sem_getvalue(&play_sync_sem, &value);
		if(value == 0)
		{
			sem_post(&play_sync_sem);
			break;
		}
		else
		{
			SEAMLESS_WRN("maybe something wrong?");
			try_cnt++;
			if(try_cnt == 1000)
			{
				player_set_status(SEAMLESS_STATUS_STOP);
				break;
			}
			usleep(10000);
		}
	}while(1);

	return SEAMLESS_OK;
}

//等待视频同步
static int wait_for_sync(void)
{
	// int value = 0;
	// sem_getvalue(&play_sync_sem, &value);
	// if(value > 0)
	// {
	//	return SEAMLESS_OK;
	// }
	// return SEAMLESS_FAIL;
	// sem_wait(&play_sync_sem);
	if(sem_trywait(&play_sync_sem) == 0)
	{
		return SEAMLESS_OK;
	}
	return SEAMLESS_FAIL;
}

//播放器状态回调
static void video_player_status_cb(void *ui_player, void *param)
{
	static int last_status = COMPLETE_STATUS;
    int status = *(int *)param;
	int show_mode = g_seamless->init_para.show_mode;

	if(last_status == status)
	{
		SEAMLESS_ERR("same status! status = %d",status);
		return ;
	}
	last_status = status;

    SEAMLESS_LOG("status = %d\n",status);

    if(status == PRE_COMPLETE_STATUS)//即将播放完毕
    {
		SEAMLESS_LOG("PRE_COMPLETE_STATUS");
		if(show_mode != SEAMLESS_MODE_SPLIT && show_mode != SEAMLESS_MODE_SPLIT_SYNC)
		{
			wakeup_proload_thread();
		}
    }
    else if(status == COMPLETE_STATUS)//已经播放完毕
    {
		if(show_mode != SEAMLESS_MODE_SPLIT && show_mode != SEAMLESS_MODE_SPLIT_SYNC)
		{
			report_playing_finish();
		}
    }
    else if(status == FIRST_VIDEO_FRAME_SHOW_STATUS)//第一帧已经显示完毕
    {
		if(show_mode == SEAMLESS_MODE_MIXED || show_mode == SEAMLESS_MODE_SPLIT)
		{
			report_playing_sync(0);
		}
		else if(show_mode == SEAMLESS_MODE_SPLIT_SYNC)
		{
			report_playing_sync(1);
		}
    }
}

//预加载任务
static void* media_preload_thread(void *arg)
{
	int ret;
	while(1)
	{
		SEAMLESS_LOG("preload down!\n");
		thread_down(&preload_wait_mutex,&preload_wait_cond);
		SEAMLESS_LOG("preload up!\n");
		// if(player_get_status() == SEAMLESS_STATUS_STOP)
		// {
		//	report_preload_finish();
		//	SEAMLESS_LOG("kill preload thread\n");
		//	pthread_exit(0);
		// }

		ret = media_preload(g_seamless);
		if(ret == SEAMLESS_FAIL)
		{
			com_err("preload_err!");
			player_set_status(SEAMLESS_STATUS_STOP);
		}
		report_preload_finish();
		if(player_get_status() == SEAMLESS_STATUS_STOP)
		{
			SEAMLESS_ERR("kill preload thread\n");
			pthread_exit(0);
		}
	}
}
//图片播放主任务
static void* image_play_main_thread(void *arg)
{
    int delay;
    int delay_cnt,delay_cnt_temp;
	int preload_cnt;

	delay_cnt = delay_cnt_temp = IMAGE_PLAY_TIME*1000/IMAGE_PLAY_TIME_STEP;
	delay = IMAGE_PLAY_TIME_STEP*1000;
	preload_cnt = IMAGE_PLAY_TIME_PRE/IMAGE_PLAY_TIME_STEP;

	media_preload(g_seamless);
	player_start();
    while(1)
    {
		while(delay_cnt_temp--)
		{
			usleep(delay);
			if((delay_cnt_temp) == preload_cnt)//预加载时间到，唤醒预加载线程
			{
				wakeup_proload_thread();
			}
			if(player_get_status() == SEAMLESS_STATUS_STOP)//中途退出
			{
				if(delay_cnt_temp > preload_cnt)//图片模式时，可以直接删除预加载线程。但是为了和其他模式兼容，重新唤醒预加载线程再删除预加载线程。
				{
					wakeup_proload_thread();
					wait_preload_finish();
					media_unload(g_seamless);
					goto image_play_main_task_exit;
				}
				else
				{
					wait_preload_finish();
					media_unload(g_seamless);
					goto image_play_main_task_exit;
				}
			}
		}
		wait_preload_finish();
		delay_cnt_temp = delay_cnt;
		player_start();
    }

image_play_main_task_exit:
	SEAMLESS_LOG("kill image main thread!\n");
	pthread_exit(0);
}
//图片播放子任务，mixed split模式生效
static void* image_play_slave_thread(void *arg)
{

	int delay;
    int delay_cnt,delay_cnt_temp;
	int preload_cnt;

	delay_cnt = delay_cnt_temp = IMAGE_PLAY_TIME*1000/IMAGE_PLAY_TIME_STEP;
	delay = IMAGE_PLAY_TIME_STEP*1000;
	preload_cnt = IMAGE_PLAY_TIME_PRE/IMAGE_PLAY_TIME_STEP;

    while(1)
    {
		SEAMLESS_LOG("image slave down!\n");
		thread_down(&image_wait_mutex,&image_wait_cond);
		// if(player_get_status() == SEAMLESS_STATUS_STOP)
		// {
		//	goto image_play_slave_task_exit;
		// }
		// while(1)
		// {
		//	if(sem_trywait(&play_start_sem) != 0)
		//	{
		//		if(player_get_status() == SEAMLESS_STATUS_STOP)
		//		{
		//			goto image_play_slave_task_exit;
		//		}
		//		usleep(10000);
		//	}
		// }
		SEAMLESS_LOG("image slave up!\n");
		image_player_start(&g_seamless->loading.image);

		while(delay_cnt_temp--)
		{
			usleep(delay);

			if(delay_cnt_temp == preload_cnt)
			{
				wakeup_proload_thread();
			}
			if (player_get_status() == SEAMLESS_STATUS_STOP)
			{
				if(delay_cnt_temp > preload_cnt)
				{
					wakeup_proload_thread();//mixed模式下，下一资源加载状态不确定，因此视频和图片都还是预加载，确保playing和loading不一致。
					report_playing_finish();
					goto image_play_slave_task_exit;
				}
				else
				{
					report_playing_finish();
					goto image_play_slave_task_exit;
				}
			}
		}
		SEAMLESS_LOG("show image finish!");
		delay_cnt_temp = delay_cnt;
		report_playing_finish();
    }

image_play_slave_task_exit:
	SEAMLESS_ERR("image_play_slave_task del!!");
    pthread_exit(0);
}

//视频播放主任务
static void* video_play_main_thread(void *arg)
{
	int index;
	char * filename;
	int ret;
	int err_num = 0;

	filename = malloc(RAT_MAX_FULL_PATH_LEN);
	if(filename == NULL)
	{
		goto video_play_main_thread_exit;
	}
	player_set_status(SEAMLESS_STATUS_PLAY);

	while(1)
	{
		if(video_player_get_status() == VIDEO_STATUS_STOP)
		{
			index = rat_npl_get_next(g_seamless->rat_video.npl);
            if(index >= 0)
            {
try_again:
                rat_npl_index2file(g_seamless->rat_video.npl,index,filename);
				video_player_stop();
				printf("filename = %s\n",filename);

				ret = video_player_start(filename);
				if(ret < 0)
				{
					index++;
					err_num++;
					if(err_num == g_seamless->rat_video.total)//如果全部播放失败，就退出
					{
						goto video_play_main_thread_exit;
					}
					goto try_again;
				}

				rat_npl_set_cur(g_seamless->rat_video.handle,index);
				err_num = 0;
            }
			else
			{
				printf("index <0!!!!\n");
			}
		}
		if(player_get_status() == SEAMLESS_STATUS_STOP)
		{
			goto video_play_main_thread_exit;
		}
		usleep(1000);
	}

video_play_main_thread_exit:
	SEAMLESS_LOG("kill video main thread\n");
	video_player_stop();
	if(filename)
	{
		free(filename);
	}

	pthread_exit(0);
	return NULL;
}

//混合播放主任务
//视频由主任务控制播放，图片由子线程播放
static void* mixed_play_main_thread(void *arg)
{
	media_preload(g_seamless);//搜索媒体，获取第一个可播放媒体
	player_start();//播放该媒体
	player_set_status(SEAMLESS_STATUS_PLAY);
	while(1)
	{
		wait_playing_finish();//等待播放完毕,播放完毕前，子线程会预加载资源。主线程被阻塞，由子线程先判断退出。
		printf("mixed_play_main_thread 1\n");
		wait_preload_finish();//等待预加载完成
		printf("mixed_play_main_thread 2\n");
		if(player_get_status() == SEAMLESS_STATUS_STOP)
		{
			printf("mixed_play_main_thread 3\n");
			goto mixed_play_main_thread_exit;
		}
		printf("mixed_play_main_thread 4\n");
		player_start();//播放媒体,视频使用robin播放，图片使用线程播放，播放中途可以退出或者暂停
		printf("mixed_play_main_thread 5\n");
	}
mixed_play_main_thread_exit:
	SEAMLESS_ERR("kill mixed main thread\n");
	pthread_exit(0);
	return NULL;
}



//分屏播放主任务
//分屏模式下，主任务只进行图片播放，视频播放由视频线程完成。
//尚未完成
static void* split_play_main_thread(void *arg)
{
	media_preload(g_seamless);//获取第一张图片
	while(wait_for_sync() == SEAMLESS_FAIL)//等待视频第一帧播放完毕
	{
		if (player_get_status() == SEAMLESS_STATUS_STOP)
		{
			media_unload(g_seamless);
			goto split_play_main_thread_exit;
		}
		usleep(10000);
	}
	player_start();//开始播放图片

	while(1)
	{
		wait_playing_finish();//等待视频播放完毕，播放完毕前，会预加载图片。等待图片播放完毕，
		wait_preload_finish();//等待图片准备完成
		if (player_get_status() == SEAMLESS_STATUS_STOP)
		{
			media_unload(g_seamless);
			goto split_play_main_thread_exit;
		}
		if(g_seamless->init_para.show_mode == SEAMLESS_MODE_SPLIT_SYNC)//在同步模式
		{
			while(wait_for_sync() == SEAMLESS_FAIL)
			{
				if (player_get_status() == SEAMLESS_STATUS_STOP)
				{
					media_unload(g_seamless);
					goto split_play_main_thread_exit;
				}
				usleep(10000);
			}
		}
		player_start();//播放下一张图片
	}

split_play_main_thread_exit:
	SEAMLESS_LOG("kill split main thread!\n");
	// esKRNL_TDel(g_seamless_ctrl->preload_task);
	// esKRNL_TDel(EXEC_prioself);
	pthread_exit(0);
	return NULL;
}


//准备资源
static int player_init(void)
{
    int show_mode;
    int enable_image = 1;
    int enable_video = 1;

    show_mode = g_seamless->init_para.show_mode;

    if(show_mode == SEAMLESS_MODE_IMAGE_ONLY)
    {
        enable_video = 0;
    }
    if(show_mode == SEAMLESS_MODE_VIDEO_ONLY)
    {
        enable_image = 0;
    }
    if(enable_video == 1)
    {
        video_player_init();
		video_player_set_win_rect(&g_seamless->init_para.video_rect);
		video_set_rotate(g_seamless->init_para.screen_rotate);
    }
    if(enable_image == 1)
    {
        image_player_init(0);
		image_player_set_scn_rect(&g_seamless->init_para.image_rect);
		image_player_set_src_rect(&g_seamless->init_para.image_rect);//w h 有效
    }


    return SEAMLESS_OK;
}

//播放当前媒体
//释放上一张图片
static int player_start(void)
{
	int media_type;
	int show_mode;
	media_type = g_seamless->loading.media_type;
	show_mode = g_seamless->init_para.show_mode;
	g_seamless->playing.media_type = media_type;
	if(show_mode != SEAMLESS_MODE_IMAGE_ONLY)
	{
		video_player_stop();
	}

	if(media_type == SEAMLESS_MEDIA_IMAGE)
	{
		if(show_mode == SEAMLESS_MODE_IMAGE_ONLY)
		{
			image_player_start(&g_seamless->loading.image);
			memcpy(&g_seamless->playing.image,&g_seamless->loading.image,sizeof(image_out_para_t));
		}
		else
		{
			image_player_set_scn_rect(&g_seamless->init_para.image_rect);
			image_player_set_src_rect(&g_seamless->init_para.image_rect);
			player_set_status(SEAMLESS_STATUS_PLAY);
			wakeup_playing_thread();
			memcpy(&g_seamless->playing.image,&g_seamless->loading.image,sizeof(image_out_para_t));
		}

		return SEAMLESS_OK;
	}
	else if(media_type == SEAMLESS_MEDIA_VIDEO)
	{
		if(show_mode == SEAMLESS_MODE_MIXED)
		{
			strcpy(g_seamless->playing.filename,g_seamless->loading.filename);
			video_player_start(g_seamless->playing.filename);
			while(wait_for_sync() == SEAMLESS_FAIL)
			{
				usleep(10000);
			}
			image_unload(&g_seamless->playing.image);
			memset(&g_seamless->playing.image,0,sizeof(image_out_para_t));
		}
		else
		{
			SEAMLESS_WRN("maybe something wrong?\n");
		}
	}

	return SEAMLESS_OK;
}

//获取播放状态
static int player_get_status(void)
{
	int status;

	pthread_mutex_lock(&status_mutex);
	status = g_seamless->status;
	pthread_mutex_unlock(&status_mutex);
	return status;
}

//设置播放状态
static int player_set_status(int status)
{
	if(status > SEAMLESS_STATUS_PLAY)
	{
		SEAMLESS_ERR("err status");
		return SEAMLESS_FAIL;
	}

	pthread_mutex_lock(&status_mutex);
	if(g_seamless->status != status)
	{
		g_seamless->status = status;
	}
	else
	{
		pthread_mutex_unlock(&status_mutex);
		return EPDK_OK;
	}
	pthread_mutex_unlock(&status_mutex);
	if(status == SEAMLESS_STATUS_STOP)
	{
		if(g_seamless->playing.media_type == SEAMLESS_MEDIA_VIDEO || \
			g_seamless->init_para.show_mode == SEAMLESS_MODE_SPLIT ||\
			g_seamless->init_para.show_mode == SEAMLESS_MODE_SPLIT_SYNC)
		{
			if(video_player_get_status() == VIDEO_STATUS_STOP)
			{
				SEAMLESS_LOG("langaojie send stop cmd!");
				video_player_stop();
				// robin_status_cb((void*)CEDAR_STAT_PRE_STOP);
				// robin_status_cb((void*)CEDAR_STAT_STOP);
				int status = PRE_COMPLETE_STATUS;
				video_player_status_cb(NULL, &status);
				status = COMPLETE_STATUS;
				video_player_status_cb(NULL, &status);
				// wakeup_playing_thread();
				SEAMLESS_LOG("media_set_status stop");
			}
		}
	}
	return SEAMLESS_OK;
}

//退出播放
static int player_exit(void)
{
	int show_mode;
    int enable_image = 1;
    int enable_video = 1;

	player_set_status(SEAMLESS_STATUS_STOP);

    show_mode = g_seamless->init_para.show_mode;

    if(show_mode == SEAMLESS_MODE_IMAGE_ONLY)
    {
        enable_video = 0;
    }
    if(show_mode == SEAMLESS_MODE_VIDEO_ONLY)
    {
        enable_image = 0;
    }
    if(enable_video == 1)
    {
        video_player_exit();
    }
    if(enable_image == 1)
    {
        image_player_exit();
		image_unload(&g_seamless->loading.image);
		image_unload(&g_seamless->playing.image);
    }

    return SEAMLESS_OK;
}

//申请资源,初始化必要参数
int SeamlessInit(seamless_init_para_t *para)
{
    int ret = 0;

    ret = check_init_para(para);
    if(ret == SEAMLESS_FAIL)
    {
        return SEAMLESS_FAIL;
    }

    if(g_seamless == NULL)
	{
		g_seamless = malloc(sizeof(seamless_ctrl_t));
		if(g_seamless == NULL)
		{
			SEAMLESS_ERR("seamless malloc err!");
			return SEAMLESS_FAIL;
		}
		memset(g_seamless,0x0,sizeof(seamless_ctrl_t));
	}
	else
	{
		SEAMLESS_WRN("seamless init alredy!");
		return 0;
	}
	memcpy(&g_seamless->init_para,para,sizeof(seamless_init_para_t));

/*检查硬件资源*/
    layer_init();
    g2d_init();
	mem_init();
/*检查存储器资源*/
	if(media_probe(g_seamless) == SEAMLESS_FAIL)//扫描指定路径所有视频和图片
	{
		goto media_err;
	}
/*根据最终模式，初始化播放器*/
	player_init();
	SEAMLESS_LOG("SeamlessInit ok\n");
	return SEAMLESS_OK;

media_err:
    free(g_seamless);
	return SEAMLESS_FAIL;
}


//开始播放
int SeamlessStart(void)
{
	int ret = 0;
	int show_mode = g_seamless->init_para.show_mode;

	sem_init(&preload_finish_sem,0,0);
	sem_init(&play_finish_sem,0,0);
	sem_init(&play_sync_sem,0,0);

	pthread_mutex_init(&preload_wait_mutex, NULL);
    pthread_cond_init(&preload_wait_cond, NULL);

	pthread_mutex_init(&image_wait_mutex, NULL);
    pthread_cond_init(&image_wait_cond, NULL);
	switch(show_mode)
	{
		case SEAMLESS_MODE_VIDEO_ONLY:
			{
				ret = pthread_create(&video_id, NULL, video_play_main_thread, NULL);
				if(ret < 0)
				{
					com_err("create thread auto play err!\n");
				}
			}
			break;
		case SEAMLESS_MODE_IMAGE_ONLY:
			{
				ret |= pthread_create(&image_id, NULL, image_play_main_thread, NULL);
				ret |= pthread_create(&preload_id, NULL, media_preload_thread, NULL);
			}
			break;
		case SEAMLESS_MODE_MIXED:
		    video_player_set_status_cb(video_player_status_cb);
			ret |= pthread_create(&mixed_id, NULL, mixed_play_main_thread, NULL);
			ret |= pthread_create(&image_slave_id, NULL, image_play_slave_thread, NULL);
			ret |= pthread_create(&preload_id, NULL, media_preload_thread, NULL);
			break;
		case SEAMLESS_MODE_SPLIT:
		case SEAMLESS_MODE_SPLIT_SYNC:
		    // video_player_set_status_cb(video_player_status_cb);
			// ret |= pthread_create(&split_id, NULL, split_play_main_thread, NULL);
			// ret |= pthread_create(&preload_id, NULL, media_preload_thread, NULL);
			// ret |= pthread_create(&image_slave_id, NULL, image_play_slave_thread, NULL);
			// ret |= pthread_create(&video_id, NULL, video_play_main_thread, NULL);
			break;
		default:
			break;
	}
	player_set_status(SEAMLESS_STATUS_PLAY);
	return SEAMLESS_OK;
}

//停止播放
int SeamlessStop(void)
{
	int show_mode = g_seamless->init_para.show_mode;

	player_set_status(SEAMLESS_STATUS_STOP);//停止所有播放动作
	usleep(10000);

	switch(show_mode)
	{
		case SEAMLESS_MODE_VIDEO_ONLY:
			pthread_join(video_id,0);
			break;
		case SEAMLESS_MODE_IMAGE_ONLY:
			pthread_join(image_id, 0);
			pthread_join(preload_id, 0);
			break;
		case SEAMLESS_MODE_MIXED:
		printf("langaojie exit 1\n");
			if(g_seamless->playing.media_type == SEAMLESS_MEDIA_VIDEO)
			{
				wakeup_playing_thread();
			}
			printf("langaojie exit 2\n");
			pthread_join(image_slave_id, 0);
			printf("langaojie exit 3\n");
			pthread_join(preload_id, 0);
			printf("langaojie exit 4\n");
			pthread_join(mixed_id, 0);
			printf("langaojie exit 5\n");
			break;
		case SEAMLESS_MODE_SPLIT:
		case SEAMLESS_MODE_SPLIT_SYNC:
			// pthread_join(g_seamless_ctrl->video_task);
			// pthread_join(g_seamless_ctrl->image_task);
			// pthread_join(g_seamless_ctrl->split_task);
			// pthread_join(g_seamless_ctrl->preload_task);
			break;
		default:
			break;
	}

	sem_destroy(&preload_finish_sem);
	sem_destroy(&play_finish_sem);
	sem_destroy(&play_sync_sem);

	pthread_mutex_destroy(&preload_wait_mutex);
    pthread_cond_destroy(&preload_wait_cond);

	pthread_mutex_destroy(&image_wait_mutex);
    pthread_cond_destroy(&image_wait_cond);

	return SEAMLESS_OK;
}

//退出无缝
int SeamlessExit(void)
{
	player_exit();
	media_unload(g_seamless);
	layer_exit();
    g2d_exit();
	mem_exit();
	free(g_seamless);
	g_seamless = NULL;
	return SEAMLESS_OK;
}
