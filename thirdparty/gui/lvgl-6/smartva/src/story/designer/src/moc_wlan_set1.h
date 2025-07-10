#ifndef __MOC_WLAN_SET1_H__
#define __MOC_WLAN_SET1_H__
#include "lvgl.h"
#define RSSI_NO_SINGAL -101
void *get_img_by_level(int level);
void hbar_wifi_signal_update(lv_obj_t *img_hbar_wifi, bool first);
void wifi_pic_res_init(void);
void wifi_pic_res_uninit(void);
#endif /*__MOC_WLAN_SET1_H__*/
