#include "charset.h"
#include "common.h"
#include "string.h"


typedef __s32 (*char2uni)(const __u8 *rawstring, __u32 boundlen, __u16 *uni);
typedef __s32 (*uni2char)(__u16 uni, __u8 *out, __u32 boundlen);
extern __s32 UTF8_char2uni(const __u8 *rawstring, __u32 boundlen, __u16 *uni);
extern __s32 UTF16BE_char2uni(const __u8 *rawstring, __u32 boundlen, __u16 *uni);
extern __s32 GBK_char2uni(const __u8 *rawstring, __u32 boundlen, __u16 *uni);
extern __s32 ISO8859_1_char2uni(const __u8 *rawstring, __u32 boundlen, __u16 *uni);
// extern __s32 UTF8_char2uni(const __u8 *rawstring, __u32 boundlen, __u16 *uni);

int font_conver_utf8(font_charset_e src_charset, __u8 *src, __u32 src_size, __u8 *dst, __u32 dst_size)
{
    __s32  src_char_len = 0;
    __s32  dst_char_len = 0;
    __s32  src_total_len = 0;
    __s32  dst_total_len = 0;
    __u16  uni = 0;
    char2uni conver2uni;

    if (/*src_charset >= CHARSET_MAX ||*/ src == NULL || src_size == 0 || dst == NULL || dst_size == 0)
    {
        return -1;
    }

    switch(src_charset)
    {
        case CHARSET_UTF_8:
            conver2uni = UTF8_char2uni;
            break;
        case CHARSET_UTF16BE:
            conver2uni = UTF16BE_char2uni;
            break;
        case CHARSET_GBK:
            conver2uni = GBK_char2uni;
            break;
        case CHARSET_ISO8859_1:
            conver2uni = GBK_char2uni;
            break;
        case CHARSET_UCS_2:
            conver2uni = UTF8_char2uni;
            break;
        default:
            conver2uni = GBK_char2uni;
            break;
    }

    while (1)
    {
        src_char_len = conver2uni(src + src_total_len, src_size - src_total_len, &uni);

        if (src_char_len <= 0)
        {
            return dst_total_len;
        }

        src_total_len += src_char_len;
        dst_char_len = UTF8_uni2char(uni, dst + dst_total_len, dst_size - dst_total_len);

        if (dst_char_len <= 0)
        {
            return dst_total_len;
        }

        dst_total_len += dst_char_len;
    }

}




__u32  GBK2UTF8(const __u8 *src, __u32 src_size,__u8 *dst, __u32 dst_size)
{
    __s32  src_char_len;
    __s32  dst_char_len;
    __s32  src_total_len;
    __s32  dst_total_len;
    __u16  uni;

    if (src == NULL || src_size == 0 || dst == NULL || dst_size == 0)
    {
        return 0;
    }

    src_char_len  = 0;
    dst_char_len  = 0;
    src_total_len = 0;
    dst_total_len = 0;

    while (1)
    {
        src_char_len = GBK_char2uni(src + src_total_len, src_size - src_total_len, &uni);

        if (src_char_len <= 0)
        {
            return dst_total_len;
        }

        src_total_len += src_char_len;
        dst_char_len = UTF8_uni2char(uni, dst + dst_total_len, dst_size - dst_total_len);

        if (dst_char_len <= 0)
        {
            return dst_total_len;
        }

        dst_total_len += dst_char_len;
    }
}

__u32  UTF82GBK(const __u8 *src, __u32 src_size,__u8 *dst, __u32 dst_size)
{
    __s32  src_char_len;
    __s32  dst_char_len;
    __s32  src_total_len;
    __s32  dst_total_len;
    __u16  uni;

    if (src == NULL || src_size == 0 || dst == NULL || dst_size == 0)
    {
        return 0;
    }

    src_char_len  = 0;
    dst_char_len  = 0;
    src_total_len = 0;
    dst_total_len = 0;

    while (1)
    {
        src_char_len = UTF8_char2uni(src + src_total_len, src_size - src_total_len, &uni);

        if (src_char_len <= 0)
        {
            return dst_total_len;
        }

        src_total_len += src_char_len;
        dst_char_len = GBK_uni2char(uni, dst + dst_total_len, dst_size - dst_total_len);
        if (dst_char_len <= 0)
        {
            return dst_total_len;
        }

        dst_total_len += dst_char_len;
    }
}
