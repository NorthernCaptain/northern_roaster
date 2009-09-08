#ifndef SCREEN_BASE_HXX
#define SCREEN_BASE_HXX
#include "game_object.hxx"
#include "boost/shared_ptr.hpp"
#include "event_notifier.hxx"
#include "textlabelflash.hxx"

class Game;

class Screen_Base: public GameObject
{
protected:
    Screen_Base               *next_screen, *prev_screen;
    TextlabelPtr               info_lbl;
    virtual bool               time_out_event();

public:
    Screen_Base(Game* _game=0, const char* _name="");
    
    SigC::Signal0<bool>       finished_signal;
    SigC::Signal0<bool>       exit_signal;

    void                      set_next_screen(Screen_Base* next) { next_screen=next;};
    void                      set_prev_screen(Screen_Base* prev) { prev_screen=prev;};

    void                      set_info(const std::string& nfo, bool flash=false);
    void                      set_warn(const std::string& nfo);
};

typedef boost::shared_ptr<Screen_Base>   Screen_BasePtr;

class GameNotifier: public EventNotifier
{
    int                        code;
public:
    GameNotifier(int code_=0): code(code_) {};
    void                      send_notification(NotificationEvent* ev);
};


enum ScreenEventType 
    { 
	se_noop=0, 
	se_update_info,
	se_update_grid,
	se_init_grid,
	se_auto_exit
    };

class ScreenEvent : public NotificationEvent
{
public:
    ScreenEventType    type;
    ScreenEvent(ScreenEventType type_) : type(type_) {};
};

#endif
