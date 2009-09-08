#ifndef LISTBOX_HXX
#define LISTBOX_HXX
#include "game_object.hxx"

class Game;

class Listbox: public GameObject
{
    Geo             *geo;
    SDL_Surface     *image;
    GRect            rgeo;
    Images           font_id;

    int              view_step_y;
    int              view_items;
    int              base_idx;
    int              cur_idx;

    void             copy_bg();

public:
    Listbox(Game* _game=0, const char* _name="", Images fnt=imFont1);
    ~Listbox();

    virtual bool     show();
    virtual bool     draw(RectInfo&);
    virtual bool     hide();

};

#endif
