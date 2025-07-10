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
#ifndef __TPLAYER_POWER_MONITOR_H__
#define __TPLAYER_POWER_MONITOR_H__

#define PKM_DEBUG 0 //not enble debug log defaultly

#define pkm_err(fmt, args...) printf("[%s, %d]error: "fmt"\r\n", __func__, __LINE__, ##args)
#if PKM_DEBUG
#define pkm_dbg(fmt, args...) printf("[%s, %d]debug "fmt"\r\n", __func__, __LINE__, ##args)
#else
#define pkm_dbg(fmt, args...)
#endif
#define pkm_vbs(fmt, args...)

#define POWER_KEY_TIMEOUT 3000

typedef int (*PowerKey2TPlayerCbk)(void *pUser, int msg, int ext1, void *param);

typedef struct tplayer_monitor {
	int					state;
	int					bPlaying;
	int					bDone;
	PowerKey2TPlayerCbk	callback;
	pthread_t			thread_key_power;
	pthread_mutex_t		state_lock;
	void				*pUserData;
} tplayer_monitor;

enum tplayer_monitor_state {
	MONITOR_STATE_STOP = -1,
	MONITOR_STATE_IDLE = 0,
	MONITOR_STATE_START,
};

typedef enum tplayer_monitor_cbk_type {
	TPLAYER_MONITOR_CALL_POWERKEY = 0,
}tplayer_monitor_cbk_type;

tplayer_monitor *tplayer_start_monitor_powerkey(void *pUserData, PowerKey2TPlayerCbk callback);
void tplayer_stop_powerkey(tplayer_monitor *p);

void send_done_2_pkm(void *ptr);
#endif
