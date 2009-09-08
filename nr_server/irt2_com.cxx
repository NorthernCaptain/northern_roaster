#include <termios.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "debug.hxx"

#include "boost/format.hpp"

#include "ace/TTY_IO.h"
#include "ace/DEV_Connector.h"

void com_attr_set(int fd, int speed=B9600)
{
    struct termios tio;           /* tty line characteristic structure */
    int i;

    if(tcgetattr (fd, &tio)==-1)
	perror("tcgetattr");

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
    tio.c_oflag &= ~(OLCUC);
    tio.c_oflag &= ~(ONLCR);
    tio.c_oflag &= ~(OCRNL);
    tio.c_oflag &= ~(ONOCR);
    tio.c_oflag &= ~(ONLRET);
    tio.c_oflag &= ~(OFILL);
    tio.c_oflag &= ~(OFDEL);
    tio.c_oflag |= NL0;
    tio.c_oflag |= CR0;
    tio.c_oflag |= TAB0;
    tio.c_oflag |= BS0;
    tio.c_oflag |= VT0;
    tio.c_oflag |= FF0;

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
	perror("tcsetattr");

    int status;

    //choosing first channel on AC2 network
    ioctl(fd, TIOCMGET, &status);

    status &= ~TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);

    usleep(10000);

    status |= TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);

    usleep(20000); 

    status &= ~TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);

    sleep(2); //need to wait while device is thinking
}

int open_com_port(char *port_name,int speed=B9600)
{
    int fp;
    DBG(5, "COMPORT::init opening comport [" << port_name << "]");
    fp=open(port_name, O_RDWR);
    if(fp==-1)
    {
      perror("Open com port");
	return -1;
    }
    
    DBG(5, "COMPORT::init: opened " << port_name <<" with fd [" << fp << "], setting attrs");
    com_attr_set(fp, speed);

    DBG(5, "COMPORT:init: " << port_name << " - OK");

    return fp;
}



int read_bytes(int fd, char* orig_buf, int bytes) 
{
    int received=0, siz;
    char* buf=orig_buf;
    int zero_read=0;
    while(true)
    {
	siz=read(fd, buf, 1);
	if(siz==-1)
	    return -1;

	if(siz==0)
	{
	    DBG(7, "IRT::read: zero read with counter " << zero_read);
	    zero_read++;
	    if(zero_read>3)
		return -1;
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
	if(*buf == '\n')
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


short int decode_temp(char* from)
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

std::string get_checksum(const std::string from, int len)
{
    unsigned char sum=0;
    for(int i = 0; i< len;i++)
	sum += from[i];
    char buf[10];
    sprintf(buf, "%02X", sum & 0xff);
    return std::string(buf);
}

//Read channels from device
int read_channels(int fd, short int *ibuf)
{
//    TIMEOBJ("irt::read_channels");
    int siz;
    char buf[128]="";

    std::string dev_address = "0001";
    std::string snd = "$";
    snd += dev_address + "RR000010";
    snd += get_checksum(snd, snd.length()) + "\r";

    DBG(7, "IRT: write: " << snd.length() << ": " << snd);

    siz=write(fd, snd.c_str(), snd.length());

    DBG(7, "IRT::read_channels: wrote " << siz << " bytes");
    siz = read_bytes(fd, buf, 0); //read back data for channel 1

    if(siz>2)
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


void set_params(ACE_TTY_IO& dev)
{
    ACE_TTY_IO::Serial_Params par;
    if(dev.control(ACE_TTY_IO::GETPARAMS, &par)==-1)
    {
	ACE_OS::perror("ERROR getting dev params: ");

    }

    DBG(5, "PORT old Values:");
    DBG(5, "  BaudRate: " << par.baudrate);
    DBG(5, "  CTS     : " << par.ctsenb);
    DBG(5, "  DSR     : " << par.dsrenb);
    DBG(5, "  DTR     : " << (par.dtrdisable ?  false : true));
    DBG(5, "  RTS     : " << par.rtsenb);
    DBG(5, "  Parity  : " << par.parityenb);
    DBG(5, "  Receiver: " << par.rcvenb);
    DBG(5, "  STOPBits: " << par.stopbits);
    DBG(5, "  XOFF    : " << par.xinenb);
    DBG(5, "  XON     : " << par.xoutenb);


    par.baudrate = 9600;
    par.ctsenb = true;
    par.rtsenb = true;
    par.dtrdisable = false;

    par.parityenb = false;
    par.stopbits = 1;

    par.xinenb =false;
    par.xoutenb=false;

    if(dev.control(ACE_TTY_IO::SETPARAMS, &par)==-1)
    {
	ACE_OS::perror("ERROR setting dev params: ");
	return;
    }

    if(dev.control(ACE_TTY_IO::GETPARAMS, &par)==-1)
    {
	ACE_OS::perror("ERROR getting new dev params: ");

    }


    DBG(5, "PORT NEW Values:");
    DBG(5, "  BaudRate: " << par.baudrate);
    DBG(5, "  CTS     : " << par.ctsenb);
    DBG(5, "  DSR     : " << par.dsrenb);
    DBG(5, "  DTR     : " << (par.dtrdisable ?  false : true));
    DBG(5, "  RTS     : " << par.rtsenb);
    DBG(5, "  Parity  : " << par.parityenb);
    DBG(5, "  Receiver: " << par.rcvenb);
    DBG(5, "  STOPBits: " << par.stopbits);
    DBG(5, "  XOFF    : " << par.xinenb);
    DBG(5, "  XON     : " << par.xoutenb);

}


int read_channels(ACE_TTY_IO& dev, short int *ibuf)
{
}

int main(int argc, char** argv)
{
    int count =0;

    DL.debug_level(15);

    ACE_TTY_IO    dev;

    ACE_DEV_Connector conn;

    short int chans[]={0,0,0,0};
    int ret;


    if(argc<2)
    {
	ret = conn.connect(dev, ACE_DEV_Addr("/dev/ttyS0"));
    } else
    {
	ret = conn.connect(dev, ACE_DEV_Addr(argv[1]));
    }

    if(ret == -1)
    {
	ACE_OS::perror("ERROR opeing device: ");
    }

    set_params(dev);

    while(1)
    {
	read_channels(dev, chans);
	for(int i=0;i<2;i++)
	    DBG(5, boost::format("CHAN[%d]: hex=%04x -> %d") %
		    i % chans[i] % chans[i]);
	DBG(5, boost::format("----------------------%d----------------------") % count++);
	sleep(2);
    }

    return 1;
}
