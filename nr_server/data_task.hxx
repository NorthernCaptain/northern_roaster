#ifndef DATA_TASK_HXX
#define DATA_TASK_HXX

#include "ace/Task.h"
#include "dev_watchdog.hxx"
#include "nr_dataflow.hxx"
#include "event_notifier.hxx"
#include "boundedqueue.hxx"
#include "boost/shared_ptr.hpp"

enum DataTaskCmd { dtask_noop, dtask_new_row, dtask_shutdown };

struct DataTaskEvent: public NotificationEvent
{
    DataTaskCmd       cmd;
    DataTaskEvent(DataTaskCmd cmd_=dtask_noop): cmd(cmd_) {};
};

struct NewTempEvent: public DataTaskEvent
{
    TempRow         row;
    NewTempEvent(const TempRow& row_): DataTaskEvent(dtask_new_row), row(row_) {};
};

class DataTask:public ACE_Task<ACE_MT_SYNCH>
{
    MTBoundedQueue<DataTaskEvent*, ACE_Thread_Mutex>     cmd_queue;
    ACE_thread_t              my_thread;

    BluesDevicePtr            dev;
    RoastStorage             *data;

    void                      process_command(DataTaskEvent*);
public:
    DataTask(const BluesDevicePtr&   dev_);
    ~DataTask();

    int                       svc();

    void                      send(DataTaskEvent* ev);

    void                      shutdown();

    RoastStorage&             get_roast_storage() { return *data;};
};

typedef boost::shared_ptr<DataTask>           DataTaskPtr;

class DataTaskNotifier: public EventNotifier
{
    DataTaskPtr               task;
public:
    DataTaskNotifier(const DataTaskPtr& task_) : task(task_) {};
    void                      send_notification(NotificationEvent* ev); 
    ~DataTaskNotifier();
};

#endif
// ------------------------------[EOF]-------------------------------------

