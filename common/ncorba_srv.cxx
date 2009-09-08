#include "ncorba_srv.hxx"

//Here we store implementaion for server part CORBA helpers

SrvNamedORB*    SrvNamedORB::instance=0;

//------------------------------- SrvNamedORB ---------------------------

SrvNamedORB::SrvNamedORB(int argc, ACE_TCHAR** argv, 
                         ACE_TCHAR* name_service,
                         ACE_TCHAR* ns_path)
                         : NamedORB(argc, argv, name_service, ns_path, true)
{
    instance=this;
    server_control_servant=0;
    orb_task=0;
    //Get POA (Poratble Object Adapter) object reference
    DBG(2, "Getting POA and friends...");
    CORBA::Object_var obj_poa=orb->resolve_initial_references("RootPOA");
    rootPOA = PortableServer::POA::_narrow(obj_poa.in());
    rootPOAMgr = rootPOA->the_POAManager();
}

SrvNamedORB::~SrvNamedORB()
{
    instance=0;
    if(server_control_servant)
        unbind_object("ServerControl.obj");
    if(orb_task)
    {
        orb_task->wait();
        delete orb_task;
    }
}

bool SrvNamedORB::create_server_control_object()
{
    if(server_control_servant) //servant and object have been already created
        return false;
    DBG(2, "Creating ServerControl servant and object");
    server_control_servant = new POA_NorthernCaptain::ServerControl_Impl();
    server_control_obj = servant_to_reference<NorthernCaptain::ServerControl>(server_control_servant);
    bind_object(server_control_obj.in(), "ServerControl.obj");
    DBG(2, "ServerControl object created and registered in Naming Service");
    return true;
}

void SrvNamedORB::orb_thread_run()
{
    if(orb_task)
        return;
    orb_task = new OrbTask(*this);
}

void SrvNamedORB::orb_wait()
{
    if(!orb_task)
        return;
    orb_task->wait();
}

//---------------------------- OrbTask methods -----------------------------

OrbTask::OrbTask(SrvNamedORB& _named_orb):named_orb_ref(_named_orb)
{
    activate(THR_NEW_LWP|THR_JOINABLE);
}

int OrbTask::svc()
{
    DBG(2, "Starting ORB main loop: CORBA::ORB->run()");
    named_orb_ref.the_rootPOAMgr()->activate();
    named_orb_ref.theORB()->run();
    DBG(2, "CORBA::ORB main loop is done, shutting down this thread");
    return 0;
}

//------------------------------ EOF ---------------------------------------
