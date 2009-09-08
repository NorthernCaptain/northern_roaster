#ifndef CURSOR_HXX
#define CURSOR_HXX
#include "game_object.hxx"

class Game;

class Cursor: public GameObject
{
    Geo             *geo;
    SDL_Surface     *image;
    GRect           *frames;
    int             max_frames, cur_frame;
    int             x,y, spot_x, spot_y;
public:
    Cursor(Game* _game=0, const char* _name="");
    ~Cursor();

    virtual bool   show();
    virtual bool   draw(RectInfo&);
    virtual bool   hide();
    
    bool           mouse_move_event(MouseEvent* mev);
};

#endif
