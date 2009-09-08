#ifndef MOVEMENT_HXX
#define MOVEMENT_HXX
#include "button.hxx"

class Movement : public GameObject
{
    Sprite       *sprite;
    int           orig_x, orig_y;
    Geo          *geo;
    int          dx, dy;
    int          steps;

    virtual bool  time_out_event();

public:
    Movement(Game* _game=0, const char* _name="");
    virtual bool   show();
    virtual bool   draw(RectInfo&) {return true;};
    virtual bool   hide();

    virtual void   start(Sprite*, ButState);
    virtual void   stop(Sprite*, ButState);
};

#endif
