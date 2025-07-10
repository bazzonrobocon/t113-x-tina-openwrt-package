/**
 * @file: msg.c
 * @autor: liujiaming
 * @url: liujiaming@allwinnertech.com
 *  smartva message system
 */
#include "msg.h"
#include <pthread.h>
#include <unistd.h>
#include "dbList.h"

static db_list_t *msg_info_list = NULL;

int sent_msg(MsgDataInfo msg)
{
    MsgDataInfo *msgtmp = NULL;

    // pmsg("msg: type:%d form:%d  to:%d  val:%d  used:%d",msg.type,msg.from,msg.to,msg.value,msg.used);
    if(msg_info_list){
        msgtmp = malloc(sizeof(MsgDataInfo));
        memcpy(msgtmp, &msg, sizeof(MsgDataInfo));
        __db_list_put_tail(msg_info_list, (void *)msgtmp);
    }
    return 0;
}

int get_msg(MsgDataInfo *msg)
{
    MsgDataInfo *msgtmp = NULL;
    if(msg_info_list){
        msgtmp = (MsgDataInfo *)__db_list_pop(msg_info_list);
        if(msgtmp){
            memcpy(msg, msgtmp, sizeof(MsgDataInfo));
            free(msgtmp);
            return 0;
        }
        // pmsg("msg: type:%d form:%d  to:%d  val:%d use:%d",msg->type,msg->from,msg->to,msg->value,msg->used);
    }
    return -1;
}

int message_sys_init(void)
{
    int ret = -1;

    pmsg("msg sys init..............");
	msg_info_list = db_list_create("msg_info_list", 0);
	if (NULL == msg_info_list) {
		perr("db_list_create Error!\n");
		return -1;
	}
    return ret;
}


int message_sys_unit(void)
{
    __db_list_destory(msg_info_list);
    return 0;
}
