/**
 * @file: sunxi_ctpdev.c
 * @autor: huangyixiu
 * @url: huangyixiu@allwinnertech.com
 */
/*********************
 *      INCLUDES
 *********************/
#ifdef __CTP_USE__
#include "sunxi_ctpdev.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>

/*********************
 *      DEFINES
 *********************/
#define CTP_DEV_POLL	0		/* simulation */
#define CTP_DEV_SYNC	1		/* sync , else async*/
#define INPUTDEV_MAX    8 
#define INPUTDEV_NAME "gt9xxnew_ts"
int ctpdev_fd;
int ctpdev_root_x;
int ctpdev_root_y;
int ctpdev_button;
pthread_t ctpdev_pthread_id;
pthread_mutex_t ctpdev_mutex;
extern int evdev_get_event_index(const char* name);

void *ctpdev_thread(void * data)
{
#if CTP_DEV_POLL
(void)data;
    while(1)
	{
		struct input_event in;
		//printf("###L:%d, F=%s\n", __LINE__, __FUNCTION__);
	    while(read(ctpdev_fd, &in, sizeof(struct input_event)) > 0)
		{
			//printf("L:%d, F=%s, in:(%d, %d, %d)\n", __LINE__, __FUNCTION__, in.type, in.code, in.value);
			if(in.type==EV_SYN && in.code==SYN_REPORT && in.value==0)
			{
				break;
			}

			if(in.type == EV_ABS)
			{
				pthread_mutex_lock(&ctpdev_mutex);
				if(in.code == ABS_MT_POSITION_X)
					ctpdev_root_x = in.value;
	            else if(in.code == ABS_MT_POSITION_Y)
					ctpdev_root_y = in.value;
				else if(in.code == ABS_PRESSURE)
				{
					if(in.value == 0)
					{
						ctpdev_button = LV_INDEV_STATE_REL;
					}
					else
					{
						ctpdev_button = LV_INDEV_STATE_PR;
					}
				}
				pthread_mutex_unlock(&ctpdev_mutex);
	        }

	    }
		usleep(15000);
		//printf("piont:%d, %d, state=%d\n", ctpdev_root_x, ctpdev_root_y, ctpdev_button);
    }
#elif CTP_DEV_SYNC
	(void)data;
	int ret;
	fd_set readfd;
	struct timeval timeout;
	struct input_event in;

	while(1)
	{
	      timeout.tv_sec=5;
	      timeout.tv_usec=0;

	      FD_ZERO(&readfd);
	      FD_SET(ctpdev_fd,&readfd);
	      ret=select(ctpdev_fd+1,&readfd,NULL,NULL,&timeout);
		  if (ret > 0)
	      {
	          if(FD_ISSET(ctpdev_fd,&readfd))
	          {
				  FD_CLR(ctpdev_fd, &readfd);
	              read(ctpdev_fd, &in, sizeof(in));
				  //printf("input info:(%d, %d, %d)\n", in.code, in.type, in.value);
				  pthread_mutex_lock(&ctpdev_mutex);
				  if(in.type == EV_ABS) {
						if(in.code == ABS_MT_POSITION_X)
							ctpdev_root_x = in.value;
			            else if(in.code == ABS_MT_POSITION_Y)
							ctpdev_root_y = in.value;
						else if(in.code == ABS_PRESSURE)
						{
							if(in.value == 0) {
								ctpdev_button = LV_INDEV_STATE_REL;
							}
							else {
								ctpdev_button = LV_INDEV_STATE_PR;
							}
						}
			      }
				  else if (in.type == EV_KEY) {
						if(in.value == 0) {
							ctpdev_button = LV_INDEV_STATE_REL;
						}
						else {
							ctpdev_button = LV_INDEV_STATE_PR;
						}
				  }
				  pthread_mutex_unlock(&ctpdev_mutex);
		     }
	      }

		  //printf("piont:%d, %d, state=%d\n", ctpdev_root_x, ctpdev_root_y, ctpdev_button);
	}

#else

    (void)data;
    while(1) {
		static int nCnt = 0;

		nCnt++;
		//printf("#####L:%d, F=%s, t=%d\n", __LINE__, __FUNCTION__, lv_tick_get());
		if(nCnt<10)
		{
			pthread_mutex_lock(&ctpdev_mutex);
			ctpdev_root_x = 700;
			ctpdev_root_y = 340;
			ctpdev_button = 0;
			pthread_mutex_unlock(&ctpdev_mutex);
		}
		else if (nCnt == 10)
		{
			//printf("L:%d, F=%s, t=%d\n", __LINE__, __FUNCTION__, lv_tick_get());
		}
		else
		{
			pthread_mutex_lock(&ctpdev_mutex);
			ctpdev_root_x -= 10;
			if(ctpdev_root_x <= 100)
			{
				//printf("L:%d, F=%s, t=%d\n", __LINE__, __FUNCTION__, lv_tick_get());
				nCnt = 0;
			}

			ctpdev_root_y = 340;
			ctpdev_button = 1;
			pthread_mutex_unlock(&ctpdev_mutex);
		}

		usleep(10000);   /*Sleep for 10 millisecond*/
    }
#endif
}

int ctpdev_init(void)
{
	pthread_attr_t attr;
	int index = 0;
	char dev[64];
	ctpdev_fd = -1;
	index = evdev_get_event_index(INPUTDEV_NAME);
	if(index >= 0)
	{
		sprintf(dev, "/dev/input/event%d", index);
		printf("evdev is %s\n", dev);
    	ctpdev_fd = open(dev,  O_RDONLY | O_NONBLOCK);
	}else
	{
		return -1;
	}

    if(ctpdev_fd == -1) {
        perror("unable open evdev interface\n");
        return -1;
    }

    ctpdev_root_x = 0;
    ctpdev_root_y = 0;
    ctpdev_button = LV_INDEV_STATE_REL;
	pthread_mutex_init (&ctpdev_mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x4000);

	int ret = pthread_create(&ctpdev_pthread_id, &attr, ctpdev_thread, NULL);
	pthread_attr_destroy(&attr);
	if (ret == -1)
	{
		printf("create thread fail\n");
		return -1;
	}
	return 0;
	//printf("ctpdev_pthread_id=%d\n", ctpdev_pthread_id);
}

/**
 * uninitialize the ctpdev interface
 */
void ctpdev_uninit(void)
{
	pthread_join(ctpdev_pthread_id, NULL);
	pthread_mutex_destroy(&ctpdev_mutex);

	ctpdev_root_x = 0;
    ctpdev_root_y = 0;
    ctpdev_button = LV_INDEV_STATE_REL;

	if (ctpdev_fd != -1)
	{
		close(ctpdev_fd);
	}
}

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool ctpdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
	//printf("#####L:%d, F=%s, t=%d\n", __LINE__, __FUNCTION__, lv_tick_get());

	pthread_mutex_lock(&ctpdev_mutex);
    data->point.x = ctpdev_root_x;
    data->point.y = ctpdev_root_y;
    data->state = ctpdev_button;
	pthread_mutex_unlock(&ctpdev_mutex);
	return false;
}

#endif
