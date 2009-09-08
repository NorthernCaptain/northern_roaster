#include "game.hxx"
#include "graphview.hxx"
#include "exceptions.hxx"
#include "debug.hxx"
#include "configman.hxx"

#include "boost/lexical_cast.hpp"

const int text_delta=32;

Graphview::Graphview(ScanTaskPtr& scan, Game* _game, const char* _name, Images fnt_)
    : GameEditable(_game, _name), font_id(fnt_), scan_task(scan)
{
    last_offset=cursor_x=0;
    frozen_view=false;
    obj_type = GO_Graphview;
    geo=get_geo_by_name(name);
    if(!geo)
	throw ExNoGeoInfo(name);
    rgeo.w=geo->l; //width of the whole digits area
    rgeo.h=geo->h;
    rgeo.x=geo->fromx;
    rgeo.y=geo->fromy;

    image=game->new_surface(geo->l, geo->h);

    view_from_x=0;
    view_l=rgeo.w;

    view_from_y= CFGMAN::instance()->get_int_value("graphview", "view_start_offset_y", 100);
    view_h     = CFGMAN::instance()->get_int_value("graphview", "view_end_offset_y", 500);
    grid_step_y= CFGMAN::instance()->get_int_value("graphview", "grid_step_y", 50);
    grid_step_x= CFGMAN::instance()->get_int_value("graphview", "grid_step_x", 60);

    CFGMAN::instance()->set_int_value("graphview", "view_start_offset_y", view_from_y);
    CFGMAN::instance()->set_int_value("graphview", "view_end_offset_y", view_h);
    CFGMAN::instance()->set_int_value("graphview", "grid_step_y", grid_step_y);
    CFGMAN::instance()->set_int_value("graphview", "grid_step_x", grid_step_x);

    view_h -= view_from_y;

    grid_color=game->get_RGBA() + geo[1].fromy*4; //color offset for grid lines
    bg_color=game->get_RGBA() + geo[1].tox*4; //color offset for background
    cursor_color=game->get_RGBA() + geo[1].toy*4; //color offset for cursor line

    graphs.push_back(GraphicPtr(new GraphicAction(this, scan_task->get_storage(), t_flags,"main_graphic_3")));
    graphs.push_back(GraphicPtr(new Graphic(this, scan_task->get_storage(), t_input, "main_graphic_1")));
    graphs.push_back(GraphicPtr(new Graphic(this, scan_task->get_storage(), t_output,"main_graphic_2")));
}

bool Graphview::show()
{
    if(shown)
	return true;
    shown=true;

    game->add_object(this);
    game->redraw_area(rgeo);
    game->drawable_add(this, geo[1].fromx); //fromx in last geo -> display lvl

    draw_view();

    DBG(5, "Graphview::show - show graphview object: " << name);
    return true;
}

bool Graphview::hide()
{
    if(!shown)
	return true;
    shown=false;
    game->del_object(this);
    game->redraw_area(rgeo);
    game->drawable_del(this, geo[1].fromx);    
    return true;
}

void Graphview::redraw()
{
    if(!shown)
	return;
    int offset;
    {
	RoastStorage* store=scan_task->get_storage();
	ROAST_LOCK(store);
	offset=store->total_rows-2;
    }

    if(offset < 0) offset = 0; 
    if(view_from_x + view_l - view_l/10 < offset)
    {
	view_from_x= MAX2(offset - view_l + view_l/4, 0);
	if(!frozen_view)
	    cursor_x=offset - view_from_x;
	draw_view();
	game->redraw_area(rgeo);
	last_offset=offset;
    } else
    {
	DBG(5, "Graphview::redraw - draw part of view: " << last_offset
	    << " to " << offset);
	if(!frozen_view)
	    cursor_x=offset - view_from_x;
	draw_view_partly(last_offset, offset);
    }
}

