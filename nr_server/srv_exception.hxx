#ifndef SRV_EXCEPTION_HXX
#define SRV_EXCEPTION_HXX
#include "nr_exception.hxx"

struct DeviceException: public NRException
{
    std::string                device;
    DeviceException(int _err, const std::string& device_): NRException(_err) { device=device_;};
    std::string                get_info() { return device + ": " + error_descr;};
};

struct SrvException: public NRException
{
    std::string                who;
    std::string                hand_descr;
    SrvException(int _err, 
		 const std::string& who_,
		 const std::string& descr_): NRException(_err) { who=who_; hand_descr=descr_;};
    std::string                get_info() { return who + ":[" + hand_descr + "] -> " + error_descr;};
};

struct NoSuchData: public SrvException
{
    NoSuchData(const std::string& who_) : SrvException( ENODEV, who_, "No such data requested") {};
};

#endif
// ------------------------------[EOF]-------------------------------------
