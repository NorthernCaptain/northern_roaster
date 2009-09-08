#ifndef NC_SERVER_CONTROL_IMPL_HXX
#define NC_SERVER_CONTROL_IMPL_HXX

//Header file for NorthernCapatin::ServerControl implementaion on the server side
//Here we describe servant that will manage server control requests
#include "ncorba_header.hxx"
#include "nc_server_controlS.h"

namespace POA_NorthernCaptain
{
    class ServerControl_Impl: public virtual ServerControl,
        public virtual PortableServer::RefCountServantBase
    {
    public:
        virtual char* get_version(CORBA::Long_out ver) throw (CORBA::SystemException);
        virtual void  shutdown() throw (CORBA::SystemException);
    };
};

#endif
