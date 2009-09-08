#include "ace/Task.h"
#include "nr_iface_impl.hxx"
#include "data_manager.hxx"
#include "data_task.hxx"
#include "debug.hxx"
#include "DBSObjects.h"

using namespace POA_NR_iface;

RoastCom_Impl::RoastCom_Impl()
{
    DBG(4, "RoastCom_Impl::constructor - new servant for RoastCom object");
}

RoastCom_Impl::~RoastCom_Impl()
{
    DBG(4, "RoastCom_Impl::destructor - deleting servant for RoastCom object");
}

//-------------------------------------------------------------------------
//Login a roaster for current device session
//return -1 if no such device to login to.
//return 1  on success
CORBA::Long    RoastCom_Impl::roaster_login(const char* dev_name, 
					    const NR_iface::DataHeader& hdr)
    throw (CORBA::SystemException)
{
    TIMEOBJ("corba:roaster_login");
    DataTaskPtr           task;

    DBG(4, "RoastCom_Impl::roaster_login: dev_name=" << dev_name << " - ruid=" 
	<< hdr.user_id << ", rname=" << hdr.user_name.in())

    try
    {
	task=DATAMAN::instance()->get_task(dev_name);
    }
    catch( NoSuchData& ex)
    {
	return -1;
    }

    RoastStorage& store=task->get_roast_storage();
    ACE_Guard<ACE_Thread_Mutex> guard(store.get_lck());

    store.set_roaster(hdr.user_id, hdr.user_name.in(), hdr.user_full_name.in());
    store.set_coffee_sort(hdr.sort_full_name.in(), hdr.sort_name.in());
    store.set_input_coffee_sort(hdr.input_sort_full_name.in(), hdr.input_sort_name.in());
    store.set_roast_lvl(hdr.lvl_full_name.in(), hdr.lvl_short_name.in());
    store.set_roast_weight(hdr.wei_full_name.in(), hdr.weight);
    store.set_roast_output_weight(hdr.output_wei_full_name.in(), hdr.output_weight);
    store.set_roast_in_state(hdr.full_in_state.in(), hdr.short_in_state.in());
    store.new_session();
    store.start_session();

    DBG(4, "RoastCom_Impl::set_data_header: dev_name=" << dev_name << " -> session_id "
	<< store.total_session_id);
    return 1;
}

CORBA::Boolean  RoastCom_Impl::roaster_logout(const char* dev_name,
					      CORBA::Long user_id)
    throw (CORBA::SystemException)
{
    TIMEOBJ("corba:roaster_logout");
    DataTaskPtr           task;

    DBG(4, "RoastCom_Impl::roaster_logout: dev_name=" << dev_name << " - r_uid=" << user_id);

    try
    {
	task=DATAMAN::instance()->get_task(dev_name);
    }
    catch( NoSuchData& ex)
    {
	return false;
    }

    RoastStorage& store=task->get_roast_storage();
    ACE_Guard<ACE_Thread_Mutex> guard(store.get_lck());
    if( user_id==store.ruid )
    {
	store.end_session();
	store.set_roaster(0, "root", "root of all evil");
    } else
    {
	DBG(0, "RoastCom_Impl::roaster_logout: ERROR - wrong user_id, can't logout: client_user_id:"
	    << user_id << ", server_user_name:" << store.ruid);
	return false;
    }
    return true;
}

CORBA::Boolean  RoastCom_Impl::roaster_finish_roast(const char* dev_name,
						    CORBA::Long user_id,
						    const char* wei_descr,
						    CORBA::Long weight,
						    const char* state_descr,
						    const char* state_short_name)
    throw (CORBA::SystemException)
{
    TIMEOBJ("corba:finish_roast");
    DataTaskPtr           task;

    DBG(4, "RoastCom_Impl::roaster_finish_roast: dev_name=" << dev_name  << " - r_uid=" << user_id << ", weight=" << weight);

    try
    {
	task=DATAMAN::instance()->get_task(dev_name);
    }
    catch( NoSuchData& ex)
    {
	return false;
    }

    RoastStorage& store=task->get_roast_storage();
    ACE_Guard<ACE_Thread_Mutex> guard(store.get_lck());

    store.set_roast_output_weight(wei_descr, weight);
    store.set_roast_state(state_descr, state_short_name);

    store.finish_session();

    return true;
}

