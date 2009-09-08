#ifndef SCREEN_FINISH_HXX
#define SCREEN_FINISH_HXX
#include "screen_base.hxx"
#include "listbox.hxx"
#include "textinput.hxx"
#include "digitlabel.hxx"
#include "scan_task.hxx"

class Screen_Finish: public Screen_Base
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
    Screen_Finish(ScanTaskPtr& scan, Game* _game=0, const char* _name="");
    ~Screen_Finish();

    virtual bool              show();
    virtual bool              hide();

};

typedef boost::shared_ptr<Screen_Finish>   Screen_FinishPtr;

#endif
