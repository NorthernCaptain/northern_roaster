#ifndef USER_IFACE_HXX
#define USER_IFACE_HXX

#include <curses.h>


class emuUI
{
    WINDOW*          wout;
    WINDOW*          wcmd;

    void             c_printf(WINDOW *window,char *format,...);

    bool             process_command(char* buf);

    void             cmd_load(const char*);
    void             cmd_unload(const char*);


public:
    emuUI();
    ~emuUI();

    void             event_loop();
};



#endif
