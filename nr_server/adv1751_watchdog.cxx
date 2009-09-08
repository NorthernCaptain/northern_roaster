// ------------------------------------------------------------------------
//                 ADV1751 Device WatchDog class methods
// ------------------------------------------------------------------------
#include "adv1751_watchdog.hxx"
#include "configman.hxx"
#include "timing.hxx"
#include "debug.hxx"


ADV1751Device::ADV1751Device(const std::string& _name): BluesDevice(_name)
{
    DBG(4, "ADV1751::contructor for device " << dev_name);
    char buf[1024]="/dev/comedi0";

    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "comedi_device", buf);
    comedi_devname=buf;

    subdev0 = CFGMAN::instance()->get_int_value(dev_name.c_str(), "comedi_subdev0", 0);
    subdev1 = CFGMAN::instance()->get_int_value(dev_name.c_str(), "comedi_subdev1", 1);

    //scan_timeout not in seconds but in milliseconds = 1/1000 of sec.
    scan_timeout          = CFGMAN::instance()->get_int_value(dev_name.c_str(), "comedi_scan_timeout", 100);

    device = 0;
}

ADV1751Device::~ADV1751Device()
{
    DBG(4, "ADV1751::destructor - deleting device " << dev_name);
    shutdown_device();

    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "comedi_device", comedi_devname.c_str());
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "comedi_subdev0", subdev0);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "comedi_subdev1", subdev1);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "comedi_scan_timeout", scan_timeout);
}

void ADV1751Device::init_device()
{
    DBG(4, "ADV1751::init_device: " << dev_name << " using comedi device " << comedi_devname << " subdevices: " << subdev0 << ", " << subdev1);
    while(true)
    {
	device = comedi_open(comedi_devname.c_str());
	if(!device)
	{
	    DBG(0, "ADV1751 ERROR opening comedi device " << comedi_devname << " :" << comedi_strerror(comedi_errno()));
	    sleep(10);
	} else
	{
	    DBG(4, "ADV1751 - device opened successfully");
	    break;
	}
    }
}

void ADV1751Device::shutdown_device()
{
    DBG(4, "ADV1751::shutdown: closing device [" << dev_name << "]");
    if(device)
    {
	comedi_close(device);
    }
}

void ADV1751Device::wait_for_scan()
{
    struct timespec ts;

    ts.tv_sec = scan_timeout / 1000;
    ts.tv_nsec = (scan_timeout - ts.tv_sec*1000) * 1000000; //convert millisec to nanosec
    DBG(10, "ADV1751 sleep[" << scan_timeout << "] for " << ts.tv_sec << " sec, " << ts.tv_nsec  << " nanosec");
    ACE_OS::nanosleep(&ts);
}

int ADV1751Device::read_channels()
{
    //here we do aquisition from pci1751 device through comedi lib
    //we use digital signals, not analog.

    DBG(10, "ADV1751 start data aquisition from " << comedi_devname);
    
    if(!device) //we don't read from dead device
	return 0;

    unsigned int bits;
    unsigned long long val=0ll;

    int ret = comedi_dio_bitfield2(device, subdev1, 0, &bits, 0);
    if(ret<0)
    {
	DBG(0, "ADV1751 ERROR reading data from subdev " << subdev1 << ": " << comedi_strerror(comedi_errno()));	
    } else
    {
	val = bits;
	val <<= 32;
    }

    ret = comedi_dio_bitfield2(device, subdev0, 0, &bits, 0);
    if(ret<0)
    {
	DBG(0, "ADV1751 ERROR reading data from subdev " << subdev0 << ": " << comedi_strerror(comedi_errno()));	
    }
    else
	val |= bits;
    {
	ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
	if(value.change_if(val))
	    DBG(4, "ADV1751: Got new data on subdevs: " << DHEX(val) );
    }
  
    return 0; //we don't read real rows, we fill our own data structure
}

TempRow ADV1751Device::get_row(int)
{
    TempRow row;
    return row;
}


void ADV1751Device::get_and_reset(BitVal64& to)
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    to = value;
    value.reset();
}
//------------------------- protected methods -------------------------------

// ------------------------------[EOF]-------------------------------------
