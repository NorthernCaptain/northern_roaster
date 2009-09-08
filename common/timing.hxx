#ifndef __TIMING_H
#define __TIMING_H

#include <time.h>
#include <sys/time.h>

#include <string>

class Timing
{
public:
    Timing(const char* iname="UNKNOWN");
    ~Timing();
    static double    get_double_time();

    protected:
    std::string      name;
    double           start_time, end_time, run_time;
};

#define TIMEOBJ(x)      Timing time_object(x)

#endif // __TIMING_H
