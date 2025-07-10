/* Copyright (c) 2019-2035 Allwinner Technology Co., Ltd. ALL rights reserved.

 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the the People's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.

 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
 * IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.


 * THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
 * PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
 * THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
 * OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _ION_ALLOC_LIST_H
#define _ION_ALLOC_LIST_H

#define ion_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(aw_ptr, type, member) ( { \
const typeof( ((type *)0)->member ) *__mptr = (aw_ptr); \
(type *)( (char *)__mptr - ion_offsetof(type,member) ); } )

static inline void aw_prefetch(const void *x) {(void)x;}
static inline void aw_prefetchw(const void *x) {(void)x;}

#define AW_LIST_LOCATION1  ((void *) 0x00100100)
#define AW_LIST_LOCATION2  ((void *) 0x00200200)

struct aw_mem_list_head {
struct aw_mem_list_head *aw_next, *aw_prev;
};

#define AW_MEM_LIST_HEAD_INIT(aw_name) { &(aw_name), &(aw_name) }

#define LIST_HEAD(aw_name) \
struct aw_mem_list_head aw_name = AW_MEM_LIST_HEAD_INIT(aw_name)

#define AW_MEM_INIT_LIST_HEAD(aw_ptr) do { \
(aw_ptr)->aw_next = (aw_ptr); (aw_ptr)->aw_prev = (aw_ptr); \
} while (0)

static inline void __aw_list_add(struct aw_mem_list_head *newList,
      struct aw_mem_list_head *aw_prev,
      struct aw_mem_list_head *aw_next)
{
    aw_next->aw_prev = newList;
    newList->aw_next = aw_next;
    newList->aw_prev = aw_prev;
    aw_prev->aw_next = newList;
}

static inline void aw_mem_list_add(struct aw_mem_list_head *newList,
		struct aw_mem_list_head *head)
{
    __aw_list_add(newList, head, head->aw_next);
}

static inline void aw_mem_list_add_tail(struct aw_mem_list_head *newList,
         struct aw_mem_list_head *head)
{
    __aw_list_add(newList, head->aw_prev, head);
}

static inline void __aw_mem_list_del(struct aw_mem_list_head * aw_prev,
        struct aw_mem_list_head * aw_next)
{
    aw_next->aw_prev = aw_prev;
    aw_prev->aw_next = aw_next;
}

static inline void aw_mem_list_del(struct aw_mem_list_head *entry)
{
    __aw_mem_list_del(entry->aw_prev, entry->aw_next);
    entry->aw_next = (struct aw_mem_list_head *)AW_LIST_LOCATION1;
    entry->aw_prev = (struct aw_mem_list_head *)AW_LIST_LOCATION2;
}

#define aw_mem_list_entry(aw_ptr, type, member) container_of(aw_ptr, type, member)

#define aw_mem_list_for_each_safe(aw_pos, aw_n, aw_head) \
for (aw_pos = (aw_head)->aw_next, aw_n = aw_pos->aw_next; aw_pos != (aw_head); \
aw_pos = aw_n, aw_n = aw_pos->aw_next)

#define aw_mem_list_for_each_entry(aw_pos, aw_head, member) \
for (aw_pos = aw_mem_list_entry((aw_head)->aw_next, typeof(*aw_pos), member); \
     aw_prefetch(aw_pos->member.aw_next), &aw_pos->member != (aw_head);  \
     aw_pos = aw_mem_list_entry(aw_pos->member.aw_next, typeof(*aw_pos), member))

#endif
