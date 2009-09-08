#include "game.hxx"
#include "game_profile.h"
#include "commonfuncs.h"
#include "game_object.hxx"
#include "debug.hxx"
#include "configman.hxx"

SDL_Surface* Game::screen=0;
SDL_Surface* Game::bg=0;

const unsigned long TIMESTEP=10l;
const int KEY_TICKS=5;
const int FIRST_KEY_TICKS=25;

Game::Game() : post_event_object(0)
{
    int i;
    focus_object=0;
    init_video();
    InitRNG();
    objects=new GameVec;
    draw_rects.rects=new GRect[max_draw_rects];
    draw_rects.r_num=0;
    done=false;
    for(i=0;i<max_images;i++)
    {
	image_bank[i]=0;
	fonts[i]=0;
    }

    init();

    key_event_got=false;

    idle_timeout_object = 0;
    idle_time = 0;
}


void Game::init()
{
    load_images();
    init_gameplay();
}

SDL_Surface* Game::load_image(const char* fname)
{
    char buf[1024];
    if(!find_full_path_for_file(fname, buf, ReadOnly))
       return 0;
    DBG(4, "LoadImage: " << buf);
    SDL_Surface *img=IMG_Load(buf), *face;
    SDL_SetAlpha(img, SDL_SRCALPHA, 255);
    face = SDL_DisplayFormatAlpha(img);
    SDL_FreeSurface(img);
    return face;
}


Game::~Game()
{
    delete[] draw_rects.rects;
    delete objects;
    for(int i=0;i<max_images;i++)
	if(image_bank[i])
	    SDL_FreeSurface(image_bank[i]);
}

bool Game::show()
{
    sge_Update_OFF();
    sge_Blit(bg, screen, 0, 0, 0, 0, bg->w, bg->h);

    for(unsigned int i=0;i<objects->size();i++)
	(*objects)[i]->show();

    flush_update();
    return true;
}

void Game::flush_update()
{
    sge_Update_ON();
    SDL_UpdateRect(screen,0,0,0,0);
    sge_Update_OFF();
}

void Game::process_mouse_press(SDL_Event& event)
{
    MouseEvent mev;
    mev.mx=event.button.x;
    mev.my=event.button.y;
    mev.bstate=event.button.button==SDL_BUTTON_LEFT ? But1Press : But2Press;
    process_event_for_all(wEvent(eMousePress,&mev));
}

void Game::process_mouse_unpress(SDL_Event& event)
{
    MouseEvent mev;
    mev.mx=event.button.x;
    mev.my=event.button.y;
    mev.bstate=event.button.button==SDL_BUTTON_LEFT ? But1Press : But2Press;
    process_event_for_all(wEvent(eMouseUnPress,&mev));
}

void Game::process_mouse_move(SDL_Event& event)
{
    MouseEvent mev;
    mev.mx=event.motion.x;
    mev.my=event.motion.y;
    mev.bstate=event.motion.state==SDL_BUTTON_LEFT ? But1Press : But2Press;
    process_event_for_all(wEvent(eMouseMove,&mev));
}

void Game::process_key_press(SDL_Event& event)
{
    if(focus_object)
	focus_object->process_any_event(wEvent(eKeyPress, &event));
    else
    {
	if(event.key.keysym.sym==SDLK_ESCAPE)
	    game_over();
    }
}

void Game::set_key_event(SDL_Event& event)
{
    key_event=event;
    key_event_got=true;
    process_key_press(key_event);
    key_ticks=FIRST_KEY_TICKS;
}

void Game::reset_key_event()
{
    key_event_got=false;
}

void Game::process_key_event()
{
    if(key_ticks)
    {
	key_ticks--;
	return;
    }
    process_key_press(key_event);
    key_ticks=KEY_TICKS;
}

