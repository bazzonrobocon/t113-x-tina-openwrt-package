/**********************
 *      includes
 **********************/
#include "moc_wlan_set1.h"
#include "ui_wlan_set1.h"
#include "moc_home.h"
#include "page.h"
#include "ui_resource.h"
#include "bs_widget.h"
#include "app_config_interface.h"

static int s_wifi_off;
/**********************
 *       variables
 **********************/
typedef struct
{
	//lv_task_t *wifi_info_update_tid;
	lv_task_t *list_update_tid;

	pthread_t wifi_scan_tid;
	int scan_quit_flag;
	pthread_mutex_t scan_mutex;

	wifi_scan_result_t * scan_results;
	int scan_valid_len;
//	net_wifi_scan_info_t p_scan_info[WIFI_MAX_SCAN_NUM];
	int scan_num;
	wifi_sta_info_t sta_info;

	lv_obj_t *on_off_sw;
	lv_obj_t *wlan_kb;

	lv_obj_t* mbox_wlan_password_err;
	lv_obj_t* mbox_wlan_conecting;
	lv_obj_t* mbox_wlan_connectok;
	lv_obj_t* mbox_wlan_disconnectok;
	lv_obj_t* mbox_wlan_disconnecterr;

	char tmp_ssid[WIFI_MAX_SSID_SIZE];		/* ssid */
	char tmp_password[WIFI_MAX_PASSWORD_SIZE];		/* password */
} wlan_set1_moc_t;

typedef struct
{
	wlan_set1_ui_t ui;
	wlan_set1_moc_t moc;
} wlan_set1_para_t;
static wlan_set1_para_t para;

static lv_style_t style0_switch_4;
static lv_style_t style1_switch_4;
static lv_style_t style2_switch_4;
static lv_style_t style3_switch_4;
static lv_style_t style0_button_3;

typedef enum {
	WIFI_CYCLE_IMAGE = 0,
	WIFI_NO_SIGNAL_IMAGE,
	WIFI_NEXT_IMAGE,
	WIFI_LEVEL1_IMAGE,
	WIFI_LEVEL2_IMAGE,
	WIFI_LEVEL3_IMAGE,
	WIFI_LEVEL4_IMAGE,
	WIFI_IMAGE_NUM
} wifi_image_t;

typedef enum {
	MBOX_WAIT_CONNECT = 0,
	MBOX_PASSWORD_ERR,
	MBOX_CONNECT_OK,
	MBOX_DISCONNECT_OK,
	MBOX_DISCONNECT_ERR,
	MBOX_CMD_NUM
} mbox_cmd_t;

static ui_image_t wifi_image_list[WIFI_IMAGE_NUM] = {
	{NULL, LV_IMAGE_PATH"wifi_cycle.png"},
	{NULL, LV_IMAGE_PATH"wifi_no_connect.png"},
	{NULL, LV_IMAGE_PATH"wifi_next.png"},
	{NULL, LV_IMAGE_PATH"wifi_level1.png"},
	{NULL, LV_IMAGE_PATH"wifi_level2.png"},
	{NULL, LV_IMAGE_PATH"wifi_level3.png"},
	{NULL, LV_IMAGE_PATH"wifi_level4.png"}
};




/**********************
 *  functions
 **********************/
static void update_list_context(wlan_set1_para_t *para);

static void hidden_obj_and_all_child(lv_obj_t *obj, bool en)
{
	#if 0
	lv_obj_set_hidden(para.ui.keyboard_1, en);
	lv_obj_set_hidden(para.ui.label_2, en);
	lv_obj_set_hidden(para.ui.text_area_1, en);
	lv_obj_set_hidden(para.ui.label_3, en);
	lv_obj_set_hidden(para.ui.button_1, en);
	lv_obj_set_hidden(para.ui.label_4, en);
	lv_obj_set_hidden(para.ui.button_2, en);
	lv_obj_set_hidden(para.ui.container_1, en);
	#endif
	#if 1
    lv_obj_t * child = lv_obj_get_child(obj, NULL);
    lv_obj_t * child_next;
    while(child) {
		lv_obj_set_hidden(child, en);
        child_next = lv_obj_get_child(obj, child);
        child = child_next;
    }
	lv_obj_set_hidden(obj, en);
	#endif
}

static bool __is_in_obj(lv_obj_t * obj, __u32 x, __u32 y)
{
	lv_area_t  obj_area;
	lv_obj_get_coords(obj, &obj_area);

	if (x > obj_area.x2 || x < obj_area.x1 || y > obj_area.y2 || y < obj_area.y1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void *get_img_by_level(int level)
{
	if (level >= -25) {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_LEVEL4_IMAGE);
	} else if (level >= -50) {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_LEVEL3_IMAGE);
	} else if (level >= -75) {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_LEVEL2_IMAGE);
	} else if (level == RSSI_NO_SINGAL) {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_NO_SIGNAL_IMAGE);
	} else {
		return get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_LEVEL1_IMAGE);
	}
}

static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_WLAN_SET1);
		//create_page(PAGE_HOME);
		switch_page(PAGE_WLAN_SET1, PAGE_HOME);
	}
}

static void btn_hbar_return_event_cb(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_WLAN_SET1);
		//create_page(PAGE_HOME);
		switch_page(PAGE_WLAN_SET1, PAGE_HOME);
	}
}

