#ifndef SCREEN_CONFIRM_HXX
#define SCREEN_CONFIRM_HXX
#include "screen_base.hxx"
#include "listbox.hxx"
#include "textinput.hxx"
#include "digitlabel.hxx"
#include "scan_task.hxx"

class Screen_Confirm: public Screen_Base
{
    ScanTaskPtr               scan_task;
    ListboxPtr                lbox;
    TextinputPtr              full_name;
    CoSorts                  *sorts, *levels, *sts;
    DigitlabelPtr             temp1_lbl, temp2_lbl;

    bool                      user_event(SDL_Event* ev);
    void                      sort_item_activated(ListboxItem item);
    void                      unprocessed_event(SDL_Event*);

    void                      sync_listbox();
    void                      goto_prev_screen();

    TextlabelPtr              bigmsg_lbl;
    TextlabelPtr              inputsort_lbl;
    TextlabelPtr              weight_lbl;

    void set_bigmsg(const std::string& nfo, bool flash);

public:
    Screen_Confirm(ScanTaskPtr& scan, Game* _game=0, const char* _name="");
    ~Screen_Confirm();

    virtual bool              show();
    virtual bool              hide();
    virtual bool              idle_time_out_event();

};

typedef boost::shared_ptr<Screen_Confirm>   Screen_ConfirmPtr;

#endif
