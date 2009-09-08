#ifndef WELLFONT_H
#define WELLFONT_H

#include "types.h"

struct TextInfo
{
    FontType       fntype;
    int            text_h; // in pixels on the screen
    int            text_l; // in pixels on the screen
    int            text_len;    //in symbols
    TextInfo() { text_h=text_l=text_len=0;fntype=TFont1;};
};


class TextRef:protected TextInfo
{
 protected:
    const char     *text;
    int            text_x, text_y;
    int            center_x, center_y;
    virtual void   get_text_params(const char* text, 
				   TextInfo* txtnfo) {};

 public:
    TextRef(FontType fn, const char* itext, int ix=0, int iy=0);
    virtual ~TextRef() {};
    virtual void   init_text();
    virtual void   set_text_xy(int ix, int iy);
    virtual void   set_text_symbols(const char* new_text);
    virtual void   set_text_symbols_xy(const char* new_text, int ix, int iy);
    const char*    get_text() { return text;};

    virtual void   show_text(Images to_img) {};
    virtual void   clear_text_area(Images from_img, Images by_img) {};
};


class TextCopy:public TextRef
{
    bool           allocated;
 public:
    TextCopy(FontType fn, const char* itext, int ix=0, int iy=0);
    ~TextCopy();
    void           set_text_symbols(const char* new_text);

};

#endif