static lv_obj_t * on_off_sw_create(lv_obj_t * par)
{
	lv_obj_t *switch_4;

	lv_style_copy(&style0_switch_4, &lv_style_pretty);
	style0_switch_4.body.main_color = lv_color_hex(0xdfdfdf);
	style0_switch_4.body.grad_color = lv_color_hex(0xdfdfdf);
	style0_switch_4.body.radius = 20;
	style0_switch_4.body.border.width = 0;
	style0_switch_4.body.border.opa = 0;
	style0_switch_4.body.padding.top = -5;
	style0_switch_4.body.padding.bottom = -5;
	style0_switch_4.body.padding.left = -5;
	style0_switch_4.body.padding.right = -5;
	style0_switch_4.body.padding.inner = -5;

	lv_style_copy(&style1_switch_4, &lv_style_pretty_color);
	style1_switch_4.body.main_color = lv_color_hex(0x0055ff);
	style1_switch_4.body.grad_color = lv_color_hex(0x55ffff);
	style1_switch_4.body.radius = 20;
	style1_switch_4.body.padding.top = 0;
	style1_switch_4.body.padding.bottom = 0;
	style1_switch_4.body.padding.left = 0;
	style1_switch_4.body.padding.right = 0;
	style1_switch_4.body.padding.inner = 0;

	lv_style_copy(&style2_switch_4, &lv_style_pretty);
	style2_switch_4.body.grad_color = lv_color_hex(0xffffff);
	style2_switch_4.body.border.width = 0;
	style2_switch_4.body.border.opa = 0;
	style2_switch_4.body.padding.top = 0;
	style2_switch_4.body.padding.bottom = 0;
	style2_switch_4.body.padding.left = 0;
	style2_switch_4.body.padding.right = 0;
	style2_switch_4.body.padding.inner = 0;

	lv_style_copy(&style3_switch_4, &lv_style_pretty);
	style3_switch_4.body.grad_color = lv_color_hex(0xffffff);

	switch_4 = lv_sw_create(par, NULL);
	lv_obj_set_pos(switch_4, 800-33-30, 0);
	lv_obj_set_size(switch_4, 33, 14);
	lv_sw_on(switch_4, LV_ANIM_OFF);
	lv_sw_set_style(switch_4, LV_SW_STYLE_BG, &style0_switch_4);
	lv_sw_set_style(switch_4, LV_SW_STYLE_INDIC, &style1_switch_4);
	lv_sw_set_style(switch_4, LV_SW_STYLE_KNOB_OFF, &style2_switch_4);
	lv_sw_set_style(switch_4, LV_SW_STYLE_KNOB_ON, &style3_switch_4);

	return switch_4;
}

static lv_obj_t* wlan_mbox_create(const char* message, lv_event_cb_t event_cb)
{
#if CONFIG_FONT_ENABLE
    static lv_style_t mbox_style;
#endif
	lv_obj_t* mbox = NULL;
	// lv_obj_set_click(lv_layer_top(), true);
	mbox = (lv_obj_t*)lv_mbox_create(lv_layer_top(), NULL);
#if CONFIG_FONT_ENABLE
	lv_style_copy(&mbox_style, &lv_style_pretty);
    mbox_style.text.font = get_font_lib()->msyh_16;
    lv_mbox_set_style(mbox, LV_MBOX_STYLE_BG, &mbox_style);
#endif
	lv_mbox_set_text(mbox, message);
	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(mbox, event_cb);
	return mbox;
}

static void wlan_mbox_event_cb(lv_obj_t *obj, lv_event_t event)
{
	// com_info("event = %d",event);
	if(obj == NULL){
		com_info("obj==NULL");
		return;
	}
	if (obj == para.moc.mbox_wlan_password_err)
	{
		if(event == LV_EVENT_DELETE){
            lv_obj_set_click(lv_layer_top(), false);
			para.moc.mbox_wlan_password_err = NULL;
		}else if(event == LV_EVENT_VALUE_CHANGED){
			lv_obj_del(obj);
		}
	}
	else if(obj == para.moc.mbox_wlan_conecting)
	{
		if(event == LV_EVENT_DELETE){
            lv_obj_set_click(lv_layer_top(), false);
			para.moc.mbox_wlan_conecting = NULL;
		}
	}
	else if(obj == para.moc.mbox_wlan_connectok)
	{
		if(event == LV_EVENT_DELETE){
            lv_obj_set_click(lv_layer_top(), false);
			para.moc.mbox_wlan_connectok = NULL;
		}else if(event == LV_EVENT_VALUE_CHANGED){
			lv_obj_del(obj);
		}
	}
	else if(obj == para.moc.mbox_wlan_disconnectok)
	{
		if(event == LV_EVENT_DELETE){
            lv_obj_set_click(lv_layer_top(), false);
			para.moc.mbox_wlan_disconnectok = NULL;
		}else if(event == LV_EVENT_VALUE_CHANGED){
			lv_obj_del(obj);
		}
	}
	else if(obj == para.moc.mbox_wlan_disconnecterr)
	{
		if(event == LV_EVENT_DELETE){
            lv_obj_set_click(lv_layer_top(), false);
			para.moc.mbox_wlan_disconnecterr = NULL;
		}else if(event == LV_EVENT_VALUE_CHANGED){
			lv_obj_del(obj);
		}
	}
	else
	{
		com_info("obj no found!");
	}
}


