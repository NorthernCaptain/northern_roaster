// ------------------------------------------------------------------------
//                 IRTCPDAQ Device WatchDog class methods
// ------------------------------------------------------------------------
#include "irt42tcpdaq_watchdog.hxx"
#include "configman.hxx"
#include "timing.hxx"
#include "dev_manager.hxx"

#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.hxx"


IRTCPDAQDevice::IRTCPDAQDevice(const std::string& _name): BluesDevice(_name)
{
    char buf[1024]="/dev/ttyS0";
    fd=-1;
    daqdev = 0;

    action_time = 0;

    DBG(4, "IRTCPDAQ::contructor for device " << dev_name);
    //reset all channel data
    for(int i=0;i<irtcpdaq_device_max_channels;i++)
	chan_data[i]=0;

    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "irtcpdaq_device", buf);
    dev_path=buf;

    strcpy(buf, "0001");
    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "irtcpdaq_dev_addr", buf);
    dev_address=buf;

    strcpy(buf, "localhost");
    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "irtcpdaq_hostname", buf);
    host_name=buf;
    host_port  = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcpdaq_hostport", 6791));


    irtcpdaq_no_temp           = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcpdaq_no_temp_value", 28));
    irtcpdaq_base_temp         = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcpdaq_base_temp_value", 28));
    scan_timeout          = CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcpdaq_scan_timeout", 1);
    action_wait_timeout   = CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcpdaq_action_timeout", 10);
}

IRTCPDAQDevice::~IRTCPDAQDevice()
{
    DBG(4, "IRTCPDAQ::destructor - deleting device " << dev_name);
    shutdown_device();
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "irtcpdaq_device", dev_path.c_str());
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "irtcpdaq_dev_addr", dev_address.c_str());
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcpdaq_no_temp_value", irtcpdaq_no_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcpdaq_base_temp_value", irtcpdaq_base_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcpdaq_action_timeout", action_wait_timeout);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcpdaq_scan_timeout", scan_timeout);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcpdaq_hostport", host_port);
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "irtcpdaq_hostname", host_name.c_str());
}

int IRTCPDAQDevice::tcp_connect()
{
    ACE_INET_Addr peer_addr;
    peer_addr.set(host_port, host_name.c_str());
    
    if(host_connect.connect(host_peer, peer_addr)==-1)
    {
	DBG(2, "IRTCPDAQ::init_device: can't connect to " << host_name << ":" << host_port << ", trying later, (10 sec)");
	ACE_OS::sleep(10);
	return -1;
    }
    DBG(7, "IRTCPDAQ::connect: connected to device at host: " << host_name << ":" << host_port);
    return 0;
}

