#ifndef SCAN_TASK_HXX
#define SCAN_TASK_HXX

#include "ace/Task.h"
#include "ace/Condition_T.h"
#include <string>
#include "nr_dataflow.hxx"
#include "event_notifier.hxx"
#include "boundedqueue.hxx"
#include "coffeesorts.hxx"

#include "ncorba_cln.hxx"
#include "nr_ifaceC.h"

#include "boost/shared_ptr.hpp"

enum ScanTaskCmd 
    { 
	stask_noop, 
	stask_shutdown, 
	stask_ask_data, 
	stask_wait,
	stask_login,
	stask_logout,
	stask_finish_roast
    };

struct ScanTaskEvent: public NotificationEvent
{
    ScanTaskCmd       cmd;
    ScanTaskEvent(ScanTaskCmd cmd_=stask_noop): cmd(cmd_) {};
};

struct WrongWeightEx
{
    int    wrong_weight;
    int    max_weight;
    WrongWeightEx(int ww, int mw) { wrong_weight = ww; max_weight = mw; };
};

class ScanTask:public ACE_Task<ACE_MT_SYNCH>
{
    ClnNamedORBPtr            orbptr;
    bool                      transport_inited;
    ORBInit                   orbi;

    MTBoundedQueue<ScanTaskEvent*, ACE_Thread_Mutex>     cmd_queue;
    ACE_thread_t              my_thread;

    std::string               dev_name;
    RoastStorage             *data;
    int                       data_end_offset;

    int                       wait_timeout;

    NR_iface::RoastCom_var    roast;
    NR_iface::RoastCoffeeSort_var roast_sort;
    NR_iface::RoastUsers_var  roast_user;
    NR_iface::RoastLevels_var roast_lvl;
    NR_iface::RoastWeights_var roast_weight;
    NR_iface::RoastStates_var roast_state;

    CoSorts                   sorts;
    CoSorts                   users;
    CoSorts                   levels;
    CoSorts                   states;
    CoSorts                   input_sorts;
    CoSorts                   weights;

    ACE_Thread_Mutex          mtx;
    ACE_Condition<ACE_Thread_Mutex>      cond;
    bool                      user_logged;
    EventNotifier            *notifier;

    void                      process_command(const ScanTaskEvent&);
    void                      ask_for_data();
    void                      new_session_login();
    void                      session_logout();

    void                      corba_init();
    bool                      send_event(NotificationEvent* ev);
public:
    ScanTask(const ORBInit& orbi_);
    ~ScanTask();

    int                       svc();

    void                      send(ScanTaskEvent* ev);

    void                      shutdown();

    CoSorts&                  get_coffee_sorts();
    void                      send_coffee_sorts();
    CoSorts&                  get_input_sorts();
    CoSorts&                  get_users();
    void                      send_users();
    CoSorts&                  get_levels();
    void                      send_levels();
    CoSorts&                  get_states();
    void                      send_states();
    CoSorts&                  get_weights();
    void                      send_weights();

    void                      set_sort_name(const std::string& short_name,
					    const std::string& full_name);
    void                      set_input_sort_name(const std::string& short_name,
					    const std::string& full_name);
    void                      set_user_name(const std::string& short_name,
					    const std::string& full_name);
    void                      set_lvl_name(const std::string& short_name,
					    const std::string& full_name);
    void                      set_state_name(const std::string& short_name,
					     const std::string& full_name);
    void                      set_in_state_name(const std::string& short_name,
					     const std::string& full_name);
    void                      set_weight_name(const std::string& short_name,
					      const std::string& full_name);
    void                      set_output_weight_name(const std::string& short_name,
						     const std::string& full_name);
    RoastStorage*             get_storage() { return data;};
    void                      clear_storage();
    void                      finish_roast();
    void                      start_loading();
    void                      start_unloading();

    void                      set_notifier(EventNotifier* ev=0) { notifier=ev;};

    std::string               server_version();

    ACE_Thread_Mutex&         get_lock() { return mtx;};
};

typedef boost::shared_ptr<ScanTask>           ScanTaskPtr;

class ScanTaskNotifier: public EventNotifier
{
    ScanTaskPtr               task;
public:
    ScanTaskNotifier(const ScanTaskPtr& task_) : task(task_) {};
    void                      send_notification(NotificationEvent* ev); 
    ~ScanTaskNotifier();
};


#endif
// ------------------------------[EOF]-------------------------------------