static void wlan_mbox_ctl(mbox_cmd_t cmd, void *data)
{
	static const char * mbox_btns[] = {"OK", ""};

	switch(cmd){
		case MBOX_WAIT_CONNECT:
		{
			char *ssid = "";
			char bxmsg[64] = "waiting connect to\n";
			if(data != NULL){
				ssid = (char *)data;
			}
			strcat(bxmsg, ssid);
			para.moc.mbox_wlan_conecting = wlan_mbox_create(bxmsg, wlan_mbox_event_cb);
			lv_refr_now(lv_obj_get_disp(lv_obj_get_screen(para.moc.mbox_wlan_conecting)));//show now
		}
		break;

		case MBOX_PASSWORD_ERR:
		{
			lv_obj_del(para.moc.mbox_wlan_conecting);
			para.moc.mbox_wlan_password_err = wlan_mbox_create("password err! ", wlan_mbox_event_cb);
			lv_mbox_add_btns(para.moc.mbox_wlan_password_err, mbox_btns);
			lv_obj_set_click(lv_layer_top(), true);
			lv_refr_now(lv_obj_get_disp(lv_obj_get_screen(para.moc.mbox_wlan_password_err)));//show now
		}
		break;

		case MBOX_CONNECT_OK:
		{
			lv_obj_del(para.moc.mbox_wlan_conecting);
			para.moc.mbox_wlan_connectok = wlan_mbox_create("connect ok! ", wlan_mbox_event_cb);
			lv_mbox_add_btns(para.moc.mbox_wlan_connectok, mbox_btns);
			lv_obj_set_click(lv_layer_top(), true);
			lv_refr_now(lv_obj_get_disp(lv_obj_get_screen(para.moc.mbox_wlan_connectok)));//show now
		}
		break;

		case MBOX_DISCONNECT_OK:
		{
			para.moc.mbox_wlan_disconnectok = wlan_mbox_create("disconnect ok! ", wlan_mbox_event_cb);
			lv_mbox_add_btns(para.moc.mbox_wlan_disconnectok, mbox_btns);
			lv_obj_set_click(lv_layer_top(), true);
			lv_refr_now(lv_obj_get_disp(lv_obj_get_screen(para.moc.mbox_wlan_disconnectok)));//show now
		}
		break;

		case MBOX_DISCONNECT_ERR:
		{
			para.moc.mbox_wlan_disconnecterr = wlan_mbox_create("disconnect err! ", wlan_mbox_event_cb);
			lv_mbox_add_btns(para.moc.mbox_wlan_disconnecterr, mbox_btns);
			lv_obj_set_click(lv_layer_top(), true);
			lv_refr_now(lv_obj_get_disp(lv_obj_get_screen(para.moc.mbox_wlan_disconnecterr)));//show now
		}
		break;


		default:
		break;

	}
}

static void on_off_sw_destory(lv_obj_t *obj)
{
	if(obj != NULL) {
		lv_obj_del(obj);
	}
}

#ifdef LV_USE_KB
lv_obj_t *wlan_kb_create(void){
	static lv_obj_t *keyboard = NULL;
	static lv_style_t style0_keyboard_1;
	static lv_style_t style1_keyboard_1;
	static lv_style_t style2_keyboard_1;

	lv_style_copy(&style0_keyboard_1, &lv_style_pretty);
	style0_keyboard_1.body.main_color = lv_color_hex(0xc0c0c0);
	style0_keyboard_1.body.radius = 0;

	lv_style_copy(&style1_keyboard_1, &lv_style_btn_rel);
	style1_keyboard_1.body.main_color = lv_color_hex(0xffffff);
	style1_keyboard_1.body.grad_color = lv_color_hex(0xffffff);
	style1_keyboard_1.body.border.width = 0;
	style1_keyboard_1.body.border.opa = 255;
	style1_keyboard_1.text.color = lv_color_hex(0x000000);
	style1_keyboard_1.text.sel_color = lv_color_hex(0x000000);
	style1_keyboard_1.text.line_space = 2;

	lv_style_copy(&style2_keyboard_1, &lv_style_btn_pr);
	style2_keyboard_1.body.main_color = lv_color_hex(0xffffff);
	style2_keyboard_1.body.grad_color = lv_color_hex(0xffffff);
	style2_keyboard_1.body.border.width = 0;
	style2_keyboard_1.body.border.opa = 255;
	style2_keyboard_1.text.color = lv_color_hex(0x000000);
	style2_keyboard_1.text.sel_color = lv_color_hex(0x000000);
	style2_keyboard_1.text.line_space = 2;

	keyboard = lv_kb_create(para.ui.container_1, NULL);
	lv_obj_set_pos(keyboard, 148, 170);
	lv_obj_set_size(keyboard, 516, 198);
	lv_kb_set_mode(keyboard, LV_KB_MODE_TEXT);
	lv_kb_set_cursor_manage(keyboard, false);
	lv_kb_set_style(keyboard, LV_KB_STYLE_BG, &style0_keyboard_1);
	lv_kb_set_style(keyboard, LV_KB_STYLE_BTN_REL, &style1_keyboard_1);
	lv_kb_set_style(keyboard, LV_KB_STYLE_BTN_PR, &style2_keyboard_1);
	return keyboard;
}

