/**********************
 *      includes
 **********************/
#include "va_font.h"

#define TTF_USE_CACHE 1

#if (TTF_USE_CACHE == 0)
typedef struct
{
    FT_Face face;
    uint8_t *dst_buffer;
    unsigned int font_size;
    unsigned int bpp;
}font_para;
struct FT_LibraryRec_ *m_ft = NULL;

static const uint8_t* copy_bitmap(FT_Bitmap *bitmap, uint8_t*dst_buffer, uint32_t size,
	uint8_t real_bpp, uint8_t dst_bpp)
{
    int mask = 0;
	uint8_t *src = NULL;
    uint32_t src_size = 0;
    uint8_t *dst = 0;
	int cnt = 0, cnt1 = 0, cnt2 = 0;

    switch(dst_bpp)
    {
        case 4:
            mask = 0xf0;
            break;
        case 2:

            mask = 0xc0;
            break;
        case 1:

            mask = 0x80;
             break;
        default:
			 com_warn("\n");
            break;
    }

    if(real_bpp < dst_bpp) {
        com_warn("\n");
    }

    src = bitmap->buffer;
    src_size = bitmap->width*bitmap->rows;
    dst = dst_buffer;
    memset(dst, 0, size*size);

    for (uint j = 0; j < src_size*dst_bpp/8; ++j)
    {
        for (int i = 0; i < 8; i += dst_bpp)
        {
            const bool ok = ((uint32_t)(src - bitmap->buffer) < src_size);
            const uint8_t val = (ok?*src:0);
            *dst |= ((val & mask) >> i);
            if (ok)
            {
                ++src;
                cnt1++;
            }
            else{
                cnt2++;
            }
        }
        ++dst;
        cnt++;
    }
    return ( const uint8_t*)dst_buffer;
}

static const uint8_t *__user_font_get_bitmap(const lv_font_t * font, uint32_t unicode_letter)
{
    font_para *para = (font_para *)font->dsc;
    uint8_t *dst_buffer = para->dst_buffer;
    uint8_t real_bpp = para->face->glyph->bitmap.pixel_mode;

    switch(real_bpp)
    {
        case 1:
            real_bpp = 1;
            break;
        case 2:
            real_bpp = 8;
            break;
        default:
            break;
    }

    if(para->bpp == 8)
    {
        return (const uint8_t *)(para->face->glyph->bitmap.buffer);
    }
    else
    {
        return copy_bitmap(&para->face->glyph->bitmap,dst_buffer,para->font_size,real_bpp,para->bpp);
    }
}

static bool __user_font_get_glyph_dsc(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out,
	uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    FT_Error error;
    FT_UInt glyph_index;
    font_para *para = (font_para *)font->dsc;

    glyph_index = FT_Get_Char_Index(para->face, unicode_letter);
    if(glyph_index == 0) {
         return false;
    }

    error = FT_Load_Glyph(para->face, glyph_index, FT_LOAD_DEFAULT);
    error |= FT_Render_Glyph(para->face->glyph, FT_RENDER_MODE_NORMAL);
    // FT_Get_Glyph(para->face->glyph, &glyph);

    if(error) {
		com_warn("\n");
	return false;
    }

    dsc_out->adv_w = para->face->glyph->metrics.horiAdvance >> 6;
    dsc_out->box_h = para->face->glyph->bitmap.rows;
    dsc_out->box_w = para->face->glyph->bitmap.width;
    dsc_out->ofs_x = para->face->glyph->bitmap_left;
    dsc_out->ofs_y = para->face->glyph->bitmap_top - para->face->glyph->bitmap.rows;
    dsc_out->bpp   = para->bpp;
	// FT_Done_Glyph(glyph);
    return true;
}

