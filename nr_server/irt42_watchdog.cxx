// ------------------------------------------------------------------------
//                 IRT Device WatchDog class methods
// ------------------------------------------------------------------------
#include "irt42_watchdog.hxx"
#include "configman.hxx"
#include "timing.hxx"

#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.hxx"


IRTDevice::IRTDevice(const std::string& _name): BluesDevice(_name)
{
    char buf[1024]="/dev/ttyS0";
    fd=-1;

    lig = new LIGDevice(_name + "_ligctl");

    //reset all channel data
    for(int i=0;i<irt_device_max_channels;i++)
	chan_data[i]=0;

    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "irt_device", buf);
    dev_path=buf;

    strcpy(buf, "0001");
    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "irt_dev_addr", buf);
    dev_address=buf;

    irt_no_temp           = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "irt_no_temp_value", 28));
    irt_base_temp         = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "irt_base_temp_value", 28));
    scan_timeout          = CFGMAN::instance()->get_int_value(dev_name.c_str(), "irt_scan_timeout", 1);
    action_wait_timeout   = CFGMAN::instance()->get_int_value(dev_name.c_str(), "irt_action_timeout", 10);
}

IRTDevice::~IRTDevice()
{
    DBG(4, "IRT::destructor - deleting device " << dev_name);
    shutdown_device();
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "irt_device", dev_path.c_str());
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "irt_dev_addr", dev_address.c_str());
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irt_no_temp_value", irt_no_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irt_base_temp_value", irt_base_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irt_action_timeout", action_wait_timeout);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "irt_scan_timeout", scan_timeout);
    if(lig)
	delete lig;
}

