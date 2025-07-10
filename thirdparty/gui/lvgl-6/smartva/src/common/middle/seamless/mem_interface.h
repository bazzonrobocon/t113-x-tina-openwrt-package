/*
 * =====================================================================================
 *
 *       Filename:  misc.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2022年03月31日 16时23分19秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef __MEM_INTERFACE_H__
#define __MEM_INTERFACE_H__

int mem_init(void);
int mem_exit(void);
void* mem_palloc(int size);
void mem_pfree(void* viraddr,int size);
void* mem_va2pa(void *viraddr);
void* mem_pa2va(void *phyaddr);
void* mem_fd2va(int fd);
int mem_va2fd(void *viraddr);
void mem_flush_cache(void *viraddr, int size);


#endif /*endif __MEM_INTERFACE_H__*/
