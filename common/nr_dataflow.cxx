#include "nr_dataflow.hxx"
#include "configman.hxx"
#include "debug.hxx"

#include <zlib.h>
#include "boost/format.hpp"


const char* const RFILE_FORMAT="ROAST-3.1";

// -----------------------------------------------------------------------
//                  RoastSession class methods
// -----------------------------------------------------------------------

RoastSession::RoastSession(int ruid_, const char* roaster_, const char* rname_)
{
    set_roaster(ruid_, rname_, roaster_);
    user_session_id=0;
    ACE_OS::time(&session_end);
    session_start=session_end;
    must_set_attr = 0;

    total_session_id=CFGMAN::instance()->get_nextval("session", "total_id", 1);
}

void RoastSession::set_roaster(int ruid_, const char* rname_, const char* roaster_)
{
    ruid=ruid_;
    ACE_OS::memset(roaster, 0, L_ROASTER);
    ACE_OS::memset(rname, 0, L_RNAME);
    ACE_OS::strncpy(roaster, roaster_, L_ROASTER);
    roaster[L_ROASTER]=0;
    ACE_OS::strncpy(rname, rname_, L_RNAME);
    rname[L_RNAME]=0;
}


void RoastSession::new_session()
{
    total_session_id=CFGMAN::instance()->get_nextval("session", "total_id", 1);    
}

RoastSession::~RoastSession()
{
}

// -----------------------------------------------------------------------
//                  RoastHeader class methods
// -----------------------------------------------------------------------


RoastHeader::RoastHeader(int ruid_, const char* roaster_):
    RoastSession(ruid_, roaster_)
{
    total_rows=0;
    dev_name[0]=0;
    roast_id=local_roast_id=roast_weight=roast_output_weight=0;
    time_step=1;

    init_fields();
}


void RoastHeader::init_fields()
{
    roast_start=roast_end=session_start=session_end=0;
    ACE_OS::memset(coffee_sort, 0, L_SORT_NAME);
    ACE_OS::memset(coffee_sort_short, 0, L_SORT_SHORT);
    ACE_OS::memset(input_coffee_sort, 0, L_SORT_NAME);
    ACE_OS::memset(input_coffee_sort_short, 0, L_SORT_SHORT);
    ACE_OS::memset(roast_lvl, 0, L_ROAST_LVL);
    ACE_OS::memset(roast_lvl_short, 0, L_LVL_SHORT);
    ACE_OS::memset(roast_state, 0, L_ROAST_LVL);
    ACE_OS::memset(roast_state_short, 0, L_LVL_SHORT);
    ACE_OS::memset(roast_in_state, 0, L_ROAST_LVL);
    ACE_OS::memset(roast_in_state_short, 0, L_LVL_SHORT);
    ACE_OS::memset(wei_descr, 0, L_WEI_DESCR);
    ACE_OS::memset(output_wei_descr, 0, L_WEI_DESCR);
    ACE_OS::strcpy(coffee_sort, "unknown sort");
    ACE_OS::strcpy(coffee_sort_short, "Unknown");
    ACE_OS::strcpy(input_coffee_sort, "unknown sort");
    ACE_OS::strcpy(input_coffee_sort_short, "Unknown");
    ACE_OS::strcpy(roast_in_state_short, "normal");
    rcn_id = 0;
}

void RoastHeader::set_dev_name(const std::string& dev_name_)
{
    ACE_OS::strncpy(dev_name, dev_name_.c_str(), L_DEV_NAME);
    dev_name[L_DEV_NAME]=0;
}

void RoastHeader::set_roast_lvl(const char* lvl, const char* short_lvl)
{
    ACE_OS::strncpy(roast_lvl, lvl, L_ROAST_LVL);
    roast_lvl[L_ROAST_LVL]=0;
    ACE_OS::strncpy(roast_lvl_short, short_lvl, L_LVL_SHORT);
    roast_lvl_short[L_LVL_SHORT]=0;
}

