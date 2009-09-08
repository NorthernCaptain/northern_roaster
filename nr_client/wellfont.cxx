// docm_prefix(///)
/****************************************************************************
*  Copyright (C) 2002 by Leo Khramov
*  email:   leo@xnc.dubna.su
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
 ****************************************************************************/
/// module description
/// Module contains class methods Text operations
#include "wellfont.h"
#include "ace/OS.h"

/////////////////// TexRef class methods //////////////////////
TextRef::TextRef(FontType fn, const char* itext, int ix, int iy)
{
    text_x=ix;text_y=iy;
    fntype=fn;
    set_text_symbols(itext);
}

void TextRef::init_text()
{
    get_text_params(text, this);
    center_x=text_x+text_l/2;
    center_y=text_y+text_h/2;
}

void TextRef::set_text_xy(int ix, int iy)
{
    text_x=ix;text_y=iy;
    center_x=text_x+text_l/2;
    center_y=text_y+text_h/2;
}

void TextRef::set_text_symbols(const char* txt)
{
    text=txt;
    init_text();
}

void TextRef::set_text_symbols_xy(const char* txt, int ix, int iy)
{
    text_x=ix;
    text_y=iy;
    set_text_symbols(txt);
}


/////////////////////////////TextCopy class////////////////////////////
TextCopy::TextCopy(FontType fn, const char* itext, int ix, int iy):
    allocated(false),TextRef(fn, itext, ix, iy)
{
}


TextCopy::~TextCopy()
{
    delete[] text;
}

void TextCopy::set_text_symbols(const char* new_text)
{
    char *ptr;
    if(allocated)
	delete[] text;
    ptr=new char[ACE_OS::strlen(new_text)+1];
    ACE_OS::strcpy(ptr, new_text);
    TextRef::set_text_symbols(ptr);
}
