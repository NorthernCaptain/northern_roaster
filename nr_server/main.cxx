#include "ace/OS_main.h"
#include "debug.hxx"
#include "configman.hxx"
#include "dev_manager.hxx"
#include "data_manager.hxx"
#include "ncorba_srv.hxx"
#include "boost/scoped_ptr.hpp"
#include "nr_iface_impl.hxx"
#include "DBSObjects.h"
#include "server_version.hxx"

void otl_test()
{
    try
    {

	otl_connect_ptr con = DBIMAN::instance()->getDBConnection();

	otl_stream_wrapper_ptr str(con->new_stream(50, "select name, full_name from worker where disabled=0 and type='roaster' and id >= :id<int>"));

	(*str) << 0;

	while(!str->eof())
	{
	    std::string id;
	    std::string name;
	    (*str) >> id >> name;
	    DBG(0, "NAME, FULL_NAME: " << id << " : " << name);
	}

    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "Got DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
    } 
}

int ACE_TMAIN(int argc, char** argv)
{
    CFGMAN::instance()->load("nr_srv.cfg");

    ACE_TCHAR  log_file_prefix[256]="nr_srv";
    CFGMAN::instance()->get_cstring_value("common", "debug_log_file_prefix", log_file_prefix);
    CFGMAN::instance()->set_cstring_value("common", "debug_log_file_prefix", log_file_prefix);
    DL.debug_level(CFGMAN::instance()->get_int_value("common", "debug_level", 1));
    int max_lines = CFGMAN::instance()->get_int_value("common", "debug_file_max_lines", 100000);
    CFGMAN::instance()->set_int_value("common", "debug_file_max_lines", max_lines);
    DL.set_log_output(log_file_prefix, max_lines);

    DBG(0, "STARTUP: " << version_s);

    new_otl_connect_wrapper_f = new_otl_connect_wrapper;
    otl_wrapper* ow = new_otl_wrapper();

    ow->otl_initialize();
    
    try 
    {
	otl_test();

	DEVMAN::instance()->activate();
	DATAMAN::instance()->activate();

	ACE_TCHAR   ns_loc[1024]="corbaloc::localhost:2809/NameService";
	ACE_TCHAR   server_name[128]="NR_SERVER_1";

	CFGMAN::instance()->get_cstring_value("corba", "main_ns_loc", ns_loc);
	CFGMAN::instance()->set_cstring_value("corba", "main_ns_loc", ns_loc);
	CFGMAN::instance()->get_cstring_value("corba", "server_name", server_name);
	CFGMAN::instance()->set_cstring_value("corba", "server_name", server_name);
	DBG(4, "SERVER NAME: " << server_name);

	SrvNamedORB  orb(argc, argv, 
			 ns_loc,
			 server_name);

        orb.create_server_control_object();

	//Registering objects and servants

	boost::scoped_ptr<POA_NR_iface::RoastCom> 
	    roast_impl_servant(new POA_NR_iface::RoastCom_Impl);

	NR_iface::RoastCom_var 
	    roast_obj(orb.servant_to_reference<NR_iface::RoastCom>(roast_impl_servant.get()));

	orb.bind_object(roast_obj.in(), "RoastCom.obj");

	boost::scoped_ptr<POA_NR_iface::RoastCoffeeSort> 
	    roast_sort_servant(new POA_NR_iface::RoastCoffeeSort_ImplDB);

	NR_iface::RoastCoffeeSort_var 
	    roast_sort_obj(orb.servant_to_reference<NR_iface::RoastCoffeeSort>(roast_sort_servant.get()));

	orb.bind_object(roast_sort_obj.in(), "RoastCoffeeSort.obj");

	boost::scoped_ptr<POA_NR_iface::RoastUsers> 
	    roast_user_servant(new POA_NR_iface::RoastUsers_ImplDB);

	NR_iface::RoastUsers_var 
	    roast_user_obj(orb.servant_to_reference<NR_iface::RoastUsers>(roast_user_servant.get()));

	orb.bind_object(roast_user_obj.in(), "RoastUsers.obj");

	boost::scoped_ptr<POA_NR_iface::RoastLevels> 
	    roast_lvl_servant(new POA_NR_iface::RoastLevels_ImplDB);

	NR_iface::RoastLevels_var 
	    roast_lvl_obj(orb.servant_to_reference<NR_iface::RoastLevels>(roast_lvl_servant.get()));

	orb.bind_object(roast_lvl_obj.in(), "RoastLevels.obj");

	boost::scoped_ptr<POA_NR_iface::RoastStates> 
	    roast_state_servant(new POA_NR_iface::RoastStates_ImplDB);

	NR_iface::RoastStates_var 
	    roast_state_obj(orb.servant_to_reference<NR_iface::RoastStates>(roast_state_servant.get()));

	orb.bind_object(roast_state_obj.in(), "RoastStates.obj");

	boost::scoped_ptr<POA_NR_iface::RoastWeights> 
	    roast_weight_servant(new POA_NR_iface::RoastWeights_Impl);

	NR_iface::RoastWeights_var 
	    roast_weight_obj(orb.servant_to_reference<NR_iface::RoastWeights>(roast_weight_servant.get()));

	orb.bind_object(roast_weight_obj.in(), "RoastWeights.obj");

	//////////////////////////////////////////////////////////

	CFGMAN::instance()->save();
        orb.orb_thread_run();
        orb.orb_wait();
	
	DATAMAN::instance()->shutdown();
	DEVMAN::instance()->shutdown();

    }
    catch(DeviceException& ex)
    {
	DBG(0, "MAIN: Fatal Device EXCEPTION: " << ex.get_info());
    }
    catch(NRException& ex)
    {
	DBG(0, "MAIN: Fatal NR Common EXCEPTION: " << ex.get_info());
    }
    catch( const CORBA::Exception& e ) 
    {
        DBG(0, "CORBA SYSTEM Exception: " << e._info());
    }
    catch(...)
    {
	DBG(0, "MAIN: Fatal unregistered exception - exiting");
    }

    ow->otl_terminate();
    
    return 0;
}
