#ifndef DATA_MANAGER_HXX
#define DATA_MANAGER_HXX

#include "ace/Singleton.h"
#include <map>
#include <string>
#include "srv_exception.hxx"
#include "data_task.hxx"

typedef std::map<std::string, DataTaskPtr>        DataTaskNameMap;

class DataManager
{
    DataTaskNameMap           tasks;
public:
    DataManager();
    ~DataManager();

    DataTask*                 new_data_task(const BluesDevicePtr& dev_ptr);
    void                      insert(const DataTaskPtr& task, const std::string& name)
    {
	tasks.insert(std::make_pair(name, task));
    };

    void                      activate();
    void                      shutdown();

    DataTaskPtr               get_task(const std::string& name) throw (NoSuchData);
};

typedef ACE_Singleton<DataManager, ACE_Thread_Mutex>    DATAMAN;

#endif
// ------------------------------[EOF]-------------------------------------
