#include "ace/OS.h"
#include "scan_task.hxx"
#include "configman.hxx"
#include "debug.hxx"
#include "screen_base.hxx"

#include "boost/scoped_ptr.hpp"
#include "boost/lexical_cast.hpp"

ScanTask::ScanTask(const ORBInit& orbi_): orbi(orbi_), cond(mtx)
{
    ACE_TCHAR  dev_str[1024]="unknown";
    CFGMAN::instance()->get_cstring_value("devices", "dev_name_0", dev_str);
    CFGMAN::instance()->set_cstring_value("devices", "dev_name_0", dev_str);

    transport_inited=false;
    user_logged=false;
    notifier=0;

    data_end_offset=0;
    dev_name=dev_str;

    data=new RoastStorage();
    data->set_dev_name(dev_name);

    DBG(2, "ScanTask::constructor - created new RoastStorage for device " << dev_name);
    wait_timeout=CFGMAN::instance()->get_int_value("common", "scan_timeout", 1);
    CFGMAN::instance()->set_int_value("common", "scan_timeout", wait_timeout);

}

ScanTask::~ScanTask()
{
    DBG(2, "ScanTask::destructor - deleting RoastStorage for device " << dev_name);
    shutdown();
    delete data;
}

void ScanTask::corba_init()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    ClnNamedORBPtr ptr(new ClnNamedORB(orbi.argc, 
				       orbi.argv, 
				       orbi.ns_loc.c_str(), 
				       orbi.ns_name.c_str()));
    orbptr.swap(ptr);
    roast=orbptr->theNS()->resolve_name<NR_iface::RoastCom>("RoastCom.obj");
    roast_sort=orbptr->theNS()->resolve_name<NR_iface::RoastCoffeeSort>("RoastCoffeeSort.obj");
    roast_user=orbptr->theNS()->resolve_name<NR_iface::RoastUsers>("RoastUsers.obj");
    roast_lvl=orbptr->theNS()->resolve_name<NR_iface::RoastLevels>("RoastLevels.obj");
    roast_state=orbptr->theNS()->resolve_name<NR_iface::RoastStates>("RoastStates.obj");
    roast_weight=orbptr->theNS()->resolve_name<NR_iface::RoastWeights>("RoastWeights.obj");
    orbptr->init_server_control();
    transport_inited=true;
    cond.signal();
}

int ScanTask::svc()
{
    DBG(2, "ScanTask::svc entering thread loop for data from device " << dev_name);
    my_thread=ACE_OS::thr_self();

    while(1)
    {
	try
	{
	    corba_init();
	    break;
	}
	catch(...)
	{
	    DBG(0, "ScanTask::svc - cought exception during CORBA init, reinitializing...");
	    sleep(1);
	}
    }

    while(1)
    {
	try
	{
	    while(cmd_queue.size())
	    {
		ScanTaskEvent *cmd;
		cmd_queue.pop(cmd);
		if(cmd->cmd==stask_shutdown)
		{
		    DBG(2, "ScanTask::svc: got shutdown message - exiting from loop");
		    delete cmd;
		    return -1;
		}
		process_command(*cmd);
		delete cmd;
	    }
	    DBG(5, "ScanTask::ask data row from dev " << dev_name);
	    ask_for_data();
	    sleep(wait_timeout);
	}
	catch( const CORBA::Exception& e ) 
	{
	    DBG(0, "ScanTask::CORBA SYSTEM Exception: " << e._info());
	    process_command(ScanTaskEvent(stask_wait));
	    try
	    {
		corba_init();
	    }
	    catch(...)
	    {
		DBG(0, "ScanTask::svc - cought exception during CORBA init, reinitializing...");
	    }
	}
    }
    return -1;
}

