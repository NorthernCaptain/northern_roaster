#ifndef NCORBA_SRV_HXX
#define NCORBA_SRV_HXX

#include "ncorba.hxx"
#include <tao/PortableServer/PortableServer.h>

#include "nc_server_control_impl.hxx"
#include "nc_server_controlC.h"

#include "ace/Task.h"

class OrbTask;

class SrvNamedORB: public NamedORB
{
    PortableServer::POA_var           rootPOA;
    PortableServer::POAManager_var    rootPOAMgr;

    POA_NorthernCaptain::ServerControl_Impl *server_control_servant;
    NorthernCaptain::ServerControl_var       server_control_obj;

    OrbTask*                          orb_task;

public:
    SrvNamedORB(int argc, ACE_TCHAR** argv, ACE_TCHAR* name_service, ACE_TCHAR* ns_path);

    ~SrvNamedORB();

    inline PortableServer::POA_var&          the_rootPOA() { return rootPOA;};
    inline PortableServer::POAManager_var&   the_rootPOAMgr() { return rootPOAMgr;};
    //TODO: Implement creation of our own POA, and don't use rootPOA
    inline PortableServer::POA_var&          the_POA()     { return rootPOA;}; //Here must be current POA, not root

    inline void bind_object(CORBA::Object_ptr obj, const ACE_TCHAR* name) { nshelper->Rebind(obj, name);}
    inline void bind_object(CORBA::Object_var& obj, const ACE_TCHAR* name) { nshelper->Rebind(obj.in(), name);}
    inline void unbind_object(const ACE_TCHAR* name) { nshelper->Unbind(name);};

    inline void shutdown() { orb->shutdown(); };

    bool        create_server_control_object();

    void        orb_thread_run();
    void        orb_wait();

    template <class OBJTYPE> OBJTYPE* servant_to_reference(PortableServer::Servant p_servant)
    {
        CORBA::Object_var  obj_ref=the_POA()->servant_to_reference(p_servant);
        return OBJTYPE::_narrow(obj_ref.in());
    }

    static SrvNamedORB*               instance;
};

//Class that run ORB::run() method in a thread, not in main loop;
class OrbTask: public ACE_Task<ACE_MT_SYNCH>
{
    SrvNamedORB&          named_orb_ref;
public:
    OrbTask(SrvNamedORB& _named_orb);

    int svc();
};

#endif
