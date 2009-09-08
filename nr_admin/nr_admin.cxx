#include "ace/OS_main.h"
#include <qapplication.h>
#include "debug.hxx"
#include "mainwin_adm.hxx"
#include "configman.hxx"

int ACE_TMAIN(int argc, ACE_TCHAR** argv)
{
    ACE_DEBUG((LDBG "Northern Roast Admin started...\n"));
    CFGMAN::instance()->load("nr_admin.cfg");

    QApplication app(argc,argv);
    NRMainWinAdm   mainwin;

    mainwin.show();

    QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );
    
    return app.exec();
}