void ScanTask::ask_for_data()
{
    bool reinit=false;
    NR_iface::DataBlockRequest_var req(new NR_iface::DataBlockRequest);
    req->dev_name=CORBA::string_dup(dev_name.c_str());
    req->from_offset=data_end_offset;
    req->length=0;

    NR_iface::DataBlock_var        block;

    roast->get_data_block(req, block);

    DBG(4, "ScanTask::ask_for_data - received data from server via corba, session_id "
	<< block->session_id << ", data rows - " << block->length);

    //server cleared his data and started from the beggining ?
    if(block->start_offset==0 && block->start_offset!=data_end_offset)
    {
	DBG(4, "ScanTask::ask_for_data - received cleared data block -> clear storage");
	clear_storage();
	reinit=true;
    }

    //TODO: Think about mutex locks here...
    for(int i=0;i<block->length;i++)
    {
	DBG(4, "ScanTask::ask_for_data [" << i << "] = " 
	    << block->rows[i].t_input << '/'
	    << block->rows[i].t_output << '/'
	    << block->rows[i].t_action << '/'
	    << block->rows[i].t_flags);
 
	TempRow r;
	r.row[t_input]=block->rows[i].t_input;
	r.row[t_output]=block->rows[i].t_output;
	r.row[t_base]=block->rows[i].t_base;
	r.row[t_action]=block->rows[i].t_action;
	r.row[t_flags] =block->rows[i].t_flags; 
	r.row_date=(time_t)(block->rows[i].row_date);
	r.daq[0]=block->rows[i].daq1;
	r.daq[1]=block->rows[i].daq2;
	data->push(r);
    }

    data_end_offset=block->start_offset + block->length;
    if(block->length || reinit)
	send_event(new ScreenEvent(reinit ? se_init_grid : se_update_grid));

    { //check for auto exit condition in main screen (roast finished)
	time_t tim, roast_end = data->roast_end;
	ACE_OS::time(&tim);
	if(roast_end != 0 && tim - roast_end > 15 && tim - roast_end < 20)
	{
	    ScreenEvent *ev=new ScreenEvent(se_auto_exit);
	    DBG(4, "ScanTask::ask_for_data - send_event auto exit: " << DHEX(ev));
	    send_event(ev);
	}
    }
}

void ScanTask::clear_storage()
{
    data->clear_all();
    data_end_offset=0;
}


void ScanTask::new_session_login()
{
    NR_iface::DataHeader_var  hdr(new NR_iface::DataHeader);
    {
	ROAST_LOCK(data);
	hdr->user_id=0;
	hdr->user_name=CORBA::string_dup(data->rname);
	hdr->user_full_name=CORBA::string_dup(data->roaster);
	hdr->sort_name=CORBA::string_dup(data->coffee_sort_short);
	hdr->sort_full_name=CORBA::string_dup(data->coffee_sort);
	hdr->input_sort_name=CORBA::string_dup(data->input_coffee_sort_short);
	hdr->input_sort_full_name=CORBA::string_dup(data->input_coffee_sort);
	hdr->lvl_short_name=CORBA::string_dup(data->roast_lvl_short);
	hdr->lvl_full_name=CORBA::string_dup(data->roast_lvl);
	hdr->wei_full_name=CORBA::string_dup(data->wei_descr);
	hdr->weight=data->roast_weight;
	hdr->short_in_state=CORBA::string_dup(data->roast_in_state_short);
	hdr->full_in_state=CORBA::string_dup(data->roast_in_state);
	//set the same as input weight because we don't know real one yet
	hdr->output_wei_full_name=CORBA::string_dup(data->wei_descr);
	hdr->output_weight=data->roast_weight;
    }
    DBG(4, "ScanTask::new_session_login: " << hdr->user_id
	<< ", name=" << hdr->user_name.in() << ", full=" << hdr->user_full_name.in());
    roast->roaster_login(dev_name.c_str(), hdr);
    user_logged=true;
    {
	ROAST_LOCK(data);
	data->start_session();
    }
    send_event(new ScreenEvent(se_update_info));
    return;
}

void ScanTask::session_logout()
{
    if(user_logged)
    {
	DBG(4, "ScanTask::session_logout - logout user " 
	    << ACE_OS::getuid() << " [" << data->rname
	    << "] from server");
	roast->roaster_logout(dev_name.c_str(), 0);
	user_logged=false;
	{
	    ROAST_LOCK(data);
	    data->end_session();
	}
    }
}

void ScanTask::process_command(const ScanTaskEvent& ev)
{
    switch(ev.cmd)
    {
    case stask_ask_data:
	DBG(5, "ScanTask::process_command - ask data row from dev " << dev_name);
	ask_for_data();
	break;
    case stask_wait:
	sleep(wait_timeout);
	break;
    case stask_login:
	new_session_login();
	break;
    case stask_logout:
	session_logout();
	break;
    default:
	break;
    }
}

std::string ScanTask::server_version()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }
    return orbptr->server_version();
}


void ScanTask::send(ScanTaskEvent* ev)
{
    cmd_queue.push(ev);
}

void ScanTask::shutdown()
{
    DBG(4, "ScanTask::shutdown: process shutdown");
    cmd_queue.push(new ScanTaskEvent(stask_shutdown));
    if(my_thread != ACE_OS::thr_self())
	wait();
}

void ScanTask::set_sort_name(const std::string& short_name,
			     const std::string& full_name)
{
    ROAST_LOCK(data);
    data->set_coffee_sort(full_name.c_str(), short_name.c_str());
}

