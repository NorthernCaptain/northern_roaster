#ifndef GAME_HXX
#define GAME_HXX
#include "globals.h"
#include <vector>
#include <map>

#include "sigc++/sigc++.h"
#include "SDL.h"
#include "SDL_image.h"
#include "sge.h"
#include "bfont.h"

#include "game_event.hxx"
#include "bilist.h"

const int VIEW_L=SCREEN_L-250;
const int VIEW_H=SCREEN_H-250;

const int MAX_GAME_COLORS=64;

/////////////////////////////Signals and slots///////////////////////
class StopMarshal
{
 public:
    typedef     bool   InType;
    typedef     bool   OutType;
    
    static OutType     default_value() { return false;};
    OutType            value() { return val;};

    bool marshal(InType v)
	{
	    val=v;
	    return false;
	};

 private:
    OutType            val;
};


typedef SigC::Signal1<bool, wEvent>  EventSignal;

class GameObject;

//Class used in Time queue for delivering time events to objects
struct TimeObject
{
  GameObject* obj;
  int         ticks_left;
  int         ticks;
  TimeObject()  {ticks=ticks_left=0; obj=0;};
  TimeObject(GameObject* wo, int t=1) { obj=wo;ticks_left=ticks=t;};
  void restart_ticks() { ticks_left=ticks; };
  bool operator == (TimeObject& o2) {return o2.obj==obj;};
};

typedef bilist<TimeObject>  TimeList;   //List of objects that want timers 

typedef std::vector<GameObject*>     GameVec;

/////////////////////////////////////////////////////////////////////
typedef std::multimap<int, GameObject*>       DrawObjList;
typedef DrawObjList::iterator                 DrawObjListIt;

const int    max_draw_rects=1000;
const int    max_images=50;

class Game:public SigC::Object
{
protected:
    TimeList         timelist;
    bool             timelist_restart;
    EventSignal      objectsignal;

    GameVec         *objects;

    GameObject      *post_event_object;
    wEvent           post_event;

    bool             done;

    RectInfo         draw_rects;
    DrawObjList      draw_objects;

    SDL_Surface     *image_bank[max_images];
    BFont_Info      *fonts[max_images];

    GameObject      *focus_object;
    SDL_Event        key_event;
    bool             key_event_got;
    int              key_ticks;

    Uint32           game_colors[MAX_GAME_COLORS];
    Uint8            game_RGBA[MAX_GAME_COLORS*4];

    GameObject      *idle_timeout_object;
    wEvent           idle_event;
    time_t           idle_time;
    time_t           last_activity_time;

    void             process_mouse_press(SDL_Event&);
    void             process_mouse_unpress(SDL_Event&);
    void             process_mouse_move(SDL_Event&);
    void             process_key_press(SDL_Event&);

    virtual bool     process_time_event();
    virtual bool     process_event_for_all(wEvent);

    void             start_frame();
    void             end_frame();
    void             draw_frame();

    static void      init_video();

    virtual void     load_images() {};
    virtual void     init();
    void             set_key_event(SDL_Event& event);
    void             reset_key_event();
    void             process_key_event();

    void             register_activity();

public:

    Game();
    virtual ~Game();
    virtual bool     show();
    virtual void     flush_update();
    virtual void     event_loop();

    void             game_over() { done=true;};
    virtual void     init_gameplay() {};

    virtual bool     add_timer(GameObject* wo, int ticks);
    virtual bool     del_timer(GameObject* wo);
    virtual bool     add_object(GameObject* wo);
    virtual bool     del_object(GameObject* wo);
    virtual void     clear_objectlist();

    void             scene_add(GameObject*);
    void             scene_del(GameObject*);
    void             drawable_add(GameObject*, int level);
    void             drawable_del(GameObject*, int level);
    void             redraw_area(const GRect& r);
    void             redraw_area(Sint16 x, Sint16 y, Uint16 w, Uint16 h);

    inline SDL_Surface* image(Images idx) { return image_bank[idx];};
    SDL_Surface*     new_surface(int l, int h);

    GRect            draw_text(SDL_Surface* surf, Images idx, int x, int y, const char* text);
    inline int       font_height(Images font_id) { return FontHeight(fonts[font_id]);};
    inline int       font_width(Images font_id, const char* text) { return TextWidthFont(fonts[font_id], text);};
    
    virtual void     set_post_event_call(GameObject* obj, wEvent ev) { post_event_object=obj; post_event=ev; };
    virtual void     set_idle_time_call(GameObject* obj, wEvent ev, time_t timeout) 
            { idle_timeout_object = obj; idle_event = ev; idle_time = timeout;};

    static SDL_Surface  *screen, *bg;
    static SDL_Surface* load_image(const char* fname);

    void             set_focus(GameObject* obj=0);
    inline Uint32    get_color(int idx) const { return game_colors[idx];};
    inline Uint8*    get_RGBA() { return game_RGBA;};

    inline void      delete_surface(SDL_Surface* face) { SDL_FreeSurface(face);};
};

#endif
