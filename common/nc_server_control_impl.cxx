#include "ncorba_srv.hxx"
#include "nc_server_control_impl.hxx"
#include "server_version.hxx"

char* POA_NorthernCaptain::ServerControl_Impl::get_version(CORBA::Long_out ver) throw (CORBA::SystemException)
{
    ver=version_l;
    return CORBA::string_dup(version_s);
}

void POA_NorthernCaptain::ServerControl_Impl::shutdown() throw (CORBA::SystemException)
{
    DBG(2, "Shutdown server message, stopping ORB");
    SrvNamedORB::instance->shutdown();
}
