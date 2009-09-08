#include "game.hxx"
#include "screen_base.hxx"
#include "exceptions.hxx"
#include "debug.hxx"

Screen_Base::Screen_Base(Game* _game, const char* _name) : GameObject(_game, _name)
{
    obj_type = GO_Screen_Base;
    prev_screen=next_screen=0;
    info_lbl=TextlabelPtr(new Textlabel("", game, "info_lbl", imFont2));
}

void Screen_Base::set_info(const std::string& nfo, bool flash)
{
    TextlabelPtr ptr=TextlabelPtr(flash ? new Textlabelflash(nfo, game, "info_lbl", imFont2) 
				 : new Textlabel(nfo, game, "info_lbl", imFont2));

    if(info_lbl->is_shown())
	info_lbl->hide();
    ptr->show();
    info_lbl.swap(ptr);
}


void Screen_Base::set_warn(const std::string& nfo)
{
    TextlabelPtr ptr=TextlabelPtr(new Textlabelflash(nfo, game, "info_lbl", imFont5));

    if(info_lbl->is_shown())
	info_lbl->hide();
    ptr->show();
    info_lbl.swap(ptr);
    game->del_timer(this);
    game->add_timer(this, 600);
}

bool Screen_Base::time_out_event() 
{
    info_lbl->hide();
    game->del_timer(this);
    return false;
}


void GameNotifier::send_notification(NotificationEvent* ev)
{
    SDL_Event sdlev;
    sdlev.type=SDL_USEREVENT;
    sdlev.user.code=code;
    sdlev.user.data1=ev;
    SDL_PushEvent(&sdlev);
}