void ScanTask::set_input_sort_name(const std::string& short_name,
			     const std::string& full_name)
{
    ROAST_LOCK(data);
    data->set_input_coffee_sort(full_name.c_str(), short_name.c_str());
}

void ScanTask::set_user_name(const std::string& short_name,
			     const std::string& full_name)
{
    ROAST_LOCK(data);
    data->set_roaster(0, short_name.c_str(), full_name.c_str());
}

void ScanTask::set_lvl_name(const std::string& short_name,
			     const std::string& full_name)
{
    ROAST_LOCK(data);
    data->set_roast_lvl(full_name.c_str(), short_name.c_str());
}

void ScanTask::set_state_name(const std::string& short_name,
			     const std::string& full_name)
{
    ROAST_LOCK(data);
    data->set_roast_state(full_name.c_str(), short_name.c_str());
}

void ScanTask::set_in_state_name(const std::string& short_name,
				 const std::string& full_name)
{
    ROAST_LOCK(data);
    data->set_roast_in_state(full_name.c_str(), short_name.c_str());
}

void ScanTask::set_weight_name(const std::string& short_name,
			     const std::string& full_name)
{
    ROAST_LOCK(data);
    data->set_roast_weight(full_name.c_str(), 
			   int(boost::lexical_cast<double>(short_name)*1000));
}

void ScanTask::set_output_weight_name(const std::string& short_name,
				      const std::string& full_name)
{
    int weight = int(boost::lexical_cast<double>(short_name)*1000);
    ROAST_LOCK(data);
    if(weight > data->roast_weight || weight < 1000)
	throw WrongWeightEx(weight, data->roast_weight);
    data->set_roast_output_weight(full_name.c_str(), 
				  weight);
}



bool ScanTask::send_event(NotificationEvent* ev)
{
    if(!notifier)
	return false;
    notifier->send_notification(ev);
    return true;
}


ScanTaskNotifier::~ScanTaskNotifier()
{
    DBG(4, "ScanTaskNotifier::destructor called");
}

void ScanTaskNotifier::send_notification(NotificationEvent* ev)
{
    ScanTaskEvent* dt_ev=dynamic_cast<ScanTaskEvent*>(ev);
    if(dt_ev)
	task->send(dt_ev);
}

CoSorts& ScanTask::get_coffee_sorts()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }
    NR_iface::Sorts_var        lst;
    roast_sort->get_coffee_sort_list(lst);
    sorts.clear();
    for(unsigned int i=0;i<lst->length();i++)
    {
	DBG(4, "ScanTask::get_coffee_sorts: add " 
	    << (*lst)[i].uniq_name.in() << "="
	    << (*lst)[i].full_name.in());
	sorts.add((*lst)[i].uniq_name.in(),
		  (*lst)[i].full_name.in());
    }
    DBG(4, "ScanTask::get_coffee_sorts - " << sorts.size() << " items loaded from server");	
    return sorts;
}

void ScanTask::send_coffee_sorts()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }

    SortMap&   sort_map=sorts.get_map();
    NR_iface::Sorts_var        lst_var(new NR_iface::Sorts);
    int i=0;

    lst_var->length(sorts.size());

    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4,"ScanTask::send_coffee_sort_list: item["
	    << it->first << "]=[" << it->second << "]");
	i++;
    }

    roast_sort->set_coffee_sort_list(lst_var.in());
    DBG(4, "ScanTask::send_coffee_sorts - " << sorts.size() << " items sent to server");
}

CoSorts& ScanTask::get_users()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }
    NR_iface::Users_var        lst;
    roast_user->get_users_list(lst);
    users.clear();
    for(unsigned int i=0;i<lst->length();i++)
    {
	DBG(4, "ScanTask::get_users: add " << (*lst)[i].uniq_name.in()
	    << "=[" << (*lst)[i].full_name.in() << "]");
	users.add((*lst)[i].uniq_name.in(),
		  (*lst)[i].full_name.in());
    }
    DBG(4, "ScanTask::get_users - " << users.size() << " items loaded from server");
    return users;
}

void ScanTask::send_users()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }

    SortMap&   sort_map=users.get_map();
    NR_iface::Users_var        lst_var(new NR_iface::Users);
    int i=0;

    lst_var->length(users.size());

    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4, "ScanTask::send_users_list: item[" << it->first << "]=["
	    << it->second << "]");
	i++;
    }

    roast_user->set_users_list(lst_var.in());
    DBG(4, "ScanTask::send_users - " << users.size() << " items sent to server");
}

