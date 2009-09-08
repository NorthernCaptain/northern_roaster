#ifndef STORAGEDB_HXX
#define STORAGEDB_HXX
#include "nr_dataflow.hxx"

class ServerStorageDB: public RoastStorage
{
protected:
    int                       roast_wh_id;

public:
    ServerStorageDB(int ruid_=0, const char* roaster_="unregistered");
    ~ServerStorageDB();
  
    virtual void              save_new_roast();
    virtual void              new_session();
    virtual void              start_session(time_t tim=0);
    virtual void              end_session(time_t tim=0);
    virtual void              start_loading(time_t tim=0);
    virtual void              start_unloading(time_t tim=0);
    virtual void              finish_session();
};

#endif