CORBA::Boolean  RoastCom_Impl::roaster_start_loading(const char* dev_name)
    throw (CORBA::SystemException)
{
    TIMEOBJ("corba:start_loading");
    DataTaskPtr           task;

    DBG(4, "RoastCom_Impl::roaster_start_loading: dev_name=" << dev_name);

    try
    {
	task=DATAMAN::instance()->get_task(dev_name);
    }
    catch( NoSuchData& ex)
    {
	return false;
    }

    RoastStorage& store=task->get_roast_storage();
    ACE_Guard<ACE_Thread_Mutex> guard(store.get_lck());

    time_t just_now;
    ACE_OS::time(&just_now);

    if(just_now - store.roast_start > 30)
    {
	store.must_set_attr = a_load_portion;
	store.start_loading(just_now);
    }

    return true;
}

CORBA::Boolean  RoastCom_Impl::roaster_start_unloading(const char* dev_name)
    throw (CORBA::SystemException)
{
    TIMEOBJ("corba:start_unloading");
    DataTaskPtr           task;

    DBG(4, "RoastCom_Impl::roaster_start_unloading: dev_name=" << dev_name);

    try
    {
	task=DATAMAN::instance()->get_task(dev_name);
    }
    catch( NoSuchData& ex)
    {
	return false;
    }

    RoastStorage& store=task->get_roast_storage();
    ACE_Guard<ACE_Thread_Mutex> guard(store.get_lck());

    time_t just_now;
    ACE_OS::time(&just_now);

    if(just_now - store.roast_end > 30)
    {
	store.must_set_attr = a_unload_portion;
	store.start_unloading(just_now);
    }

    return true;
}


//-------------------------------------------------------------------------
// Return requested data rows from device storage
CORBA::Boolean RoastCom_Impl::get_data_block(const NR_iface::DataBlockRequest& req,
					     NR_iface::DataBlock_out data)
    throw (CORBA::SystemException)
{
    TIMEOBJ("corba:get_data_block");

    DBG(4, "RoastCom_Impl::get_data_block - data request for " 
	<< req.dev_name.in() << " offset:" << req.from_offset);
    NR_iface::DataBlock_var   block(new NR_iface::DataBlock);
    DataTaskPtr           task;

    try
    {
	task=DATAMAN::instance()->get_task(req.dev_name.in());
    }
    catch( NoSuchData& ex)
    {
	block->length=0;
	block->session_id=-1;
	data=block._retn();
	return false;
    }

    RoastStorage& store=task->get_roast_storage();
    ACE_Guard<ACE_Thread_Mutex> guard(store.get_lck());

    //fill up answer header
    block->session_id=store.total_session_id;
    block->user_id=store.ruid;
    block->start_offset=req.from_offset;
    block->length=store.size() - block->start_offset;
    if(req.length > 0)
	block->length= block->length < req.length ? block->length : req.length;

    if(block->length < 0)
    {
	block->length=store.size();
	block->start_offset=0;
    }

    if(block->length > 0)
    {
	DBG(5, "RoastCom_Impl::get_data_block - transfer data from " << block->start_offset 
	    << ", len=" << block->length <<" elements");
	block->rows.length(block->length); //set data array length
	
	for(int i=0;i<block->length;i++) //pack data rows for transfer
	{
	    TempRow& r=store[i + block->start_offset];
	    block->rows[i].t_input=r.row[t_input];
	    block->rows[i].t_output=r.row[t_output];
	    block->rows[i].t_base=r.row[t_base];
	    block->rows[i].t_action=r.row[t_action];
	    block->rows[i].t_flags=r.row[t_flags];
	    block->rows[i].row_date=(long)(r.row_date);
	    block->rows[i].daq1=r.daq[0];
	    block->rows[i].daq2=r.daq[1];
	}
    }
    data=block._retn();
    return true;
}
// ------------------------ RoastCoffeeSort_Impl---------------------------
					     
RoastCoffeeSort_Impl::RoastCoffeeSort_Impl()
{
    DBG(4, "RoastCoffeeSort_Impl::constructor - loading from " << COFFEE_SORT_FNAME);
    sorts.load_file(COFFEE_SORT_FNAME);
}

RoastCoffeeSort_Impl::~RoastCoffeeSort_Impl()
{
    DBG(4, "RoastCoffeeSort_Impl::destructor");
    sorts.save_file(COFFEE_SORT_FNAME);
}

