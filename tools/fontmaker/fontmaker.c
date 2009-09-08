#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "SDL.h"
#include "SDL_image.h"

#define IMG_SavePNG(surface, file) \
               IMG_SavePNG_RW(surface, SDL_RWFromFile(file, "wb"))

extern int IMG_SavePNG_RW(SDL_Surface *surface, SDL_RWops *src);

Uint32 GetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y);
void   PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
 

SDL_Rect checkrect (SDL_Surface *surface);
SDL_Surface * Create(int width,int height,SDL_Surface* from);
void DrawLine(SDL_Surface *surface);
void copy_area(SDL_Surface* from, SDL_Rect* r, SDL_Surface* to);

int main(int argc, char *argv[])
{
	SDL_Surface *old_font;
	SDL_Rect rec;

	SDL_Surface * newsurf;

    if (argc < 3) {
        printf("%s - converts a PNG image in a BFont font image (PNG)!\n",argv[0]);
        printf("%s - modified by Leo to provide PNG support for transparency\n", argv[0]);
        printf("USAGE: %s <in.png> <output.png>\n",argv[0]);
        exit(1);
    }

    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
	    fprintf(stderr,"Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
	}
    atexit(SDL_Quit);
	
    if ( (old_font = IMG_Load(argv[1])) != NULL) {
	printf("%s - [%s] -> font [%s]\n", argv[0], argv[1], argv[2]);
	SDL_SetAlpha(old_font, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);

	rec = checkrect(old_font);
	
	newsurf = Create(rec.w,rec.h,old_font);
	
	copy_area(old_font, &rec, newsurf);

	DrawLine(newsurf);
	
	IMG_SavePNG(newsurf,argv[2]);

    } else
	printf("%s - can't load source image file: %s\n", argv[0], argv[1]);

    SDL_Quit();

    return 0;
}

void DrawLine(SDL_Surface *surface)
{
	int x,y,stop;

	Uint32 background;
	Uint32 pixel;
	Uint8 r,g,b;

	background = GetPixel(surface,0,surface->h-1);

	SDL_GetRGB(background, surface->format, &r,&g,&b);

	x=0;
	r=255;g=0; b=255;
	while (x<surface->w) {
		pixel = SDL_MapRGB(surface->format,r,g,b);
		PutPixel(surface,x,0,pixel);
		x++;
	}

	x=0;
	while (x < surface->w) {
		y=1; stop=0;
		while ((y < surface->h) && (!stop)) {
			pixel = GetPixel(surface,x,y);
			if (pixel != background)
				stop = 1;
			y++;
		}
		if (y < surface->h) {
			PutPixel(surface,x,0,background);
		}
		x++;
	}

	x=0;
	while (x < surface->w) {
		pixel = GetPixel(surface,x,0);
		if (pixel == background) {
			PutPixel(surface,x-1,0,background);
			while (pixel == background)	{
				x++;
				pixel = GetPixel(surface,x,0);
			}
			PutPixel(surface,x,0,background);
		}
		x++;
	}
}

void copy_area(SDL_Surface* from, SDL_Rect* r, SDL_Surface* to)
{
    int x,y;
    Uint32 color, bg;
    bg = GetPixel(from,0,from->h-1);
    
    for(y=0;y<r->h;y++)
	for(x=0;x<r->w;x++)
	{
	    color=GetPixel(from, x+r->x, y+r->y);
	    if(color!=bg)
		PutPixel(to, x, y, color);
	}
}


SDL_Rect checkrect (SDL_Surface *surface)
{
	SDL_Rect rect;

	int x,y;
	int stop=0;

	Uint32 background;
	Uint32 pixel;
	Uint8 r,g,b;

	rect.x=0;
	rect.w= surface->w;

	background = GetPixel(surface,0,surface->h-1);

	SDL_GetRGB(background, surface->format, &r,&g,&b);

	y = 0;
	while ( ( y < surface->h ) && (!stop)) {
		x = 0;
		while ((x < surface->w) && (!stop)) {
			pixel = GetPixel(surface,x,y);
			if (pixel != background)
				stop = 1;
			x++;
		}
		y++;
	}

	rect.y = y - 2;

	stop = 0;
	y = surface->h-1;
	while ( ( y > 0) && (!stop)) {

		x = 0;
		while ((x < surface->w) && (!stop)) {
			pixel = 0;
			pixel = GetPixel(surface,x,y);

			r=g=b=0;
			SDL_GetRGB(pixel, surface->format, &r,&g,&b);

			if (pixel != background)
				stop = 1;
			x++;
		}
		if (!stop)
			y--;
	}
	y++;
	rect.h = y - rect.y + 2;

	return rect;
}


SDL_Surface * Create(int width,int height,SDL_Surface *from)
{

    SDL_Surface *surface;
    
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA, 
				   width, height, 
				   from->format->BitsPerPixel, 
				   from->format->Rmask, 
				   from->format->Gmask, 
				   from->format->Bmask, 
				   from->format->Amask);
    if(surface == NULL) {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    return surface;
}


void   PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

Uint32 GetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y)
{

   Uint8  *bits;
   Uint32 Bpp;

   if (X<0) puts("x too small in GetPixel!");
   if (X>=Surface->w) puts("x too big in GetPixel!");

   Bpp = Surface->format->BytesPerPixel;

   bits = ((Uint8 *)Surface->pixels)+Y*Surface->pitch+X*Bpp;

   // Get the pixel
   switch(Bpp) {
      case 1:
         return *((Uint8 *)Surface->pixels + Y * Surface->pitch + X);
         break;
      case 2:
         return *((Uint16 *)Surface->pixels + Y * Surface->pitch/2 + X);
         break;
      case 3: { // Format/endian independent
         Uint8 r, g, b;
         r = *((bits)+Surface->format->Rshift/8);
         g = *((bits)+Surface->format->Gshift/8);
         b = *((bits)+Surface->format->Bshift/8);
         return SDL_MapRGB(Surface->format, r, g, b);
         }
         break;
      case 4:
         return *((Uint32 *)Surface->pixels + Y * Surface->pitch/4 + X);
         break;
   }

    return -1;
}