void IRTCPDAQDevice::init_device() throw(DeviceException)
{
    try
    {
	tcp_connect();
	daqdev = (ADV1751Device*)(DEVMAN::instance()->get_device(dev_path).get());
	if(daqdev == 0)
	    DBG(0, "IRTCPDAQ::init_device: ERROR - no DAQ device found with name " << dev_path);
    }

    catch(DeviceException& ex)
    {
	DBG(2, "IRTCPDAQ::init_device: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
}

void IRTCPDAQDevice::shutdown_device()
{
    if(fd!=-1)
    {
	DBG(4, "IRTCPDAQ::shutdown: closing device [" << dev_name << "] fd = " << fd);
	close(fd);
	fd=-1;
    }
}

void IRTCPDAQDevice::wait_for_scan()
{
    sleep(scan_timeout);
}

int IRTCPDAQDevice::read_channels() throw(DeviceException)
{
    IRTCPDAQChannels  chan_idx[]={ irtcpdaq_chan1, irtcpdaq_chan2, irtcpdaq_chan3, irtcpdaq_chan4, irtcpdaq_flags};
    try
    {
	TempType tmp_data[20]={a_nothing, a_nothing, a_nothing, a_nothing, a_nothing, a_nothing, a_nothing, a_nothing, a_nothing};
	while(!read_channels(fd, tmp_data))
	{
	    DBG(6, "IRTCPDAQ: nothing to process, device is deaf");
	    ACE_OS::sleep(5);
	}
	//lig->read_action(&tmp_data[5]);
	//chan_data[irtcpdaq_flags] = chan_data[irtcpdaq_chan4] = tmp_data[5];
	read_daq();
	chan_data[irtcpdaq_flags] = chan_data[irtcpdaq_chan4] = 0xff;
	chan_data[irtcpdaq_chan1] = tmp_data[0];
	chan_data[irtcpdaq_chan2] = tmp_data[1];
	chan_data[irtcpdaq_chan3] = irtcpdaq_base_temp*10;
	convert_action_field(chan_data[irtcpdaq_flags]);
	for(unsigned i=0;i<sizeof(chan_idx)/sizeof(IRTCPDAQChannels);i++)
	    DBG(6, "IRTCPDAQ::read_channels: CHAN[" << i << "]: hex=0x" 
		<< DHEX(chan_data[chan_idx[i]]) << " -> " << int(chan_data[chan_idx[i]]));
	DBG(6, "IRTCPDAQ::read_channels ===========================================");
    }
    catch(DeviceException& ex)
    {
	DBG(2, "IRTCPDAQ::read_channels: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
    return 1; //only one row at a time
}

TempRow IRTCPDAQDevice::get_row(int)
{
    TempRow row;

    ACE_OS::time(&row.row_date);

    row.row[t_input]  = chan_data[irtcpdaq_chan1] == irtcpdaq_no_temp ? no_temp : chan_data[irtcpdaq_chan1];
    row.row[t_output] = chan_data[irtcpdaq_chan2] == irtcpdaq_no_temp ? no_temp : chan_data[irtcpdaq_chan2];
    row.row[t_base]   = chan_data[irtcpdaq_chan3];
    row.row[t_action] = chan_data[irtcpdaq_chan4];
    row.row[t_flags]  = chan_data[irtcpdaq_flags];
    row.daq[0]=daqval.low();
    row.daq[1]=daqval.hi();
    return row;
}

//------------------------- protected methods -------------------------------

//Convert special channel (action) from device values to our internal ones
//ignore next values during some timeout
void IRTCPDAQDevice::convert_action_field(TempType& action)
{
    time_t now_t;
    ACE_OS::time(&now_t);
    DBG(6, "IRTCPDAQ:convert: [0x" << DHEX(daqval.low()) << "]");
    if(action_time + action_wait_timeout > now_t)
    {
	DBG(6, "IRTCPDAQ::convert_action_field - ignore actions due timeout for " << dev_name);
	action=a_skip;
	return;
    }
    if(daqval.mask != 0ll)
    {
	if((daqval.low() & 0x10)==0)
	{
	    DBG(6, "IRTCPDAQ::convert_action_field - detected LOAD action for " << dev_name);
	    action=a_load_portion;
	    action_time=now_t;
	    return;
	}
	if((daqval.low() & 0x1)==0)
	{
	    DBG(6, "IRTCPDAQ::convert_action_field - detected UNLOAD action for " << dev_name);
	    action=a_unload_portion;
	    action_time=now_t;
	    return;
	}
    }
    action = a_nothing;
}


void IRTCPDAQDevice::read_daq()
{
    if(!daqdev)
	return;
    daqdev->get_and_reset(daqval);
}

union Float
{
    unsigned char ar[4];
    float         f;
    unsigned int  i;
};


short int IRTCPDAQDevice::decode_temp(const char* from)
{
    char  buf[32];
    union Float f;

    strncpy(buf, from, 8);
    buf[8]=0;

    sscanf(buf, "%X", &f.i);

    unsigned char ch;
    
    ch=f.ar[0];
    f.ar[0]=f.ar[3];
    f.ar[3]=ch;

    ch=f.ar[1];
    f.ar[1]=f.ar[2];
    f.ar[2]=ch;

    DBG(7, "IRTCPDAQ:decode_temp: char: " << buf << " float: " << f.f << " int: " << (short int)((f.f+0.01)*10.0));
    return (short int)((f.f+0.01)*10.0);
}

std::string IRTCPDAQDevice::get_checksum(const std::string from, int len)
{
    unsigned char sum=0;
    for(int i = 0; i< len;i++)
	sum += from[i];
    char buf[10];
    sprintf(buf, "%02X", sum & 0xff);
    return std::string(buf);
}

const int WAIT_TIME = 20;

int IRTCPDAQDevice::tcp_write(const char* from, int sz)
{
    if(host_peer.get_handle() == ACE_INVALID_HANDLE)
    {
	if(tcp_connect() == -1)
	{
	    return -1;
	}
    }
    
    ACE_Time_Value  timeout(WAIT_TIME);
    int ret = host_peer.send_n(from, sz, &timeout);
    DBG(7, "IRTCPDAQ::tcp_write: wrote " << ret << " bytes: " << from);
    if(ret == -1)
    {
	DBG(4, "TCP send error: " << ACE_OS::strerror(ACE_OS::last_error()));
	host_peer.close();
	return -1;
    }
    
    return sz;
}

int IRTCPDAQDevice::tcp_read_string(std::string& to)
{
    int ret = 0;
    
    if(host_peer.get_handle() == ACE_INVALID_HANDLE)
    {
	return 0;
    }
    char sym;
    ACE_Time_Value  timeout(WAIT_TIME);
    to.clear();

    while(true)
    {
	ret = ACE::handle_read_ready (host_peer.get_handle(),
                                      &timeout);
	
	if(ret == -1 && errno == ETIME)
	{
	    DBG(7, "TCP receive: timed out");
	    return 0;
	}

	if(ret == -1)
	{
	    DBG(7, "TCP receive wait error: " << ACE_OS::strerror(ACE_OS::last_error()));
	    host_peer.close();
	    return -1;
	}
	
	ret = host_peer.recv_n( &sym, 1, &timeout );
	
	if(ret == 0)
	{
	    DBG(7, "TCP connection closed by foreign host");
	    host_peer.close();
	    return 0;
	}

	if(ret == -1)
	{
	    DBG(7, "TCP receive error: " << ACE_OS::strerror(ACE_OS::last_error()));
	    host_peer.close();
	    return -1;
	}
	if(sym == '\n' || sym == '\r')
	    return to.length();
	to += sym;
    }

    return -5; //impossible situation
}



//Read channels from device
int IRTCPDAQDevice::read_channels(int fd, short int *ibuf) throw(DeviceException)
{
//    TIMEOBJ("irtcpdaq::read_channels");
    int siz;
    std::string from_host;
    std::string snd = "$";
    snd += dev_address + "RR000010";
    snd += get_checksum(snd, snd.length()) + "\n";


    siz=tcp_write(snd.c_str(), snd.length());

    DBG(7, "IRTCPDAQ::read_channels: wrote " << siz << " bytes");

    if(siz == -1)
	return 0;

    siz = tcp_read_string(from_host); //read back data for channel 1

    if(siz>1)
    {
	DBG(7, "IRTCPDAQ::read_channels: read:" << from_host);

	snd = get_checksum(from_host.c_str(), siz-2);
	if(from_host[siz-2] != snd[0] || from_host[siz-1]!=snd[1])
	{
	    DBG(5, "IRTCPDAQ::read_channels: WRONG checksum: " << (from_host.c_str() + siz -2) 
		<< " need: " << snd);
	    return 0;
	}
	
	
	*ibuf = decode_temp(from_host.c_str()+7); //first channel starts from 7 symbol
	ibuf++;
	*ibuf = decode_temp(from_host.c_str()+15); //second from 23 symbol
    } else
	return 0;
    return 1;
}

// ------------------------------[EOF]-------------------------------------
