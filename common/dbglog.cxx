#include <iomanip>
#include <ace/Thread.h>
#include <dbglog.h>
#include <fstream>

MCOMMON_DLL_EXP_IMP DebugLog DL;

std::ostream& DebugLog::timestamp (std::ostream& s)
{
    ACE_Date_Time	dt;
    char                time_buf[32];
// "%04ld-%02ld-%02ldT%02ld:%02ld:%02ld.%03ld>"
    sprintf( time_buf, "%02ld:%02ld:%02ld.%03ld ",
	     /*dt.year(),
	     dt.month(),
	     dt.day(),*/
	     dt.hour(),
	     dt.minute(),
	     dt.second(),
	     dt.microsec() / 1000 );

    return s << time_buf << std::setw(8) << ACE_Thread::self() << ':';
}

void DebugLog::flush_out()
{
    dout->flush();
    n_flushes ++;
    if(max_flushes && n_flushes >= max_flushes)
    {
	close_out();
	open_out();
	n_flushes = 0;
    }
}


void DebugLog::close_out()
{
    delete dout;
    
    std::string fname2 = fname + "ged";
    ACE_OS::rename(fname.c_str(), fname2.c_str());
}

void DebugLog::open_out()
{
    fname = file_name_prefix;
    struct tm      tm_data;
    time_t         tim;

    ACE_OS::time(&tim);
    ACE_OS::localtime_r(&tim, &tm_data);

    tm_data.tm_mon+=1;
    
    //file name: 2004-09-21_10_44-root-ColE.big_roaster.roast
    char buf[1024];
    sprintf(buf, "-%d-%02d-%02d_%02d_%02d_%02d.%d.log",
	    (tm_data.tm_year+1900), tm_data.tm_mon, tm_data.tm_mday,
	    tm_data.tm_hour, tm_data.tm_min, tm_data.tm_sec, int(startup));

    fname += buf;

    dout = new std::ofstream(fname.c_str());
}

void DebugLog::set_log_output(const std::string& pref, int imax_flushes)
{
    file_name_prefix = pref;
    max_flushes = imax_flushes;
    open_out();
}