//main event loop
void Game::event_loop()
{
    /* Main loop */
    SDL_Event  event;
    Uint32 ticks_old, ticks_new;
    dbgprintf(("Entered in event_loop\n"));

    post_event_object=0;
    ticks_old=SDL_GetTicks();
    done=false;

    register_activity();

    while(!done)
    {
	start_frame();

	while(SDL_PollEvent(&event))
	{
	    switch (event.type) 
	    {
	    case SDL_KEYDOWN:
		{
		    PROFILE("key_down");
		    dbgprintf(("KeyPress event\n"));
		    set_key_event(event);
		    register_activity();
		    break;
		}
	    case SDL_KEYUP:
		{
		    reset_key_event();
		    register_activity();
		    break;
		}
	    case SDL_MOUSEBUTTONDOWN:
		{
		    PROFILE("mouse_press");
		    dbgprintf(("MouseButtonPress event\n"));
		    process_mouse_press(event);
		    register_activity();
		    break;
		}
	    case SDL_MOUSEBUTTONUP:
		{
		    PROFILE("mouse_unpress");
		    dbgprintf(("MouseButtonUnpress event\n"));
		    process_mouse_unpress(event);
		    register_activity();
		    break;
		}
	    case SDL_MOUSEMOTION:
		{
		    PROFILE("mouse_move");
		    //		    dbgprintf(("MouseMove event\n"));
		    process_mouse_move(event);
		    register_activity();
		    break;
		}

	    case SDL_QUIT:
		game_over();
		break;

	    case SDL_USEREVENT:
		{
		    PROFILE("user_event");
		    dbgprintf(("UserDefined event\n"));
		    process_event_for_all(wEvent(eUserEvent,&event));
		    break;
		}
	    }
	}

	if(key_event_got)
	    process_key_event();

	if(idle_timeout_object && idle_time!=0)
	{
	    time_t now_t;
	    ACE_OS::time(&now_t);
	    if(idle_time + last_activity_time < now_t)
	    {
		GameObject *obj = idle_timeout_object;
		idle_time = 0;
		idle_timeout_object = 0;
		DBG(4, "Game::event_loop - Idle timeout reached - processing idle action");
		obj->process_any_event(idle_event);
	    }
	}

	if(post_event_object)
	{
	    PROFILE("post_event");
	    post_event_object->process_any_event(post_event);
	    post_event_object=0;
	}

	ticks_new=SDL_GetTicks();
	ticks_new-=ticks_old;
	if(ticks_new<TIMESTEP)
	    SDL_Delay(TIMESTEP-ticks_new);
	ticks_old=SDL_GetTicks();
	{
	    PROFILE("time_event");
	    process_time_event();
	}

	draw_frame();

	end_frame();
    }
}


void Game::register_activity()
{
    ACE_OS::time(&last_activity_time);
}

void Game::start_frame()
{
    DBG(10, "=============== start frame ===============\n");
    CProfileManager::Increment_Frame_Counter();
    PROFILE("start_frame");
}

void Game::end_frame()
{
    draw_rects.r_num=0;
    DBG(10, "===============  end frame ===============\n");
}

void Game::draw_frame()
{
    PROFILE("draw_frame");
    if(draw_rects.r_num)
    {
	for(int i=0;i<draw_rects.r_num;i++)
	    SDL_BlitSurface(bg, &draw_rects.rects[i], screen, &draw_rects.rects[i]);
	for(DrawObjListIt it=draw_objects.begin(); it!=draw_objects.end();it++)
	    it->second->draw(draw_rects);
	SDL_UpdateRects(screen, draw_rects.r_num, draw_rects.rects);
    }
}

void Game::init_video()
{
    int flags=SDL_SWSURFACE;
    /* Init SDL */
    if ( SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0 ) {
	fprintf(stderr, "Couldn't load SDL: %s\n", SDL_GetError());
	exit(1);
    }

    /* Clean up on exit */
    atexit(SDL_Quit);
    
    char answer[100]="no";
    CFGMAN::instance()->get_cstring_value("common", "full_screen_mode", answer);
    CFGMAN::instance()->set_cstring_value("common", "full_screen_mode", answer);
    if(ACE_OS::strcmp(answer, "yes")==0)
	flags|=SDL_FULLSCREEN;
    
    /* Initialize the display */
    screen = SDL_SetVideoMode(SCREEN_L, SCREEN_H, 16, flags);
    if ( screen == NULL ) 
    {
	fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
	exit(1);
    }	
    /* Set window title */
    SDL_WM_SetCaption("Northern Roaster Client", "nr_client");

    SDL_EnableUNICODE(true);
}


void Game::scene_add(GameObject* go)
{
    objects->push_back(go);
}

void Game::scene_del(GameObject* go)
{
    for(GameVec::iterator it=objects->begin(); it!=objects->end(); it++)
    {
	if(*it==go)
	{
	    objects->erase(it);
	    return;
	}
    }
}

