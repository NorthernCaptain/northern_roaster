#include "ace/OS_main.h"
#include "configman.hxx"
#include "debug.hxx"
#include "scan_task.hxx"
#include "textinput.hxx"
#include "game_profile.h"
#include "game_wrapper.hxx"
#include "coffeesorts.hxx"

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"

#include "version_cln.hxx"

int ACE_TMAIN(int argc, char** argv)
{
    CFGMAN::instance()->load("nr_cln.cfg");

    ACE_TCHAR  log_file_prefix[256]="nr_cln";
    CFGMAN::instance()->get_cstring_value("common", "debug_log_file_prefix", log_file_prefix);
    CFGMAN::instance()->set_cstring_value("common", "debug_log_file_prefix", log_file_prefix);
    DL.debug_level(CFGMAN::instance()->get_int_value("common", "debug_level", 1));
    int max_lines = CFGMAN::instance()->get_int_value("common", "debug_file_max_lines", 100000);
    CFGMAN::instance()->set_int_value("common", "debug_file_max_lines", max_lines);
    DL.set_log_output(log_file_prefix, max_lines);

    DBG(0, "STARTUP: " << ClnVersion);
    
    try 
    {

	ACE_TCHAR   ns_loc[1024]="corbaloc::localhost:2809/NameService";
	ACE_TCHAR   server_name[128]="NR_SERVER_1";

	CFGMAN::instance()->get_cstring_value("corba", "main_ns_loc", ns_loc);
	CFGMAN::instance()->set_cstring_value("corba", "main_ns_loc", ns_loc);
	CFGMAN::instance()->get_cstring_value("corba", "server_name", server_name);
	CFGMAN::instance()->set_cstring_value("corba", "server_name", server_name);

	if(argc>1)
	    ACE_OS::strcpy(server_name, argv[1]);
	
	if(argc>2)
	    ACE_OS::strcpy(ns_loc, argv[2]);

	DBG(4, "SERVER NAME : " << server_name);
	DBG(4, "CORBA NS LOC: " << ns_loc);

	ScanTaskPtr scan_task(new ScanTask(ORBInit(argc, argv, server_name, ns_loc)));

	scan_task->activate();
	
	CProfileManager::Reset();  

	boost::shared_ptr<GameWrapper> game(new GameWrapper);

	DecodeKOI8::init();

	game->set_scan_task(scan_task);
	game->init_gameplay();
	game->show();
	game->event_loop();

	CProfileManager::Show_All_Profile();

	//	scan_task->send(new ScanTaskEvent(stask_logout));
	scan_task->shutdown();

    }
    catch( const CORBA::Exception& e ) 
    {
        DBG(0, "CORBA SYSTEM Exception: " << e._info());
    }
#if 0
    catch(...)
    {
	DBG(0, "MAIN: Fatal unregistered exception - exiting");
    }
#endif
    
    return 0;
}
