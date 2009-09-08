#ifndef ADV1751_WATCHDOG_HXX
#define ADV1751_WATCHDOG_HXX

#include "dev_watchdog.hxx"
#include "srv_exception.hxx"

#include <comedilib.h>

// Class implements protocol for ADV1751 (advantech pci-1751) devices

struct BitVal64
{
    unsigned long long   val, defval;
    unsigned long long   mask;

    BitVal64() { val = defval= 0x00ffffff00ffffffll; mask = 0ll;};
    bool change_if(unsigned long long newval) 
    { 
	unsigned long long newmask = val ^ newval; 
	if(newmask!=0ll && mask == 0ll)
	{ 
	    val = newval;
	    mask = newmask;
	}
	return newmask != 0ll; 
    };
    
    void reset() { mask = 0ll;};
    int hi() {  return (int)(val >> 32); };
    int low() { return (int)(val & 0xffffffffll);};
};

class ADV1751Device: public BluesDevice
{
protected:
    BitVal64                  value;
    std::string               comedi_devname;
    int                       subdev0, subdev1;
    int                       scan_timeout;

    comedi_t                  *device;
    ACE_Thread_Mutex          mtx;

public:
    ADV1751Device(const std::string& _name);
    ~ADV1751Device();

    void                      init_device();
    void                      shutdown_device();
    int                       read_channels();
    TempRow                   get_row(int idx=0);
    void                      wait_for_scan();

    bool                      is_online() { return 1;};
    void                      get_and_reset(BitVal64& to);
    
};

#endif
// ------------------------------[EOF]-------------------------------------
