#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "types.h"

#define GEO_NAME          20

struct Geo
{
    Images           im;
    int              fromx, fromy;
    int              tox,toy;
    unsigned int     l,h;
    inline void      init(Images from_im=imNone, int ix=0, int iy=0, 
			  unsigned int il=0, unsigned int ih=0)
    {
	im=from_im;
	fromx=ix;
	fromy=iy;
	l=il;
	h=ih;
    };
};

struct GeoObject
{
  char             name[GEO_NAME];
  Geo              *geo;
};

extern Geo* get_geo_by_name(const char* name);
extern Geo* get_mini_geo_by_name(const char* name);

#endif
