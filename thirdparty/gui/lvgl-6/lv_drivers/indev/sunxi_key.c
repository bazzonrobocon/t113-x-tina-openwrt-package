/**
 * @file: sunxi_key.c
 * @autor: huangyixiu
 * @url: huangyixiu@allwinnertech.com
 */
/*********************
 *      INCLUDES
 *********************/
#include "sunxi_key.h"
//#include "input-event-codes.h"
#if USE_SUNXI_KEY != 0

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#define INPUTDEV_MAX    8
#define GPADC_KEY_NAME	"sunxi-gpadc0"
#define IR_NAME 		"sunxi-ir"
p_keydev_callback keydev_callback = NULL;
/*********************
 *      DEFINES
 *********************/
#ifdef USE_SUNXI_ADC_KEY
int adckey_dev_fd = -1;
#endif
#ifdef USE_SUNXI_IR_KEY
int irkey_dev_fd = -1;
#endif
pthread_mutex_t keydev_mutex;
pthread_t keydev_pthread_id;

uint32_t last_key;
lv_indev_state_t last_key_state = LV_INDEV_STATE_REL;

uint32_t key_switch(uint32_t in);
extern int evdev_get_event_index(const char* name);
typedef struct sunxi_key_manager_tag {
	int key_dev_fd;
	char dev_name[64];
} sunxi_key_manager_t;
static sunxi_key_manager_t sunxi_key_manager[]  = {
	{0, GPADC_KEY_NAME},
	{0, IR_NAME},
};
void *keydev_thread(void * data)
{
	(void)data;
	int ret, index = 0;
	fd_set readfd;
	int max_fd = -1;
	struct timeval timeout;
	struct input_event in;
	static int press_cnt;

	for (index = 0; index < sizeof(sunxi_key_manager)/sizeof(sunxi_key_manager[0]); index++) {
		if (sunxi_key_manager[index].key_dev_fd != 0) {
			if (max_fd < sunxi_key_manager[index].key_dev_fd) {
				max_fd = sunxi_key_manager[index].key_dev_fd;
			}
		}
	}

	while(1)
	{
		timeout.tv_sec=5;
	    timeout.tv_usec=0;

	    FD_ZERO(&readfd);
		for (index = 0; index < sizeof(sunxi_key_manager)/sizeof(sunxi_key_manager[0]); index++) {
			if (sunxi_key_manager[index].key_dev_fd != 0) {
			    FD_SET(sunxi_key_manager[index].key_dev_fd, &readfd);
			}
		}

	    ret = select(max_fd+1, &readfd, NULL, NULL, &timeout);
		if (ret > 0)
	    {
			for (index = 0; index < sizeof(sunxi_key_manager)/sizeof(sunxi_key_manager[0]); index++) {
				if (FD_ISSET(sunxi_key_manager[index].key_dev_fd, &readfd)) {
					FD_CLR(sunxi_key_manager[index].key_dev_fd, &readfd);
					read(sunxi_key_manager[index].key_dev_fd, &in, sizeof(in));
					pthread_mutex_lock(&keydev_mutex);
					if(in.type == 1) {
						last_key = in.code;
						if (in.value == 0) {
							last_key_state = LV_INDEV_STATE_REL;
						}
						else {
							last_key_state = LV_INDEV_STATE_PR;
						}
						if (keydev_callback) {
							if (last_key_state) {
								press_cnt++;
								if (press_cnt++ > 40) {
									last_key_state = LV_INDEV_STATE_LONG_PR;
								}
							} else {
								press_cnt = 0;
							}
							keydev_callback(key_switch(last_key), last_key_state);
						}
					}
					pthread_mutex_unlock(&keydev_mutex);
				}
			}
		}
	}
}

