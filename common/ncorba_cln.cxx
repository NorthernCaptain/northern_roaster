#include "ncorba_cln.hxx"

ClnNamedORB::ClnNamedORB(int argc, ACE_TCHAR** argv, 
                         const ACE_TCHAR* name_service, 
			 const ACE_TCHAR* ns_path)
                         : NamedORB(argc, argv, name_service, ns_path, false)
{
}

ClnNamedORB::~ClnNamedORB()
{
}

bool ClnNamedORB::init_server_control()
{
    server_control_obj=nshelper->resolve_name<NorthernCaptain::ServerControl>("ServerControl.obj");
    CORBA::Long ver_l;
    CORBA::String_var  ver_s(server_control_obj->get_version(ver_l));
    DBG(2, "Connected to ServerControl object, server name: [" 
        << ver_s.in() 
        << "], digital version is "
        << ver_l / 100 << "." 
        << (ver_l % 100) / 10 << "."
        << ver_l % 10);
    return true;
}

std::string ClnNamedORB::server_version()
{
    CORBA::Long ver_l;
    CORBA::String_var  ver_s(server_control_obj->get_version(ver_l));
    return std::string(ver_s.in());
}

void ClnNamedORB::server_shutdown()
{
    DBG(5, "Shutdown serve via ServerControl object");
    server_control_obj->shutdown();
}