CORBA::Long RoastCoffeeSort_Impl::get_coffee_sort_list(NR_iface::Sorts_out lst)
    throw (CORBA::SystemException)
{
    unsigned int i=0;
    NR_iface::Sorts_var lst_var(new NR_iface::Sorts);

    SortMap&   sort_map=sorts.get_map();

    lst_var->length(sort_map.size());
    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4, "RoastCoffeeSort::get_coffee_sort_list: item[" << it->first << "]=[" << it->second->name << "]");

	i++;
    }
    lst=lst_var._retn();
    return sort_map.size();
}

CORBA::Long RoastCoffeeSort_Impl::get_input_coffee_sort_list(const char* dummy, NR_iface::Sorts_out lst)
    throw (CORBA::SystemException)
{
    unsigned int i=0;
    NR_iface::Sorts_var lst_var(new NR_iface::Sorts);

    SortMap&   sort_map=sorts.get_map();

    lst_var->length(sort_map.size());
    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4, "RoastCoffeeSort::get_coffee_sort_list: item[" << it->first << "]=[" << it->second->name << "]");
	i++;
    }
    lst=lst_var._retn();
    return sort_map.size();
}

CORBA::Boolean RoastCoffeeSort_Impl::set_coffee_sort_list(const NR_iface::Sorts& lst)
    throw (CORBA::SystemException)
{
    DBG(4, "RoastCoffeeSort::set_coffee_sort_list with elements: " << lst.length());
    sorts.clear();
    for(unsigned int i=0; i < lst.length(); i++)
	sorts.add(lst[i].uniq_name.in(), lst[i].full_name.in());
    
    sorts.save_file(COFFEE_SORT_FNAME);

    DBG(4, "RoastCoffeeSort::set_coffee_sort_list - set elements: " << sorts.size());

    return true;
}

// ------------------------ RoastCoffeeSort_ImplDB---------------------------
					     
RoastCoffeeSort_ImplDB::RoastCoffeeSort_ImplDB()
{
    DBG(4, "RoastCoffeeSort_ImplDB::constructor - loading from v_roast_sku");
}

RoastCoffeeSort_ImplDB::~RoastCoffeeSort_ImplDB()
{
    DBG(4, "RoastCoffeeSort_ImplDB::destructor");
}

CORBA::Long RoastCoffeeSort_ImplDB::get_coffee_sort_list(NR_iface::Sorts_out lst)
    throw (CORBA::SystemException)
{
    unsigned int i=0;
    int len = 0;
    NR_iface::Sorts_var lst_var(new NR_iface::Sorts);

    otl_connect_ptr con;

    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return 0;
    }

    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select count(*) from v_roast_sku"));
	DBG(4, "SQL: select count(*) from v_roast_sku");
	(*str) >> len;
    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastSorts_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	try
	{
	    con = DBIMAN::instance()->getDBConnection();
	}
	catch(...) {}
    } 


    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select name, full_name from v_roast_sku"));
	DBG(4, "SQL: select name, full_name from v_roast_sku");
	lst_var->length(len);
	for(i=0;i<(unsigned)len && !str->eof();i++)
	{
	    std::string sname, sname2;
	    (*str) >> sname;
	    (*str) >> sname2;
	    (*lst_var)[i].uniq_name=CORBA::string_dup(sname.c_str());
	    (*lst_var)[i].full_name=CORBA::string_dup(sname2.c_str());
	    DBG(4, "RoastSorts::get_sorts_list: item[" << sname << "]=[" << sname2 << "]");
	}
    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastSorts_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	try
	{
	    con = DBIMAN::instance()->getDBConnection();
	}
	catch(...) {}
    } 

    lst=lst_var._retn();
    return i>(unsigned)len ? len : i;

}

