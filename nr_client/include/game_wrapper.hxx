#ifndef GAME_WRAPPER_HXX
#define GAME_WRAPPER_HXX
#include "game.hxx"
#include "scan_task.hxx"
#include "screen_intro.hxx"
#include "screen_intro2.hxx"
#include "screen_finish.hxx"
#include "screen_login.hxx"
#include "screen_main.hxx"
#include "screen_confirm.hxx"

class GameWrapper: public Game
{
    void                      load_images();

    ScanTaskPtr               scan_task;
    Screen_LoginPtr           screen_login;
    Screen_IntroPtr           screen_intro;
    Screen_Intro2Ptr          screen_intro2;
    Screen_FinishPtr          screen_finish;
    Screen_MainPtr            screen_main;
    Screen_ConfirmPtr         screen_confirm;

    void                      init_colors();

public:
    GameWrapper();
    ~GameWrapper();

    void                      set_scan_task(ScanTaskPtr& ptr) { scan_task=ptr;};
    void                      init_gameplay();
};


#endif