void Graphview::full_redraw()
{
    if(!shown)
	return;
    int offset;
    {
	RoastStorage* store=scan_task->get_storage();
	ROAST_LOCK(store);
	offset=store->total_rows-2;
    }

    if(offset < 0) offset = 0; 
    view_from_x= MAX2(offset - view_l + view_l/4, 0);
    if(!frozen_view)
	cursor_x=offset - view_from_x;
    draw_view();
    game->redraw_area(rgeo);
    last_offset=offset;    
}

void Graphview::draw_view_partly(int from, int to)
{
    if(from < 0)
	from = 0;

    int len=to - from;
    if(len<=0)
	return;

    int vx=from - view_from_x;
    
    sge_Blit(game->image(geo->im), image,
	     geo->fromx+vx, 
	     geo->fromy,
	     vx,
	     0,
	     len+text_delta, geo->h);

    if(cursor_x >= vx && cursor_x <= vx + len)
    {
	sge_Line(image, 
		 cursor_x, 0, cursor_x, rgeo.h,
		 cursor_color[0],
		 cursor_color[1],
		 cursor_color[2]);
	
    }

    for(unsigned i=0;i<graphs.size();i++)
    {
	graphs[i]->draw_graph(vx-1, len+1+text_delta, cursor_x-1);
    }
    sge_UpdateRect(image, vx-1, 0, len+2+text_delta, rgeo.h);

    SDL_Rect lgeo;
    lgeo.x=vx+rgeo.x-1;
    lgeo.y=rgeo.y;
    lgeo.w=len+2+text_delta;
    lgeo.h=rgeo.h;
    game->redraw_area(lgeo);    
    
    last_offset=to;
}

void Graphview::draw_back()
{
    SDL_Surface* image=game->image(geo->im);
    int img_x=geo->fromx;
    int img_y=geo->fromy;

    sge_FilledRect(image, img_x, img_y, 
		   img_x+rgeo.w, img_y+rgeo.h,
		   bg_color[0],
		   bg_color[1],
		   bg_color[2]);

    if(grid_step_y) //draw grid here
    {
	int grids=view_h / grid_step_y;
	int g_off=(view_from_y + grid_step_y - 1 ) / grid_step_y;

	for(int i=0; i<grids;i++)
	{
	    int y=scale_y((g_off + i)*grid_step_y);
	    sge_Line(image, 
		     img_x, img_y+y, img_x+rgeo.w, img_y+y,
		     grid_color[0],
		     grid_color[1],
		     grid_color[2]);
	    game->draw_text(image, font_id, 
			    img_x, img_y+y, 
			    boost::lexical_cast<std::string>((g_off + i)*grid_step_y).c_str());
	}

    }

    if(grid_step_x) //draw grid here for x axis
    {
	int grids=view_l / grid_step_x;
	int g_off=grid_step_x - (view_from_x % grid_step_x);
	int g_mul=(view_from_x + g_off)/ grid_step_x;

	for(int i=0; i<grids;i++)
	{
	    int x=i*grid_step_x+g_off+img_x;
	    sge_Line(image, 
		     x, img_y, x, img_y+rgeo.h,
		     grid_color[0],
		     grid_color[1],
		     grid_color[2]);
	    if(x+text_delta-img_x < rgeo.w)
		game->draw_text(image, font_id, 
				x+1, img_y, 
				boost::lexical_cast<std::string>(float((g_mul + i)*grid_step_x)/60.0).c_str());
	}

    }

    sge_UpdateRect(image, img_x, img_y, rgeo.w, rgeo.h);
}

void Graphview::draw_view()
{
    draw_back();

    sge_Blit(game->image(geo->im), image,
	     geo->fromx, 
	     geo->fromy,
	     0,
	     0,
	     geo->l, geo->h);

    for(unsigned i=0;i<graphs.size();i++)
    {
	graphs[i]->set_view_x(view_from_x);
	graphs[i]->draw_graph(0, view_l);
    }
    sge_UpdateRect(image, 0, 0, rgeo.w, rgeo.h);
}

bool Graphview::draw(RectInfo& rinfo)
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
	}
    return true;
}

Graphview::~Graphview()
{
    game->delete_surface(image);
}