void IRTDevice::init_device() throw(DeviceException)
{
    try
    {
	fd=open_com_port(dev_path.c_str(), B9600); //open IRT at 9600 speed
	lig->init_device();
    }

    catch(DeviceException& ex)
    {
	DBG(2, "IRT::init_device: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
}

void IRTDevice::shutdown_device()
{
    if(fd!=-1)
    {
	DBG(4, "IRT::shutdown: closing device [" << dev_name << "] fd = " << fd);
	close(fd);
	fd=-1;
    }
}

void IRTDevice::wait_for_scan()
{
    sleep(scan_timeout);
}

int IRTDevice::read_channels() throw(DeviceException)
{
    IRTChannels  chan_idx[]={ irt_chan1, irt_chan2, irt_chan3, irt_chan4, irt_flags};
    try
    {
	TempType tmp_data[20];
	read_channels(fd, tmp_data);
	lig->read_action(&tmp_data[5]);
	chan_data[irt_flags] = chan_data[irt_chan4] = tmp_data[5];
	chan_data[irt_chan1] = tmp_data[0];
	chan_data[irt_chan2] = tmp_data[1];
	chan_data[irt_chan3] = irt_base_temp*10;
	convert_action_field(chan_data[irt_flags]);
	for(unsigned i=0;i<sizeof(chan_idx)/sizeof(IRTChannels);i++)
	    DBG(6, "IRT::read_channels: CHAN[" << i << "]: hex=0x" 
		<< DHEX(chan_data[chan_idx[i]]) << " -> " << int(chan_data[chan_idx[i]]));
	DBG(6, "IRT::read_channels ===========================================");
    }
    catch(DeviceException& ex)
    {
	DBG(2, "IRT::read_channels: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
    return 1; //only one row at a time
}

TempRow IRTDevice::get_row(int)
{
    TempRow row;

    ACE_OS::time(&row.row_date);

    row.row[t_input]  = chan_data[irt_chan1] == irt_no_temp ? no_temp : chan_data[irt_chan1];
    row.row[t_output] = chan_data[irt_chan2] == irt_no_temp ? no_temp : chan_data[irt_chan2];
    row.row[t_base]   = chan_data[irt_chan3];
    row.row[t_action] = chan_data[irt_chan4];
    row.row[t_flags]  = chan_data[irt_flags];
    return row;
}

//------------------------- protected methods -------------------------------

//Convert special channel (action) from device values to our internal ones
//ignore next values during some timeout
void IRTDevice::convert_action_field(TempType& action)
{
    time_t now_t;
    ACE_OS::time(&now_t);
    DBG(6, "IRT:convert: [0x" << DHEX((int)(action)) << "]");
    if(action_time + action_wait_timeout > now_t)
    {
	DBG(6, "IRT::convert_action_field - ignore actions due timeout for " << dev_name);
	action=a_skip;
	return;
    }
    if((action & 0x8)==0)
    {
	DBG(6, "IRT::convert_action_field - detected LOAD action for " << dev_name);
	action=a_load_portion;
	action_time=now_t;
	return;
    }
    if((action & 0x4)==0)
    {
	DBG(6, "IRT::convert_action_field - detected UNLOAD action for " << dev_name);
	action=a_unload_portion;
	action_time=now_t;
	return;
    }
    action = a_nothing;
}


void IRTDevice::com_attr_set(int fd, int speed) throw(DeviceException)
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

int IRTDevice::open_com_port(const char *port_name, int speed) throw(DeviceException)
{
    int fp;
    DBG(4, "IRT::init: opening comport [" << port_name << "]");
    fp = ::open(port_name, O_RDWR);
    if(fp==-1)
    {
      	throw DeviceException(errno, dev_name + "/open(" + port_name + ")");
    }
    
    DBG(5, "IRT::init: opened " << port_name <<" with fd [" << fp << "], setting attrs");
    com_attr_set(fp, speed);

    DBG(4, "IRT::init: " << port_name << " - OK");

    return fp;
}

int IRTDevice::read_bytes(int fd, char* orig_buf, int bytes) throw(DeviceException)
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
	    DBG(7, "IRT::read: zero read with counter " << zero_read);
	    zero_read++;
	    if(zero_read>3)
		throw DeviceException(ENODATA, dev_name + "/zero_read");
	    continue;
	}
	zero_read=0;
	DBG(8, "IRT::read: " << siz <<" bytes -----------------------------");
	for(int i=0;i<siz;i++)
	    DBG(8, "IRT::read: [" << i << "]: 0x" << DHEX(((int)(buf[i]) & 0xff)) 
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


short int IRTDevice::decode_temp(char* from)
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

    return (short int)((f.f+0.01)*10.0);
}

std::string IRTDevice::get_checksum(const std::string from, int len)
{
    unsigned char sum=0;
    for(int i = 0; i< len;i++)
	sum += from[i];
    char buf[10];
    sprintf(buf, "%02X", sum & 0xff);
    return std::string(buf);
}

//Read channels from device
int IRTDevice::read_channels(int fd, short int *ibuf) throw(DeviceException)
{
//    TIMEOBJ("irt::read_channels");
    int siz;
    char buf[128]="";
    std::string snd = "$";
    snd += dev_address + "RR000010";
    snd += get_checksum(snd, snd.length()) + "\r";


    siz=write(fd, snd.c_str(), snd.length());

    DBG(7, "IRT::read_channels: wrote " << siz << " bytes");
    siz = read_bytes(fd, buf, 0); //read back data for channel 1

    if(siz>1)
    {
	DBG(7, "IRT::read_channels: read:" << buf);

	snd = get_checksum(buf, siz-2);
	if(buf[siz-2] != snd[0] || buf[siz-1]!=snd[1])
	{
	    DBG(5, "IRT::read_channels: WRONG checksum: " << (buf + siz -2) 
		<< " need: " << snd);
	    return 0;
	}


	*ibuf = decode_temp(buf+7); //first channel starts from 7 symbol
	ibuf++;
	*ibuf = decode_temp(buf+23); //second from 23 symbol
    }
    return 1;
}

// ------------------------------[EOF]-------------------------------------