void RoastHeader::set_roast_state(const char* lvl, const char* short_lvl)
{
    ACE_OS::strncpy(roast_state, lvl, L_ROAST_LVL);
    roast_state[L_ROAST_LVL]=0;
    ACE_OS::strncpy(roast_state_short, short_lvl, L_LVL_SHORT);
    roast_state_short[L_LVL_SHORT]=0;
}

void RoastHeader::set_roast_in_state(const char* lvl, const char* short_lvl)
{
    ACE_OS::strncpy(roast_in_state, lvl, L_ROAST_LVL);
    roast_in_state[L_ROAST_LVL]=0;
    ACE_OS::strncpy(roast_in_state_short, short_lvl, L_LVL_SHORT);
    roast_in_state_short[L_LVL_SHORT]=0;
}

void RoastHeader::set_coffee_sort(const char* lvl, const char* short_lvl)
{
    ACE_OS::strncpy(coffee_sort, lvl, L_SORT_NAME);
    coffee_sort[L_SORT_NAME]=0;
    ACE_OS::strncpy(coffee_sort_short, short_lvl, L_SORT_SHORT);
    coffee_sort_short[L_SORT_SHORT]=0;
}

void RoastHeader::set_input_coffee_sort(const char* lvl, const char* short_lvl)
{
    ACE_OS::strncpy(input_coffee_sort, lvl, L_SORT_NAME);
    input_coffee_sort[L_SORT_NAME]=0;
    ACE_OS::strncpy(input_coffee_sort_short, short_lvl, L_SORT_SHORT);
    input_coffee_sort_short[L_SORT_SHORT]=0;
}

void RoastHeader::set_roast_weight(const char* lvl, int value)
{
    roast_weight=value;
    ACE_OS::strncpy(wei_descr, lvl, L_WEI_DESCR);
    wei_descr[L_WEI_DESCR]=0;
}

void RoastHeader::set_roast_output_weight(const char* lvl, int value)
{
    roast_output_weight=value;
    ACE_OS::strncpy(output_wei_descr, lvl, L_WEI_DESCR);
    output_wei_descr[L_WEI_DESCR]=0;
}

void RoastHeader::set_rcn_id(int ircn_id)
{
    rcn_id = ircn_id;
}

void RoastHeader::save_header(gzFile fd)
{
    char buf[256];
    ACE_OS::memset(buf, '-', 256);
    gzwrite(fd, (void*)RFILE_FORMAT, 9);
    gzwrite(fd, &total_session_id, sizeof(total_session_id));
    gzwrite(fd, &user_session_id, sizeof(user_session_id));
    gzwrite(fd, rname, L_RNAME);
    gzwrite(fd, roaster, L_ROASTER);
    gzwrite(fd, &session_start, sizeof(session_start));
    gzwrite(fd, &session_end, sizeof(session_end));    
    gzwrite(fd, &roast_id, sizeof(roast_id));
    gzwrite(fd, &local_roast_id, sizeof(local_roast_id));
    gzwrite(fd, coffee_sort_short, L_SORT_SHORT);
    gzwrite(fd, coffee_sort, L_SORT_NAME);
    gzwrite(fd, roast_lvl, L_ROAST_LVL);
    gzwrite(fd, roast_lvl_short, L_LVL_SHORT);
    gzwrite(fd, dev_name, L_DEV_NAME);
    gzwrite(fd, wei_descr, L_WEI_DESCR);
    gzwrite(fd, &roast_weight, sizeof(roast_weight));
    gzwrite(fd, &roast_start, sizeof(roast_start));
    gzwrite(fd, &roast_end, sizeof(roast_end));
    gzwrite(fd, &total_rows, sizeof(total_rows));
    gzwrite(fd, buf, 256); //reserve for future use
}

// -----------------------------------------------------------------------
//                  RoastStorage class methods
// -----------------------------------------------------------------------

