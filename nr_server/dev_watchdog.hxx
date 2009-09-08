#ifndef DEVWATCHDOG_HXX
#define DEVWATCHDOG_HXX

#include <string>
#include "boost/shared_ptr.hpp"
#include "nr_dataflow.hxx"

//Abstract class for all temperature devices and emulators

class BluesDevice
{
protected:
    std::string               dev_name;

public:
    BluesDevice(const std::string& _name);
    virtual ~BluesDevice() {};

    virtual void              init_device()=0;
    virtual void              shutdown_device()=0;
    virtual int               read_channels()=0; //return number of rows read
    virtual TempRow           get_row(int idx=0)=0;
    virtual void              wait_for_scan()=0;

    virtual bool              is_online()=0;

    const std::string&        get_name() { return dev_name;};
};

typedef boost::shared_ptr<BluesDevice>     BluesDevicePtr;

#endif
// ------------------------------[EOF]-------------------------------------