CoSorts& ScanTask::get_input_sorts()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }
    input_sorts.clear();
    if(data->coffee_sort_short[0]==0) //nothing to choose
	return input_sorts;

    NR_iface::Sorts_var        lst;
    roast_sort->get_input_coffee_sort_list(data->coffee_sort_short, lst);

    for(unsigned int i=0;i<lst->length();i++)
    {
	DBG(4, "ScanTask::get_input_sorts: add " << (*lst)[i].uniq_name.in() << "=[" <<
	    (*lst)[i].full_name.in() << "]");
	input_sorts.add((*lst)[i].uniq_name.in(),
		  (*lst)[i].full_name.in(),(*lst)[i].qty);
    }
    DBG(4, "ScanTask::get_input_sorts - " << input_sorts.size() << " items loaded from server");
    return input_sorts;
}

CoSorts& ScanTask::get_levels()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }
    NR_iface::Levels_var        lst;
    roast_lvl->get_lvl_list(lst);
    levels.clear();
    for(unsigned int i=0;i<lst->length();i++)
    {
	DBG(4, "ScanTask::get_levels: add " << (*lst)[i].uniq_name.in() << "=["
	    << (*lst)[i].full_name.in() << "]");
	levels.add((*lst)[i].uniq_name.in(),
		  (*lst)[i].full_name.in());
    }
    DBG(4, "ScanTask::get_levels - " << levels.size() << " items loaded from server");
    return levels;
}


void ScanTask::send_levels()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }

    SortMap&   sort_map=levels.get_map();
    NR_iface::Levels_var        lst_var(new NR_iface::Levels);
    int i=0;

    lst_var->length(levels.size());

    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4, "ScanTask::send_lvl_list: item[" << it->first << "]=[" << it->second << "]");
	i++;
    }

    roast_lvl->set_lvl_list(lst_var.in());
    DBG(4, "ScanTask::send_levels - " << levels.size() << " items sent to server");
}

CoSorts& ScanTask::get_states()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }
    NR_iface::States_var        lst;
    roast_state->get_state_list(lst);
    states.clear();
    for(unsigned int i=0;i<lst->length();i++)
    {
	DBG(4, "ScanTask::get_states: add " << (*lst)[i].uniq_name.in() << "=["
	    << (*lst)[i].full_name.in() << "]");
	states.add((*lst)[i].uniq_name.in(),
		  (*lst)[i].full_name.in());
    }
    DBG(4, "ScanTask::get_states - " << states.size() << " items loaded from server");
    return states;
}



void ScanTask::send_states()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }

    SortMap&   sort_map=states.get_map();
    NR_iface::States_var        lst_var(new NR_iface::States);
    int i=0;

    lst_var->length(states.size());

    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4, "ScanTask::send_sts_list: item[" << it->first << "]=["
	    << it->second << "]");
	i++;
    }

    roast_state->set_state_list(lst_var.in());
    DBG(4, "ScanTask::send_states - " << states.size() << " items sent to server");
}

CoSorts& ScanTask::get_weights()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }
    NR_iface::Weights_var        lst;
    roast_weight->get_weight_list(lst);
    weights.clear();
    for(unsigned int i=0;i<lst->length();i++)
    {
	DBG(4, "ScanTask::get_levels: add " << (*lst)[i].uniq_name.in()
	    << "=[" << (*lst)[i].full_name.in() << "]");
	weights.add((*lst)[i].uniq_name.in(),
		    (*lst)[i].full_name.in());
    }
    DBG(4, "ScanTask::get_weights - " << weights.size() << " items loaded from server");
    return weights;
}

void ScanTask::send_weights()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }

    SortMap&   sort_map=weights.get_map();
    NR_iface::Weights_var        lst_var(new NR_iface::Weights);
    int i=0;

    lst_var->length(weights.size());

    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4, "ScanTask::send_lvl_list: item[" << it->first << "]=["
	    << it->second << "]");
	i++;
    }

    roast_weight->set_weight_list(lst_var.in());
    DBG(4, "ScanTask::send_weights - " << weights.size() << " items sent to server");
}


void ScanTask::finish_roast()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }

    roast->roaster_finish_roast(dev_name.c_str(), 
				0,
				data->output_wei_descr,
				data->roast_output_weight,
				data->roast_state,
				data->roast_state_short);

}

void ScanTask::start_loading()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }

    DBG(4, "ScanTask::start_loading - loading by user command");
    roast->roaster_start_loading(dev_name.c_str());
}

void ScanTask::start_unloading()
{
    ACE_Guard<ACE_Thread_Mutex>  guard(mtx);
    if(!transport_inited)
    {
	cond.wait(mtx);
    }

    DBG(4, "ScanTask::start_unloading - unloading by user command");
    roast->roaster_start_unloading(dev_name.c_str());
}

// ------------------------------[EOF]-------------------------------------

