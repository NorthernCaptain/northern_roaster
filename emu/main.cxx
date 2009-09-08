#include "dbglog.h"
#include "configman.hxx"
#include "user_iface.hxx"
#include "eversion.hxx"

int main(int argc, char** argv)
{
    CFGMAN::instance()->load("nr_emu.cfg");
    DL.debug_level(CFGMAN::instance()->get_int_value("common", "debug_level", 1));

    DBG(0, "STARTUP: " << emuVersion);

    {
	emuUI   ui;

	ui.event_loop();

    }
    
    return 0;
}
