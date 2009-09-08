#include "storagedb.hxx"
#include "DBSObjects.h"
#include "configman.hxx"
#include "debug.hxx"
#include "Datetime.h"

// -------------------------- ServerStorageDB -------------------------------

ServerStorageDB::ServerStorageDB(int _ruid, const char* roaster_):
    RoastStorage(_ruid, roaster_)
{
    autosave=true;
    roast_wh_id=CFGMAN::instance()->get_int_value("common", "roast_warehouse_id", 1);
    CFGMAN::instance()->set_int_value("common", "roast_warehouse_id", roast_wh_id);
}

ServerStorageDB::~ServerStorageDB()
{
}

void ServerStorageDB::new_session()
{
    RoastStorage::new_session();
}


void ServerStorageDB::start_session(time_t row_date)
{
    DBG(4, "ServerStorage::start_session - activate roast session");

    if(row_date == 0)
	ACE_OS::time(&row_date);

    RoastStorage::start_session(row_date);

    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();
    }
    catch(...)
    {
	return;
    }

    for(int i=0;i<2;i++)
    {
	try
	{
	    otl_stream_wrapper_ptr str(con->new_stream(50, 
						       "exec p_roast_new_session :id<int>, :worker<char[32]>, :in_sku<char[32]>, :out_sku<char[32]>, :wh_id<int>, :qty<int>, :i_in_state<char[32]>, :i_level<char[32]>, :date<timestamp>"));
	    DBG(4, "SQL: exec p_roast_new_session "

		<< total_session_id << ", "
		<< rname << ", "
		<< input_coffee_sort_short << ", "
		<< coffee_sort_short << ", "
		<< roast_wh_id << ", "
		<< roast_weight << ", "
		<< roast_in_state_short << ", "
		<< roast_lvl_short);


	    (*str) << total_session_id;
	    (*str) << rname;
	    (*str) << input_coffee_sort_short;
	    (*str) << coffee_sort_short;
	    (*str) << roast_wh_id;
	    (*str) << roast_weight;
	    (*str) << roast_in_state_short; 
	    (*str) << roast_lvl_short; 
	    (*str) << Datetime(row_date);

	    break;
	}
	catch(DBQueryException& ex)
	{
	    DBG(-1, "ServerStorageDB:new_session ERROR DBQueryException:");
	    DBG(-1, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
    }
}

void ServerStorageDB::end_session(time_t row_date)
{
    DBG(4, "ServerStorage::end_session - Deactivate roast session");

    if(row_date == 0)
	ACE_OS::time(&row_date);

    RoastStorage::end_session(row_date);

    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();    
    }
    catch(...)
    {
	return;
    }

    for(int i=0;i<2;i++)
    {
	try
	{
	    otl_stream_wrapper_ptr str(con->new_stream(50, 
						       "exec p_roast_end_session :id<int>, :date<timestamp>"));
	    DBG(4, "SQL: exec p_roast_end_session " << total_session_id );
	    (*str) << total_session_id;
	    (*str) << Datetime(row_date);
	    break;
	}
	catch(DBQueryException& ex)
	{
	    DBG(-1, "ServerStorageDB:end_session ERROR DBQueryException:");
	    DBG(-1, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
    }

    total_session_id = 0;
}

void ServerStorageDB::finish_session()
{
    time_t row_date;
    
    DBG(4, "ServerStorage::finish_session - finalize roast session");

    ACE_OS::time(&row_date);

    save_new_roast();

    RoastStorage::finish_session();

    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();    
    }
    catch(...)
    {
	return;
    }

    for(int i=0;i<2;i++)
    {
	try
	{
	    otl_stream_wrapper_ptr str(con->new_stream(50, 
						       "exec p_roast_finish_session :id<int>, :w<int>, :st<char[32]>, :date<timestamp>"));
	    DBG(4, "SQL: exec p_roast_finish_session " << total_session_id
		<< ", " << roast_output_weight << ", '" << roast_state_short << "'");
	    (*str) << total_session_id;
	    (*str) << roast_output_weight;
	    (*str) << roast_state_short;
	    (*str) << Datetime(row_date);
	    break;
	}
	catch(DBQueryException& ex)
	{
	    DBG(-1, "ServerStorageDB:finish_session ERROR DBQueryException:");
	    DBG(-1, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
    }
}

void ServerStorageDB::start_loading(time_t row_date)
{
    DBG(4, "ServerStorage::start_loading - start loading coffee into roaster, session_id = " << total_session_id);
    
    if(row_date == 0)
	ACE_OS::time(&row_date);

    RoastStorage::start_loading(row_date);

    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();    
    }
    catch(...)
    {
	return;
    }

    for(int i=0;i<2;i++)
    {
	try
	{
	    otl_stream_wrapper_ptr str(con->new_stream(50, 
						       "exec p_roast_start_loading :id<int>, :date<timestamp>"));
	    DBG(4, "SQL: exec p_roast_start_loading " << total_session_id );
	    (*str) << total_session_id;
	    (*str) << Datetime(row_date);
	    break;
	}
	catch(DBQueryException& ex)
	{
	    DBG(-1, "ServerStorageDB:start_loading ERROR DBQueryException:");
	    DBG(-1, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
    }
}

void ServerStorageDB::start_unloading(time_t row_date)
{
    DBG(4, "ServerStorage::start_unloading - start unloading coffee from roaster, session_id = " << total_session_id);

    if(row_date == 0)
	ACE_OS::time(&row_date);

    RoastStorage::start_unloading(row_date);

    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();    
    }
    catch(...)
    {
	return;
    }

    for(int i=0;i<2;i++)
    {
	try
	{
	    otl_stream_wrapper_ptr str(con->new_stream(50, 
						       "exec p_roast_start_unloading :id<int>, :date<timestamp>"));
	    DBG(4, "SQL: exec p_roast_start_unloading " << total_session_id );
	    (*str) << total_session_id;
	    (*str) << Datetime(row_date);
	    break;
	}
	catch(DBQueryException& ex)
	{
	    DBG(-1, "ServerStorageDB:start_unloading ERROR DBQueryException:");
	    DBG(-1, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
    }
}


void ServerStorageDB::save_new_roast()
{
    int count_rows=0;
    if(!autosave)
    {
	DBG(4, "ServerStorage::save_new_roast - NO saving - autosave mode is OFF, roaster: " << rname);
	return;
    }

    if(!active_session)
    {
	DBG(4, "ServerStorage::save_new_roast - NO saving - session inactive, roaster: " << rname);
	return;
    }

    if(unsaved_row + 1 >= total_rows)
    {
	DBG(4, "ServerStorage::save_new_roast - NO saving - nothing to save, roaster: " << rname);
	return;
    } 
    int rows=total_rows - unsaved_row;
    DBG(4, "ServerStorage::save_new_roast - saving " << rows << " rows into db for session " << total_session_id);
    otl_connect_ptr con;
    try
    {
	con = DBIMAN::instance()->getDBConnection();    
    }
    catch(...)
    {
	return;
    }

    for(int i=0;i<rows;i++)
    {
	try
	{
	    otl_stream_wrapper_ptr str(con->new_stream(50, 
						       "exec p_roast_add_row :id<int>, :date<timestamp>, :input<int>, :inner<int>, :outer<int>, :addon<int>, :flags<int>, :daq1<int>, :daq2<int>"));
	    (*str) << total_session_id;
	    (*str) << Datetime(rdata[i+unsaved_row].row_date);
	    (*str) << rdata[i+unsaved_row].row[t_input];
	    (*str) << rdata[i+unsaved_row].row[t_output];
	    (*str) << rdata[i+unsaved_row].row[t_base];
	    (*str) << rdata[i+unsaved_row].row[t_action];
	    (*str) << rdata[i+unsaved_row].row[t_flags];
	    (*str) << rdata[i+unsaved_row].daq[0];
	    (*str) << rdata[i+unsaved_row].daq[1];
	    count_rows++;
	}
	catch(DBQueryException& ex)
	{
	    DBG(0, "ServerStorageDB:save_session ERROR DBQueryException:");
	    DBG(0, ex.getLocalizedMessage(""));
	    DBIMAN::instance()->badConnection(con);
	    try
	    {
		con = DBIMAN::instance()->getDBConnection();
	    }
	    catch(...) {}
	}
    }
    DBG(4, "ServerStorage::save_new_roast - saved " << count_rows << " rows into db for session " << total_session_id);
    unsaved_row += rows;
}

// ------------------------------[EOF]-------------------------------------


