// ------------------------------------------------------------------------
//                 IRTCP Device WatchDog class methods
// ------------------------------------------------------------------------
#include "irt42tcp_watchdog.hxx"
#include "configman.hxx"
#include "timing.hxx"

#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.hxx"


IRTCPDevice::IRTCPDevice(const std::string& _name): BluesDevice(_name)
{
    char buf[1024]="/dev/ttyS0";
    fd=-1;

    lig = new LIGDevice(_name + "_ligctl");

    action_time = 0;

    //reset all channel data
    for(int i=0;i<irtcp_device_max_channels;i++)
	chan_data[i]=0;

    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "irtcp_device", buf);
    dev_path=buf;

    strcpy(buf, "0001");
    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "irtcp_dev_addr", buf);
    dev_address=buf;

    strcpy(buf, "localhost");
    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "irtcp_hostname", buf);
    host_name=buf;
    host_port  = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcp_hostport", 6791));


    irtcp_no_temp           = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcp_no_temp_value", 28));
    irtcp_base_temp         = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcp_base_temp_value", 28));
    scan_timeout          = CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcp_scan_timeout", 1);
    action_wait_timeout   = CFGMAN::instance()->get_int_value(dev_name.c_str(), "irtcp_action_timeout", 10);
}

IRTCPDevice::~IRTCPDevice()
{
    DBG(4, "IRTCP::destructor - deleting device " << dev_name);
    shutdown_device();
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "irtcp_device", dev_path.c_str());
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "irtcp_dev_addr", dev_address.c_str());
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcp_no_temp_value", irtcp_no_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcp_base_temp_value", irtcp_base_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcp_action_timeout", action_wait_timeout);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcp_scan_timeout", scan_timeout);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irtcp_hostport", host_port);
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "irtcp_hostname", host_name.c_str());
    if(lig)
	delete lig;
}

int IRTCPDevice::tcp_connect()
{
    ACE_INET_Addr peer_addr;
    peer_addr.set(host_port, host_name.c_str());
    
    if(host_connect.connect(host_peer, peer_addr)==-1)
    {
	DBG(2, "IRTCP::init_device: can't connect to " << host_name << ":" << host_port << ", trying later, (10 sec)");
	ACE_OS::sleep(10);
	return -1;
    }
    DBG(7, "IRTCP::connect: connected to device at host: " << host_name << ":" << host_port);
    return 0;
}

