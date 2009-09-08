#include "game.hxx"
#include "movement.hxx"
#include "exceptions.hxx"


Movement::Movement(Game* _game, const char* _name) : GameObject(_game, _name), sprite(0)
{
    geo=get_geo_by_name(name);
    if(!geo)
	throw ExNoGeoInfo(name);
}

bool Movement::show()
{
    if(shown)
	return true;
    shown=true;
    game->add_timer(this, geo->fromx); //fromx in  geo -> timer duration
    return true;
}

bool Movement::hide()
{
    if(!shown)
	return true;
    shown=false;
    game->del_timer(this);
    return true;
}

bool Movement::time_out_event()
{
    dbgprintf(("Movement::time_out_event()\n"));
    if(sprite)
    {
	steps--;

	if(steps)
	{
	    int ldx=LRAND()%4 - 2;
	    int ldy=LRAND()%4 - 2;
	    sprite->move_by(-dx + ldx, -dy + ldy);
	    dx=ldx; dy=ldy;
	} else
	{
	    sprite->move_by(-dx, -dy);
	    sprite=0;
	    dx=dy=0;
	}
    }
    return false;
}


void Movement::start(Sprite* spr, ButState st)
{
    if(st==stHighlight)
    {
	sprite=spr;
	steps=10;
    }
    else
	sprite=0;
    dbgprintf(("Movement::start with state %d\n", st)); 
}

void Movement::stop(Sprite* spr, ButState st)
{
    sprite=0;
    dbgprintf(("Movement::stop with state %d\n", st)); 
}