CORBA::Long RoastCoffeeSort_ImplDB::get_input_coffee_sort_list(const char* sort_name, NR_iface::Sorts_out lst)
    throw (CORBA::SystemException)
{
    unsigned int i=0;
    int len = 0;
    NR_iface::Sorts_var lst_var(new NR_iface::Sorts);

    otl_connect_ptr con;

    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return 0;
    }

    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select count(*) from v_roast_input_sku where out_name=:n<char[32]>"));

	DBG(4, "SQL: select count(*) from v_roast_input_sku where out_name='" << sort_name << "'");

	(*str) << sort_name;

	(*str) >> len;

    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastSorts_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	con = DBIMAN::instance()->getDBConnection();
    } 


    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select name, full_name, whQty from v_roast_input_sku where out_name=:n<char[32]>"));

	DBG(4, "SQL: select name, full_name, whQty from v_roast_input_sku where out_name='" << sort_name << "'");

	(*str) << sort_name;

	lst_var->length(len);
	for(i=0;i<(unsigned)len && !str->eof();i++)
	{
	    std::string sname, sname2;
	    long qty;
	    (*str) >> sname;
	    (*str) >> sname2;
	    (*str) >> qty;
	    (*lst_var)[i].uniq_name=CORBA::string_dup(sname.c_str());
	    (*lst_var)[i].full_name=CORBA::string_dup(sname2.c_str());
	    (*lst_var)[i].qty = qty;
	    DBG(4, "RoastSorts::get_input_sorts_list: item[" << sname << "]=[" << sname2 << "]");
	}
    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastSorts_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	try
	{
	    con = DBIMAN::instance()->getDBConnection();
	}
	catch(...) {}
    } 

    lst=lst_var._retn();
    return i>(unsigned)len ? len : i;

}

CORBA::Boolean RoastCoffeeSort_ImplDB::set_coffee_sort_list(const NR_iface::Sorts& lst)
    throw (CORBA::SystemException)
{
    int count=0;
    std::string sname="";
    DBG(4, "RoastSorts::set_sorts_list with elements: " << lst.length());
    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return false;
    }

    for(unsigned int i=0; i < lst.length(); i++)
    {
	try
	{	    
	    sname.clear();
	    {
		otl_stream_wrapper_ptr str(con->new_stream(50, "select full_name from v_roast_sku where name=:n<char[32]>"));
		DBG(4, "SQL: select full_name from v_roast_sku where name='" << lst[i].uniq_name.in() << "'");
		(*str) << lst[i].uniq_name.in();
		if(!str->eof()) 
		    (*str) >> sname;
	    }
	    if(sname.empty())
	    {
		otl_stream_wrapper_ptr str(con->new_stream(50, "exec p_roast_new_sku :n<char[32]>, :fn<char[256]>"));
		DBG(4, "SQL: exec p_roast_new_sku '"
		    << lst[i].uniq_name.in() << "', '"
		    << lst[i].full_name.in() << "'");
		(*str) << lst[i].uniq_name.in();
		(*str) << lst[i].full_name.in();
		count++;
	    }
	}

	catch(DBQueryException& ex)
	{
	    DBG(-1, "RoastSorts_ImplDB:save ERROR DBQueryException:");
	    DBG(-1, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
	
    }

    DBG(4, "RoastSorts::set_sorts_list - set elements: " << count);
    return true;
}

// ------------------------ RoastUsers_Impl---------------------------
					     
RoastUsers_Impl::RoastUsers_Impl()
{
    DBG(4, "RoastUsers_Impl::constructor - loading from " << USERS_FNAME);
    users.load_file(USERS_FNAME);
}

RoastUsers_Impl::~RoastUsers_Impl()
{
    DBG(4, "RoastUsers_Impl::destructor");
    users.save_file(USERS_FNAME);
}

CORBA::Long RoastUsers_Impl::get_users_list(NR_iface::Users_out lst)
    throw (CORBA::SystemException)
{
    unsigned int i=0;
    NR_iface::Users_var lst_var(new NR_iface::Users);
    SortMap&   sort_map=users.get_map();

    lst_var->length(sort_map.size());
    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4, "RoastUsers::get_users_list: item[" << it->first << "]=[" << it->second->name << "]");
	i++;
    }
    lst=lst_var._retn();
    return sort_map.size();
}

CORBA::Boolean RoastUsers_Impl::set_users_list(const NR_iface::Users& lst)
    throw (CORBA::SystemException)
{
    DBG(4, "RoastUsers::set_users_list with elements: " << lst.length());
    users.clear();
    for(unsigned int i=0; i < lst.length(); i++)
	users.add(lst[i].uniq_name.in(), lst[i].full_name.in());
    
    users.save_file(USERS_FNAME);

    DBG(4, "RoastUsers::set_users_list - set elements: " << users.size());

    return true;
}

// ------------------------ RoastUsers_ImplDB---------------------------
					     
RoastUsers_ImplDB::RoastUsers_ImplDB()
{
    DBG(4, "RoastUsers_ImplDB::constructor - loading from WORKER table");
}

