#include "game_object.hxx"
#include "game.hxx"
#include "exceptions.hxx"

bool GameObject::show()
{
    if(shown)
	return false;
    shown=true;
    return draw();
}

bool GameObject::draw()
{
    return false;
}

bool GameObject::hide()
{
    if(!shown)
	return false;
    shown=false;
    return true;
}

bool GameObject::process_any_event(wEvent ev)
{
    switch(ev.type)
    {
    case eMousePress:
	return mouse_press_event((MouseEvent*) ev.data);
    case eMouseUnPress:
	return mouse_unpress_event((MouseEvent*) ev.data);
    case eKeyPress:
	return key_press_event((SDL_Event*) ev.data);
    case eTimeOut:
	return time_out_event();
    case eMouseMove:
	return mouse_move_event((MouseEvent*) ev.data);
    case eUserEvent:
	return user_event((SDL_Event*) ev.data);
    case aIdleTimeout:
	return idle_time_out_event();
    default:
	break;
    }
    return true;
}

bool  GameObject::collide(GRect& one, GRect& two, GRect& diff)
{
    if(one.x + one.w < two.x) return false;
    if(one.y + one.h < two.y) return false;
    if(one.x > two.x + two.w +1) return false;
    if(one.y > two.y + two.h +1) return false;

    diff.x= MAX2(one.x, two.x);
    diff.y= MAX2(one.y, two.y);
    diff.w= MIN2(one.x+one.w+1, two.x+two.w+1) - diff.x;
    diff.h= MIN2(one.y+one.h+1, two.y+two.h+1) - diff.y;

    return true;
}


void GameObject::my_focus()
{
    next_focus_signal.emit();
}

///////////////////////////////GameEditable///////////////////////////////
GameEditable::GameEditable(Game* _game, const char* _name): GameObject(_game, _name)
{
    focused=false;
}


void GameEditable::my_focus()
{
    game->set_focus(this);
}

////////////////////////////////ImageObject///////////////////////////////

ImageObject::ImageObject(Game *_game, SDL_Surface* _face): GameObject(_game),
							   face(_face)
{
}

ImageObject::ImageObject(const char* fname, Game *_game): GameObject(_game),
							  face(0)
{
    obj_type=GO_Image;
    face=load_image(fname);
    if(!face)
	throw ExCantLoad(fname);
    size.w=face->w;
    size.h=face->h;
    size.x=size.y=0;
}


ImageObject::~ImageObject()
{
    if(face)
	SDL_FreeSurface(face);
}

SDL_Surface* ImageObject::load_image(const char* fname)
{
    SDL_Surface *img=IMG_Load(fname), *face;
    if(!img)
	return 0;
    SDL_SetAlpha(img, SDL_SRCALPHA, 255);
    face = SDL_DisplayFormatAlpha(img);
    SDL_FreeSurface(img);
    return face;
}

bool ImageObject::draw()
{
    game->redraw_area(size);
    return true;
}

bool ImageObject::draw(RectInfo& rinfo)
{
    GRect  rect;
    for(int i=0;i<rinfo.r_num;i++)
	if(collide(size, rinfo.rects[i], rect))
	{
	    sge_Blit(face, Game::screen, 
		     rect.x-size.x, 
		     rect.y-size.y,
		     rect.x,
		     rect.y,
		     rect.w, rect.h);
	}
    return true;
}

