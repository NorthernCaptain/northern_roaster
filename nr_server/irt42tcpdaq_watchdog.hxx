#ifndef IRTCPDAQ_WATCHDOG_HXX
#define IRTCPDAQ_WATCHDOG_HXX

#include "dev_watchdog.hxx"
#include "srv_exception.hxx"

#include "adv1751_watchdog.hxx"

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"

// Class implements protocol for communication with IRTCPDAQ devices over tcp/ip

//indexes of channels in chan_data array
enum IRTCPDAQChannels { irtcpdaq_chan1=0, irtcpdaq_chan2=2, irtcpdaq_chan3=4, irtcpdaq_chan4=6, irtcpdaq_flags = 7};

//Maximum number of channels
const int irtcpdaq_device_max_channels=8;

class IRTCPDAQDevice: public BluesDevice
{
protected:
    std::string               dev_path; //unix path to device (com port) '/dev/ttyS0' - default
    int                       fd;  //device file descriptor
    int                       scan_timeout; //number of seconds between scans

    TempType                  irtcpdaq_no_temp; //value that tells us - no temperature set in device
    TempType                  irtcpdaq_base_temp; //base (room) temperature

    time_t                    action_time; //time of last action
    int                       action_wait_timeout; //number of seconds to ignore next action
    std::string               dev_address;

    std::string               host_name;   //host name to connect to via tcp
    int                       host_port;   //port number to connect to via tcp
    ACE_SOCK_Connector        host_connect;
    ACE_SOCK_Stream           host_peer;


    TempType                  chan_data[irtcpdaq_device_max_channels];

    ADV1751Device*            daqdev;
    BitVal64                  daqval;

    int                       read_channels(int fd, short int *ibuf) throw(DeviceException); //read channels into ibuf
    void                      read_daq();
    void                      convert_action_field(TempType&);

    std::string               get_checksum(const std::string from, int len);
    short                     decode_temp(const char* from);

    int                       tcp_connect();
    int                       tcp_write(const char* from, int sz);
    int                       tcp_read_string(std::string& to);
public:
    IRTCPDAQDevice(const std::string& _name);
    ~IRTCPDAQDevice();

    void                      init_device() throw(DeviceException);
    void                      shutdown_device();
    int                       read_channels() throw(DeviceException);
    TempRow                   get_row(int idx=0);
    void                      wait_for_scan();

    bool                      is_online() { return fd!=-1;};
};

#endif
// ------------------------------[EOF]-------------------------------------