RoastStorage::RoastStorage(int ruid_, const char* roaster_):
    RoastHeader(ruid_, roaster_)
{
    autosave=false;
    active_session=false;
    unsaved_row=0;
    fd=0;
    save_point=0;
    max_dead_rows=CFGMAN::instance()->get_int_value("common", "max_dead_rows", 1000);
    CFGMAN::instance()->set_int_value("common", "max_dead_rows", max_dead_rows);
    save_interval=CFGMAN::instance()->get_int_value("common", "save_interval", 30);
    CFGMAN::instance()->set_int_value("common", "save_interval", save_interval);
    low_input_threshold=CFGMAN::instance()->get_int_value("common", "low_input_threshold", 2000);
    CFGMAN::instance()->set_int_value("common", "low_input_threshold", low_input_threshold);
}

RoastStorage::~RoastStorage()
{
    ACE_Guard<ACE_Thread_Mutex>      guard(mtx);
    close_roast();
}

void RoastStorage::close_roast()
{
}

void RoastStorage::new_session()
{
    if(active_session)
	end_session();

    RoastHeader::new_session();
}

void RoastStorage::push(TempRow& row, time_t tim)
{
    ACE_Guard<ACE_Thread_Mutex>      guard(mtx);

    if(tim==0)
	ACE_OS::time(&tim);


    if(must_set_attr != 0)
    {
	row.row[t_flags] = must_set_attr;
	must_set_attr = 0;
    }

    rdata.push_back(row);
    total_rows=rdata.size();

    DBG(5, "RS::push [" 
	<< row.row[t_input]
	<< "/" << row.row[t_output]
	<< "/" << row.row[t_action]
	<< "/" << row.row[t_flags]
	<< "], total " << rdata.size());


    if(row[t_input] > double(low_input_threshold)/10.0)
    {
	//Check condition for session activation (automatic)
	if(!active_session)
	{
	    start_session(row.row_date);
	}
	
	DBG(6, "FLAGS: " << row.row[t_flags] << " " << active_session
	    << " = " << (active_session && row.row[t_flags] == a_load_portion));

	if(active_session && row.row[t_flags] == a_load_portion && roast_start == 0)
	{
	    start_loading(row.row_date);
	}
	
	if(active_session && row.row[t_flags] == a_unload_portion && roast_end == 0)
	{
	    start_unloading(row.row_date);
	}

    }
    //TODO: Add session deactivation by device commands
    //Check condition for session deactivation 
    if(active_session && row[t_input] < double(low_input_threshold)/10.0
       && roast_start!=0)
    {
	end_session(row.row_date);
    }

    //Need to save?
    if(save_point < tim && active_session)
    {
	save_new_roast();
	save_point=tim + save_interval;
    }

    //clear dead rows
    if(!active_session && total_rows > max_dead_rows && roast_end + 3*save_interval < tim)
    {
	DBG(4, "RoastStorage::push - too many dead rows -> clear them all");
	clear_all();
    }
}


void RoastStorage::clear_all()
{
    rdata.clear();
    total_rows=unsaved_row=0;
    must_set_attr = 0;
}

void RoastStorage::clear_till_row(int offset)
{
    if(offset<=0)
    {
	clear_all();
	return;
    }

    RoastVec tmp_vec;
    for(unsigned int i=offset;i<rdata.size();i++)
	tmp_vec.push_back(rdata[i]);

    rdata.clear();
    for(unsigned int i=0;i<tmp_vec.size();i++)
	rdata.push_back(tmp_vec[i]);

    unsaved_row=0;
    total_rows=rdata.size();    
}


void RoastStorage::save_rows(gzFile fd)
{
    int rows=total_rows - unsaved_row;
    DBG(5, "RoastStorage::save_rows - saving " << rows << " rows");
    for(int i=0; i< rows; i++)
    {
	gzwrite(fd, &(rdata[i+unsaved_row].row[0]), temp_max_vals*sizeof(TempType));
    }

}

void RoastStorage::save_new_roast()
{
}