#if LV_USE_ANIMATION
static void kb_hide_anim_end(lv_anim_t * a)
{
    lv_obj_del(a->var);
    para.moc.wlan_kb = NULL;
}
#endif

static void keyboard_event_cb(lv_obj_t * keyboard, lv_event_t event)
{
    (void) keyboard;    /*Unused*/

    lv_kb_def_event_cb(para.moc.wlan_kb, event);

    if(event == LV_EVENT_APPLY || event == LV_EVENT_CANCEL) {
#if LV_USE_ANIMATION
        lv_anim_t a;
        a.var = para.moc.wlan_kb;
        a.start = lv_obj_get_y(para.moc.wlan_kb);
        a.end = LV_VER_RES;
        a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
        a.path_cb = lv_anim_path_linear;
        a.ready_cb = kb_hide_anim_end;
        a.act_time = 0;
        a.time = 300;
        a.playback = 0;
        a.playback_pause = 0;
        a.repeat = 0;
        a.repeat_pause = 0;
        lv_anim_create(&a);
#else
        lv_obj_del(para.moc.wlan_kb);
        para.moc.wlan_kb = NULL;
#endif
    }
}
#endif // LV_USE_KB

static void wifi_menu_ssid_psk_save(wifi_data_t wifi)
{
	write_string_type_param(WLAN_SCENE, WLAN_MANU_SSID, wifi.manu_ssid,
		sizeof(wifi.manu_ssid));
	com_info("wifi_ssid = %s",wifi.manu_ssid);
	write_string_type_param(WLAN_SCENE, WLAN_MANU_PASSWORD, wifi.manu_password,
		sizeof(wifi.manu_password));
	com_info("manu_password = %s",wifi.manu_password);
}

static void wifi_menu_read_ssid_psk(wifi_data_t *wifi)
{
	read_string_type_param(WLAN_SCENE, WLAN_MANU_SSID, wifi->manu_ssid,
					   sizeof(wifi->manu_ssid));
	read_string_type_param(WLAN_SCENE, WLAN_MANU_PASSWORD, wifi->manu_password,
					   sizeof(wifi->manu_password));
}

static void wifi_menu_save_state_param(wifi_data_t wifi)
{
	int param;
	param = (int)(wifi.manu_on);
	write_int_type_param(WLAN_SCENE, WLAN_MANU_ON, param);
	param = (int)(wifi.manu_connected);
	write_int_type_param(WLAN_SCENE, WLAN_MANU_CONNECTED, param);
}

static void select_event_cb(lv_obj_t * btn, lv_event_t event)
{
	const char *str = NULL;
	if (btn != NULL && event == LV_EVENT_CLICKED) {
		hidden_obj_and_all_child(para.ui.container_1, 0);
		lv_obj_move_foreground(para.ui.container_1);
		if(para.moc.wlan_kb == NULL) {
            para.moc.wlan_kb = wlan_kb_create();
            lv_kb_set_ta(para.moc.wlan_kb, para.ui.text_area_1);
            lv_obj_set_event_cb(para.moc.wlan_kb, keyboard_event_cb);
        }
		hidden_obj_and_all_child(para.ui.container_mark, 0);
		memset(para.moc.tmp_ssid, 0, WIFI_MAX_SSID_SIZE);
		str = lv_list_get_btn_text(btn);
		memcpy(para.moc.tmp_ssid, str, strlen(str));
		lv_label_set_text(para.ui.label_2, para.moc.tmp_ssid);
	}
}

static void connect_event_cb(lv_obj_t * btn, lv_event_t event)
{
	int ret = -1;
	const char *str = NULL;
	wifi_data_t wifi;
	static lv_style_t style0_label_1;
	wifi_sta_cn_para_t *cn_para = NULL;

	get_wifi_data(&wifi);

	lv_style_copy(&style0_label_1, &lv_style_transp);
	if (btn != NULL && event == LV_EVENT_CLICKED) {

		if(wifi.is_on != 1){
			return;
		}

		memset(para.moc.tmp_password, 0, WIFI_MAX_PASSWORD_SIZE);
		str = lv_ta_get_text(para.ui.text_area_1);
		memcpy(para.moc.tmp_password, str, strlen(str));

		com_info("connecting:%s, %s\n", para.moc.tmp_ssid, para.moc.tmp_password);
		wlan_mbox_ctl(MBOX_WAIT_CONNECT, &para.moc.tmp_ssid);

		cn_para = malloc(sizeof(wifi_sta_cn_para_t) + WIFI_MAX_SSID_SIZE + WIFI_MAX_PASSWORD_SIZE);
		if(NULL == cn_para) {
			com_warn("connect wifi failed: cn_para mem request failed!\n");
		} else {
			cn_para->sec					= WIFI_SEC_WPA2_PSK;
			cn_para->fast_connect	= 0;
			cn_para->ssid					= (char*)cn_para + sizeof(wifi_sta_cn_para_t);
			cn_para->password				= (char*)cn_para + sizeof(wifi_sta_cn_para_t) + WIFI_MAX_SSID_SIZE;
			memcpy(cn_para->ssid, para.moc.tmp_ssid, WIFI_MAX_SSID_SIZE);
			memcpy(cn_para->password, para.moc.tmp_password, WIFI_MAX_PASSWORD_SIZE);

			ret = wifi_sta_connect(cn_para);

			if(WMG_STATUS_SUCCESS == ret) {
				com_info("connect new wifi:%s, %s ok\n", para.moc.tmp_ssid, para.moc.tmp_password);
				wifi.is_connected = 1;
				wifi.manu_connected = 1;
				memcpy(wifi.ssid, para.moc.tmp_ssid, WIFI_MAX_SSID_SIZE);
				memcpy(wifi.password, para.moc.tmp_password, WIFI_MAX_PASSWORD_SIZE);
				memcpy(wifi.manu_ssid, para.moc.tmp_ssid, WIFI_MAX_SSID_SIZE);
				memcpy(wifi.manu_password, para.moc.tmp_password, WIFI_MAX_PASSWORD_SIZE);
				set_wifi_data(&wifi);
				wifi_menu_ssid_psk_save(wifi);
				wifi_menu_save_state_param(wifi);
				update_list_context(&para);
				wlan_mbox_ctl(MBOX_CONNECT_OK, NULL);
			}
			else {
				wlan_mbox_ctl(MBOX_PASSWORD_ERR, NULL);
			}

			free(cn_para);
		}
	}
}

