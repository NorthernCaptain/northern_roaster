#ifndef SCREEN_INTRO_HXX
#define SCREEN_INTRO_HXX
#include "screen_base.hxx"
#include "listbox.hxx"
#include "textinput.hxx"
#include "digitlabel.hxx"
#include "scan_task.hxx"

class Screen_Intro: public Screen_Base
{
    ScanTaskPtr               scan_task;
    ListboxPtr                lbox;
    TextinputPtr              full_name;
    CoSorts                  *sorts, *levels, *sts;
    DigitlabelPtr             temp1_lbl, temp2_lbl;
    ListboxPtr                lvl_box;
    TextinputPtr              lvl_full_name;

    ListboxPtr                sts_box;
    TextinputPtr              sts_full_name;

    bool                      user_event(SDL_Event* ev);
    void                      sort_item_activated(ListboxItem item);
    void                      sort_edit_activated(std::string&);
    void                      lvl_item_activated(ListboxItem item);
    void                      sts_edit_activated(std::string&);
    void                      sts_item_activated(ListboxItem item);
    void                      rcn_edit_activated(std::string&);
    void                      unprocessed_event(SDL_Event*);

    void                      sync_listbox();
    void                      goto_prev_screen();

    TextlabelPtr              bigmsg_lbl;

    void set_bigmsg(const std::string& nfo, bool flash);

public:
    Screen_Intro(ScanTaskPtr& scan, Game* _game=0, const char* _name="");
    ~Screen_Intro();

    virtual bool              show();
    virtual bool              hide();
    virtual bool              idle_time_out_event();

};

typedef boost::shared_ptr<Screen_Intro>   Screen_IntroPtr;

#endif
