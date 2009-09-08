#ifndef NCORBA_CLN_HXX
#define NCORBA_CLN_HXX

#include "ncorba.hxx"
#include "nc_server_controlC.h"
#include "boost/shared_ptr.hpp"

class ClnNamedORB: public NamedORB
{
protected:
    NorthernCaptain::ServerControl_var    server_control_obj;

public:
    ClnNamedORB(int argc, ACE_TCHAR** argv, 
		const ACE_TCHAR* name_service, 
		const ACE_TCHAR* ns_path);

    ~ClnNamedORB();

    void server_shutdown();

    bool init_server_control();
    std::string server_version();

    inline NorthernCaptain::ServerControl_var& the_server_control() { return server_control_obj;};
};

typedef boost::shared_ptr<ClnNamedORB>              ClnNamedORBPtr;

#endif
