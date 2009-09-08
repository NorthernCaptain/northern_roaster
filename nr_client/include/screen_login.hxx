#ifndef SCREEN_LOGIN_HXX
#define SCREEN_LOGIN_HXX
#include "screen_base.hxx"
#include "listbox.hxx"
#include "textinput.hxx"
#include "digitlabel.hxx"
#include "scan_task.hxx"

class Screen_Login: public Screen_Base
{
    ScanTaskPtr               scan_task;
    ListboxPtr                lbox;
    TextinputPtr              full_name;
    TextinputPtr              short_name;
    CoSorts*                  sorts;
    DigitlabelPtr             temp1_lbl, temp2_lbl;

    bool                      user_event(SDL_Event* ev);
    void                      item_selected(ListboxItem item);
    void                      item_activated(ListboxItem item);
    void                      edit_activated(std::string&);
    void                      unprocessed_event(SDL_Event*);

    void                      sync_listbox();

    void                      process_exit();

public:
    Screen_Login(ScanTaskPtr& scan, Game* _game=0, const char* _name="");
    ~Screen_Login();

    virtual bool              show();
    virtual bool              hide();

};

typedef boost::shared_ptr<Screen_Login>   Screen_LoginPtr;

#endif
