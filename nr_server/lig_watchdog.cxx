// ------------------------------------------------------------------------
//                 LIG Device WatchDog class methods
// ------------------------------------------------------------------------
#include "lig_watchdog.hxx"
#include "configman.hxx"
#include "timing.hxx"

#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.hxx"


LIGDevice::LIGDevice(const std::string& _name): BluesDevice(_name)
{
    char buf[1024]="/dev/ttyS0";
    fd=-1;

    //reset all channel data
    for(int i=0;i<lig_device_max_channels;i++)
	chan_data[i]=0;

    CFGMAN::instance()->get_cstring_value(dev_name.c_str(), "lig_device", buf);
    dev_path=buf;
    lig_no_temp           = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "lig_no_temp_value", 28));
    lig_base_temp         = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "lig_base_temp_value", 28));
    scan_timeout          = CFGMAN::instance()->get_int_value(dev_name.c_str(), "lig_scan_timeout", 1);
    action_wait_timeout   = CFGMAN::instance()->get_int_value(dev_name.c_str(), "lig_action_timeout", 10);
}

LIGDevice::~LIGDevice()
{
    DBG(4, "LIG::destructor - deleting device " << dev_name);
    shutdown_device();
    CFGMAN::instance()->set_cstring_value(dev_name.c_str(), "lig_device", dev_path.c_str());
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "lig_no_temp_value", lig_no_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "lig_base_temp_value", lig_base_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "lig_action_timeout", action_wait_timeout);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "lig_scan_timeout", scan_timeout);
}

void LIGDevice::init_device() throw(DeviceException)
{
    try
    {
	fd=open_com_port(dev_path.c_str(), B9600); //open LIG at 9600 speed
    }
    catch(DeviceException& ex)
    {
	DBG(2, "LIG::init_device: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
}

void LIGDevice::shutdown_device()
{
    if(fd!=-1)
    {
	DBG(4, "LIG::shutdown: closing device [" << dev_name << "] fd = " << fd);
	close(fd);
	fd=-1;
    }
}

void LIGDevice::wait_for_scan()
{
    sleep(scan_timeout);
}

int LIGDevice::read_channels() throw(DeviceException)
{
    LIGChannels  chan_idx[]={ lig_chan1, lig_chan2, lig_chan3, lig_chan4, lig_flags};
    try
    {
	TempType tmp_data[20];
	read_channels(fd, tmp_data);
	chan_data[lig_flags] = chan_data[lig_chan4] = tmp_data[0];
	chan_data[lig_chan1] = get_temperature(tmp_data[0], t_input);
	chan_data[lig_chan2] = get_temperature(tmp_data[1], t_output);
	chan_data[lig_chan3] = lig_base_temp*10;
	convert_action_field(chan_data[lig_flags]);
	for(unsigned i=0;i<sizeof(chan_idx)/sizeof(LIGChannels);i++)
	    DBG(6, "LIG::read_channels: CHAN[" << i << "]: hex=0x" 
		<< DHEX(chan_data[chan_idx[i]]) << " -> " << int(chan_data[chan_idx[i]]));
	DBG(6, "LIG::read_channels ===========================================");
    }
    catch(DeviceException& ex)
    {
	DBG(2, "LIG::read_channels: EXCEPTION: " << ex.get_info());
	shutdown_device();
	throw;
    }
    return 1; //only one row at a time
}

struct TempInterval
{
	int from;
	int to;
	float delta;
	float end_diff;
};

static TempInterval correct_input_temp[] =
{/*
	{   0, 100,  13 , 34},
	{ 100, 125,  45 , 83},
        { 125, 149, 128 , 16},
        { 149, 162, 144 , 9 },
        { 162, 172, 153 ,-8 },
        { 172, 200, 145 ,-3 },
*/
	{   0, 300,   0, 0  },
	{ -10,   0,   0 , 0 }  //end of table
};   

static TempInterval correct_output_temp[] =
{
/*        {   0, 24 ,  -8 , 0 },
        {  25, 39 ,  -8 ,-1 },
        {  39, 96 ,  -7 , 0 },
        {  96,139 ,  -6 ,-3 },
        { 139,237 ,  -9 ,-8 },
        { 237,249 , -16 ,-2 },
        { 249,300 , -18 ,-8 },
*/
	{   0, 500,   0 , 0 }, 
        { -10,   0,   0 , 0 }  //end of table
};

TempType LIGDevice::get_temperature(TempType inp, TempChans which)
{
    int i;
    int val = (((inp>>6) & 0x3fc) | (inp & 0x3));
    float dev_temp=(276*(val-142)/1000 + lig_base_temp);

    TempInterval *correction = which == t_input ? correct_input_temp : correct_output_temp;

    float from = 0;
    float to = 0;
    for(i = 0; correction[i].from != -10; i++)
    {
       float raw_temp=dev_temp;
       from = correction[i].from;
       to = correction[i].to;
       if(dev_temp>from && dev_temp<=to)
       {
	dev_temp += correction[i].delta +
	            (dev_temp - from)/(to - from)*correction[i].end_diff;
	DBG(6, "LIG::get_temp: raw=" << raw_temp 
	    << ", delta=" << correction[i].delta << ", scale=" 
	    << float((dev_temp - from)/(to - from)*correction[i].end_diff)
	    << ", result=" << dev_temp);
	break;
       }
    }

    if(correction[i].from == -10) //we missed our region, take last one
	dev_temp  += correction[i-1].delta +
                     (dev_temp - from)/(to - from)*correction[i-1].end_diff;

    return (TempType)(round(dev_temp*10.0));
}

TempRow LIGDevice::get_row(int)
{
    TempRow row;

    ACE_OS::time(&row.row_date);

    row.row[t_input]  = chan_data[lig_chan1] == lig_no_temp ? no_temp : chan_data[lig_chan1];
    row.row[t_output] = chan_data[lig_chan2] == lig_no_temp ? no_temp : chan_data[lig_chan2];
    row.row[t_base]   = chan_data[lig_chan3];
    row.row[t_action] = chan_data[lig_chan4];
    row.row[t_flags]  = chan_data[lig_flags];
    return row;
}

//------------------------- protected methods -------------------------------

//Convert special channel (action) from device values to our internal ones
//ignore next values during some timeout
void LIGDevice::convert_action_field(TempType& action)
{
    time_t now_t;
    ACE_OS::time(&now_t);
    DBG(6, "LIG:convert: [0x" << DHEX((int)(action)) << "]");
    if(action_time + action_wait_timeout > now_t)
    {
	DBG(6, "LIG::convert_action_field - ignore actions due timeout for " << dev_name);
	action=a_skip;
	return;
    }
    if((action & 0x8)==0)
    {
	DBG(6, "LIG::convert_action_field - detected LOAD action for " << dev_name);
	action=a_load_portion;
	action_time=now_t;
	return;
    }
    if((action & 0x4)==0)
    {
	DBG(6, "LIG::convert_action_field - detected UNLOAD action for " << dev_name);
	action=a_unload_portion;
	action_time=now_t;
	return;
    }
    action = a_nothing;
}


void LIGDevice::com_attr_set(int fd, int speed) throw(DeviceException)
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

int LIGDevice::open_com_port(const char *port_name, int speed) throw(DeviceException)
{
    int fp;
    DBG(4, "LIG::init: opening comport [" << port_name << "]");
    fp = ::open(port_name, O_RDWR);
    if(fp==-1)
    {
      	throw DeviceException(errno, dev_name + "/open(" + port_name + ")");
    }
    
    DBG(5, "LIG::init: opened " << port_name <<" with fd [" << fp << "], setting attrs");
    com_attr_set(fp, speed);

    DBG(4, "LIG::init: " << port_name << " - OK");

    return fp;
}

int LIGDevice::read_bytes(int fd, char* orig_buf, int bytes) throw(DeviceException)
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
	    DBG(7, "LIG::read: zero read with counter " << zero_read);
	    zero_read++;
	    if(zero_read>3)
		throw DeviceException(ENODATA, dev_name + "/zero_read");
	    continue;
	}
	zero_read=0;
	DBG(7, "LIG::read: " << siz <<" bytes -----------------------------");
	for(int i=0;i<siz;i++)
	    DBG(7, "LIG::read: [" << i << "]: 0x" << DHEX(((int)(buf[i]) & 0xff)) 
		<< " -> " << ((int)(buf[i]) & 0xff));
	received += siz;
	buf+=siz;
	bytes-=siz;
    }

    buf=orig_buf;

    return received;
}

