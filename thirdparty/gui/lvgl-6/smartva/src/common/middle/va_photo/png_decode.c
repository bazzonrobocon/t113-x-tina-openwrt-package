#include "png.h"
#include "jmorecfg.h"

#ifndef png_infopp_NULL
#define png_infopp_NULL NULL
#endif
#ifndef int_p_NULL
#define int_p_NULL (int *)NULL
#endif

int png_decode(char *filename,void *output_buf,int *width,int *height,int *comp)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 img_width, img_height;
	int bit_depth, color_type, interlace_type,channels;
	FILE *fp;
	int number_passes = 1;
	// unsigned char *p_buf = NULL;

	if((fp = fopen(filename, "rb")) == NULL)
		return -1;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		fclose(fp);
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		printf("png decode err!\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		fclose(fp);
		return -1;
	}

	png_init_io(png_ptr, fp);


	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &img_width, &img_height, &bit_depth, &color_type,
				&interlace_type, int_p_NULL, int_p_NULL);
	channels = png_get_channels(png_ptr,info_ptr);

	printf("width  = %ld\n",img_width);
	printf("height = %ld\n",img_height);
	printf("depth  = %d\n",bit_depth);
	printf("channels = %d\n",channels);
	printf("color_type = %d\n",color_type);
	printf("interlace_type = %d\n",interlace_type);


    png_color_16 my_background, *image_background;

    if (png_get_bKGD(png_ptr, info_ptr, &image_background))
		png_set_background(png_ptr, image_background,
                         PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
    else
		png_set_background(png_ptr, &my_background,
                         PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

   /* Flip the RGB pixels to BGR (or RGBA to BGRA) */
	// if (color_type & PNG_COLOR_MASK_COLOR)
	//	png_set_bgr(png_ptr);

	if(color_type==PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);//要求转换索引颜色到RGB
	}

	if(color_type==PNG_COLOR_TYPE_GRAY && bit_depth<8)
	{
		png_set_expand_gray_1_2_4_to_8(png_ptr);//要求位深度强制8bit
	}

	if(bit_depth==16)
	{
		png_set_strip_16(png_ptr);//要求位深度强制8bit
	}

	if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))//要求转换到BGRA
	{
		png_set_tRNS_to_alpha(png_ptr);
		printf("png_get_valid\n");
	}

	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png_ptr);//灰度必须转换成RGB
	}

	number_passes = png_set_interlace_handling(png_ptr);//1 or 7

	printf("number_passes = %d\n",number_passes);

	png_read_update_info(png_ptr, info_ptr);

	// int rowbytes = png_get_rowbytes(png_ptr,info_ptr);
	// int pass,y;
	// for (pass = 0; pass < number_passes; pass++)
	// {
	//	p_buf = (unsigned char *)output_buf;
	//	for (y = 0; y < img_height; y++)
	//	{
	//		png_read_rows(png_ptr, &p_buf, png_bytepp_NULL, 1);
	//		p_buf += rowbytes;
	//	}
    // }
	// channels = rowbytes/img_width;

	int rowbytes = png_get_rowbytes(png_ptr,info_ptr);
	png_bytep rptr[2];
	int pass,i;
	unsigned char *fbptr;
	channels = rowbytes/img_width;
	for (pass = 0; pass < number_passes; pass++)
	{
		fbptr = output_buf;
		for(i=0; i<img_height; i++)
		{
			rptr[0] = (png_bytep) fbptr;
			png_read_rows(png_ptr, rptr, NULL, 1);
			fbptr += img_width*channels;
		}
	}


	png_read_end(png_ptr, info_ptr);


	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);


	fclose(fp);

	*width = img_width;
	*height = img_height;
	*comp = channels;
	if(channels != 3 && channels != 4)
	{
		return -1;
	}
	return 0;
}
