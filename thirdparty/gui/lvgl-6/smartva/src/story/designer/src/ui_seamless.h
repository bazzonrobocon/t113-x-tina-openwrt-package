#ifndef __UI_SEAMLESS_H__
#define __UI_SEAMLESS_H__

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *      includes
 **********************/
#include "lvgl.h"


/**********************
 *       variables
 **********************/
typedef struct
{
	uint8_t id;
	lv_obj_t *cont;
} seamless_ui_t;


/**********************
 * functions
 **********************/
void seamless_auto_ui_create(seamless_ui_t *ui);
void seamless_auto_ui_destory(seamless_ui_t *ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__UI_SEAMLESS_H__*/
