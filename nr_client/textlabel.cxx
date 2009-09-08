#include "game.hxx"
#include "textlabel.hxx"
#include "exceptions.hxx"
#include "debug.hxx"

const int frame_width=2;

Textlabel::Textlabel(const std::string& text, Game* _game, const char* _name, Images fnt) 
    : GameEditable(_game, _name), data(text)
{
    obj_type = GO_Textlabel;
    geo=get_geo_by_name(name);
    if(!geo)
	throw ExNoGeoInfo(name);
    font_id=fnt;
    rgeo.w=geo->l;
    rgeo.h=geo->h;
    rgeo.x=geo->fromx;
    rgeo.y=geo->fromy;

    image=game->new_surface(geo->l, geo->h);

    view_step_y = rgeo.h/2 - game->font_height(font_id)/2;
    view_items=rgeo.h / view_step_y;
    base_idx=cur_idx=0;

    just=JustLeft;
}

bool Textlabel::show()
{
    if(shown)
	return true;
    shown=true;

    game->redraw_area(rgeo);
    game->drawable_add(this, geo[1].fromx); //fromx in last geo -> display lvl

    draw_view();

    DBG(5, "Textlabel::show - show textlabel object: " << name);
    return true;
}

bool Textlabel::hide()
{
    if(!shown)
	return true;
    shown=false;
    game->redraw_area(rgeo);
    game->drawable_del(this, geo[1].fromx);    
    return true;
}

void Textlabel::draw_view()
{
    sge_Blit(game->image(geo->im), image,
	     geo->fromx, 
	     geo->fromy,
	     0,
	     0,
	     geo->l, geo->h);
    switch(just)
    {
    case JustLeft:
	game->draw_text(image, font_id, 
			0, view_step_y, 
			data.c_str());
	break;
    case JustRight:
	{
	    int tx=rgeo.w - game->font_width(font_id, data.c_str());
	    game->draw_text(image, font_id, 
			    tx>0 ? tx : 0, view_step_y, 
			    data.c_str());	    
	}
	break;
    case JustCenter:
	{
	    int tx=rgeo.w/2 - game->font_width(font_id, data.c_str())/2;
	    game->draw_text(image, font_id, 
			    tx>0 ? tx : 0, view_step_y, 
			    data.c_str());
	}
	break;
    default:
	break;
    }

    sge_UpdateRect(image, 0, 0, rgeo.w, rgeo.h);
}

bool Textlabel::draw(RectInfo& rinfo)
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
	    /*	    dbgprintf(("textlabel:draw_rect: from %d, %d to %d, %d %dx%d\n", 
		     -rgeo.x+rect.x, 
		     -rgeo.y+rect.y,
		     rect.x,
		     rect.y,
		     rect.w, rect.h));*/
	}
    return true;
}

void Textlabel::set_text(const std::string& str)
{
    data=str;
    draw_view();
    if(shown)
	game->redraw_area(rgeo);
}

Textlabel::~Textlabel()
{
    game->delete_surface(image);
}