static void disconnect_event_cb(lv_obj_t * btn, lv_event_t event)
{
	int ret;
	wifi_data_t wifi;
	static lv_style_t style0_label_1;
	get_wifi_data(&wifi);

	lv_style_copy(&style0_label_1, &lv_style_transp);
	if (btn != NULL && event == LV_EVENT_CLICKED) {
		if(wifi.is_on != 1){
			return;
		}
		ret = wifi_sta_disconnect();
		if(WMG_STATUS_SUCCESS == ret) {
			com_info("disconnect wifi ok\n");
			wifi_sta_remove_networks(NULL);
			wifi.is_connected = 0;
			wifi.manu_connected = 0;
			memset(wifi.ssid, 0, WIFI_MAX_SSID_SIZE);
			memset(wifi.password, 0, WIFI_MAX_PASSWORD_SIZE);
			memset(wifi.manu_ssid, 0, WIFI_MAX_SSID_SIZE);
			memset(wifi.manu_password, 0, WIFI_MAX_PASSWORD_SIZE);
			set_wifi_data(&wifi);
			// wifi_menu_ssid_psk_save(wifi);
			wifi_menu_save_state_param(wifi);
			update_list_context(&para);
			wlan_mbox_ctl(MBOX_DISCONNECT_OK, NULL);
		}
		else {
			wlan_mbox_ctl(MBOX_DISCONNECT_ERR, NULL);
		}
	}
}

static void out_kb_area_event_cb(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		hidden_obj_and_all_child(para.ui.container_1, 1);
		hidden_obj_and_all_child(para.ui.container_mark, 1);
		update_list_context(&para);
		lv_ta_set_text(para.ui.text_area_1, "");
	}
	#if 0
	lv_indev_t * indev = lv_indev_get_act();
	if (__is_in_obj(para.ui.container_1, indev->proc.types.pointer.act_point.x, indev->proc.types.pointer.act_point.y)) {
		;
	}
	else {
		hidden_obj_and_all_child(para.ui.container_1, 1);
		hidden_obj_and_all_child(para.ui.container_mark, 1);
		update_list_context(&para);
	}
	#endif
}

static void on_off_event_cb(lv_obj_t * btn, lv_event_t event)
{
	int ret;
	wifi_data_t wifi;
	wifi_sta_cn_para_t *cn_para = NULL;

	get_wifi_data(&wifi);

	if (event == LV_EVENT_CLICKED)
	{
		if(wifi.is_on != 1) {
			ret = wifi_on(WIFI_STATION);
			s_wifi_off = 0;
			if(WMG_STATUS_SUCCESS == ret) {
				com_info("open wifi ok\n");
				wifi.is_on = 1;
				wifi.manu_on = 1;

				#if 1
				wifi_menu_read_ssid_psk(&wifi);
				com_info("wait connect..wifi.manu_connected == %d\n",wifi.manu_connected);
				if(wifi.manu_connected == 0 && (strlen(wifi.manu_ssid) != 0)) {
					com_info("wait connect..wifi.manu_ssid == %s\n",wifi.manu_ssid);
					com_info("wait connect..wifi.manu_password == %s\n",wifi.manu_password);

					cn_para = malloc(sizeof(wifi_sta_cn_para_t) + WIFI_MAX_SSID_SIZE + WIFI_MAX_PASSWORD_SIZE);

					if(NULL == cn_para)
					{
						com_warn("wifi connect failed: cn_para mem request failed!\n");
					} else {
						cn_para->sec			= WIFI_SEC_WPA2_PSK;
						cn_para->fast_connect	= 0;
						cn_para->ssid			= (char*)cn_para + sizeof(wifi_sta_cn_para_t);
						cn_para->password		= (char*)cn_para + sizeof(wifi_sta_cn_para_t) + WIFI_MAX_SSID_SIZE;
						memcpy(cn_para->ssid, wifi.manu_ssid, WIFI_MAX_SSID_SIZE);
						memcpy(cn_para->password, wifi.manu_password, WIFI_MAX_PASSWORD_SIZE);

						ret = wifi_sta_connect(cn_para);

						if(WMG_STATUS_SUCCESS == ret) {
							com_info("connect wifi ok\n");
							wifi.is_connected = 1;
						}
					}
				}
				#endif
			}
		}
		else
		{
			if(wifi.is_connected) {
				ret = wifi_sta_disconnect();
				if(WMG_STATUS_SUCCESS == ret) {
					wifi.is_connected = 0;
					wifi.manu_connected = 0;
				}
			}

			s_wifi_off = 1;
			ret = wifi_off();

			if(WMG_STATUS_SUCCESS == ret) {
				com_info("close wifi ok\n");
				wifi.is_on = 0;
				wifi.manu_on = 0;
			}
		}

		set_wifi_data(&wifi);
		wifi_menu_save_state_param(wifi);
		update_list_context(&para);
	}
}

