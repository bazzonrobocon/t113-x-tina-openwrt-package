#ifndef __CHARSET_H__
#define __CHARSET_H__

#include "common.h"

typedef enum{
    CHARSET_ISO8859_1 = 0,
    CHARSET_UCS_2 = 1,
    CHARSET_UTF16BE = 2,
    CHARSET_UTF_8 = 3,
    CHARSET_ASCII = 4,
	CHARSET_UNICODE = 5,
	CHARSET_UTF_32 = 6,
    CHARSET_GBK = 7,
    CHARSET_MAX,
}font_charset_e;
// __u32  UTF82GBK(const __u8 *src, __u32 src_size,__u8 *dst, __u32 dst_size);
// __u32  GBK2UTF8(const __u8 *src, __u32 src_size,__u8 *dst, __u32 dst_size);

int font_conver_utf8(font_charset_e src_charset, __u8 *src, __u32 src_size, __u8 *dst, __u32 dst_size);
#endif
