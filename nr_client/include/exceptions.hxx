#ifndef EXCEP_HXX
#define EXCEP_HXX

#include <string>

class ExMain
{
protected:
    std::string  ex_descr;
public:
    ExMain(const std::string& info): ex_descr(info) {};
    virtual std::string get_info() { return ex_descr;};
    virtual ~ExMain() {};
};

class ExCantLoad: public ExMain
{
public:
    ExCantLoad(const char* fname):ExMain(fname) {};
    virtual std::string get_info() { return std::string("Can't load file: ") + ex_descr;};
};

class ExNoGeoInfo: public ExMain
{
public:
    ExNoGeoInfo(const char* fname):ExMain(fname) {};
    virtual std::string get_info() { return std::string("No geometry information for: ") + ex_descr;};
};


#endif