RoastUsers_ImplDB::~RoastUsers_ImplDB()
{
    DBG(4, "RoastUsers_ImplDB::destructor");
}

CORBA::Long RoastUsers_ImplDB::get_users_list(NR_iface::Users_out lst)
    throw (CORBA::SystemException)
{
    unsigned int i=0;
    int len = 0;
    NR_iface::Users_var lst_var(new NR_iface::Users);

    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return 0;
    }

    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select count(*) from v_roast_worker"));
	DBG(4, "SQL: select count(*) from v_roast_worker");

	(*str) >> len;

    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastUsers_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}

    } 


    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select name, full_name from v_roast_worker"));

	DBG(4, "SQL: select name, full_name from v_roast_worker");

	lst_var->length(len);
	for(i=0;i<(unsigned)len && !str->eof();i++)
	{
	    std::string sname, sname2;
	    (*str) >> sname;
	    (*str) >> sname2;
	    (*lst_var)[i].uniq_name=CORBA::string_dup(sname.c_str());
	    (*lst_var)[i].full_name=CORBA::string_dup(sname2.c_str());
	    DBG(4, "RoastUsers::get_users_list: item[" << sname << "]=[" << sname2 << "]");
	}
    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastUsers_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	try
	{
	    con = DBIMAN::instance()->getDBConnection();
	}
	catch(...) {}
    } 

    lst=lst_var._retn();
    return i>(unsigned)len ? len : i;
}

CORBA::Boolean RoastUsers_ImplDB::set_users_list(const NR_iface::Users& lst)
    throw (CORBA::SystemException)
{
    int count=0;
    std::string sname="";
    DBG(4, "RoastUsers::set_users_list with elements: " << lst.length());

    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return false;
    }

    for(unsigned int i=0; i < lst.length(); i++)
    {
	try
	{	    
	    sname.clear();
	    {
		otl_stream_wrapper_ptr str(con->new_stream(50, "select full_name from v_roast_worker where name=:n<char[32]>"));
		DBG(4, "SQL: select full_name from v_roast_worker where name='" << lst[i].uniq_name.in() << "'");
		(*str) << lst[i].uniq_name.in();
		if(!str->eof()) 
		    (*str) >> sname;
	    }
	    if(sname.empty())
	    {
		otl_stream_wrapper_ptr str(con->new_stream(50, "exec p_roast_new_worker 'roaster', :n<char[32]>, :fn<char[256]>"));
		DBG(4, "SQL: exec p_roast_new_worker 'roaster', '" << lst[i].uniq_name.in()
		    << "', '" << lst[i].full_name.in() << "'" );
		(*str) << lst[i].uniq_name.in();
		(*str) << lst[i].full_name.in();
		count++;
	    }
	}

	catch(DBQueryException& ex)
	{
	    DBG(-1, "RoastUsers_ImplDB:save ERROR DBQueryException:");
	    DBG(-1, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
	
    }

    DBG(4, "RoastUsers::set_users_list - set elements: " << count);
    return true;
}

// ------------------------ RoastLevels_Impl---------------------------
					     
RoastLevels_Impl::RoastLevels_Impl()
{
    DBG(4, "RoastLevels_Impl::constructor - loading from " << LEVELS_FNAME);
    levels.load_file(LEVELS_FNAME);
}

RoastLevels_Impl::~RoastLevels_Impl()
{
    DBG(4, "RoastLevels_Impl::destructor");
    levels.save_file(LEVELS_FNAME);
}

CORBA::Long RoastLevels_Impl::get_lvl_list(NR_iface::Levels_out lst)
    throw (CORBA::SystemException)
{
    unsigned int i=0;
    NR_iface::Levels_var lst_var(new NR_iface::Levels);
    SortMap&   sort_map=levels.get_map();

    lst_var->length(sort_map.size());
    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4, "RoastLevels::get_lvl_list: item[" << it->first << "]=[" << it->second->name << "]");
	i++;
    }
    lst=lst_var._retn();
    return sort_map.size();
}

CORBA::Boolean RoastLevels_Impl::set_lvl_list(const NR_iface::Levels& lst)
    throw (CORBA::SystemException)
{
    DBG(4, "RoastLevels::set_lvl_list with elements: " << lst.length());
    levels.clear();
    for(unsigned int i=0; i < lst.length(); i++)
	levels.add(lst[i].uniq_name.in(), lst[i].full_name.in());
    
    levels.save_file(LEVELS_FNAME);

    DBG(4, "RoastLevels::set_lvl_list - set elements: " << levels.size());

    return true;
}

