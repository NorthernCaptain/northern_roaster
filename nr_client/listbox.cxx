#include "game.hxx"
#include "listbox.hxx"
#include "exceptions.hxx"
#include "debug.hxx"

const int frame_width=2;

Listbox::Listbox(Game* _game, const char* _name, Images fnt) : GameEditable(_game, _name)
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
    cursor_color=game->get_RGBA()+geo[1].fromy*4;

    view_step_y = game->font_height(font_id) + frame_width*2;
    view_items=rgeo.h / view_step_y;
    base_idx=cur_idx=0;
}

bool Listbox::show()
{
    if(shown)
	return true;
    shown=true;

    game->drawable_add(this, geo[1].fromx); //fromx in last geo -> display lvl

    draw_view();

    DBG(5, "Listbox::show - show listbox object: " << name);
    return true;
}

void Listbox::redraw()
{
    if(!shown)
	return;
    draw_view();
    game->redraw_area(rgeo);
}

ListboxItem Listbox::get_current()
{
    if((std::size_t)(cur_idx+base_idx) >= data.size())
	return ListboxItem();
    return data[cur_idx+base_idx];
}

bool Listbox::key_press_event(SDL_Event* ev)
{
    switch(ev->key.keysym.sym)
    {
    case SDLK_UP:
	cursor_up();
	break;
    case SDLK_DOWN:
	cursor_down();
	break;
    case SDLK_RETURN:
	DBG(4, "Listbox::activate - entered text: " << data[cur_idx+base_idx].descr);
	activate_signal.emit(data[cur_idx+base_idx]);
	break;
    case SDLK_TAB:
	next_focus_signal.emit();
	break;
    case SDLK_ESCAPE:
	escape_signal.emit();
	break;
    default:
	unprocessed_signal.emit(ev);
	break;
    }
    return true;
}

bool Listbox::cursor_up()
{
    if(cur_idx + base_idx==0)
	return false;

    if(cur_idx==0)
    {
	base_idx--;
	draw_view();
	game->redraw_area(rgeo);
    }
    else
    {
	cur_idx--;
	draw_item(cur_idx+1, base_idx+cur_idx+1);
	draw_item(cur_idx, base_idx+cur_idx);
	
	GRect rec;
	rec.x=rgeo.x;
	rec.y=rgeo.y+cur_idx*view_step_y;
	rec.w=rgeo.w;
	rec.h=view_step_y*2;
	game->redraw_area(rec);
    }

    item_selected_signal.emit(data[cur_idx+base_idx]);

    return true;
}

bool Listbox::cursor_down()
{
    if(unsigned(cur_idx+base_idx+1) >= data.size())
	return false;

    if(cur_idx+1 >= view_items)
    {
	base_idx++;
	draw_view();
	game->redraw_area(rgeo);
    }
    else
    {
	cur_idx++;
	draw_item(cur_idx - 1, base_idx+cur_idx - 1);
	draw_item(cur_idx, base_idx+cur_idx);
	
	GRect rec;
	rec.x=rgeo.x;
	rec.y=rgeo.y + (cur_idx-1)*view_step_y;
	rec.w=rgeo.w;
	rec.h=view_step_y*2;
	
	game->redraw_area(rec);
    }

    item_selected_signal.emit(data[cur_idx+base_idx]);

    return true;
}

void Listbox::draw_view()
{
    copy_bg();
    
    int len= unsigned(view_items) < data.size() - base_idx ? view_items : data.size() - base_idx;
    
    for(int i=0;i<len;i++)
    {
	draw_item(i, base_idx+i);
    }
}

void Listbox::draw_item(int view_pos, int idx)
{
    sge_Blit(game->image(geo->im), image,
	     geo->fromx, 
	     geo->fromy + view_pos*view_step_y,
	     0,
	     view_pos*view_step_y,
	     geo->l, view_step_y);
    if(view_pos==cur_idx)
    {
	if(focused)
	    sge_FilledRect(image, 
			   0, view_pos*view_step_y, 
			   geo->l - 1, 
			   view_pos*view_step_y + view_step_y - 1, 
			   cursor_color[0],
			   cursor_color[1],
			   cursor_color[2]);
	else
	{
	    sge_Rect(image, 
		     0, view_pos*view_step_y, 
		     geo->l - 1, 
		     view_pos*view_step_y + view_step_y - 1, 
		     cursor_color[0],
		     cursor_color[1],
		     cursor_color[2]);
	    sge_Rect(image, 
		     1, view_pos*view_step_y+1, 
		     geo->l - 2, 
		     view_pos*view_step_y + view_step_y - 2, 
		     cursor_color[0],
		     cursor_color[1],
		     cursor_color[2]);
	}
    }

    GRect rec=game->draw_text(image, font_id, 
			      frame_width, 
			      view_pos*view_step_y+frame_width, 
			      data[idx].descr.c_str());
    sge_UpdateRect(image, 0, view_pos*view_step_y, rgeo.w, view_step_y);	
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
    game->drawable_del(this, geo[1].fromx);    
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
	}
    return true;
}

void Listbox::set_focus()
{
    GameObject::set_focus();

    if(!shown)
	return;

    draw_item(cur_idx, base_idx+cur_idx);
    
    GRect rec;
    rec.x=rgeo.x;
    rec.y=rgeo.y + cur_idx*view_step_y;
    rec.w=rgeo.w;
    rec.h=view_step_y*2;
    
    game->redraw_area(rec);
}

void Listbox::unset_focus()
{
    GameObject::unset_focus();

    if(!shown)
	return;

    draw_item(cur_idx, base_idx+cur_idx);
    
    GRect rec;
    rec.x=rgeo.x;
    rec.y=rgeo.y + cur_idx*view_step_y;
    rec.w=rgeo.w;
    rec.h=view_step_y*2;
    
    game->redraw_area(rec);
}

Listbox::~Listbox()
{
    game->delete_surface(image);
}


void Listbox::set_cursor_by_item_idx(const std::string& idx)
{
    for(unsigned int i=0;i<data.size();i++)
	if(data[i].id==idx)
	{
	    cur_idx=i;
	    base_idx=cur_idx/view_items*view_items;
	    cur_idx = cur_idx - base_idx;
	    draw_view();
	    game->redraw_area(rgeo);
	}
}


void  Listbox::sort()
{
    std::sort(data.begin(), data.end());
}
