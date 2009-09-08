#include "game.hxx"
#include "sprite.hxx"
#include "exceptions.hxx"


Sprite::Sprite(Game* _game, const char* _name) : GameObject(_game, _name)
{
    geo=get_geo_by_name(name);
    if(!geo)
	throw ExNoGeoInfo(name);
    max_frames=0;
    for(Geo *ptr=geo; ptr->l!=0; ptr++)
	max_frames++;

    frames=new GRect[max_frames];
    image=game->image(geo->im);

    for(int i=0;i<max_frames;i++)
    {
	frames[i].x=geo[i].tox;
	frames[i].y=geo[i].toy;
	frames[i].w=geo[i].l;
	frames[i].h=geo[i].h;
    }
    cur_frame=0;
}

bool Sprite::show()
{
    if(shown)
	return true;
    shown=true;
    game->redraw_area(frames[cur_frame]);
    game->drawable_add(this, geo[max_frames].fromx); //fromx in last geo -> display lvl
    return true;
}

bool Sprite::hide()
{
    if(!shown)
	return true;
    shown=false;
    game->redraw_area(frames[cur_frame]);
    game->drawable_del(this, geo[max_frames].fromx);    
    return true;
}

bool Sprite::move_by(int dx, int dy)
{
    int i;

    if(dx==0 && dy==0)
	return false;
    if(shown)
	game->redraw_area(frames[cur_frame]);
    for(i=0;i<max_frames;i++)
    {
	frames[i].x+=dx;
	frames[i].y+=dy;
    }
    if(shown)
	game->redraw_area(frames[cur_frame]);
    return true;
}

bool Sprite::draw(RectInfo& rinfo)
{
    GRect  rect;
    for(int i=0;i<rinfo.r_num;i++)
	if(collide(frames[cur_frame], rinfo.rects[i], rect))
	{
	    sge_Blit(image, Game::screen, 
		     geo[cur_frame].fromx+rect.x-frames[cur_frame].x, 
		     geo[cur_frame].fromy+rect.y-frames[cur_frame].y,
		     rect.x,
		     rect.y,
		     rect.w, rect.h);
	    dbgprintf(("draw_rect: from %d, %d to %d, %d %dx%d\n", 
		     geo[cur_frame].fromx+rect.x-frames[cur_frame].x, 
		     geo[cur_frame].fromy+rect.y-frames[cur_frame].y,
		     rect.x,
		     rect.y,
		     rect.w, rect.h));
	}
    return true;
}

Sprite::~Sprite()
{
    delete[] frames;
}