void IRTCPDevice::init_device() throw(DeviceException)
{
    try
    {
	tcp_connect();
		lig->init_device();
    }

    catch(DeviceException& ex)
    {
	DBG(2, "IRTCP::init_device: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
}

void IRTCPDevice::shutdown_device()
{
    if(fd!=-1)
    {
	DBG(4, "IRTCP::shutdown: closing device [" << dev_name << "] fd = " << fd);
	close(fd);
	fd=-1;
    }
}

void IRTCPDevice::wait_for_scan()
{
    sleep(scan_timeout);
}

int IRTCPDevice::read_channels() throw(DeviceException)
{
    IRTCPChannels  chan_idx[]={ irtcp_chan1, irtcp_chan2, irtcp_chan3, irtcp_chan4, irtcp_flags};
    try
    {
	TempType tmp_data[20]={a_nothing, a_nothing, a_nothing, a_nothing, a_nothing, a_nothing, a_nothing, a_nothing, a_nothing};
	while(!read_channels(fd, tmp_data))
	{
	    DBG(6, "IRTCP: nothing to process, device is deaf");
	    ACE_OS::sleep(5);
	}
	//lig->read_action(&tmp_data[5]);
	//chan_data[irtcp_flags] = chan_data[irtcp_chan4] = tmp_data[5];
	chan_data[irtcp_flags] = chan_data[irtcp_chan4] = 0xff;
	chan_data[irtcp_chan1] = tmp_data[0];
	chan_data[irtcp_chan2] = tmp_data[1];
	chan_data[irtcp_chan3] = irtcp_base_temp*10;
	convert_action_field(chan_data[irtcp_flags]);
	for(unsigned i=0;i<sizeof(chan_idx)/sizeof(IRTCPChannels);i++)
	    DBG(6, "IRTCP::read_channels: CHAN[" << i << "]: hex=0x" 
		<< DHEX(chan_data[chan_idx[i]]) << " -> " << int(chan_data[chan_idx[i]]));
	DBG(6, "IRTCP::read_channels ===========================================");
    }
    catch(DeviceException& ex)
    {
	DBG(2, "IRTCP::read_channels: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
    return 1; //only one row at a time
}

TempRow IRTCPDevice::get_row(int)
{
    TempRow row;

    ACE_OS::time(&row.row_date);

    row.row[t_input]  = chan_data[irtcp_chan1] == irtcp_no_temp ? no_temp : chan_data[irtcp_chan1];
    row.row[t_output] = chan_data[irtcp_chan2] == irtcp_no_temp ? no_temp : chan_data[irtcp_chan2];
    row.row[t_base]   = chan_data[irtcp_chan3];
    row.row[t_action] = chan_data[irtcp_chan4];
    row.row[t_flags]  = chan_data[irtcp_flags];
    row.daq[0]=row.daq[1]=0;
    return row;
}

//------------------------- protected methods -------------------------------

//Convert special channel (action) from device values to our internal ones
//ignore next values during some timeout
void IRTCPDevice::convert_action_field(TempType& action)
{
    time_t now_t;
    ACE_OS::time(&now_t);
    DBG(6, "IRTCP:convert: [0x" << DHEX((int)(action)) << "]");
    if(action_time + action_wait_timeout > now_t)
    {
	DBG(6, "IRTCP::convert_action_field - ignore actions due timeout for " << dev_name);
	action=a_skip;
	return;
    }
    if((action & 0x8)==0)
    {
	DBG(6, "IRTCP::convert_action_field - detected LOAD action for " << dev_name);
	action=a_load_portion;
	action_time=now_t;
	return;
    }
    if((action & 0x4)==0)
    {
	DBG(6, "IRTCP::convert_action_field - detected UNLOAD action for " << dev_name);
	action=a_unload_portion;
	action_time=now_t;
	return;
    }
    action = a_nothing;
}


void IRTCPDevice::com_attr_set(int fd, int speed) throw(DeviceException)
{
    struct termios tio;           /* tty line characteristic structure */
    int i;

    if(tcgetattr (fd, &tio)==-1)
	throw DeviceException(errno, dev_name + "/tcgetattr");

    for(i=0;i<NCCS;i++)
    {
	tio.c_cc[i]=0;
    }

    cfmakeraw(&tio);

    //handle input characters
    tio.c_iflag |= IGNBRK;
    tio.c_iflag &= ~(BRKINT);
    tio.c_iflag &= ~(INPCK | ISTRIP); //parity checking and stripping of parity bit
    tio.c_iflag |= INPCK;
    tio.c_iflag &= ~(PARMRK);
    tio.c_iflag &= ~(INLCR);
    tio.c_iflag &= ~(IGNCR);
    tio.c_iflag &= ~(ICRNL);
    tio.c_iflag &= ~(IUCLC);
    tio.c_iflag &= ~(IXON | IXOFF | IXANY); //no software flow control
    tio.c_iflag &= ~(IMAXBEL);

    //handle output characters
    tio.c_oflag &= ~(OPOST); //raw output

    //control options
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~CSTOPB; //1 stop bit
    tio.c_cflag |= CREAD;
    tio.c_cflag &= ~PARENB; //no parity
    tio.c_cflag &= ~(HUPCL);
    tio.c_cflag |= CLOCAL;
    tio.c_cflag &= ~(CRTSCTS); //no hardware flow control
    cfsetispeed(&tio, speed);
    cfsetospeed(&tio, speed);

    //local options (convertions)
    tio.c_lflag &= ~(ISIG);
    tio.c_lflag &= ~(ICANON);
    tio.c_lflag &= ~(XCASE);
    tio.c_lflag &= ~(ECHO);
    tio.c_lflag &= ~(ECHOE);
    tio.c_lflag &= ~(ECHOK);
    tio.c_lflag &= ~(ECHONL);
    tio.c_lflag &= ~(ECHOCTL);
    tio.c_lflag &= ~(ECHOPRT);
    tio.c_lflag &= ~(ECHOKE);
    tio.c_lflag &= ~(NOFLSH);
    tio.c_lflag &= ~(TOSTOP);
    tio.c_lflag &= ~(IEXTEN);

    //Wait for fisrt character from device for 2 seconds
    tio.c_cc[VMIN]  = 0;
    tio.c_cc[VTIME] = 20;  //sec * 10

    if(tcsetattr (fd, TCSAFLUSH, &tio)==-1)
	throw DeviceException(errno, dev_name + "/tcsetattr");

    sleep(1); //need to wait while device is thinking
}

int IRTCPDevice::open_com_port(const char *port_name, int speed) throw(DeviceException)
{
    int fp;
    DBG(4, "IRTCP::init: opening comport [" << port_name << "]");
    fp = ::open(port_name, O_RDWR);
    if(fp==-1)
    {
      	throw DeviceException(errno, dev_name + "/open(" + port_name + ")");
    }
    
    DBG(5, "IRTCP::init: opened " << port_name <<" with fd [" << fp << "], setting attrs");
    com_attr_set(fp, speed);

    DBG(4, "IRTCP::init: " << port_name << " - OK");

    return fp;
}

int IRTCPDevice::read_bytes(int fd, char* orig_buf, int bytes) throw(DeviceException)
{
    int received=0, siz;
    char* buf=orig_buf;
    int zero_read=0;
    while(true)
    {
	siz=read(fd, buf, 1);
	if(siz==-1)
	    throw DeviceException(errno, dev_name + "/read");

	if(siz==0)
	{
	    DBG(7, "IRTCP::read: zero read with counter " << zero_read);
	    zero_read++;
	    if(zero_read>3)
		throw DeviceException(ENODATA, dev_name + "/zero_read");
	    continue;
	}
	zero_read=0;
	DBG(8, "IRTCP::read: " << siz <<" bytes -----------------------------");
	for(int i=0;i<siz;i++)
	    DBG(8, "IRTCP::read: [" << i << "]: 0x" << DHEX(((int)(buf[i]) & 0xff)) 
		<< " -> " << buf[i]);
	if(*buf == '\r')
	{
	    break;
	}
	received += siz;
	buf+=siz;
    }

    *buf=0;

    return received;
}

union Float
{
    unsigned char ar[4];
    float         f;
    unsigned int  i;
};


short int IRTCPDevice::decode_temp(const char* from)
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

    DBG(7, "IRTCP:decode_temp: char: " << buf << " float: " << f.f << " int: " << (short int)((f.f+0.01)*10.0));
    return (short int)((f.f+0.01)*10.0);
}

std::string IRTCPDevice::get_checksum(const std::string from, int len)
{
    unsigned char sum=0;
    for(int i = 0; i< len;i++)
	sum += from[i];
    char buf[10];
    sprintf(buf, "%02X", sum & 0xff);
    return std::string(buf);
}

const int WAIT_TIME = 20;

int IRTCPDevice::tcp_write(const char* from, int sz)
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
    DBG(7, "IRTCP::tcp_write: wrote " << ret << " bytes: " << from);
    if(ret == -1)
    {
	DBG(4, "TCP send error: " << ACE_OS::strerror(ACE_OS::last_error()));
	host_peer.close();
	return -1;
    }
    
    return sz;
}

int IRTCPDevice::tcp_read_string(std::string& to)
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
int IRTCPDevice::read_channels(int fd, short int *ibuf) throw(DeviceException)
{
//    TIMEOBJ("irtcp::read_channels");
    int siz;
    std::string from_host;
    std::string snd = "$";
    snd += dev_address + "RR000010";
    snd += get_checksum(snd, snd.length()) + "\n";


    siz=tcp_write(snd.c_str(), snd.length());

    DBG(7, "IRTCP::read_channels: wrote " << siz << " bytes");

    if(siz == -1)
	return 0;

    siz = tcp_read_string(from_host); //read back data for channel 1

    if(siz>1)
    {
	DBG(7, "IRTCP::read_channels: read:" << from_host);

	snd = get_checksum(from_host.c_str(), siz-2);
	if(from_host[siz-2] != snd[0] || from_host[siz-1]!=snd[1])
	{
	    DBG(5, "IRTCP::read_channels: WRONG checksum: " << (from_host.c_str() + siz -2) 
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
