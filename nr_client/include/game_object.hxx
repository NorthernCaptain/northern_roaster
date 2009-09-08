#ifndef GAME_OBJECT_HXX
#define GAME_OBJECT_HXX
#include "globals.h"
#include "sigc++/sigc++.h"
#include "SDL.h"
#include "game_event.hxx"
#include "geometry.hxx"

class Game;

enum ObjectType
    {
	GO_Base,
	GO_Image,
	GO_Imager,
	GO_Listbox,
	GO_Screen_Intro,
	GO_Textlabel,
	GO_Textlabelflash,
	GO_Textinput,
	GO_Screen_Base,
	GO_Digitlabel,
	GO_Graphview
    };

class GameObject:public SigC::Object
{
protected:
    ObjectType        obj_type;
    bool              shown;
    Game*             game;
    SDL_Rect          size;
    SigC::Connection  connection;
    char              name[GEO_NAME];
    bool              focused;

    virtual bool  mouse_press_event(MouseEvent* ev) { return false;};
    virtual bool  mouse_unpress_event(MouseEvent* ev) { return false;};
    virtual bool  mouse_move_event(MouseEvent* ev) { return false;};
    virtual bool  key_press_event(SDL_Event* ev) { return false;};
    virtual bool  user_event(SDL_Event* ev) { return false;};
    virtual bool  time_out_event() { return false;};
    virtual bool  idle_time_out_event() { return false;};

public:
    GameObject(Game* _game=0, const char* _name="") 
    { 
	game=_game; 
	shown=false; 
	obj_type=GO_Base;
	strncpy(name, _name, GEO_NAME);
	name[GEO_NAME-1]=0;
    };
    virtual ~GameObject() {};

    virtual bool  show();
    virtual bool  draw();
    virtual bool  draw(RectInfo& r) { return false;};

    virtual bool  hide();
    virtual void  set_focus() { focused=true;};
    virtual void  unset_focus() { focused=false;};
    virtual void  my_focus();

    bool          is_shown() { return shown;};

    virtual bool  process_any_event(wEvent ev);

    virtual bool  set_connection(SigC::Connection conn) 
	{ connection=conn; return true;};

    inline  SigC::Connection& get_connection() { return connection;};
    inline  int   x() const { return size.x;};
    inline  int   y() const { return size.y;};
    inline  unsigned   l() const { return size.w;};
    inline  unsigned   h() const { return size.h;};
    inline  ObjectType get_self_type() const { return obj_type;};

    static  bool  collide(GRect& one, GRect& two, GRect& diff);

    SigC::Signal0<void>    next_focus_signal;
};

class GameEditable: public GameObject
{
public:
    GameEditable(Game* _game=0, const char* _name="");

    void          my_focus();
};

class ImageObject:public GameObject
{
protected:
    SDL_Surface      *face;

protected:
    ImageObject(Game* _game=0, SDL_Surface* _face=0);
    ImageObject(const char* fname, Game* _game=0);
    ~ImageObject();

    virtual bool draw();
    bool         draw(RectInfo& rinfo);

    static SDL_Surface*     load_image(const char* fname);
};

#endif
