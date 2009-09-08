#include "data_manager.hxx"
#include "debug.hxx"

DataManager::DataManager()
{
    DBG(4, "DataManager::constructor - ready for new data tasks");
}

DataManager::~DataManager()
{
    DBG(4, "DataManager::destructor - stopping");
    shutdown();
}

DataTask* DataManager::new_data_task(const BluesDevicePtr& device)
{
    return new DataTask(device);
}

void DataManager::activate()
{
    DBG(4, "DataManager::activate - activating " << tasks.size() << " data tasks");
    for(DataTaskNameMap::iterator it=tasks.begin(); it!=tasks.end(); it++)
	it->second->activate();
}

void DataManager::shutdown()
{
    DBG(4, "DataManager::shutdown - shutting down " << tasks.size() << " data tasks");
    for(DataTaskNameMap::iterator it=tasks.begin(); it!=tasks.end(); it++)
	it->second->shutdown();
    tasks.erase(tasks.begin(), tasks.end());
}

DataTaskPtr  DataManager::get_task(const std::string& name) throw (NoSuchData)
{
    DataTaskNameMap::iterator it=tasks.find(name);
    if(it==tasks.end())
	throw NoSuchData(std::string("DataManager::get_task with dev=") + name);
    return it->second;
}


// ------------------------------[EOF]-------------------------------------