//Read channels from device
int LIGDevice::read_channels(int fd, short int *ibuf) throw(DeviceException)
{
//    TIMEOBJ("lig::read_channels");
    int siz;
    char ch;
    char buf[32];
    ch=0x00; //first channel
    siz=write(fd, &ch, 1);
    DBG(7, "LIG::read_channels 0: wrote [0x" << DHEX(int(ch)) << "] = " << siz << " bytes");
    read_bytes(fd, buf, 2); //read back data for channel 1

    *ibuf = (((short int)(buf[0]) << 8) & 0xff00) |
	((short int)(buf[1]) & 0xff);

    ibuf++;

    ch=0x01; //second channel
    siz=write(fd, &ch, 1);
    DBG(7, "LIG::read_channels 1: wrote [0x" << DHEX(int(ch)) << "] = " << siz << " bytes");
    read_bytes(fd, buf, 2); //read back data for channel 2
    *ibuf = (((short int)(buf[0]) << 8) & 0xff00) |
	((short int)(buf[1]) & 0xff);
    return 1;
}

//Read action info from device
int LIGDevice::read_action(short int *ibuf) throw(DeviceException)
{
//    TIMEOBJ("lig::read_channels");
    int siz;
    char ch;
    char buf[32];
    ch=0x00; //first channel
    siz=write(fd, &ch, 1);
    DBG(7, "LIG::read_channels 0: wrote [0x" << DHEX(int(ch)) << "] = " << siz << " bytes");
    read_bytes(fd, buf, 2); //read back data for channel 1

    *ibuf = (((short int)(buf[0]) << 8) & 0xff00) |
	((short int)(buf[1]) & 0xff);

    return 1;
}

// ------------------------------[EOF]-------------------------------------
