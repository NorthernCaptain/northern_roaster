#include "game.hxx"
#include "digitlabel.hxx"
#include "exceptions.hxx"
#include "debug.hxx"

const int frame_width=2;

Digitlabel::Digitlabel(int val, Game* _game, const char* _name) 
    : GameEditable(_game, _name), value(val)
{
    obj_type = GO_Digitlabel;
    geo=get_geo_by_name(name);
    if(!geo)
	throw ExNoGeoInfo(name);
    rgeo.w=geo->l; //width of the whole digits area
    rgeo.h=geo->h;
    rgeo.x=geo->fromx;
    rgeo.y=geo->fromy;

    image=game->new_surface(geo->l, geo->h);

    view_step_y = geo[1].l; //one digit width
    view_items=geo->l / view_step_y; //number of digits to display
}

bool Digitlabel::show()
{
    if(shown)
	return true;
    shown=true;

    game->redraw_area(rgeo);
    game->drawable_add(this, geo[1].fromx); //fromx in last geo -> display lvl

    draw_view();

    DBG(5, "Digitlabel::show - show digitlabel object: " << name);
    return true;
}

bool Digitlabel::hide()
{
    if(!shown)
	return true;
    shown=false;
    game->redraw_area(rgeo);
    game->drawable_del(this, geo[1].fromx);    
    return true;
}

void Digitlabel::draw_view()
{
    int val=value, n;
    int buf[100];
    sge_Blit(game->image(geo->im), image,
	     geo->fromx, 
	     geo->fromy,
	     0,
	     0,
	     geo->l, geo->h);
    for(int i=view_items-1;i>=0;i--)
    {
	n=val%10;
	buf[i]=n+1;
	val /= 10;
	if(n==0 && val==0 && i<view_items-1)
	    buf[i]=0;
    }

    for(int i=0; i<view_items;i++)
    {
	sge_Blit(game->image(geo[1].im), image,
		 view_step_y*buf[i], 0,
		 (view_step_y+geo[1].fromy)*i, 0,
		 geo[1].l, geo[1].h);
    }

    sge_UpdateRect(image, 0, 0, rgeo.w, rgeo.h);
}

bool Digitlabel::draw(RectInfo& rinfo)
{
    GRect  rect;

    for(int i=0;i<rinfo.r_num;i++)
	if(collide(rgeo, rinfo.rects[i], rect))
	{
	    sge_Blit(image, Game::screen, 
		     rect.x - rgeo.x, 
		     rect.y - rgeo.y,
		     rect.x,
		     rect.y,
		     rect.w, rect.h);
	    /*	    dbgprintf(("digitlabel:draw_rect: from %d, %d to %d, %d %dx%d\n", 
		     -rgeo.x+rect.x, 
		     -rgeo.y+rect.y,
		     rect.x,
		     rect.y,
		     rect.w, rect.h)); */
	}
    return true;
}

void Digitlabel::set_value(int val)
{
    value=val;
    draw_view();
    if(shown)
	game->redraw_area(rgeo);
}

Digitlabel::~Digitlabel()
{
    game->delete_surface(image);
}
