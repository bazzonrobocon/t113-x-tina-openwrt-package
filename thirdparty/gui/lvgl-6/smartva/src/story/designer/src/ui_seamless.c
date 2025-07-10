/**********************
 *      includes
 **********************/
#include "ui_seamless.h"
#include "lvgl.h"
#include "common.h"
#include "ui_resource.h"


/**********************
 *       variables
 **********************/

/**********************
 *  images and fonts
 **********************/

/**********************
 *  functions
 **********************/
void seamless_auto_ui_create(seamless_ui_t *ui)
{

}

void seamless_auto_ui_destory(seamless_ui_t *ui)
{
	lv_obj_clean(ui->cont);
}
