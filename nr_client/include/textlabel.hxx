#ifndef TEXTLABEL_HXX
#define TEXTLABEL_HXX
#include "game_object.hxx"

#include <vector>
#include <string>
#include "boost/shared_ptr.hpp"

class Game;

enum TextJustify { JustLeft, JustRight, JustCenter};

class Textlabel: public GameEditable
{
protected:
    Geo             *geo;
    SDL_Surface     *image;
    GRect            rgeo;
    Images           font_id;

    std::string      data;

    int              view_step_y;
    int              view_items;
    int              base_idx;
    int              cur_idx;

    TextJustify      just;

    virtual void     draw_view();

public:
    Textlabel(const std::string& text="", Game* _game=0, const char* _name="", Images fnt=imFont1);
    ~Textlabel();

    virtual bool     show();
    virtual bool     draw(RectInfo&);
    virtual bool     hide();

    virtual void     set_text(const std::string& new_data);
    void             justify_mode(TextJustify j) { just=j;};

    void             my_focus() { GameObject::my_focus();};
};

typedef boost::shared_ptr<Textlabel>             TextlabelPtr;

#endif