lv_font_t *ttf_font_create(const char* fileName, int size, int bpp)
{
    FT_Error error;
    lv_font_t *font = NULL;
    font_para *para = NULL;

	font = (lv_font_t *)malloc(sizeof(lv_font_t));
	if(font == NULL) {
		com_err("\n");
		goto END;
	}
	memset(font, 0, sizeof(lv_font_t));

	para = (font_para *)malloc(sizeof(font_para));
	if(para == NULL) {
		com_err("\n");
		goto END;
	}
    memset(para, 0, sizeof(font_para));
	para->dst_buffer = (uint8_t *)malloc(size*size);
	if(para->dst_buffer == NULL) {
		com_err("\n");
		goto END;
	}
    para->font_size = size;
    para->bpp = bpp;

    error = FT_New_Face(m_ft, fileName, 0, &para->face);
    if(error) {
        com_err("\n");
		goto END;
    }

    error = FT_Set_Pixel_Sizes(para->face, 0, size);
    if(error) {
        com_err("\n");
		goto END;
    }

    font->get_glyph_bitmap = __user_font_get_bitmap;
    font->get_glyph_dsc = __user_font_get_glyph_dsc;
    font->line_height = (para->face->size->metrics.height >> 6);
    font->base_line = -(para->face->size->metrics.descender >> 6);
    font->subpx = LV_FONT_SUBPX_NONE;
    font->dsc = para;
	return font;

	END:
		if(font) {
			if(para) {
				if(para->dst_buffer) {
					free(para->dst_buffer);
					para->dst_buffer = NULL;
				}
				free(para);
				para = NULL;
			}
			free(font);
			font = NULL;
		}

    return NULL;
}

void ttf_font_destory(lv_font_t *font)
{
	font_para *para = NULL;
	if(font) {
		para = (font_para *)font->dsc;
		if(para) {
			if(para->dst_buffer) {
				free(para->dst_buffer);
				para->dst_buffer = NULL;
			}
			free(para);
			para = NULL;
		}
		free(font);
	}
}

void ttf_font_init(void)
{
	FT_Init_FreeType(&m_ft);
}

void ttf_font_uninit(void)
{
	FT_Done_FreeType(m_ft);
}
#endif

#if (TTF_USE_CACHE == 1)

typedef struct
{
	int				  cache_mode;

	FT_Library        library;
	FTC_Manager       cache_manager;
	FTC_ImageCache    image_cache;
	FTC_SBitCache     sbit_cache;
	FTC_CMapCache     cmap_cache;
	FTC_SBit		  sbit;
}ttf_ctrl_t;

typedef struct ttf_font_dsc_s
{
	FT_Face face;		//ttf face
	FT_Size size;		//字体大小
    int width;          //字体宽度
	int bpp;			//字体位深
	int style;			//字体风格，正常 加粗 下划线
	lv_font_t  font;	//提供给lvgl的字体描述信息
}ttf_font_dsc_t;


static ttf_ctrl_t *g_ttf_ctrl = NULL;

//获取字形描述信息
static bool get_glyph_dsc_no_cache(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    FT_Error error;
    FT_UInt glyph_index;
    ttf_font_dsc_t * dsc = (ttf_font_dsc_t *)(font->dsc);

    if(dsc->face->size != dsc->size)
    {
        FT_Set_Pixel_Sizes(dsc->face, 0, dsc->width);//重设size
    }

    glyph_index = FT_Get_Char_Index(dsc->face, unicode_letter);//根据unicode搜索ttf，返回字形索引
    if(glyph_index == 0)
    {
        // com_warn("can not find letter = 0x%x\n",unicode_letter);
        return false;
    }
    error = FT_Load_Glyph(dsc->face, glyph_index, FT_LOAD_DEFAULT);//尝试加载字形到字形槽，每个face只有一个槽
    error |= FT_Render_Glyph(dsc->face->glyph, FT_RENDER_MODE_NORMAL);//将字形槽内字形转换为位图,8bit灰度图

    if(error)
    {
        return false;
    }

    dsc_out->adv_w = (dsc->face->glyph->metrics.horiAdvance >> 6);
    dsc_out->box_h = dsc->face->glyph->bitmap.rows;
    dsc_out->box_w = dsc->face->glyph->bitmap.width;
    dsc_out->ofs_x = dsc->face->glyph->bitmap_left;
    dsc_out->ofs_y = dsc->face->glyph->bitmap_top - dsc->face->glyph->bitmap.rows;
    dsc_out->bpp = 8;

    return true;
}

