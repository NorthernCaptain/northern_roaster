#ifndef SPRITE_HXX
#define SPRITE_HXX
#include "game_object.hxx"

class Game;

class Sprite: public GameObject
{
    Geo             *geo;
    SDL_Surface     *image;
    GRect           *frames;
    int             max_frames, cur_frame;
public:
    Sprite(Game* _game=0, const char* _name="");
    ~Sprite();

    virtual bool   show();
    virtual bool   draw(RectInfo&);
    virtual bool   hide();

    virtual bool   move_by(int dx, int dy);
};

#endif
