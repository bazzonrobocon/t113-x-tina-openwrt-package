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

#include <stdlib.h>
#include "tplayer.h"
#include "tplayer_suspend_test.h"
#include "tplayerdemo.h"

/* for keep tplayerdemo simple
 * context of tplayer_suspend_test will be define in static. */

static struct tplayer_suspend_test_context {
	DemoPlayerContext	*player;
	int					player_cnt;
	int					thrd_quit;
	int					running;
	int					sus_cnt;
	pthread_t			sus_thrd;
	pthread_mutex_t		sus_mutex;
}tst_ctx;

static void *suspend_thread(void *param)
{
	/* -check the flag
	 * -loop of pause->suspend->play
	 * -if quit flag --> exit*/
	int ret = 0, try_cnt = 0;
	tst_ctx.running = 1;

	while (1) {
		tp_dbg("step[1]-check flag");
		pthread_mutex_lock(&tst_ctx.sus_mutex);
		if (tst_ctx.thrd_quit == 1) {
			pthread_mutex_unlock(&tst_ctx.sus_mutex);
			break;
		}
		pthread_mutex_unlock(&tst_ctx.sus_mutex);

		tp_dbg("step[2]-pause players");
		for (int i = 0; i < tst_ctx.player_cnt; i++) {
try_pause:
			ret = TPlayerPause(tst_ctx.player[i].mTPlayer);
			if (ret < 0) {
				tp_err("TPlayerPause[%d]failed in suspend, please check\n", i);
//				goto thrd_exit;
				if (try_cnt < 10) {
					usleep(500*1000);
					goto try_pause; //* make sure it will paused. usally happen when player reseting.
				} else
					goto thrd_exit;
			}

			tp_dbg("player[%d]-paused", i);
			try_cnt = 0;
		}

		tp_dbg("step[3]-suspend, commands:\n"
				"%s\n"
				"%s\n",
				TPLAYER_SUSPEND_SET_DELAY,TPLAYER_START_ECHO_SUSPEND);

		system(TPLAYER_SUSPEND_SET_DELAY);
		usleep(100*1000);
		system(TPLAYER_START_ECHO_SUSPEND);

		tp_dbg("step[4]-wake and start players\n");

		for (int i = 0; i < tst_ctx.player_cnt; i++) {
			ret = TPlayerStart(tst_ctx.player[i].mTPlayer);
			if (ret < 0) {
				tp_dbg("TPlayerStart[%d]failed in suspend, please check\n", i);
				//* if start failed, that must be something wrong.
				//* the pause step upon will check the player if reset or not.
				goto thrd_exit;
			}
			tp_info("player[%d]-started", i);
		}

		tst_ctx.sus_cnt++;
		tp_info("suspend for %d times\n", tst_ctx.sus_cnt);

		sleep(TPLAYER_SUSPEND_CYCLE);
	}

thrd_exit:
	tst_ctx.running = 0;
	tp_info("exiting\n");
	return;

}

int tplayer_start_suspend(DemoPlayerContext *player, int cnt)
{
	/* -check context exist or not
	 * -check the flag or status
	 * -init context
	 * -start the thread
	 *
	 * *player: context of players
	 * *cnt: cnt of players*/

	int ret = 0;
	/* check the status */
	if (tst_ctx.running == 1) {
		tp_err("suspending...");
		return -1;
	}

	/* init context */
	tst_ctx.player = player;
	tst_ctx.player_cnt = cnt;
	tst_ctx.thrd_quit = 0;
	tst_ctx.running = 0;
	tst_ctx.sus_cnt = 0;

	ret = pthread_mutex_init(&tst_ctx.sus_mutex, NULL);
	if (ret < 0) {
		tp_err("mutex create failed\n");
		return -1;
	}

	/* start the thread */
	ret = pthread_create(&tst_ctx.sus_thrd, NULL, suspend_thread, NULL);
	if (ret < 0) {
		tp_err("thread create fail\n");
		pthread_mutex_destroy(&tst_ctx.sus_mutex);
		return -1;
	}

	return 0;
}

int tplayer_stop_suspend(DemoPlayerContext *player, int cnt)
{
	/* -check the status or flag
	 * -stop the thread*/
	int ret = 0;
	if (tst_ctx.running == 0) {
		tp_err("suspend not running...\n");
		return -1;
	}
	pthread_mutex_lock(&tst_ctx.sus_mutex);
	tst_ctx.thrd_quit = 1;
	pthread_mutex_unlock(&tst_ctx.sus_mutex);
	pthread_join(tst_ctx.sus_thrd, &ret);
	pthread_mutex_destroy(&tst_ctx.sus_mutex);
	return 0;
}
