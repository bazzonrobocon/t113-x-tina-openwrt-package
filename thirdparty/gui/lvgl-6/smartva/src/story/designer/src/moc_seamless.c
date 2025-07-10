/**********************
 *      includes
 **********************/
#include "moc_seamless.h"
#include "ui_seamless.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "seamless.h"

/**********************
 *       variables
 **********************/
typedef struct
{
	seamless_ui_t ui;
} seamless_para_t;
static seamless_para_t para;


/**********************
 *  functions
 **********************/
static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		destory_page(PAGE_SEAMLESS);
		create_page(PAGE_HOME);
	}
}

static void* seamless_self_test_thread(void *arg)
{
	seamless_init_para_t para;
	memset(&para,0,sizeof(seamless_init_para_t));
	para.show_mode          = SEAMLESS_MODE_MIXED;  //同步模式暂未支持
	para.screen_rotate      = SEAMLESS_ROTATE_0;
	para.screen_size.width  = 800;
	para.screen_size.height = 480;
	para.source_path        = "/mnt/exUDISK/";
	para.play_order         = 0;
	para.image_rect.x       = 40;                    //图片播放逻辑显示区域，以旋转后左上角为原点。有旋转请确认本参数正确
	para.image_rect.y       = 40;
	para.image_rect.width   = 800;
	para.image_rect.height  = 480;
	para.video_rect.x       = 40;					//视频播放逻辑显示区域，以旋转后左上角为原点。有旋转请确认本参数正确
	para.video_rect.y       = 40;
	para.video_rect.width   = 800;
	para.video_rect.height  = 480;

	while(1)
	{
		SeamlessInit(&para);
		SeamlessStart();
		sleep(5);
		SeamlessStop();
		SeamlessExit();
		system("echo 3 > /proc/sys/vm/drop_caches;free");
		sleep(1);
	}
	return NULL;
}
static int create_seamless(void)
{
	para.ui.cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(para.ui.cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color = LV_COLOR_BLUE;
	cont_style.body.grad_color = LV_COLOR_BLUE;
	lv_cont_set_style(para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_cont_set_layout(para.ui.cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(para.ui.cont, LV_FIT_NONE);

	static lv_style_t back_btn_style;
	lv_style_copy(&back_btn_style, &lv_style_pretty);
	back_btn_style.text.font = &lv_font_roboto_28;
	lv_obj_t *back_btn = lv_btn_create(para.ui.cont, NULL);
	lv_obj_align(back_btn, para.ui.cont, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_t *back_lable = lv_label_create(back_btn, NULL);
	lv_label_set_text(back_lable, LV_SYMBOL_LEFT);
	lv_obj_set_event_cb(back_btn, back_btn_event);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_REL, &back_btn_style);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_PR, &back_btn_style);

	lv_obj_t *scn = lv_scr_act();					// clear screen for alpha setting
	static lv_style_t scn_style;
	lv_style_copy(&scn_style, &lv_style_plain);
	scn_style.body.main_color.full = 0x00000000;
	scn_style.body.grad_color.full = 0x00000000;
	lv_obj_set_style(scn, &scn_style);

	cont_style.body.main_color = LV_COLOR_WHITE;
	cont_style.body.grad_color = LV_COLOR_WHITE;
//	cont_style.body.main_color.full = 0x00000000;	// clear container for alpha setting
//	cont_style.body.grad_color.full = 0x00000000;
	cont_style.body.opa = 0;
	lv_cont_set_style(para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);


	seamless_auto_ui_create(&para.ui);


	seamless_init_para_t para;
	memset(&para,0,sizeof(seamless_init_para_t));
	para.show_mode          = SEAMLESS_MODE_MIXED;  //同步模式暂未支持
	para.screen_rotate      = SEAMLESS_ROTATE_0;
	para.screen_size.width  = 800;
	para.screen_size.height = 480;
	para.source_path        = "/mnt/exUDISK/";
	para.play_order         = 0;
	para.image_rect.x       = 0;                    //图片播放逻辑显示区域，以旋转后左上角为原点。有旋转请确认本参数正确
	para.image_rect.y       = 0;
	para.image_rect.width   = 800;
	para.image_rect.height  = 480;
	para.video_rect.x       = 0;					//视频播放逻辑显示区域，以旋转后左上角为原点。有旋转请确认本参数正确
	para.video_rect.y       = 0;
	para.video_rect.width   = 800;
	para.video_rect.height  = 480;
	SeamlessInit(&para);
	SeamlessStart();
	// static pthread_t  video_id;
	// pthread_create(&video_id, NULL, seamless_self_test_thread, NULL);
	return 0;
}

static int destory_seamless(void)
{
	SeamlessStop();
	SeamlessExit();
	seamless_auto_ui_destory(&para.ui);
	lv_obj_del(para.ui.cont);

	return 0;
}

static int show_seamless(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);

	return 0;
}

static int hide_seamless(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static page_interface_t page_seamless =
{
	.ops =
	{
		create_seamless,
		destory_seamless,
		show_seamless,
		hide_seamless,
	},
	.info =
	{
		.id         = PAGE_SEAMLESS,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_SEAMLESS(void)
{
	reg_page(&page_seamless);
}
