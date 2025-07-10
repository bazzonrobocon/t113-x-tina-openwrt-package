/*
 * =====================================================================================
 *
 *       Filename:  media_search.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年02月25日 15时58分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef __MEDIA_SEARCH_H__
#define __MEDIA_SEARCH_H__

int media_probe(seamless_ctrl_t *h_seamless);
int media_preload(seamless_ctrl_t *h_seamless);
int media_unload(seamless_ctrl_t *h_seamless);

#endif /*endif __MEDIA_SEARCH_H__*/
