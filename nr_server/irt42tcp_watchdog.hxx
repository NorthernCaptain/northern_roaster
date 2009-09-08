#ifndef IRTCP_WATCHDOG_HXX
#define IRTCP_WATCHDOG_HXX

#include "dev_watchdog.hxx"
#include "srv_exception.hxx"

#include "lig_watchdog.hxx"

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"

// Class implements protocol for communication with IRTCP devices over tcp/ip

//indexes of channels in chan_data array
enum IRTCPChannels { irtcp_chan1=0, irtcp_chan2=2, irtcp_chan3=4, irtcp_chan4=6, irtcp_flags = 7};

//Maximum number of channels
const int irtcp_device_max_channels=8;

class IRTCPDevice: public BluesDevice
{
protected:
    std::string               dev_path; //unix path to device (com port) '/dev/ttyS0' - default
    int                       fd;  //device file descriptor
    int                       scan_timeout; //number of seconds between scans

    TempType                  irtcp_no_temp; //value that tells us - no temperature set in device
    TempType                  irtcp_base_temp; //base (room) temperature

    time_t                    action_time; //time of last action
    int                       action_wait_timeout; //number of seconds to ignore next action
    std::string               dev_address;

    std::string               host_name;   //host name to connect to via tcp
    int                       host_port;   //port number to connect to via tcp
    ACE_SOCK_Connector        host_connect;
    ACE_SOCK_Stream           host_peer;


    TempType                  chan_data[irtcp_device_max_channels];

    LIGDevice*                lig;

    void                      com_attr_set(int fd, int speed) throw(DeviceException);
    int                       open_com_port(const char *port_name, int speed) throw(DeviceException); //return fd of the port
    int                       read_bytes(int fd, char* orig_buf, int n_bytes) throw(DeviceException); //read excatly N bytes
    int                       read_channels(int fd, short int *ibuf) throw(DeviceException); //read channels into ibuf
    void                      convert_action_field(TempType&);

    std::string               get_checksum(const std::string from, int len);
    short                     decode_temp(const char* from);

    int                       tcp_connect();
    int                       tcp_write(const char* from, int sz);
    int                       tcp_read_string(std::string& to);
public:
    IRTCPDevice(const std::string& _name);
    ~IRTCPDevice();

    void                      init_device() throw(DeviceException);
    void                      shutdown_device();
    int                       read_channels() throw(DeviceException);
    TempRow                   get_row(int idx=0);
    void                      wait_for_scan();

    bool                      is_online() { return fd!=-1;};
};

#endif
// ------------------------------[EOF]-------------------------------------