// ------------------------ RoastLevels_ImplDB---------------------------
					     
RoastLevels_ImplDB::RoastLevels_ImplDB()
{
    DBG(4, "RoastLevels_ImplDB::constructor - loading from roast_level table");
}

RoastLevels_ImplDB::~RoastLevels_ImplDB()
{
    DBG(4, "RoastLevels_ImplDB::destructor");
}

CORBA::Long RoastLevels_ImplDB::get_lvl_list(NR_iface::Levels_out lst)
    throw (CORBA::SystemException)
{
    unsigned int i=0;
    int len = 0;
    NR_iface::Levels_var lst_var(new NR_iface::Levels);

    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return 0;
    }

    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select count(*) from v_roast_level"));
	DBG(4, "SQL: select count(*) from v_roast_level");
	(*str) >> len;

    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastLevels_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	try
	{
	    con = DBIMAN::instance()->getDBConnection();
	}
	catch(...) {}
    } 


    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select name, full_name from v_roast_level"));
	DBG(4, "SQL: select name, full_name from v_roast_level");
	lst_var->length(len);
	for(i=0;i<(unsigned)len && !str->eof();i++)
	{
	    std::string sname, sname2;
	    (*str) >> sname;
	    (*str) >> sname2;
	    (*lst_var)[i].uniq_name=CORBA::string_dup(sname.c_str());
	    (*lst_var)[i].full_name=CORBA::string_dup(sname2.c_str());
	    DBG(4, "RoastLevels::get_levels_list: item[" << sname << "]=[" << sname2 << "]");
	}
    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastLevels_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	try
	{
	    con = DBIMAN::instance()->getDBConnection();
	}
	catch(...) {}
    } 

    lst=lst_var._retn();
    return i>(unsigned)len ? len : i;
}