static void update_list_context(wlan_set1_para_t *para)
{
	int i;
	lv_obj_t *tmp_btn;
	lv_obj_t *label;
	lv_obj_t *image;
	lv_obj_t *next_image;
	wifi_data_t wifi;
	int scan_num;
	static lv_style_t style0_label_1;

	lv_style_copy(&style0_label_1, &lv_style_transp);
	lv_list_clean(para->ui.list_1);
	get_wifi_data(&wifi);

	pthread_mutex_lock(&para->moc.scan_mutex);
	scan_num = para->moc.scan_num;
	pthread_mutex_unlock(&para->moc.scan_mutex);

	// btn1
	tmp_btn = lv_list_add_btn(para->ui.list_1, NULL, NULL);
	lv_btn_set_layout(tmp_btn, LV_LAYOUT_OFF);

	label = lv_label_create(tmp_btn, NULL);
	#if CONFIG_FONT_ENABLE
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;
	style0_label_1.text.font = get_font_lib()->msyh_16;

	lv_label_set_text(label, get_text_by_id(LANG_WLAN_OPEN_WLAN));
	lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style0_label_1);
	#else
	lv_label_set_text(label, "open wlan");
	#endif
	lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);

	para->moc.on_off_sw = on_off_sw_create(tmp_btn);
	lv_obj_align(para->moc.on_off_sw, label, LV_ALIGN_OUT_RIGHT_MID, 625, 0);
	lv_obj_set_event_cb(tmp_btn, on_off_event_cb);
	lv_obj_set_event_cb(para->moc.on_off_sw, on_off_event_cb);
	if(wifi.is_on == 1) {
		lv_sw_on(para->moc.on_off_sw, LV_ANIM_OFF);
	}
	else {
		lv_sw_off(para->moc.on_off_sw, LV_ANIM_OFF);
	}

	// btn2
	if(wifi.is_on == 1 && wifi.is_connected) {
		tmp_btn = lv_list_add_btn(para->ui.list_1, NULL, NULL);
		lv_btn_set_layout(tmp_btn, LV_LAYOUT_OFF);

		image = lv_img_create(tmp_btn, NULL);
		lv_img_set_src(image, get_img_by_level(wifi.rssi));
		lv_obj_align(image, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);

		if(strlen(wifi.ssid) != 0) {
			label = lv_label_create(tmp_btn, NULL);

			#if CONFIG_FONT_ENABLE
			lv_style_copy(&style0_label_1, &lv_style_transp);
			style0_label_1.text.color = lv_color_hex(0x000000);
			style0_label_1.text.line_space = 2;
			style0_label_1.text.font = get_font_lib()->msyh_16;

			lv_label_set_text(label, wifi.ssid);
			lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style0_label_1);
			#else
			lv_label_set_text(label, wifi.ssid);
			#endif

			lv_obj_align(label, image, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
		}

		lv_style_copy(&style0_button_3, &lv_style_btn_rel);
		style0_button_3.body.main_color = lv_color_hex(0x55aaff);
		style0_button_3.body.grad_color = lv_color_hex(0x55aaff);
		style0_button_3.body.radius = 0;

		lv_btn_set_style(tmp_btn, LV_BTN_STYLE_REL, &style0_button_3);
		lv_btn_set_style(tmp_btn, LV_BTN_STYLE_PR, &style0_button_3);
	}

	// btn3
	if(wifi.is_on == 1) {
		tmp_btn = lv_list_add_btn(para->ui.list_1, NULL, NULL);
		lv_btn_set_layout(tmp_btn, LV_LAYOUT_OFF);

		label = lv_label_create(tmp_btn, NULL);

		#if CONFIG_FONT_ENABLE
		lv_style_copy(&style0_label_1, &lv_style_transp);
		style0_label_1.text.color = lv_color_hex(0x000000);
		style0_label_1.text.line_space = 2;
		style0_label_1.text.font = get_font_lib()->msyh_16;

		lv_label_set_text(label, get_text_by_id(LANG_WLAN_SELECT_WLAN));
		lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style0_label_1);
		#else
		lv_label_set_text(label, "select the wlan");
		#endif

		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);

		image = lv_img_create(tmp_btn, NULL);
		lv_img_set_src(image, get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_CYCLE_IMAGE));
		lv_obj_align(image, label, LV_ALIGN_OUT_RIGHT_MID, 590, 0);
	}

	// btn more
	if(wifi.is_on == 1) {
		for(i=0; i<scan_num; i++) {
			tmp_btn = lv_list_add_btn(para->ui.list_1, NULL, NULL);
			lv_btn_set_layout(tmp_btn, LV_LAYOUT_OFF);
			image = lv_img_create(tmp_btn, NULL);
			lv_img_set_src(image, get_img_by_level(para->moc.scan_results  ? para->moc.scan_results[i].rssi : RSSI_NO_SINGAL));
			lv_obj_align(image, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);

			label = lv_label_create(tmp_btn, NULL);

			#if CONFIG_FONT_ENABLE
			lv_style_copy(&style0_label_1, &lv_style_transp);
			style0_label_1.text.color = lv_color_hex(0x000000);
			style0_label_1.text.line_space = 2;
			style0_label_1.text.font = get_font_lib()->msyh_16;

			lv_label_set_text(label, para->moc.scan_results ? para->moc.scan_results[i].ssid : "");
			lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style0_label_1);
			#else
			lv_label_set_text(label, para->moc.scan_results ? para->moc.scan_results[i].ssid : "");
			#endif

			lv_obj_align(label, image, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

			next_image = lv_img_create(tmp_btn, NULL);
			lv_img_set_src(next_image, get_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]), WIFI_NEXT_IMAGE));
			lv_obj_align(next_image, image, LV_ALIGN_OUT_RIGHT_MID, 665, 0);

			lv_obj_set_event_cb(tmp_btn, select_event_cb);
		}
	}
}