//获取字形描述信息
static bool get_glyph_dsc_enable_cache(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next)
{
     //__err("langaojie get_glyph_dsc_cb_cache %p ",font);
    FT_Error error;
    // FT_Face face;
    ttf_font_dsc_t * dsc = (ttf_font_dsc_t *)(font->dsc);

    // error = FTC_Manager_LookupFace(g_ttf_ctrl->cache_manager, dsc->face_id, &face);//先找到face,用以查找charmap。目前只有一个，这步操作省略
    // if(error)
    // {
    //     //__err("langaojie 1");
    //     return false;
    // }

    FT_Size face_size;
    struct FTC_ScalerRec_ scaler;
    scaler.face_id = (FTC_FaceID)dsc->face;
    scaler.width   = 0;
    scaler.height  = dsc->width;
    scaler.pixel   = 1;
    error = FTC_Manager_LookupSize(g_ttf_ctrl->cache_manager, &scaler, &face_size);//尝试获取某种大小的字体信息
    if(error)
    {
        com_warn("FTC_Manager_LookupSize err!");
        return false;
    }

    //FT_UInt charmap_index = FT_Get_Charmap_Index(face->charmap);//目前只有一个charmap，此步省略
    FT_UInt glyph_index = FTC_CMapCache_Lookup(g_ttf_ctrl->cmap_cache, (FTC_FaceID)dsc->face, 0, unicode_letter);
    if(glyph_index == 0)
    {
        // com_err("can not find letter = 0x%x\n",unicode_letter);
        return false;
    }

    FTC_ImageTypeRec desc_type;
    desc_type.face_id = (FTC_FaceID)dsc->face;
    desc_type.flags = FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL;
    desc_type.height = dsc->width;
    desc_type.width = 0;//dsc->size;

    error = FTC_SBitCache_Lookup(g_ttf_ctrl->sbit_cache, &desc_type, glyph_index, &g_ttf_ctrl->sbit, NULL);
    if(error)
    {
        com_warn("FTC_SBitCache_Lookup error");
        return false;
    }

    dsc_out->adv_w = g_ttf_ctrl->sbit->xadvance;
    dsc_out->box_h = g_ttf_ctrl->sbit->height;
    dsc_out->box_w = g_ttf_ctrl->sbit->width;
    dsc_out->ofs_x = g_ttf_ctrl->sbit->left;
    dsc_out->ofs_y = g_ttf_ctrl->sbit->top - g_ttf_ctrl->sbit->height;
    dsc_out->bpp = 8;

    return true;
}

//获取字形bitmap，默认为8bpp
//在特殊情况下，可以转换为bpp = 1 2 4 8
static const uint8_t * get_glyph_bitmap_no_cache(const lv_font_t * font, uint32_t unicode_letter)
{
    ttf_font_dsc_t * dsc = (ttf_font_dsc_t *)(font->dsc);
    return (const uint8_t *)(dsc->face->glyph->bitmap.buffer);
}

//获取字形bitmap，默认为8bpp
//在特殊情况下，可以转换为bpp = 1 2 4 8
static const uint8_t * get_glyph_bitmap_enable_cache(const lv_font_t * font, uint32_t unicode_letter)
{
    //__err("langaojie get_glyph_bitmap_cb_cache");
    return (const uint8_t *)g_ttf_ctrl->sbit->buffer;
}

//库内部获取face回调，这里直接返回初始化好的face即可
static FT_Error face_requester_cb(FTC_FaceID face_id, FT_Library library, FT_Pointer *pRequestData, FT_Face *pFace)
{
    if(face_id == NULL)
    {
        return FT_Err_Invalid_Argument;
    }
    *pFace = face_id;
    return FT_Err_Ok;
}

