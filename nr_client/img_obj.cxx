#include "game.hxx"
#include "listbox.hxx"
#include "exceptions.hxx"
#include "debug.hxx"

Listbox::Listbox(Game* _game, const char* _name, Images fnt) : GameObject(_game, _name)
{
    obj_type = GO_Listbox;
    geo=get_geo_by_name(name);
    if(!geo)
	throw ExNoGeoInfo(name);
    font_id=fnt;
    rgeo.w=geo->l;
    rgeo.h=geo->h;
    rgeo.x=geo->fromx;
    rgeo.y=geo->fromy;

    image=game->new_surface(geo->l, geo->h);

    view_step_y = game->font_height(font_id) + 1;
    view_items=rgeo.h / view_step_y;
    base_idx=cur_idx=0;
}

bool Listbox::show()
{
    if(shown)
	return true;
    shown=true;

    game->redraw_area(rgeo);
    game->drawable_add(this, 1); //fromx in last geo -> display lvl

    copy_bg();

    for(int i=0;i<view_items;i++)
    {
	GRect rec=game->draw_text(image, font_id, 0, i*view_step_y, "Just test string");
	sge_UpdateRect(image, rec.x, rec.y, rec.w, rec.h);
    }

    DBG(5, "Listbox::show - show listbox object: " << name);
    return true;
}

void Listbox::copy_bg()
{
    sge_Blit(game->image(geo->im), image,
	     geo->fromx, 
	     geo->fromy,
	     0,
	     0,
	     geo->l, geo->h);
    sge_UpdateRect(image, 0, 0, rgeo.w, rgeo.h);
}

bool Listbox::hide()
{
    if(!shown)
	return true;
    shown=false;
    game->redraw_area(rgeo);
    game->drawable_del(this, 1);    
    return true;
}

bool Listbox::draw(RectInfo& rinfo)
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
	    dbgprintf(("listbox:draw_rect: from %d, %d to %d, %d %dx%d\n", 
		     -rgeo.x+rect.x, 
		     -rgeo.y+rect.y,
		     rect.x,
		     rect.y,
		     rect.w, rect.h));
	}
    return true;
}

Listbox::~Listbox()
{
}