void hbar_wifi_signal_update(lv_obj_t *img_hbar_wifi, bool first)
{
	wifi_data_t wifi;
	get_wifi_data(&wifi);
	static int rssi = 0;
	if(rssi == wifi.rssi && first == false){
		return;
	}
	rssi = wifi.rssi;
	if(wifi.is_on && wifi.is_connected)
	{
		// com_info("wifi signal : %d   pic=%x",wifi.rssi,get_img_by_level(wifi.rssi));
		lv_img_set_src(img_hbar_wifi, get_img_by_level(wifi.rssi));
	}
	else
	{
		lv_img_set_src(img_hbar_wifi, (void *)get_img_by_level(RSSI_NO_SINGAL));
		// com_info("wifi no signal ..");
	}
}

static void wlanloop_task(lv_task_t * task)
{
	wifi_data_t wifi;
	wlan_set1_para_t *para;
	static int timecnt = 0;
	timecnt++;
	get_wifi_data(&wifi);
	para = (wlan_set1_para_t *)task->user_data;
	if(wifi.update_flag) {
		update_list_context(para);

		wifi.update_flag = 0;
		set_wifi_data(&wifi);
	}
	if(timecnt > 10){
		hbar_time_update(para->ui.label_hbar_timer);
		hbar_wifi_signal_update(para->ui.img_hbar_wifi, false);
		timecnt = 0;
	}
}

void* wifi_scan_thread(void *arg)
{
	int counter = 0;
	wifi_data_t wifi;
	wlan_set1_para_t *para = (wlan_set1_para_t *)arg;
	int scan_valid_len;
	uint32_t arr_size = 0;
	int ret = -1;

	while(1)
	{
		get_wifi_data(&wifi);
		para = (wlan_set1_para_t *)arg;

		if(para->moc.scan_quit_flag == 1) {
			pthread_exit((void*)0);
		}

		if(counter == 0) {
			if (s_wifi_off) continue;
			if(wifi.is_on) {
			    ret = wifi_get_scan_results(para->moc.scan_results, NULL, &scan_valid_len, WIFI_MAX_SCAN_NUM);

				pthread_mutex_lock(&para->moc.scan_mutex);
				para->moc.scan_num = scan_valid_len;
				pthread_mutex_unlock(&para->moc.scan_mutex);

				wifi.update_flag = 1;
				set_wifi_data(&wifi);
			}
		}

		counter++;
		if(counter >= 150) {
			counter = 0;
		}

		usleep(100 * 1000);
	}
}

static void wifi_info_update_task(lv_task_t * task)
{
#if DISABLE_WIFI_MG
	wifi_data_t wifi;
	wlan_set1_para_t *para;
	int is_wifi_connected = 0;
	wifi_sta_info_t sta_info;

	get_wifi_data(&wifi);
	para = (wlan_set1_para_t *)task->user_data;

	if(wifi.is_on != 1) {
		return;
	}

	if(wifi.is_connected)
	{
		memset(&sta_info, 0, sizeof(wifi_sta_info_t));
		wifi_sta_get_info(&sta_info);

		wifi.rssi = sta_info.rssi;
		memcpy(wifi.ssid, sta_info.ssid, strlen(sta_info.ssid));
		memcpy(&para->moc.sta_info, &sta_info, sizeof(wifi_sta_info_t));
	}
	else
	{
		wifi.rssi = 0;
		memset(wifi.ssid, 0, sizeof(wifi.ssid));
		memset(&para->moc.sta_info, 0, sizeof(wifi_sta_info_t));
	}
	set_wifi_data(&wifi);
#endif
}

