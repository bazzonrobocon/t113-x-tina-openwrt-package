/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <pthread.h>
#include <linux/input.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "power_monitor.h"

#define DEV_INPUT_KEY_PATH "/dev/input/event*"


static pthread_t thread_key_power;

static int open_input(int *fd_array, int *num)
{
	char *filename = NULL;
	glob_t globbuf;
	unsigned i;
	int success = 0;
	int fd_num = 0;

	if (!fd_array || !num)
		return -1;
	/* get all the matching event filenames */
	glob(DEV_INPUT_KEY_PATH, 0, NULL, &globbuf);

	/* for each event file */
	if (globbuf.gl_pathc > *num)
		return -1;
	for (i = 0; i < globbuf.gl_pathc; ++i) {
		filename = globbuf.gl_pathv[i];

		/* open this input layer device file */
		fd_array[fd_num] = open(filename, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (fd_array[fd_num] >= 0) {
			fd_num++;
			success = 1;
		}
	}
	*num = fd_num;
	globfree(&globbuf);
	if (!success)
		return -1;

	return 0;
}

/*==========================================================
 * Function: scan_powerkey
 * Descriptions:
 * 	thread of scaning the power key
 * =========================================================*/
static void *scan_powerkey(void *param)
{
	tplayer_monitor *p = (tplayer_monitor *)param;

	struct epoll_event ev;
	int ret, epollfd, i, input_event_count = 128;
	int input_event_fd[input_event_count];
	struct epoll_event events[input_event_count];
	struct input_event key_event;
	int nevents, timeout = POWER_KEY_TIMEOUT;
	int sysret;

	/* init epoll */
	epollfd = epoll_create(40);
	if (epollfd == -1) {
		pkm_err("epoll create failed");
		goto thrd_exit;
	}
	open_input(input_event_fd, &input_event_count);
	for (i = 0; i < input_event_count; i++) {
		ev.data.fd = input_event_fd[i];
		ev.events = EPOLLIN | EPOLLWAKEUP;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, input_event_fd[i], &ev)) {
			pkm_err("epoll_ctl failed.");
			return NULL;
		}
	}
	p->bDone = 1;

	/* scan loop */
	while (1) {
		pthread_mutex_lock(&p->state_lock);
		if (p->state == MONITOR_STATE_STOP)
			goto thrd_exit;
		pthread_mutex_unlock(&p->state_lock);

		memset(&events, 0, sizeof(events));
		pkm_vbs("waiting for event");
//		if (!p->bDone)
//			continue;

		nevents = epoll_wait(epollfd, events, input_event_count, timeout);
		if (nevents == -1) {
			if (errno == EINTR)
				continue;
			pkm_err("epoll_wait failed");
			break;
		}
		pkm_vbs("get event");

		/* judge the event */
		for (i = 0; i < nevents; i++) {
			if (read(events[i].data.fd, &key_event, sizeof(struct input_event))== sizeof(struct input_event)) {
				if (key_event.type == EV_KEY && key_event.code == KEY_POWER) {
					pkm_dbg("--------KEY_POWER, value = %d --------------", key_event.value);
					switch (key_event.value) {
					case 1: //press the power key
						if (!p->bDone) {
							pkm_dbg("last call still processing...");
							break;
						}
						p->bDone = 0;
						p->callback(p->pUserData, TPLAYER_MONITOR_CALL_POWERKEY, 0, NULL);
						pkm_dbg("----------send callback msg done");
						break;
					case 0: //release the power key
					case 2: //hold pressing on the power key
					default:
						break;
					} //* switch end.
				}
			}
		}


		usleep(500);
	} /* scan loop end */

thrd_exit:
	pkm_dbg("thread exit...");
	return NULL;

}

void send_done_2_pkm(void *ptr)
{
	tplayer_monitor *p = (tplayer_monitor *)ptr;
	p->bDone=1;
}

tplayer_monitor *tplayer_start_monitor_powerkey(void *pUserData, PowerKey2TPlayerCbk callback)
{
	tplayer_monitor *p = (tplayer_monitor *)malloc(sizeof(tplayer_monitor));
	int ret = 0;
	pkm_dbg("creating powerkey monitor context!");

	if (!p) {
		pkm_err("tplayer monitor create failed");
		return NULL;
	}

	memset(p, 0x00, sizeof(tplayer_monitor));

	p->state = MONITOR_STATE_START;

	ret = pthread_mutex_init(&p->state_lock, NULL);
	if (ret < 0) {
		pkm_err("mutex init failed\n");
		free(p);
		p = NULL;
		return NULL;
	}
	ret = pthread_create(&p->thread_key_power, NULL, scan_powerkey, p);
	if (ret < 0) {
		pkm_err("create thread of scan_powerkey failed");
		free(p);
		p = NULL;
		return NULL;
	}

	p->callback = callback;
	p->pUserData = pUserData;

	pkm_dbg("create done!");
	return p;
}

void tplayer_stop_powerkey(tplayer_monitor *p)
{
	int ret = 0;
	pkm_dbg("stop powerkey scanning");
	if (p != NULL) {
		pthread_mutex_lock(&p->state_lock);
		p->state = MONITOR_STATE_STOP;
		pthread_mutex_unlock(&p->state_lock);
		pkm_dbg("waiting for thread exit");
		pthread_join(p->thread_key_power, &ret);
		pthread_mutex_destroy(&p->state_lock);
		free(p);

	}
	pkm_dbg("stop powerkey scanning done!");

	return;
}