bool Graphview::key_press_event(SDL_Event* ev)
{
    switch(ev->key.keysym.sym)
    {
    case SDLK_RETURN:
	break;
    case SDLK_ESCAPE:
	escape_signal.emit();
	break;
    case SDLK_TAB:
	break;
    default:
	DBG(5, "Graphiew::unicode: " << ev->key.keysym.unicode);
	unprocessed_signal.emit(ev);
	break;
    }
    return true;    
}

///////////////////////////Graphic////////////////////////////////

Graphic::Graphic(Graphview* parent_, RoastStorage* store_, int column_,
		 const char* name):
    store(store_), column(column_), parent(parent_)
{
    view_offset=0;
    geo=get_geo_by_name(name);
    if(!geo)
	throw ExNoGeoInfo(name);
    
    color=parent->game->get_RGBA() + geo->fromy*4;
}

void Graphic::draw_graph(int from_x, int len, int cursor_x)
{
    if(from_x<0)
    {
	len += from_x;
	from_x = 0;
    }

    int x=view_offset + from_x;
    int last_offset;

    ROAST_LOCK(store);

    last_offset=store->total_rows -2;

    if(x > last_offset) //no such data - nothing to draw
	return;

    len = x + len < last_offset ? len : last_offset - x;

    for(int i=0;i<len;i++)
    {
	int val1=(*store)[i+x].get_int(column);
	int val2=(*store)[i+x+1].get_int(column);

	if(val1 < parent->view_from_y)
	    val1=parent->view_from_y;
	if(val2 < parent->view_from_y)
	    val2=parent->view_from_y;

	if(cursor_x == i + x - view_offset)
	{
	    sge_Rect(parent->image,
		     x - view_offset +i +1, 
		     parent->scale_y(val2),
		     x - view_offset +i + text_delta, 
		     parent->scale_y(val2)+parent->game->font_height(geo->im),
		     color[0],
		     color[1],
		     color[2]);
	    parent->game->draw_text(parent->image, geo->im, 
				    x - view_offset +i + 4, 
				    parent->scale_y(val2),
				    boost::lexical_cast<std::string>(val2).c_str());
	}
	
	sge_AALine(parent->image, 
		   x - view_offset +i, parent->scale_y(val1), 
		   x - view_offset +i +1, parent->scale_y(val2),
		   color[0],
		   color[1],
		   color[2]);
	sge_AALine(parent->image, 
		   x - view_offset +i, parent->scale_y(val1)+1, 
		   x - view_offset +i +1, parent->scale_y(val2)+1,
		   color[0],
		   color[1],
		   color[2]);
	sge_AALine(parent->image, 
		   x - view_offset +i, parent->scale_y(val1)+2, 
		   x - view_offset +i +1, parent->scale_y(val2)+2,
		   color[0],
		   color[1],
		   color[2]);
    }
}

///////////////////// GraphicAction - draw actions///////////////////////////

GraphicAction::GraphicAction(Graphview* parent_, RoastStorage* store_, int column_,
		 const char* name):
    Graphic(parent_, store_, column_, name) 
{
    unload_color=parent->game->get_RGBA() + geo->fromx*4;
}

void GraphicAction::draw_graph(int from_x, int len, int cursor_x)
{
    int x=view_offset + from_x;

    ROAST_LOCK(store);

    int last_offset=store->total_rows -2;

    if(x > last_offset) //no such data - nothing to draw
	return;

    if(x < 0)
	x = 0;

    len = x + len < last_offset ? len : last_offset - x;

    for(int i=0;i<len;i++)
    {
	int val1=(*store)[i+x].row[column];

	if(val1 == a_load_portion)
	{
	    sge_Line(parent->image, 
		     x - view_offset +i, 0, 
		     x - view_offset +i, parent->geo->h,
		     color[0],
		     color[1],
		     color[2]);
	}
	if(val1 == a_unload_portion)
	{
	    sge_Line(parent->image, 
		     x - view_offset +i, 0, 
		     x - view_offset +i, parent->geo->h,
		     unload_color[0],
		     unload_color[1],
		     unload_color[2]);
	}
    }
}