//===========================================================================
/// global process_time_event()
///     go throuht the list of time objects, decsrease ticks_left and call
///     objects on timeout
/// tags Game
bool Game::process_time_event()
{
  bool processed=false;
  TimeList* ptimelist=timelist.get_next();
  timelist_restart=false;
  while(ptimelist)
  {
    TimeObject& obj=ptimelist->get_object();
    obj.ticks_left--;
    if(obj.ticks_left<=0 && obj.obj)
    {
      processed=true;
      obj.restart_ticks();
      if(!obj.obj->process_any_event(wEvent(eTimeOut))) 
	//if we return false -> don't process any objects
	break;
    }
    if(timelist_restart)
	break;
    ptimelist=ptimelist->get_next();
  }
  return processed;
}

//===========================================================================
/// global add_timer(GameObject*, int N_ticks)
///     add object to time list for ticking every N_ticks
/// tags Game
bool    Game::add_timer(GameObject* wo, int ticks)
{
    char* p=0;
  TimeObject obj(wo,ticks);
  if(wo==0)
      *p=0;
  TimeList *ptobject=new TimeList(obj);
  timelist.add(ptobject);
  return true;
}

//===========================================================================
/// global del_timer(GameObject*)
///     delete object from time list
/// tags Game
bool    Game::del_timer(GameObject* wo)
{
  TimeObject obj(wo);
  TimeList *plist=timelist.del(obj);
  if(plist)
  {
    delete plist;
    timelist_restart=true;
    return true;
  }
  return false;
}


//===========================================================================
/// global process_event_for_all(wEvent)
///     go throuht the list of objects, and call process_event for each
///     if object return false -> break the loop
/// tags Game
bool Game::process_event_for_all(wEvent ev)
{
  return objectsignal.emit(ev);
}

//===========================================================================
/// global add_object(GameObject*)
///     add object to list of objects that want events
/// tags Game
bool    Game::add_object(GameObject* wo)
{
  SigC::Connection conn;
  DBG(6, "Game::add_object " << DHEX(wo));
  conn=objectsignal.connect(SigC::slot(*wo, &GameObject::process_any_event));
  wo->set_connection(conn);
  return true;
}

//===========================================================================
/// global del_object(GameObject*, int N_ticks)
///     delete object from list of objects that want events
/// tags Game
bool    Game::del_object(GameObject* wo)
{
  wo->get_connection().disconnect();
  return true;
}

//===========================================================================
/// global clear_objectlist()
///     clear all list of objects
/// tags Game
void    Game::clear_objectlist()
{
  objectsignal.clear();
}

//===========================================================================
/// global drawable_add(GameObj*, int lvl)
///     add object to scene for drawing
/// tags Game
void    Game::drawable_add(GameObject* obj, int lvl)
{
    draw_objects.insert(std::make_pair(lvl, obj));
}

//===========================================================================
/// global drawable_del(GameObj*, int lvl)
///     delete object from scene
/// tags Game
void    Game::drawable_del(GameObject* obj, int lvl)
{
    std::pair<DrawObjListIt, DrawObjListIt> p=draw_objects.equal_range(lvl);
    for(DrawObjListIt it=p.first; it!=p.second;it++)
	if(it->second==obj)
	{
	    draw_objects.erase(it);
	    return;
	}
}


//===========================================================================
/// global redraw_area(const GRect& r)
///     set rectagle fro redraw in draw_frame()
///     don't do any draw in this procedure
/// tags Game
void    Game::redraw_area(const GRect& r)
{
    draw_rects << r;
}


//===========================================================================
/// global redraw_area(const GRect& r)
///     set rectagle fro redraw in draw_frame()
///     don't do any draw in this procedure
/// tags Game
void    Game::redraw_area(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    GRect r;
    r.x=x;
    r.y=y;
    r.w=w;
    r.h=h;
    draw_rects << r;
}


SDL_Surface* Game::new_surface(int l, int h)
{
    SDL_Surface *surf;
    surf=SDL_CreateRGBSurface(SDL_SRCALPHA | SDL_SWSURFACE,
			      l, h,
			      screen->format->BitsPerPixel,
			      screen->format->Rmask,
			      screen->format->Gmask,
			      screen->format->Bmask,
			      screen->format->Amask);
    SDL_SetAlpha(surf, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
    return surf;
}

GRect Game::draw_text(SDL_Surface* surf, Images idx, int x, int y, const char* text)
{
    GRect r;
    r.x=x;
    r.y=y;
    r.h=FontHeight(fonts[idx]);
    r.w=PutStringFont(surf, fonts[idx], x, y, text);
    return r;
}

void  Game::set_focus(GameObject* obj)
{
    if(focus_object) 
	focus_object->unset_focus(); 
    focus_object=obj; 
    if(focus_object) 
	focus_object->set_focus();
}
