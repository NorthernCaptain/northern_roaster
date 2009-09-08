// ------------------------------------------------------------------------
//                 AC2 Device WatchDog class methods
// ------------------------------------------------------------------------
#include "ac2_watchdog.hxx"
#include "configman.hxx"
#include "timing.hxx"

#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "debug.hxx"

AC2Device::AC2Device(const std::string& _name): BluesDevice(_name)
{
    char buf[1024]="/dev/ttyS0";
    fd=-1;

    //reset all channel data
    for(int i=0;i<device_max_channels;i++)
	chan_data[i]=0;

    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "ac2_device", buf);
    dev_path=buf;
    ac2_no_temp           = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "ac2_no_temp_value", -21846));
    ac2_load_portion_min  = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "ac2_load_min", -21846));
    ac2_load_portion_max  = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "ac2_load_max", -21846));
    ac2_unload_portion_min= (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "ac2_unload_min", a_unload_portion));
    ac2_unload_portion_max= (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "ac2_unload_max", a_unload_portion));
    scan_timeout          = CFGMAN::instance()->get_int_value(dev_name.c_str(), "ac2_scan_timeout", 1);
    action_wait_timeout   = CFGMAN::instance()->get_int_value(dev_name.c_str(), "ac2_action_timeout", 10);
}

AC2Device::~AC2Device()
{
    DBG(4, "AC2::destructor - deleting device " << dev_name);
    shutdown_device();
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "ac2_device", dev_path.c_str());
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "ac2_no_temp_value", ac2_no_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "ac2_load_min", ac2_load_portion_min);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "ac2_load_max", ac2_load_portion_max);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "ac2_unload_min", ac2_unload_portion_min);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "ac2_unload_max", ac2_unload_portion_max);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "ac2_action_timeout", action_wait_timeout);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "ac2_scan_timeout", scan_timeout);
}

