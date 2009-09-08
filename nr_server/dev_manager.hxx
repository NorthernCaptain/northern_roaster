#ifndef DEV_MANAGER_HXX
#define DEV_MANAGER_HXX

#include "ace/Singleton.h"
#include <map>
#include <string>
#include "dev_watchdog.hxx"
#include "ac2_watchdog.hxx"
#include "emu_watchdog.hxx"
#include "lig_watchdog.hxx"
#include "irt42_watchdog.hxx"
#include "irt42tcp_watchdog.hxx"
#include "irt42tcpdaq_watchdog.hxx"
#include "adv1751_watchdog.hxx"

#include "dev_task.hxx"

class DevManager;
typedef std::map< std::string, BluesDevice* 
		 (DevManager::*)(const std::string&) >  DevCreateMap;

typedef std::vector<DevTaskPtr>                         DevTaskVec;

const int dev_max_devices=100;

class DevManager
{
    DevCreateMap              create_map;
    DevTaskVec                tasks;

    BluesDevice*              new_ac2_trm34(const std::string& _name) { return new AC2Device(_name);};
    BluesDevice*              new_emu_trm34(const std::string& _name) { return new EMUDevice(_name);};
    BluesDevice*              new_lig_adc  (const std::string& _name) { return new LIGDevice(_name);};
    BluesDevice*              new_irt_adc  (const std::string& _name) { return new IRTDevice(_name);};
    BluesDevice*              new_irtcp_adc  (const std::string& _name) { return new IRTCPDevice(_name);};
    BluesDevice*              new_irtcp_daq  (const std::string& _name) { return new IRTCPDAQDevice(_name);};
    BluesDevice*              new_advantech1751  (const std::string& _name) { return new ADV1751Device(_name);};
public:
    DevManager();
    ~DevManager();

    void                      activate();
    void                      shutdown();
    BluesDevicePtr            get_device(const std::string& devname);
};

typedef ACE_Singleton<DevManager, ACE_Thread_Mutex>     DEVMAN;

#endif
// ------------------------------[EOF]-------------------------------------