void wifi_pic_res_uninit(void)
{
	free_image_buff_form_list(wifi_image_list, sizeof(wifi_image_list)/sizeof(wifi_image_list[0]));
}

static void update_common_font(void)
{
	static lv_style_t style0_label_1;
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;
	style0_label_1.text.font = get_font_lib()->msyh_16;

	lv_label_set_text(para.ui.label_1, get_text_by_id(LANG_WLAN_WLAN));
	lv_label_set_style(para.ui.label_1, LV_LABEL_STYLE_MAIN, &style0_label_1);

	lv_label_set_text(para.ui.label_3, get_text_by_id(LANG_WLAN_CONNECT));
	lv_label_set_style(para.ui.label_3, LV_LABEL_STYLE_MAIN, &style0_label_1);

	lv_label_set_text(para.ui.label_4, get_text_by_id(LANG_WLAN_DISCONNECT));
	lv_label_set_style(para.ui.label_4, LV_LABEL_STYLE_MAIN, &style0_label_1);
}

static void text_area_event_handler(lv_obj_t * text_area, lv_event_t event)
{
    (void) text_area;    /*Unused*/
    if(event == LV_EVENT_CLICKED) {
        if(para.moc.wlan_kb == NULL) {
            para.moc.wlan_kb = wlan_kb_create();
            lv_kb_set_ta(para.moc.wlan_kb, para.ui.text_area_1);
            lv_obj_set_event_cb(para.moc.wlan_kb, keyboard_event_cb);
        }
    }

}

static void wlan_set1_moc_create(void)
{
	//lv_task_t *scan_once_tid;
	pthread_attr_t attr;

	para.moc.scan_results = malloc(sizeof(wifi_scan_result_t) * WIFI_MAX_SCAN_NUM);
	if(NULL == para.moc.scan_results)
	{
		com_warn("scan results mem request failed, do not create scan thread!\n");
	} else {
		pthread_attr_init(&attr);

		pthread_attr_setstacksize(&attr, 0x40000);
		pthread_mutex_init(&para.moc.scan_mutex, NULL);

		pthread_create(&para.moc.wifi_scan_tid, &attr, wifi_scan_thread, &para);
		pthread_attr_destroy(&attr);
	}
	//para.moc.wifi_info_update_tid = lv_task_create(wifi_info_update_task, 500, LV_TASK_PRIO_MID, &para);
	//lv_task_ready(para.moc.wifi_info_update_tid);

	para.moc.list_update_tid = lv_task_create(wlanloop_task, 300, LV_TASK_PRIO_MID, &para);
	lv_task_ready(para.moc.list_update_tid);

	update_list_context(&para);
	hidden_obj_and_all_child(para.ui.container_1, 1);
	hidden_obj_and_all_child(para.ui.container_mark, 1);
	lv_obj_set_event_cb(para.ui.text_area_1, text_area_event_handler);
	lv_obj_set_event_cb(para.ui.button_1, connect_event_cb);
	lv_obj_set_event_cb(para.ui.button_2, disconnect_event_cb);
	lv_obj_set_event_cb(para.ui.container_mark, out_kb_area_event_cb);
	lv_obj_set_event_cb(para.ui.btn_hbar_return, btn_hbar_return_event_cb);
	lv_obj_set_event_cb(para.ui.btn_hbar_home, btn_hbar_return_event_cb);

	#if CONFIG_FONT_ENABLE
	update_common_font();
	#endif
	hbar_time_update(para.ui.label_hbar_timer);
	hbar_wifi_signal_update(para.ui.img_hbar_wifi, true);
}

static void wlan_set1_moc_destory(void)
{
	para.moc.scan_quit_flag = 1;
	pthread_join(para.moc.wifi_scan_tid, NULL);

	lv_task_del(para.moc.list_update_tid);
	//lv_task_del(para.moc.wifi_info_update_tid);
	pthread_mutex_destroy(&para.moc.scan_mutex);
	lv_list_clean(para.ui.list_1);
	if(para.moc.scan_results)
	{
		free(para.moc.scan_results);
	}
}



static int create_wlan_set1(void)
{
	memset(&para, 0, sizeof(wlan_set1_para_t));
	para.ui.cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(para.ui.cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color = LV_COLOR_BLUE;
	cont_style.body.grad_color = LV_COLOR_BLUE;
	lv_cont_set_style(para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_cont_set_layout(para.ui.cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(para.ui.cont, LV_FIT_NONE);

	#if 0
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
	#endif

	wlan_set1_auto_ui_create(&para.ui);
	wlan_set1_moc_create();
	return 0;
}

static int destory_wlan_set1(void)
{
	wlan_set1_moc_destory();
	wlan_set1_auto_ui_destory(&para.ui);
	lv_obj_del(para.ui.cont);

	return 0;
}

static int show_wlan_set1(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);

	return 0;
}

static int hide_wlan_set1(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static int msg_proc_wlan_set1(MsgDataInfo *msg)
{
	return 0;
}

static page_interface_t page_wlan_set1 =
{
	.ops =
	{
		create_wlan_set1,
		destory_wlan_set1,
		show_wlan_set1,
		hide_wlan_set1,
		msg_proc_wlan_set1,
	},
	.info =
	{
		.id         = PAGE_WLAN_SET1,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_WLAN_SET1(void)
{
	reg_page(&page_wlan_set1);
}
