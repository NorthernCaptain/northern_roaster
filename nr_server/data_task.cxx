#include "data_task.hxx"
#include "debug.hxx"
#include "storagedb.hxx"

DataTask::DataTask(const BluesDevicePtr& dev_): dev(dev_)
{
    const std::string& dev_name=dev->get_name();
    data=new ServerStorageDB();
    data->set_dev_name(dev_name);
    DBG(2, "DataTask::constructor - created new RoastStorage for device " << dev_name);
}

DataTask::~DataTask()
{
    DBG(2, "DataTask::destructor - deleting RoastStorage for device " << dev->get_name());
    delete data;
}

int DataTask::svc()
{
    DBG(2, "DataTask::svc entering thread loop for data from device " << dev->get_name());
    my_thread=ACE_OS::thr_self();

    while(1)
    {
	DataTaskEvent* ev=0;
	cmd_queue.pop(ev);
	DBG(6, "DataTask::event: " << DHEX(int(ev)));
	if(ev->cmd==dtask_shutdown)
	{
	    DBG(2, "DataTask::svc - shutdown command - exiting thread loop, dev " << dev->get_name());
	    delete ev;
	    return -1;
	}
	process_command(ev);
	delete ev;
    }
    return -1;
}

void DataTask::process_command(DataTaskEvent* ev)
{
    switch(ev->cmd)
    {
    case dtask_new_row:
	DBG(6, "DataTask::process_command - new data row from dev " << dev->get_name());
	data->push(((NewTempEvent*)(ev))->row);
	break;
    default:
	break;
    }
}

void DataTask::send(DataTaskEvent* ev)
{
    cmd_queue.push(ev);
}

void DataTask::shutdown()
{
    DBG(4, "DataTask::shutdown: process shutdown");
    cmd_queue.push(new DataTaskEvent(dtask_shutdown));
    if(my_thread != ACE_OS::thr_self())
	wait();
}


DataTaskNotifier::~DataTaskNotifier()
{
    DBG(4, "DataTaskNotifier::destructor called");
}

void DataTaskNotifier::send_notification(NotificationEvent* ev)
{
    DataTaskEvent* dt_ev=dynamic_cast<DataTaskEvent*>(ev);
    if(dt_ev)
	task->send(dt_ev);
}

// ------------------------------[EOF]-------------------------------------

