#include <stdio.h>
#include <X11/Xlib.h>
#include <unistd.h>

int main(int arc, char** argv)
{
    Display *disp=XOpenDisplay(0);
    if(disp)
    {
	XCloseDisplay(disp);
	if(arc<=1)
	{
	    char *ar[]={"/bin/bash", 0};
	    execv(ar[0], ar);
	}
	else
	    execv(argv[1], argv+1);
    } else
	printf("This login is currently unavailable.\n");
    return 0;
}
