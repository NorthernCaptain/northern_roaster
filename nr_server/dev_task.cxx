#include "dev_task.hxx"
#include "debug.hxx"
#include "srv_exception.hxx"
#include "data_task.hxx"

DevTask::DevTask(const BluesDevicePtr& device_):device(device_)
{
    notifier=0;
    DBG(2, "DevTask::constructor - new dev task for dev:" << device->get_name());
}

DevTask::~DevTask()
{
    DBG(2, "DevTask::destructor - deleting dev task for dev:" << device->get_name());
    device->shutdown_device();
    if(notifier)
    {
	DBG(4, 
	    "DevTask::destructor - deleting notifier " << DHEX(notifier) << " for DataTask");
	delete notifier;
    }
}

int  DevTask::svc()
{
    int  tries=0;
    int  id=0;
    DBG(2, "DevTask::svc - entering thread loop, dev:" << device->get_name());
    try
    {
	device->init_device();
    }
    catch(DeviceException& ex)
    {
	DBG(0, "DevTask::svc Device EXCEPTION: " << ex.get_info() << ", exiting thread loop");
	return -1;
    }
    while(1)
    {
	int n_rows=0;
	id++;
	while(cmd_queue.size())
	{
	    DevTaskCmd cmd;
	    cmd_queue.pop(cmd);
	    if(cmd.type==dcmd_shutdown)
	    {
		DBG(2, "DevTask::svc: got shutdown message - exiting from loop");
		return -1;
	    }
	    process_command(cmd);
	}
	try
	{
	    n_rows=device->read_channels();
	    tries=0;
	}
	catch(DeviceException& ex)
	{
	    tries++;
	    DBG(0, "DevTask::svc read Device EXCEPTION: " << ex.get_info());
	    if(tries>1)
	    {
		DBG(0, "DevTask::svc max tries reached, exiting thread loop");
		return -1;
	    }
	    device->shutdown_device();
	    try
	    {
		device->init_device();
	    }
	    catch(DeviceException& ex)
	    {
		DBG(0, "DevTask::svc init Device EXCEPTION: " << ex.get_info() << ", exiting thread loop");
		return -1;
	    }
	    continue; //don't wait, let's try again
	}

	for(int n=0;n<n_rows;n++)
	{
	    TempRow row=device->get_row(n);
	    if(notifier)
		notifier->send_notification(new NewTempEvent(row));
	}

	device->wait_for_scan();
    }
    return -1;
}

void DevTask::process_command(DevTaskCmd& cmd)
{
}

void DevTask::shutdown()
{
    DBG(4, "DevTask::shutdown: process shutdown");
    cmd_queue.push(DevTaskCmd(dcmd_shutdown));
    if(my_thread != ACE_OS::thr_self())
	wait();
}

// ------------------------------[EOF]-------------------------------------