//初始化字库
void ttf_font_init(void)
{
    FT_Error error;
    unsigned int maxfaces = 0;  //cache支持face个数，0表示2个。使能cache有效
    unsigned int maxsizes = 0;  //cache支持size个数，0表示4种font_size。使能cache有效
    unsigned long maxbytes = 0; //cache字节数，0表示200000字节。使能cache有效
    int cache_mode = 1;         //使能cashe，默认

    if(g_ttf_ctrl != NULL)
    {
        return ;
    }

    g_ttf_ctrl = malloc(sizeof(ttf_ctrl_t));
    if(g_ttf_ctrl == NULL)
    {
        return ;
    }
    memset(g_ttf_ctrl,0,sizeof(ttf_ctrl_t));

    error = FT_Init_FreeType(&g_ttf_ctrl->library);//初始化字库
    if(error)
    {
        com_warn("init freetype fail error = %d",error);
        goto lib_init_err;
    }

    g_ttf_ctrl->cache_mode = cache_mode;

    if(g_ttf_ctrl->cache_mode == 1)//如果使能cache，做额外初始化动作
    {
        error = FTC_Manager_New(g_ttf_ctrl->library, maxfaces, maxsizes, maxbytes, (FTC_Face_Requester)face_requester_cb, NULL, &g_ttf_ctrl->cache_manager);
        if(error)
        {
            com_warn("FTC_Manager_New fail error = %d",error);
            goto lib_init_err;
        }
        error = FTC_CMapCache_New(g_ttf_ctrl->cache_manager, &g_ttf_ctrl->cmap_cache);
        if(error)
        {
            com_warn("FTC_CMapCache_New fail error = %d",error);
            goto lib_init_err;
        }
        error = FTC_ImageCache_New(g_ttf_ctrl->cache_manager, &g_ttf_ctrl->image_cache);
        if(error)
        {
            com_warn("FTC_ImageCache_New fail error = %d",error);
            goto lib_init_err;
        }
        error = FTC_SBitCache_New(g_ttf_ctrl->cache_manager, &g_ttf_ctrl->sbit_cache);
        if(error)
        {
            com_warn("FTC_SBitCache_New fail error = %d",error);
            goto lib_init_err;
        }
    }

    return ;

lib_init_err:
    if(g_ttf_ctrl->cache_mode == 1)
    {
        FTC_Manager_Done(g_ttf_ctrl->cache_manager);
    }

    FT_Done_FreeType(g_ttf_ctrl->library);
    free(g_ttf_ctrl);
    g_ttf_ctrl = NULL;
    return ;
}

//销毁字库
void ttf_font_uninit(void)
{
    if(g_ttf_ctrl->cache_mode == 1)
    {
        FTC_Manager_Done(g_ttf_ctrl->cache_manager);
    }
    FT_Done_FreeType(g_ttf_ctrl->library);
}

//创建某种字形
lv_font_t *ttf_font_create(const char* fileName, int size, int bpp)
{
    FT_Error error;
    FT_Face face;
    ttf_font_dsc_t *dsc;

    dsc = malloc(sizeof(ttf_font_dsc_t));//创建存放解析ttf描述信息结构体
    if(dsc == NULL)
    {
        return NULL;
    }
    memset(dsc,0,sizeof(ttf_font_dsc_t));

    error = FT_New_Face(g_ttf_ctrl->library, fileName, 0, &face);
    if(error)
    {
        com_warn("create face error(%d)", error);
        goto create_err;
    }
    error = FT_Set_Pixel_Sizes(face, 0, size);//先设置一下size，获取一些参数

    dsc->bpp   = bpp;
    dsc->width = size;
    dsc->style = 0;
    dsc->face  = face;
    dsc->size  = face->size;

    if(g_ttf_ctrl->cache_mode == 0)
    {
        dsc->font.get_glyph_dsc = get_glyph_dsc_no_cache;
        dsc->font.get_glyph_bitmap = get_glyph_bitmap_no_cache;
        dsc->font.line_height = (face->size->metrics.height >> 6);  //向右移位6次，取整数部分
        dsc->font.base_line = -(face->size->metrics.descender >> 6);//向右移位6次，取整数部分
    }
    else if(g_ttf_ctrl->cache_mode == 1)
    {
        FT_Size face_size;
        struct FTC_ScalerRec_ scaler;
        scaler.face_id = face;
        scaler.width   = 0;
        scaler.height  = size;
        scaler.pixel   = 1;   //0：宽高表示像素宽高 1：1/64点
        error = FTC_Manager_LookupSize(g_ttf_ctrl->cache_manager, &scaler, &face_size);//尝试获取某种大小的字体信息
        if(error)
        {
            free(dsc);
            com_warn("load font size err!");
            return NULL;
        }

        dsc->font.get_glyph_dsc = get_glyph_dsc_enable_cache;
        dsc->font.get_glyph_bitmap = get_glyph_bitmap_enable_cache;
        dsc->font.line_height = (face_size->face->size->metrics.height >> 6);  //向右移位6次，取整数部分
        dsc->font.base_line = -(face_size->face->size->metrics.descender >> 6);//向右移位6次，取整数部分
    }

    dsc->font.subpx = LV_FONT_SUBPX_NONE;
    dsc->font.dsc = dsc;

    return &dsc->font;

create_err:
    free(dsc);
    return NULL;
}

//销毁指定字形
void ttf_font_destory(lv_font_t *font)
{
    ttf_font_dsc_t *dsc = font->dsc;
    free(dsc);
}

#endif