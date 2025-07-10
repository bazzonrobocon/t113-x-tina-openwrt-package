/*
 * =====================================================================================
 *
 *       Filename:  misc.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年03月31日 16时23分04秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "ion_mem_alloc.h"
#include "seamless.h"

static struct SunxiMemOpsS *ionHdle = NULL;

/*
*初始化操作
*根据实际情况，可以不实现
*/
int mem_init(void)
{
    int ret;
    ionHdle = GetMemAdapterOpsS();
	ret = ionHdle->open();
	if (ret != SEAMLESS_OK)
    {
		SEAMLESS_ERR("pico open ION error: %d\n", ret);
        return SEAMLESS_FAIL;
	}

    return SEAMLESS_OK;
}

//退出操作
//根据实际情况，可以不实现
int mem_exit(void)
{
    if(!ionHdle)
    {
        SEAMLESS_ERR("pico ION NULL!\n");
        return SEAMLESS_FAIL;
    }
    ionHdle->close();
    ionHdle = NULL;
    return SEAMLESS_OK;
}

//申请物理连续内存
void* mem_palloc(int size)
{
    if(!ionHdle)
    {
        SEAMLESS_ERR("pico ION NULL!\n");
        return NULL;
    }

    return ionHdle->palloc(size);
}

//释放由palloc申请的内存
void mem_pfree(void* viraddr,int size)
{
    if(!ionHdle)
    {
        SEAMLESS_ERR("pico ION NULL!\n");
        return;
    }
    ionHdle->pfree(viraddr);
}

//虚拟转物理地址
void* mem_va2pa(void *viraddr)
{
    if(!ionHdle)
    {
        SEAMLESS_ERR("pico ION NULL!\n");
        return NULL;
    }

    return ionHdle->cpu_get_phyaddr(viraddr);
}

//物理转虚拟地址
void* mem_pa2va(void *phyaddr)
{
    if(!ionHdle)
    {
        SEAMLESS_ERR("pico ION NULL!\n");
        return NULL;
    }

    return ionHdle->cpu_get_viraddr(phyaddr);
}

//由fd获取虚拟地址
void* mem_fd2va(int fd)
{
    if(!ionHdle)
    {
        SEAMLESS_ERR("pico ION NULL!\n");
        return NULL;
    }

    return ionHdle->get_viraddr_byFd(fd);
}

//由虚拟地址获取fd
int mem_va2fd(void *viraddr)
{
    if(!ionHdle)
    {
        SEAMLESS_ERR("pico ION NULL!\n");
        return SEAMLESS_FAIL;
    }

    return ionHdle->get_bufferFd(viraddr);
}

//刷dcache，同步数据
void mem_flush_cache(void *viraddr, int size)
{
    if(!ionHdle)
    {
        SEAMLESS_ERR("pico ION NULL!\n");
        return;
    }
    ionHdle->flush_cache(viraddr,size);
}
