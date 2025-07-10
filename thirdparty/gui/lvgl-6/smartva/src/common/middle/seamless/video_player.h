/*
 * =====================================================================================
 *
 *       Filename:  video_player.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年02月25日 15时54分52秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef __VIDEO_PLAYER_H__
#define __VIDEO_PLAYER_H__

typedef enum{
    VIDEO_STATUS_STOP,
    VIDEO_STATUS_PLAY,
    VIDEO_STATUS_OTHER,
}video_player_status_e;

int video_player_init(void);
int video_player_exit(void);
int video_player_get_status(void);
int video_player_start(char *filename);
int video_player_stop(void);
int video_player_set_win_rect(seamless_rect_t *rect);
int video_player_set_win_zoom(void);
int video_set_rotate(int rotate);//seamless_rotate_e
int video_player_set_status_cb(void *cb);

#endif /*__VIDEO_PLAYER_H__*/
