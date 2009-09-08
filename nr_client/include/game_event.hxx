#ifndef GAME_EVENT_HXX
#define GAME_EVENT_HXX

#include "SDL.h"

enum EvType { 
    eTimeOut, 
    eKeyPress, 
    eKeyRelease, 
    eMouseMove, 
    eMousePress,
    eMouseUnPress,
    eExpose,
    eDelete,
    eFocusOut,
    eUserEvent,
  
    aEmpty,
    aGameOver,
    aIntroExit,
    aKeyPressed,
    aSwitchChanged,
    aInputDone,
    aRedraw,
    aPickUp,
    aDetectCollision,
    aAutoExit,
    aIdleTimeout
};


enum ButtonState { But1Press, But2Press, But3Press }; 

struct wEvent
{
    EvType type;
    void   *data;
    wEvent(EvType t=aEmpty,void *idata=0) { type=t;data=idata;};
};

struct MouseEvent
{
    int         mx,my;
    ButtonState bstate;
};

typedef SDL_Rect           GRect;
typedef SDL_Surface*       GSurface;

const int SCREEN_L=1024;
const int SCREEN_H=768;


struct RectInfo
{
    GRect     *rects;
    int        r_num;
    RectInfo(GRect* _rects=0, int _r_num=0): rects(_rects), r_num(_r_num) {};
    RectInfo& operator << (const GRect& r) 
    { 
	GRect& cur=rects[r_num++];
	if(r.x<0) cur.x=0; else cur.x=r.x;
	if(r.y<0) cur.y=0; else cur.y=r.y;
	if(cur.x + r.w > SCREEN_L) cur.w=SCREEN_L - cur.x; else cur.w=r.w;
	if(cur.y + r.h > SCREEN_H) cur.h=SCREEN_H - cur.y; else cur.h=r.h;
	return *this;
    };
};

#endif
