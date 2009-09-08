#include "ncorba.hxx"

//Implementaion of CORBA helpers - classes that ease work with CORBA

//---------------------------- NamedORB -------------------------------
NamedORB::NamedORB(int argc, 
                    ACE_TCHAR** argv, 
                    const ACE_TCHAR* name_service,
                    const ACE_TCHAR* name_path, bool create_ns_path)
{
    nshelper=0;

    DBG(2, "Initializing CORBA::ORB...");
    orb = CORBA::ORB_init(argc, argv);
    DBG(2, "Connecting to NameService: [" << name_service << "]...");
    nshelper = new NameService (orb.in());
    nshelper->Bootstrap(name_service);
    std::string ns_path=std::string("/Northern/Captain/") + name_path;
    DBG(2, "Searching NS_PATH: [" << ns_path << "]...");
    nshelper->SetContext(ns_path, create_ns_path);
}

NamedORB::~NamedORB()
{
    if(nshelper)
    {
        DBG(2, "Deleting NameService...");
        delete nshelper;
    }
    DBG(2, "Destroing ORB...");
    orb->destroy();
    DBG(2, "Work with CORBA is done");
}


