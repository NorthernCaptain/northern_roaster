/*
 * png_save.c
 *
 * You know, I can't remember where I got this code. I'm pretty sure
 * it was freely posted on a mailing list.
 *
 * If you are the author, and object to its redistribution, let me know.
 *
 */

#include "SDL.h"
#include "SDL_image.h"
#include "png.h"
#include <stdlib.h>

/* Save a PNG type image to an SDL datasource */
static void png_write_data(png_structp ctx, png_bytep area, png_size_t size)
{
	SDL_RWops *src;

	src = (SDL_RWops *)png_get_io_ptr(ctx);
	SDL_RWwrite(src, area, size, 1);
}

static void png_io_flush(png_structp ctx)
{
	SDL_RWops *src;

	src = (SDL_RWops *)png_get_io_ptr(ctx);
	/* how do I flush src? */
}

static int png_colortype_from_surface(SDL_Surface *surface)
{
	int colortype = PNG_COLOR_MASK_COLOR; /* grayscale not supported */

	if (surface->format->palette)
		colortype |= PNG_COLOR_MASK_PALETTE;
	else if (surface->format->Amask)
		colortype |= PNG_COLOR_MASK_ALPHA;
		
	return colortype;
}

static void png_user_warn(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: warning: %s\n", str);
}

static void png_user_error(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: error: %s\n", str);
}

int IMG_SavePNG_RW(SDL_Surface *surface, SDL_RWops *src)
{
  png_structp png_ptr = 0;
  png_infop info_ptr = 0;
  png_colorp palette = 0;
  png_bytep *row_pointers = 0;
  int i;
  int colortype;
  int result = -1;
  
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, png_user_error, png_user_warn);
  
  if (png_ptr == NULL)
    {
      IMG_SetError("Couldn't allocate memory for PNG file");
      return -1;
    }
  
  /* Allocate/initialize the image information data.  REQUIRED */
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL)
    {
      IMG_SetError("Couldn't create image information for PNG file");
      goto done;
    }
  
  /* Set error handling. */
  if (setjmp(png_ptr->jmpbuf))
    {
      /* If we get here, we had a problem reading the file */
      IMG_SetError("Error writing the PNG file");
      goto done;
    }
  
  png_set_write_fn(png_ptr, src, png_write_data, png_io_flush);
  
  /* Set the image information here.  Width and height are up to 2^31,
   * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
   * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
   * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
   * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
   * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
   * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
   */
  colortype = png_colortype_from_surface(surface);
  png_set_IHDR(png_ptr, info_ptr, surface->w, surface->h, 8,
	       colortype, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  
  /* set the palette if there is one.  REQUIRED for indexed-color images */
  if (colortype & PNG_COLOR_MASK_PALETTE)
    {
      palette = (png_colorp)malloc(surface->format->palette->ncolors * sizeof (png_color));
      
      if (!palette)
	{
	  IMG_SetError("Couldn't allocate memory for PNG palette");
	  goto done;
	}
      
      for (i = 0; i < surface->format->palette->ncolors; i++)
	{
	  palette[i].red   = surface->format->palette->colors[i].r;
	  palette[i].green = surface->format->palette->colors[i].g;
	  palette[i].blue  = surface->format->palette->colors[i].b;
	}
      
      png_set_PLTE(png_ptr, info_ptr, palette, surface->format->palette->ncolors);
    }
  
  /* Write the file header information.  REQUIRED */
  png_write_info(png_ptr, info_ptr);
  
  /* pack pixels into bytes */
  png_set_packing(png_ptr);
  
  /* Create the array of pointers to image data */
  row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*surface->h);
  
  if ( (row_pointers == NULL) ) 
    {
      IMG_SetError("Couldn't allocate PNG row pointers");
      goto done;
    }
  
  for (i = 0; i < surface->h; i++)
    row_pointers[i] = (png_bytep)(Uint8 *)surface->pixels + i*surface->pitch;
  
  /* write out the entire image data in one call */
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);
  result = 0;  /* success! */
  
 done:
  if (row_pointers)
    free(row_pointers);
  
  if (info_ptr->palette)
    free(info_ptr->palette);
  
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  
  return result;
}