void AC2Device::init_device() throw(DeviceException)
{
    try
    {
	fd=open_com_port(dev_path.c_str(), B9600); //open AC2 at 9600 speed
    }
    catch(DeviceException& ex)
    {
	DBG(2, "AC2::init_device: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
}

void AC2Device::shutdown_device()
{
    if(fd!=-1)
    {
	DBG(4, "AC2::shutdown: closing device [" << dev_name << "] fd = " << fd);
	close(fd);
	fd=-1;
    }
}

void AC2Device::wait_for_scan()
{
    sleep(scan_timeout);
}

int AC2Device::read_channels() throw(DeviceException)
{
    AC2Channels  chan_idx[]={ ac2_chan1, ac2_chan2, ac2_chan3, ac2_chan4, ac2_flags};
    try
    {
	read_channels(fd, chan_data);
	chan_data[ac2_flags] = chan_data[ac2_chan4];
	convert_action_field(chan_data[ac2_flags]);
	for(unsigned i=0;i<sizeof(chan_idx)/sizeof(AC2Channels);i++)
	    DBG(6, "AC2::read_channels: CHAN["
		<< i << "]: hex=" << DHEX(chan_data[chan_idx[i]])
		<< " -> " << int(chan_data[chan_idx[i]]));
	DBG(6, "AC2::read_channels ===========================================");
    }
    catch(DeviceException& ex)
    {
	DBG(2, "AC2::read_channels: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
    return 1; //only one row at a time
}

TempRow AC2Device::get_row(int)
{
    TempRow row;

    ACE_OS::time(&row.row_date);

    row.row[t_input]  = chan_data[ac2_chan1] == ac2_no_temp ? no_temp : chan_data[ac2_chan1];
    row.row[t_output] = chan_data[ac2_chan2] == ac2_no_temp ? no_temp : chan_data[ac2_chan2];
    row.row[t_base]   = chan_data[ac2_chan3] == ac2_no_temp ? no_temp : chan_data[ac2_chan3];
    row.row[t_action] = chan_data[ac2_chan4] == ac2_no_temp ? no_temp : chan_data[ac2_chan4];
    row.row[t_flags]  = chan_data[ac2_flags];
    return row;
}

//------------------------- protected methods -------------------------------

//Convert special channel (action) from device values to our internal ones
//ignore next values during some timeout
void AC2Device::convert_action_field(TempType& action)
{
    time_t now_t;
    ACE_OS::time(&now_t);
    if(action_time + action_wait_timeout > now_t)
    {
	DBG(6, "AC2::convert_action_field - ignore actions due timeout for " << dev_name);
	action=a_skip;
	return;
    }
    if(action >= ac2_load_portion_min && action <= ac2_load_portion_max)
    {
	DBG(6, "AC2::convert_action_field - detected LOAD action for " << dev_name);
	action=a_load_portion;
	action_time=now_t;
	return;
    }
    if(action >= ac2_unload_portion_min && action <= ac2_unload_portion_max)
    {
	DBG(6, "AC2::convert_action_field - detected UNLOAD action for " << dev_name);
	action=a_unload_portion;
	action_time=now_t;
	return;
    }
    action = a_nothing;
}


void AC2Device::com_attr_set(int fd, int speed) throw(DeviceException)
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
    tio.c_cflag |= CSTOPB; //2 stop bits
    tio.c_cflag |= CREAD;
    tio.c_cflag |= PARENB;
    tio.c_cflag &= ~(PARODD); //Even parity
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

    int status;

    //choosing first channel on AC2 network
    if(ioctl(fd, TIOCMGET, &status)==-1)
	throw DeviceException(errno, dev_name + "/ioctl_get");

    status &= ~TIOCM_RTS;
    if(ioctl(fd, TIOCMSET, &status)==-1)
	throw DeviceException(errno, dev_name + "/ioctl_set_rts");

    usleep(10000);

    status |= TIOCM_RTS;
    if(ioctl(fd, TIOCMSET, &status)==-1)
	throw DeviceException(errno, dev_name + "/ioctl_set_rts2");

    usleep(20000); 

    status &= ~TIOCM_RTS;
    if(ioctl(fd, TIOCMSET, &status)==-1)
	throw DeviceException(errno, dev_name + "/ioctl_set_rts3");

    sleep(2); //need to wait while device is thinking
}

int AC2Device::open_com_port(const char *port_name, int speed) throw(DeviceException)
{
    int fp;
    DBG(4, "AC2::init: opening comport [" << port_name << "]");
    fp = ::open(port_name, O_RDWR);
    if(fp==-1)
    {
      	throw DeviceException(errno, dev_name + "/open(" + port_name + ")");
    }
    
    DBG(5, "AC2::init: opened " << port_name <<" with fd [" << fp << "], setting attrs");
    com_attr_set(fp, speed);

    DBG(4, "AC2::init: " << port_name << " - OK");

    return fp;
}

int AC2Device::read_bytes(int fd, char* orig_buf, int bytes) throw(DeviceException)
{
    int received=0, siz;
    char* buf=orig_buf;
    int zero_read=0;
    while(bytes)
    {
	siz=read(fd, buf, bytes);
	if(siz==-1)
	    throw DeviceException(errno, dev_name + "/read");

	if(siz==0)
	{
	    DBG(7, "AC2::read: zero read with counter " << zero_read);
	    zero_read++;
	    if(zero_read>3)
		throw DeviceException(ENODATA, dev_name + "/zero_read");
	    continue;
	}
	zero_read=0;
	DBG(7, "AC2::read: " << siz <<" bytes -----------------------------");
	for(int i=0;i<siz;i++)
	    DBG(7, "AC2::read: [" << i << "]: 0x" << DHEX(((int)(buf[i]) & 0xff)) << " -> " << ((int)(buf[i]) & 0xff));
	received += siz;
	buf+=siz;
	bytes-=siz;
    }

    buf=orig_buf;

    return received;
}

//Read channels from device
int AC2Device::read_channels(int fd, short int *ibuf) throw(DeviceException)
{
    TIMEOBJ("ac2::read_channels");
    int siz;
    char ch;
    char buf[128];
    ch=0x71; //global sync command
    siz=write(fd, &ch, 1);
    DBG(7, "AC2::read_channels: wrote [0x" << DHEX(int(ch)) << "] = " << siz << "bytes");
    read_bytes(fd, buf, 2); //read back our command and ACK
    if(buf[1]!=0x55) //not an acknowledge byte
	throw DeviceException(EINVAL, dev_name + "/read_ack");

    ch=0x03; //read 16 bytes of memory (command)
    siz=write(fd, &ch, 1);
    DBG(7, "AC2::read_channels: wrote [0x" << DHEX(int(ch)) << "] = " << siz << "bytes");
    ch=0xa0; //start address
    siz=write(fd, &ch, 1);
    DBG(7, "AC2::read_channels: wrote [0x" << DHEX(int(ch)) << "] = " << siz << "bytes");
    read_bytes(fd, buf, 2); //read back our commands (echo)
    read_bytes(fd, (char*)ibuf, 17); //read 16 bytes of data + 1 CRC
    return 1;
}

// ------------------------------[EOF]-------------------------------------