void RoastStorage::start_session(time_t row_date)
{
    DBG(4, "RoastStorage::start_session - activate roast session");
    time_t tim;
    if(row_date == 0)
	ACE_OS::time(&tim);
    else
	tim = row_date;

    active_session=true;
    session_start=tim;

    roast_start = 0;
    roast_end = 0;
    session_end = 0;

    unsaved_row=total_rows - 120;
    if(unsaved_row > 0)
	clear_till_row(unsaved_row);
    if(unsaved_row < 0)
	unsaved_row = 0;
}

void RoastStorage::end_session(time_t row_date)
{
    DBG(4, "RoastStorage::end_session - Deactivate roast session");
    time_t tim;
    if(row_date == 0)
	ACE_OS::time(&tim);
    else
	tim = row_date;

    session_end=tim;
    save_new_roast();
    active_session=false;
}

void RoastStorage::finish_session()
{
    DBG(4, "RoastStorage::finish_session - finished with params: session_id = " 
	<< total_session_id << ", weight=" << roast_output_weight << ", state=" 
	<< roast_state_short);
}

void RoastStorage::start_loading(time_t row_date)
{
    DBG(4, "RoastStorage::start_loading - start loading coffee into roaster, session_id = " << total_session_id);
    time_t tim;
    if(row_date == 0)
	ACE_OS::time(&tim);
    else
	tim = row_date;

    active_session=true;
    roast_start=tim;
}

void RoastStorage::start_unloading(time_t row_date)
{
    DBG(4, "RoastStorage::start_unloading - start unloading coffee from roaster, session_id = " << total_session_id);
    time_t tim;
    if(row_date == 0)
	ACE_OS::time(&tim);
    else
	tim = row_date;

    roast_end=tim;
}

void RoastStorage::generate_file_name()
{
    struct tm      tm_data;

    ACE_OS::localtime_r(&roast_start, &tm_data);

    tm_data.tm_mon+=1;
    
    file_name=(boost::format("%d-%02d") % (tm_data.tm_year+1900)  % tm_data.tm_mon).str();
    
    DBG(4, "RoastStorage::generate_file_name: file_dir=" << file_name);
    ACE_OS::mkdir(file_name.c_str(), 0755);

    //file name: 2004-09-21_10_44-root-ColE.big_roaster.roast
    char buf[1024];
    sprintf(buf, "/%d-%02d-%02d_%02d_%02d-%s-%s.%s.roast",
	    (tm_data.tm_year+1900), tm_data.tm_mon, tm_data.tm_mday,
	    tm_data.tm_hour, tm_data.tm_min, rname, coffee_sort_short,
	    dev_name);
    file_name += buf;
    /*
    file_name += (boost::format("/%d-%02d-%02d_%02d_%02d-%s-%s.%s.roast")
	% (tm_data.tm_year+1900) % tm_data.tm_mon % tm_data.tm_mday 
	% tm_data.tm_hour % tm_data.tm_min % rname % coffee_sort_short
	% dev_name).str();
    */
    DBG(4, "RoastStorage::generate_file_name: file=" << file_name);
}

// -------------------------- ServerStorage -------------------------------

ServerStorage::ServerStorage(int _ruid, const char* roaster_):
    RoastStorage(_ruid, roaster_)
{
    autosave=true;
}

ServerStorage::~ServerStorage()
{
}

void ServerStorage::save_new_roast()
{
    if(!autosave)
    {
	DBG(4, "ServerStorage::save_new_roast - NO saving - autosave mode is OFF, roaster: " << rname);
	return;
    }

    DBG(4, "ServerStorage::save_new_roast - saving active session for roaster:" << rname);
    close_roast();
    generate_file_name();
    fd=gzopen(file_name.c_str(), "wb");

    if(fd==0)
    {
	DBG(0, "ServerStorage:save_new_roast: ERROR:can't write roast to: " << file_name << ", err="
	    << ACE_OS::strerror(errno));
	return;   //should throw exception here!
    }

    save_header(fd);
    save_rows(fd);
    gzclose(fd);
}

// ------------------------------[EOF]-------------------------------------

