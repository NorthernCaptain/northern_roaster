#ifndef NR_EXCEPTION_HXX
#define NR_EXCEPTION_HXX

#include "ace/OS.h"
#include <string>

struct NRException
{
    int           error_code;
    std::string   error_descr;

    NRException(const std::string& descr, int error_code_)
        : error_code(error_code_), error_descr(descr) {};
    NRException(int error_code_) : error_code(error_code_)
    { error_descr=ACE_OS::strerror(error_code);};
    NRException() { error_code=0; error_descr="NR: Unknown exception";};

    virtual std::string  get_info() { return error_descr;};
    virtual ~NRException() {};
};

#endif
