#include "game.hxx"
#include "cursor.hxx"
#include "exceptions.hxx"


Cursor::Cursor(Game* _game, const char* _name) : GameObject(_game, _name)
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
    spot_x=geo[max_frames].tox;
    spot_y=geo[max_frames].toy;
    x=y=0;
}

bool Cursor::show()
{
    if(shown)
	return true;
    shown=true;
    SDL_ShowCursor(SDL_DISABLE);
    game->add_object(this);
    game->redraw_area(frames[cur_frame]);
    game->drawable_add(this, geo[max_frames].fromx); //fromx in last geo -> display lvl
}

bool Cursor::hide()
{
    if(!shown)
	return true;
    shown=false;
    game->del_object(this);
    game->redraw_area(frames[cur_frame]);
    game->drawable_del(this, geo[max_frames].fromx);    
}

bool Cursor::draw(RectInfo& rinfo)
{
    GRect  rect;
    for(int i=0;i<rinfo.r_num;i++)
	if(collide(frames[cur_frame], rinfo.rects[i], rect))
	{
	    sge_Blit(image, Game::screen, 
		     geo[cur_frame].fromx+rect.x-frames[cur_frame].x, 
		     geo[cur_frame].fromy+rect.y-frames[cur_frame].y,
		     rect.x-spot_x,
		     rect.y-spot_y,
		     rect.w, rect.h);
	    dbgprintf(("Cursor::draw_rect: from %d, %d to %d, %d %dx%d\n", 
		     geo[cur_frame].fromx+rect.x-frames[cur_frame].x, 
		     geo[cur_frame].fromy+rect.y-frames[cur_frame].y,
		     rect.x-spot_x,
		     rect.y-spot_y,
		     rect.w, rect.h));
	}
}

Cursor::~Cursor()
{
    delete[] frames;
}

bool  Cursor::mouse_move_event(MouseEvent* mev)
{
    if(!shown)
	return false;
    if(mev->mx==x && mev->my==y)
	return false;

    game->redraw_area(x-spot_x, y-spot_y, frames[cur_frame].w, frames[cur_frame].h);
    frames[cur_frame].x=x=mev->mx;
    frames[cur_frame].y=y=mev->my;
    game->redraw_area(x-spot_x, y-spot_y, frames[cur_frame].w, frames[cur_frame].h);
    return true;
}