CORBA::Boolean RoastLevels_ImplDB::set_lvl_list(const NR_iface::Levels& lst)
    throw (CORBA::SystemException)
{
    int count=0;
    std::string sname="";
    DBG(4, "RoastLevels::set_levels_list with elements: " << lst.length());
    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return false;
    }

    for(unsigned int i=0; i < lst.length(); i++)
    {
	try
	{	    
	    sname.clear();
	    {
		otl_stream_wrapper_ptr str(con->new_stream(50, "select full_name from v_roast_level where name=:n<char[32]>"));
		DBG(4, "SQL: select full_name from v_roast_level where name='" << lst[i].uniq_name.in() << "'");
		(*str) << lst[i].uniq_name.in();
		if(!str->eof()) 
		    (*str) >> sname;
	    }
	    if(sname.empty())
	    {
		otl_stream_wrapper_ptr str(con->new_stream(50, 
							   "exec p_roast_new_level :n<char[32]>, :fn<char[256]>"));
		DBG(4, "SQL: exec p_roast_new_level '" 
		    << lst[i].uniq_name.in()
		    << "', '" << lst[i].full_name.in() << "'" );		
		(*str) << lst[i].uniq_name.in();
		(*str) << lst[i].full_name.in();
		count++;
	    }
	}

	catch(DBQueryException& ex)
	{
	    DBG(-1, "RoastLevels_ImplDB:save ERROR DBQueryException:");
	    DBG(-1, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
	
    }

    DBG(4, "RoastLevels::set_levels_list - set elements: " << count);
    return true;
}


// ------------------------ RoastWeights_Impl---------------------------
					     
RoastWeights_Impl::RoastWeights_Impl()
{
    DBG(4, "RoastWeights_Impl::constructor - loading from " << WEIGHTS_FNAME);
    weights.load_file(WEIGHTS_FNAME);
}

RoastWeights_Impl::~RoastWeights_Impl()
{
    DBG(4, "RoastWeights_Impl::destructor");
    weights.save_file(WEIGHTS_FNAME);
}

CORBA::Long RoastWeights_Impl::get_weight_list(NR_iface::Weights_out lst)
    throw (CORBA::SystemException)
{
    unsigned int i=0;
    NR_iface::Weights_var lst_var(new NR_iface::Weights);
    SortMap&   sort_map=weights.get_map();

    lst_var->length(sort_map.size());
    for(SortMap::iterator it=sort_map.begin(); it!=sort_map.end(); it++)
    {
 	(*lst_var)[i].uniq_name=CORBA::string_dup(it->first.c_str());
 	(*lst_var)[i].full_name=CORBA::string_dup(it->second->name.c_str());
	DBG(4, "RoastWeights::get_weight_list: item[" << it->first << "]=[" << it->second->name << "]");
	i++;
    }
    lst=lst_var._retn();
    return sort_map.size();
}

CORBA::Boolean RoastWeights_Impl::set_weight_list(const NR_iface::Weights& lst)
    throw (CORBA::SystemException)
{
    DBG(4, "RoastWeights::set_weight_list with elements: " << lst.length());
    weights.clear();
    for(unsigned int i=0; i < lst.length(); i++)
	weights.add(lst[i].uniq_name.in(), lst[i].full_name.in());
    
    weights.save_file(WEIGHTS_FNAME);

    DBG(4, "RoastWeights::set_weight_list - set elements: " << weights.size());

    return true;
}


// ------------------------ RoastStates_ImplDB---------------------------
					     
RoastStates_ImplDB::RoastStates_ImplDB()
{
    DBG(4, "RoastStates_ImplDB::constructor - loading from roast_status table");
}

RoastStates_ImplDB::~RoastStates_ImplDB()
{
    DBG(4, "RoastStates_ImplDB::destructor");
}

CORBA::Long RoastStates_ImplDB::get_state_list(NR_iface::States_out lst)
    throw (CORBA::SystemException)
{
    TIMEOBJ("corba:get_state_list");
    unsigned int i=0;
    int len = 0;
    NR_iface::States_var lst_var(new NR_iface::States);

    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return 0;
    }

    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select count(*) from v_roast_status"));

	(*str) >> len;

    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastStates_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	try
	{
	    con = DBIMAN::instance()->getDBConnection();
	}
	catch(...) {}
    } 


    try
    {

	otl_stream_wrapper_ptr str(con->new_stream(50, "select name, full_name from v_roast_status"));
	DBG(4, "SQL: select name, full_name from v_roast_status");
	lst_var->length(len);
	for(i=0;i<(unsigned)len && !str->eof();i++)
	{
	    std::string sname, sname2;
	    (*str) >> sname;
	    (*str) >> sname2;
	    (*lst_var)[i].uniq_name=CORBA::string_dup(sname.c_str());
	    (*lst_var)[i].full_name=CORBA::string_dup(sname2.c_str());
	    DBG(4, "RoastStates::get_states_list: item[" << sname  << "]=[" << sname2 <<"]");
	}
    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "RoastStates_ImplDB: ERROR DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
	DBIMAN::instance()->badConnection(con);
	try
	{
	    con = DBIMAN::instance()->getDBConnection();
	}
	catch(...) {}
    } 

    lst=lst_var._retn();
    return i>(unsigned)len ? len : i;
}

CORBA::Boolean RoastStates_ImplDB::set_state_list(const NR_iface::States& lst)
    throw (CORBA::SystemException)
{
    int count=0;
    std::string sname="";
    DBG(4, "RoastStates::set_states_list with elements: " << lst.length());
    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return false;
    }

    for(unsigned int i=0; i < lst.length(); i++)
    {
	try
	{	    
	    sname.clear();
	    {
		otl_stream_wrapper_ptr str(con->new_stream(50, "select full_name from v_roast_status where name=:n<char[32]>"));
		DBG(4, "SQL: select full_name from v_roast_status where name='" << lst[i].uniq_name.in() << "'");
		(*str) << lst[i].uniq_name.in();
		if(!str->eof()) 
		    (*str) >> sname;
	    }
	    if(sname.empty())
	    {
		otl_stream_wrapper_ptr str(con->new_stream(50, 
							   "exec p_roast_new_status :n<char[32]>, :fn<char[256]>"));
		DBG(4, "SQL: exec p_roast_new_status '" << lst[i].uniq_name.in()
		    << "', '" << lst[i].full_name.in() << "'");
		(*str) << lst[i].uniq_name.in();
		(*str) << lst[i].full_name.in();
		count++;
	    }
	}

	catch(DBQueryException& ex)
	{
	    DBG(-1, "RoastStates_ImplDB:save ERROR DBQueryException:");
	    DBG(-1, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
	
    }

    DBG(4, "RoastStates::set_states_list - set elements: " << count);
    return true;
}




// ------------------------------[EOF]-------------------------------------
