#include "configman.hxx"
#include "user_iface.hxx"
#include "eversion.hxx"

#include <string.h>

emuUI::emuUI()
{
    initscr();
    int oh = CFGMAN::instance()->get_int_value("ui", "output_height", 10);
    int ch = CFGMAN::instance()->get_int_value("ui", "command_height", 5);

    wout = newwin(oh, 80, 0, 0);
    wcmd = newwin(ch, 80, oh, 0);

    scrollok(wout, TRUE);
    scrollok(wcmd, TRUE);
    c_printf(wout, "%s\n", emuVersion);
}

emuUI::~emuUI()
{
    endwin();
}


void emuUI::c_printf(WINDOW *window,char *format,...)
{
    va_list     vl;
    char        str[1024];

    va_start(vl,format);
    vsprintf(str,format,vl);
    va_end(vl);
    wprintw(window,str);
    wrefresh(window);
}

bool emuUI::process_command(char* buf)
{
    if(strncmp(buf, "quit", 4)==0)
	return false;
    if(strncmp(buf, "load", 4)==0)
    {
	cmd_load(buf);
	return true;
    }
    
    if(strncmp(buf, "unload", 6)==0)
    {
	cmd_unload(buf);
	return true;
    }

    c_printf(wcmd, "Unknown command: %s\n", buf);

    return true;
}

void emuUI::event_loop()
{
    char buf[1024];

    while(true)
    {
	c_printf(wcmd, "Cmd# ");
	wgetnstr(wcmd, buf, 70);
	if(!process_command(buf))
	    break;
    }

}

void emuUI::cmd_load(const char* buf)
{
    c_printf(wcmd, "Push LOAD command\n");
}

void emuUI::cmd_unload(const char* buf)
{
    c_printf(wcmd, "Push UNLOAD command\n");
}
