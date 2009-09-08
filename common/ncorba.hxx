#ifndef NCORBA_HXX
#define NCORBA_HXX
#include "ncorba_header.hxx"
#include "NameService.h"

// Here we build classes to help and ease work with CORBA
// These helpers will do all dirty work for us...

//This class incapsulates CORBA:ORB_var and NamingService
//It inits CORBA, and destroys it on deletion
class NamedORB
{
protected:

    CORBA::ORB_var                   orb;
    NameService*                     nshelper;

public:
    NamedORB(int argc, 
            ACE_TCHAR** argv, 
            const ACE_TCHAR* name_service,
            const ACE_TCHAR* name_path, bool create_ns_path=false);
    virtual ~NamedORB();

    inline CORBA::ORB_var&           theORB() { return orb;};
    inline NameService*              theNS()  { return nshelper;};

//      template <class OBJTYPE> OBJTYPE* ref_by_name(const ACE_TCHAR* name)
//      {
//          return nshelper->resolve_name <OBJTYPE> (name);
//      };
};

struct ORBInit
{
    int                       argc;
    ACE_TCHAR               **argv;
    std::string               ns_loc;
    std::string               ns_name;

    ORBInit(int argc_=0, ACE_TCHAR** argv_=0, 
	    const std::string& ns_name_="", 
	    const std::string& ns_loc_="") : argc(argc_), argv(argv_), ns_loc(ns_loc_), ns_name(ns_name_)
    {
    };
};

#endif
