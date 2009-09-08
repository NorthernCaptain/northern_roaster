// ------------------------------------------------------------------------
//                 EMU Device WatchDog class methods
// ------------------------------------------------------------------------
#include "emu_watchdog.hxx"
#include "configman.hxx"
#include "timing.hxx"
#include "debug.hxx"


EMUDevice::EMUDevice(const std::string& _name): BluesDevice(_name)
{
    DBG(4, "EMU::contructor for device " << dev_name);
    emu_no_temp           = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "emu_no_temp_value", -21846));
    emu_load_portion_min  = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "emu_load_min", -21846));
    emu_load_portion_max  = (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "emu_load_max", -21846));
    emu_unload_portion_min= (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "emu_unload_min", a_unload_portion));
    emu_unload_portion_max= (TempType)(CFGMAN::instance()->get_int_value(dev_name.c_str(), "emu_unload_max", a_unload_portion));
    scan_timeout          = CFGMAN::instance()->get_int_value(dev_name.c_str(), "emu_scan_timeout", 1);
    action_wait_timeout   = CFGMAN::instance()->get_int_value(dev_name.c_str(), "emu_action_timeout", 10);
    action_time = 0;
}

EMUDevice::~EMUDevice()
{
    DBG(4, "EMU::destructor - deleting device " << dev_name);
    shutdown_device();
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "emu_no_temp_value", emu_no_temp);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "emu_load_min", emu_load_portion_min);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "emu_load_max", emu_load_portion_max);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "emu_unload_min", emu_unload_portion_min);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "emu_unload_max", emu_unload_portion_max);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "emu_action_timeout", action_wait_timeout);
    CFGMAN::instance()->set_int_value(dev_name.c_str(), "emu_scan_timeout", scan_timeout);
}

void EMUDevice::init_device()
{
    base_temp1 = 360;
    base_temp2 = 260;
    delta = 10;
    value=12;
}

void EMUDevice::shutdown_device()
{
    DBG(4, "EMU::shutdown: closing device [" << dev_name << "]");
}

void EMUDevice::wait_for_scan()
{
    sleep(scan_timeout);
}

int EMUDevice::read_channels()
{
    return 1; //only one row at a time
}

TempRow EMUDevice::get_row(int)
{
    TempRow row;
    
    value += double(delta)/40.0;

    base_temp1 = int(value*value);
    if(abs(base_temp1) > 4300 || base_temp1 < 100)
    {
	delta = -delta;
	value += double(delta)/40.0;
	base_temp1 = int(value*value);
    }

    base_temp2 += delta;

    ACE_OS::time(&row.row_date);

    row.row[t_input]  = base_temp1 + rand()%4 - 2;
    row.row[t_output] = base_temp2 + rand()%6 - 3;
    row.row[t_base]   = 20 + rand()%3;
    row.row[t_action] = base_temp1;
    row.row[t_flags] = base_temp1;
    convert_action_field(row.row[t_flags]);

    DBG(6, "EMU::get_row - generate [0] " << row.row[t_input]);
    DBG(6, "EMU::get_row - generate [1] " << row.row[t_output]);
    DBG(6, "EMU::get_row - generate [2] " << row.row[t_base]);
    DBG(6, "EMU::get_row - generate [3] " << row.row[t_action]);
    DBG(6, "EMU::get_row - generate [F] " << std::hex << row.row[t_flags] << "/"  << std::dec << row.row[t_flags]);
    return row;
}

//------------------------- protected methods -------------------------------

//Convert special channel (action) from device values to our internal ones
//ignore next values during some timeout
void EMUDevice::convert_action_field(TempType& action)
{
    time_t now_t;
    ACE_OS::time(&now_t);
    if(action_time + action_wait_timeout > now_t)
    {
	DBG(6, "EMU::convert_action_field - ignore actions due timeout for " << dev_name);
	action=a_skip;
	return;
    }
    if(action >= emu_load_portion_min && action <= emu_load_portion_max)
    {
	DBG(6, "EMU::convert_action_field - detected LOAD action for " << dev_name);
	action=a_load_portion;
	action_time=now_t;
	return;
    }
    if(action >= emu_unload_portion_min && action <= emu_unload_portion_max)
    {
	DBG(6, "EMU::convert_action_field - detected UNLOAD action for " << dev_name);
	action=a_unload_portion;
	action_time=now_t;
	return;
    }
    action = a_nothing;
}

// ------------------------------[EOF]-------------------------------------
