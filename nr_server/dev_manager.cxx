#include "debug.hxx"
#include "dev_manager.hxx"
#include "configman.hxx"
#include "srv_exception.hxx"
#include "data_manager.hxx"

#include "boost/lexical_cast.hpp"

DevManager::DevManager()
{
    create_map["ac2_trm34"] = &DevManager::new_ac2_trm34;
    create_map["emu_trm34"] = &DevManager::new_emu_trm34;
    create_map["lig_adc"]   = &DevManager::new_lig_adc;
    create_map["irt_adc"]   = &DevManager::new_irt_adc;
    create_map["irtcp_adc"]   = &DevManager::new_irtcp_adc;
    create_map["irtcp_daq"]   = &DevManager::new_irtcp_daq;
    create_map["adv1751"] = &DevManager::new_advantech1751;

    for(int i=0;i<dev_max_devices;i++) //we will handle no more than 100 devices ;)
    {
	std::string d_name="dev_name_" + boost::lexical_cast<std::string>(i);
	std::string d_type="dev_type_" + boost::lexical_cast<std::string>(i);

	char buf[2048];
	buf[0]=0;
	CFGMAN::instance()->get_cstring_value("devices", d_name.c_str(), buf);
	if(buf[0]==0)
	{
	    DBG(5, "DevManager::constructor: read " << tasks.size() << " devices");
	    break;
	}
	d_name=buf;
	buf[0]=0;
	CFGMAN::instance()->get_cstring_value("devices", d_type.c_str(), buf);
	if(buf[0]==0)
	{
	    DBG(0, "DevManager::constructor: ERROR: type not set for device " << d_name << " skipping it");
	    continue;
	}
	d_type=buf;

	//creating device task and device watchdog

	BluesDevice* (DevManager::*method)(const std::string&);
	method=create_map[d_type];
	if(method==0)
	{
	    DBG(0, "DevManager::constructor: ERROR: unknown device type " <<
		d_type << " for device " << d_name << ", skipping it");
	    continue;
	}
	
	DBG(4, "DevManager::constructor: Creating device process for device " <<d_name << " type=" << d_type);
	BluesDevicePtr dev((this->*method)(d_name));
	DevTaskPtr task(new DevTask(dev));

	DataTaskPtr dtask(new DataTask(dev));
	task->set_notifier(new DataTaskNotifier(dtask));
	DATAMAN::instance()->insert(dtask, dev->get_name());

	tasks.push_back(task);
    }
    if(tasks.size()==0) //no tasks -> throw
	throw SrvException(ENODEV, "DevManager::constructor", 
			   "zero (0) devices were configured, edit your cfg file");
}

DevManager::~DevManager()
{
    shutdown();
}

void DevManager::activate()
{
    DBG(2, "DevManager::activate - activating " << tasks.size() << " device tasks");
    for(unsigned int i=0;i < tasks.size(); i++)
	tasks[i]->activate();
}

void DevManager::shutdown()
{
    DBG(2, "DevManager::shutdown - stopping " << tasks.size() << " device tasks");
    for(unsigned int i=0;i < tasks.size(); i++)
    {
	tasks[i]->shutdown();
	//	delete tasks[i];
    }
    tasks.clear();
}

BluesDevicePtr DevManager::get_device(const std::string& devname)
{
    
    for(unsigned int i=0;i < tasks.size(); i++)
    {
	if(tasks[i]->get_device()->get_name()==devname)
	    return tasks[i]->get_device();
    }
    return BluesDevicePtr((BluesDevice*)0);
}
// ------------------------------[EOF]-------------------------------------

