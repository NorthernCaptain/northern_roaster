#ifndef AC2_WATCHDOG_HXX
#define AC2_WATCHDOG_HXX

#include "dev_watchdog.hxx"
#include "srv_exception.hxx"

// Class implements protocol for communication with OWEN AC2 + TRM34 devices

//indexes of channels in chan_data array
enum AC2Channels { ac2_chan1=0, ac2_chan2=2, ac2_chan3=4, ac2_chan4=6, ac2_flags = 7};

//Maximum number of channels
const int device_max_channels=32;

class AC2Device: public BluesDevice
{
protected:
    std::string               dev_path; //unix path to device (com port) '/dev/ttyS0' - default
    int                       fd;  //device file descriptor
    int                       scan_timeout; //number of seconds between scans

    TempType                  ac2_no_temp; //value that tells us - no temperature set in device
    TempType                  ac2_load_portion_min; //value to detect loading of portion of coffee
    TempType                  ac2_unload_portion_min; //value to detect unloading
    TempType                  ac2_load_portion_max; //value to detect loading of portion of coffee
    TempType                  ac2_unload_portion_max; //value to detect unloading

    time_t                    action_time; //time of last action
    int                       action_wait_timeout; //number of seconds to ignore next action

    TempType                  chan_data[device_max_channels];

    void                      com_attr_set(int fd, int speed) throw(DeviceException);
    int                       open_com_port(const char *port_name, int speed) throw(DeviceException); //return fd of the port
    int                       read_bytes(int fd, char* orig_buf, int n_bytes) throw(DeviceException); //read excatly N bytes
    int                       read_channels(int fd, short int *ibuf) throw(DeviceException); //read channels into ibuf
    void                      convert_action_field(TempType&);

public:
    AC2Device(const std::string& _name);
    ~AC2Device();

    void                      init_device() throw(DeviceException);
    void                      shutdown_device();
    int                       read_channels() throw(DeviceException);
    TempRow                   get_row(int idx=0);
    void                      wait_for_scan();

    bool                      is_online() { return fd!=-1;};
};

#endif
// ------------------------------[EOF]-------------------------------------
