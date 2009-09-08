#ifndef EMU_WATCHDOG_HXX
#define EMU_WATCHDOG_HXX

#include "dev_watchdog.hxx"
#include "srv_exception.hxx"

// Class implements protocol for EMU (emulation) devices

class EMUDevice: public BluesDevice
{
protected:
    TempType                  base_temp1;
    TempType                  base_temp2;
    TempType                  delta;

    double                    value;

    int                       scan_timeout; //number of seconds between scans

    TempType                  emu_no_temp; //value that tells us - no temperature set in device
    TempType                  emu_load_portion_min; //value to detect loading of portion of coffee
    TempType                  emu_unload_portion_min; //value to detect unloading
    TempType                  emu_load_portion_max; //value to detect loading of portion of coffee
    TempType                  emu_unload_portion_max; //value to detect unloading

    time_t                    action_time; //time of last action
    int                       action_wait_timeout; //number of seconds to ignore next action

    void                      convert_action_field(TempType&);

public:
    EMUDevice(const std::string& _name);
    ~EMUDevice();

    void                      init_device();
    void                      shutdown_device();
    int                       read_channels();
    TempRow                   get_row(int idx=0);
    void                      wait_for_scan();

    bool                      is_online() { return 1;};
};

#endif
// ------------------------------[EOF]-------------------------------------