int keydev_init(void)
{
	int ret = 0, index = 0;
	pthread_attr_t attr;

	for (int i = 0; i < sizeof(sunxi_key_manager)/sizeof(sunxi_key_manager[0]); i++) {
		char input_name[64];
		index = evdev_get_event_index(sunxi_key_manager[i].dev_name);
		sprintf(input_name, "/dev/input/event%d", index);
		sunxi_key_manager[i].key_dev_fd = open(input_name,  O_RDONLY | O_NONBLOCK);
	    if (sunxi_key_manager[i].key_dev_fd == -1) {
			printf("open fail:%s\n", sunxi_key_manager[i].dev_name);
	    }
	}
	for (int i= 0; i < sizeof(sunxi_key_manager)/sizeof(sunxi_key_manager[0]); i++) {
		if (sunxi_key_manager[i].key_dev_fd != -1) {
			ret |= 1;
		}
	}
	if (!ret) {
		perror("sunxi_key unable open evdev interface:");
		return -1;
	}
	last_key = 0;
	last_key_state = LV_INDEV_STATE_REL;
	pthread_mutex_init (&keydev_mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x4000);

	ret = pthread_create(&keydev_pthread_id, &attr, keydev_thread, NULL);
	pthread_attr_destroy(&attr);
	if (ret == -1) {
		printf("create thread fail\n");
		return -1;
	}
	return 0;
}

void keydev_uninit(void)
{
	int index = 0;

	pthread_join(keydev_pthread_id, NULL);
	pthread_mutex_destroy(&keydev_mutex);
	last_key = 0;
	last_key_state = LV_INDEV_STATE_REL;

	for (index = 0; index < sizeof(sunxi_key_manager)/sizeof(sunxi_key_manager[0]); index++) {
		if (sunxi_key_manager[index].key_dev_fd != -1) {
			close(sunxi_key_manager[index].key_dev_fd);
			sunxi_key_manager[index].key_dev_fd = 0;
		}
	}
}

uint32_t key_switch(uint32_t in)
{
	uint32_t out = 0;

	switch(in)
	{
		case KEY_ENTER:	/* enter */
			out = LV_KEY_ENTER;
			break;
		case KEY_BACK: /* BACK */
			//out = LV_KEY_BACKSPACE;
			out = LV_KEY_RETURN;
			break;

		case KEY_NEXT:	/* NEXT */
			out = LV_KEY_NEXT;
			break;
		case KEY_PREVIOUS:	/* PREV */
			out = LV_KEY_PREV;
			break;

		case KEY_VOLUMEDOWN:	/* VOL- */
			out = LV_KEY_VOLUME_DOWN;
			break;
		case KEY_VOLUMEUP:	/* VOL+ */
			out = LV_KEY_VOLUME_UP;
			break;

		case KEY_LEFT:
			out = LV_KEY_LEFT;
			break;
		case KEY_RIGHT:
			out = LV_KEY_RIGHT;
			break;

		case KEY_UP:
			out = LV_KEY_UP;
			break;
		case KEY_DOWN:
			out = LV_KEY_DOWN;
			break;
		case KEY_POWER:
			out = LV_KEY_POWER_OFF;
			break;
		default:
			break;
	}

	return out;
}


bool keydev_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/

	pthread_mutex_lock(&keydev_mutex);
    data->state = last_key_state;
    data->key = key_switch(last_key);
	// printf("key=%d, state=%d\n", data->key, data->state);
	pthread_mutex_unlock(&keydev_mutex);
    return false;
}

void keydev_register_hook(p_keydev_callback func)
{
	keydev_callback = func;
}


int evdev_get_event_index(const char* name)
{
	FILE *in;
	int index = -1;
	char inputdev_name[64];
	for(int i = 0; i < INPUTDEV_MAX; i++){
		char value[64];
		sprintf(inputdev_name, "/sys/class/input/event%d/device/name",i);
		if (access(inputdev_name, F_OK) != 0) continue;
		in = fopen(inputdev_name, "rb");
		if (!in){
			perror("open input error\n");
			continue;
		}
		if ((fscanf(in, "%63s", value) == 1) &&
					(strcmp(name, value) == 0)) {
				printf("### name = %s index = %d\n", name, i);
				index = i;
				fclose(in);
				break;
		}
		fclose(in);
	}
	return index;
}


#endif
