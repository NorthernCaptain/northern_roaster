#ifndef IRT_WATCHDOG_HXX
#define IRT_WATCHDOG_HXX

#include "dev_watchdog.hxx"
#include "srv_exception.hxx"

#include "lig_watchdog.hxx"
// Class implements protocol for communication with OWEN IRT + TRM34 devices

//indexes of channels in chan_data array
enum IRTChannels { irt_chan1=0, irt_chan2=2, irt_chan3=4, irt_chan4=6, irt_flags = 7};

//Maximum number of channels
const int irt_device_max_channels=8;

class IRTDevice: public BluesDevice
{
protected:
    std::string               dev_path; //unix path to device (com port) '/dev/ttyS0' - default
    int                       fd;  //device file descriptor
    int                       scan_timeout; //number of seconds between scans

    TempType                  irt_no_temp; //value that tells us - no temperature set in device
    TempType                  irt_base_temp; //base (room) temperature

    time_t                    action_time; //time of last action
    int                       action_wait_timeout; //number of seconds to ignore next action
    std::string               dev_address;

    TempType                  chan_data[irt_device_max_channels];

    LIGDevice*                lig;

    void                      com_attr_set(int fd, int speed) throw(DeviceException);
    int                       open_com_port(const char *port_name, int speed) throw(DeviceException); //return fd of the port
    int                       read_bytes(int fd, char* orig_buf, int n_bytes) throw(DeviceException); //read excatly N bytes
    int                       read_channels(int fd, short int *ibuf) throw(DeviceException); //read channels into ibuf
    void                      convert_action_field(TempType&);

    std::string               get_checksum(const std::string from, int len);
    short                     decode_temp(char* from);

public:
    IRTDevice(const std::string& _name);
    ~IRTDevice();

    void                      init_device() throw(DeviceException);
    void                      shutdown_device();
    int                       read_channels() throw(DeviceException);
    TempRow                   get_row(int idx=0);
    void                      wait_for_scan();

    bool                      is_online() { return fd!=-1;};
};

#endif
// ------------------------------[EOF]-------------------------------------
