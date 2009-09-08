#ifndef DEV_TASK_HXX
#define DEV_TASK_HXX

#include "ace/OS.h"
#include "ace/Task.h"
#include "dev_watchdog.hxx"
#include "boundedqueue.hxx"
#include "boost/shared_ptr.hpp"
#include "event_notifier.hxx"

enum DevTaskCmdType { dcmd_noop, dcmd_shutdown, dcmd_reinit};

struct DevTaskCmd
{
    DevTaskCmdType      type;
    DevTaskCmd(DevTaskCmdType _t=dcmd_noop) { type=_t;};
};

class DevTask: public ACE_Task<ACE_MT_SYNCH>
{
    BluesDevicePtr       device;
    MTBoundedQueue<DevTaskCmd, ACE_Thread_Mutex>     cmd_queue;
    ACE_thread_t         my_thread;

    EventNotifier*       notifier;

    void                 process_command(DevTaskCmd&);
public:
    DevTask(const BluesDevicePtr& device_);
    ~DevTask();

    int                 svc();

    void                shutdown();
    void                set_notifier(EventNotifier* notifier_) { notifier=notifier_;};
    BluesDevicePtr      get_device() { return device;};
};

typedef boost::shared_ptr<DevTask>        DevTaskPtr;

#endif
// ------------------------------[EOF]-------------------------------------
