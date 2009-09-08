#include "ace/OS_main.h"
#include "debug.hxx"
#include "ncorba_cln.hxx"

int ACE_TMAIN(int argc, char** argv)
{
    DL.debug_level(20);
    
    try 
    {

	ACE_TCHAR   ns_loc[1024]="corbaloc::localhost:2809/NameService";
	ACE_TCHAR   server_name[128]="NR_SERVER_1";

	if(argc>1)
	    ACE_OS::strcpy(server_name, argv[1]);
	
	if(argc>2)
	    ACE_OS::strcpy(ns_loc, argv[2]);

	DBG(4, "SERVER NAME : " << server_name);
	DBG(4, "CORBA NS LOC: " << ns_loc);

	ClnNamedORB  orb(argc, argv, 
			 ns_loc,
			 server_name);

        orb.init_server_control();

	DBG(4, "MAIN: shutting down server " << server_name);

	orb.server_shutdown();
    }
    catch( const CORBA::Exception& e ) 
    {
        DBG(0, "CORBA SYSTEM Exception: " << e._info());
    }
    catch(...)
    {
	DBG(0, "MAIN: Fatal unregistered exception - exiting");
    }
    
    return 0;
}
