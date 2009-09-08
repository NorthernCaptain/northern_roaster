#ifndef SCREEN_INTRO2_HXX
#define SCREEN_INTRO2_HXX
#include "screen_base.hxx"
#include "listbox.hxx"
#include "textinput.hxx"
#include "digitlabel.hxx"
#include "scan_task.hxx"

class Screen_Intro2: public Screen_Base
{
    ScanTaskPtr               scan_task;
    CoSorts                  *levels, *weights;
    DigitlabelPtr             temp1_lbl, temp2_lbl;
    ListboxPtr                lvl_box;
    TextinputPtr              lvl_full_name;
    TextinputPtr              wei_full_name;

    bool                      user_event(SDL_Event* ev);
    void                      lvl_edit_activated(std::string&);
    void                      lvl_item_activated(ListboxItem item);
    void                      wei_edit_activated(std::string& wei);
    void                      unprocessed_event(SDL_Event*);

    void                      sync_listbox();
    void                      goto_prev_screen();

public:
    Screen_Intro2(ScanTaskPtr& scan, Game* _game=0, const char* _name="");
    ~Screen_Intro2();

    virtual bool              show();
    virtual bool              hide();
    virtual bool              idle_time_out_event();

};

typedef boost::shared_ptr<Screen_Intro2>   Screen_Intro2Ptr;

#endif
