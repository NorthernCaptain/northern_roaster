#ifndef DIGITLABEL_HXX
#define DIGITLABEL_HXX
#include "game_object.hxx"

#include <vector>
#include <string>
#include "boost/shared_ptr.hpp"

class Game;


class Digitlabel: public GameEditable
{
protected:
    Geo             *geo;
    SDL_Surface     *image;
    GRect            rgeo;

    int              value;

    int              view_step_y;
    int              view_items;

    virtual void     draw_view();

public:
    Digitlabel(int val=0, Game* _game=0, const char* _name="");
    ~Digitlabel();

    virtual bool     show();
    virtual bool     draw(RectInfo&);
    virtual bool     hide();

    virtual void     set_value(int val=0);

    void             my_focus() { GameObject::my_focus();};
};

typedef boost::shared_ptr<Digitlabel>             DigitlabelPtr;

#endif
