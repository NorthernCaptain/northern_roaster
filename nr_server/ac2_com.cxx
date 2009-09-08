#include <termios.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "debug.hxx"

#include "boost/format.hpp"

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
    while(bytes)
    {
	siz=read(fd, buf, bytes);
	if(siz==-1)
	    return -1;
	DBG(5, "COMPORT::read " << siz <<" bytes -----------------------------");
	for(int i=0;i<siz;i++)
	    DBG(5, boost::format("COMPORT::read [%x]: 0x%x -> %d") % i % ((int)(buf[i]) & 0xff) % ((int)(buf[i]) & 0xff));
	received += siz;
	buf+=siz;
	bytes-=siz;
    }

    buf=orig_buf;

    return received;
}


int read_channels(int fd, unsigned short int *ibuf)
{
    int siz;
    char ch;
    char buf[128];
    ch=0x71;
    siz=write(fd, &ch, 1);
    DBG(5, boost::format("COMPORT::read_channels: wrote [0x%x] = %d bytes") % ch % siz);
    read_bytes(fd, buf, 2);
    
    ch=0x03;
    siz=write(fd, &ch, 1);
    DBG(5, boost::format("COMPORT::read_channels: wrote [0x%x] = %d bytes") % ch % siz);
    ch=0xa0;
    siz=write(fd, &ch, 1);
    DBG(5, boost::format("COMPORT::read_channels: wrote [0x%x] = %d bytes") % ch % siz);
    read_bytes(fd, buf, 2);
    read_bytes(fd, (char*)ibuf, 17);
    return 1;
}

int main(int argc, char** argv)
{
    DL.debug_level(5);

    int fd;
    if(argc<2)
    {
	fd=open_com_port("/dev/ttyS0");
    } else
	fd=open_com_port(argv[1]);

    if(fd==-1)
    {
	close(fd);
	return 0;
    }

    int count=1;
    unsigned short int chans[32]; 

    while(1)
    {
	read_channels(fd, chans);
	for(int i=0;i<8;i++)
	    DBG(5, boost::format("CHAN[%d]: hex=%04x -> %d") %
		    i % chans[i] % chans[i]);
	DBG(5, boost::format("----------------------%d----------------------") % count++);
	sleep(2);
    }

    close(fd);
    return 1;
}
