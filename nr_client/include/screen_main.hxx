#ifndef SCREEN_MAIN_HXX
#define SCREEN_MAIN_HXX
#include "screen_base.hxx"
#include "textlabel.hxx"
#include "digitlabel.hxx"
#include "scan_task.hxx"
#include "graphview.hxx"

class Game;

class Screen_Main: public Screen_Base
{
    ScanTaskPtr               scan_task;
    TextlabelPtr              uname_lbl;
    TextlabelPtr              sortname_lbl;
    TextlabelPtr              ltime_lbl;
    TextlabelPtr              load_lbl;
    TextlabelPtr              unload_lbl;
    DigitlabelPtr             temp1_lbl, temp2_lbl;

    GraphviewPtr              graph;

    int                       data_offset;
    TextlabelPtr              time_lbl;
    TextlabelPtr              level_lbl;
    TextlabelPtr              weight_lbl;
    TextlabelPtr              nroast_lbl;
    TextlabelPtr              troast_lbl;

    void                      unprocessed_event(SDL_Event*);
    bool                      user_event(SDL_Event* ev);
    void                      update_grid_data();
    void                      reinit_grid_data();
    void                      process_escape();

public:
    Screen_Main(ScanTaskPtr& scan, Game* _game=0, const char* _name="");
    ~Screen_Main();

    virtual bool              show();
    virtual bool              hide();
    bool              hide_only();
    virtual bool              process_any_event(wEvent ev);

};

typedef boost::shared_ptr<Screen_Main>   Screen_MainPtr;

#endif
