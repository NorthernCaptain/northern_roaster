#ifndef NR_DATAFLOW_HXX
#define NR_DATAFLOW_HXX

#include "ace/OS.h"
#include "ace/Task.h"
#include <vector>
#include <string>
#include <zlib.h>
#include <math.h>

typedef short int       TempType;  //temperature * 10

enum TempChans 
    { 
	t_input=0, 
	t_output=1, 
	t_base=2, 
	t_action=3,
	t_flags=4
    };

enum ChanAction 
    { 
	a_nothing=-2000,           //nothing to do
	a_load_portion=-2100,      //load portion of coffee
	a_unload_portion=-2200,    //unload portion of coffee
	a_skip=-2300               //skipping contol value
    };

const int temp_max_vals=5;     //3 channels + 1 reserve in each row
const int daq_max_vals=2;      //number of daq data values - now we have 2 daq x 24 bits
const TempType no_temp=-1000;  //value that tells us - channel temperature is unknown

struct TempRow
{
    TempType         row[temp_max_vals];
    time_t           row_date;
    int              daq[daq_max_vals];
    double           operator[] (int idx) const { return double(row[idx])/10.0; };
    int              get_int(int idx) const { return int(round(double(row[idx])/10.0));};
    bool             is_set(int idx) { return row[idx]!=no_temp;};
};


const int                    L_ROASTER=80;
const int                    L_RNAME  =32;
//Just header for one roast session
class RoastSession
{
protected:
public:
    int                       total_session_id;
    int                       user_session_id;      //for future use
    char                      roaster[L_ROASTER+1]; //roaster full name
    char                      rname[L_RNAME+1];     //roaster login name
    int                       ruid;                 //roaster id
    time_t                    session_start;        //start time of session
    time_t                    session_end;          //end time of session
    int                       max_dead_rows;        //how many unused rows to store in memory
    TempType                  low_input_threshold;  //low limit that indicates start of roast session (in C*10)
    int                       save_interval;        //number of seconds between saves in active mode
    time_t                    save_point;           //next time to save
    bool                      active_session;       //is session active?
    TempType                  must_set_attr;

public:
    RoastSession(int ruid_=0, const char* roaster_="unregistered", const char* rname_="root");
    virtual ~RoastSession();
    virtual void              set_roaster(int ruid_, const char* rname_, const char* roaster_);
    virtual void              new_session();
};

const int                     L_SORT_NAME=80;
const int                     L_SORT_SHORT=32;
const int                     L_DEV_NAME=32;
const int                     L_ROAST_LVL=32;
const int                     L_LVL_SHORT=32;
const int                     L_WEI_DESCR=32;

class RoastHeader: public RoastSession
{
protected:
public:
    int                       roast_id;             //for future use (default=0)
    int                       local_roast_id;       //for future use (default=0)
    char                      coffee_sort[L_SORT_NAME+1];
    char                      coffee_sort_short[L_SORT_SHORT+1];
    char                      input_coffee_sort[L_SORT_NAME+1];
    char                      input_coffee_sort_short[L_SORT_SHORT+1];
    char                      roast_lvl[L_ROAST_LVL+1]; //level of roasting (full)
    char                      roast_lvl_short[L_LVL_SHORT+1];
    char                      dev_name[L_DEV_NAME+1];
    char                      wei_descr[L_WEI_DESCR+1];
    int                       roast_weight;         //weight of coffee kg*1000 -> gramms
    char                      output_wei_descr[L_WEI_DESCR+1];
    char                      roast_state[L_ROAST_LVL+1]; //status of roasting
    char                      roast_state_short[L_LVL_SHORT+1];
    char                      roast_in_state[L_ROAST_LVL+1]; //status of roasting
    char                      roast_in_state_short[L_LVL_SHORT+1];
    int                       roast_output_weight;  //output weight of coffee kg*1000 -> gramms
    time_t                    roast_start;          //start time of current roast
    time_t                    roast_end;            //end time of current roast
    int                       total_rows;           //total number of measures in this roast
    int                       time_step;            //time step of measurements in seconds
    int                       rcn_id;               //rcn id for input sku

    void                      save_header(gzFile fd);
    void                      init_fields();
public:
    RoastHeader(int ruid_=0, const char* roaster_="unregistered");
    void                      set_dev_name(const std::string& dev_name_);
    void                      set_roast_lvl(const char* lvl, const char* short_lvl);
    void                      set_roast_state(const char* state, const char* short_state);
    void                      set_roast_in_state(const char* state, const char* short_state);
    void                      set_coffee_sort(const char* sort_full, const char* sort_short);
    void                      set_input_coffee_sort(const char* sort_full, const char* sort_short);
    void                      set_roast_weight(const char* lvl, int value);
    void                      set_roast_output_weight(const char* lvl, int value);
    void                      set_rcn_id(int ircn_id);
};

//Vector that stores temperatures (rows)
typedef std::vector<TempRow>           RoastVec;


#define ROAST_LOCK(data)                ACE_Guard<ACE_Thread_Mutex>  guard(data->get_lck())

//Main class stores all information about one roast
class RoastStorage: public RoastHeader
{
protected:
    ACE_Thread_Mutex          mtx;
    RoastVec                  rdata;

    int                       unsaved_row;
    std::string               file_name;    //full path name for current roast
    gzFile                    fd;           //file descriptor for current roast
    bool                      autosave;

    virtual void              save_rows(gzFile fd);
    virtual void              close_roast();
    virtual void              generate_file_name();

public:
    RoastStorage(int ruid_=0, const char* roaster_="unregistered");
    ~RoastStorage();

    virtual void              push(TempRow& row, time_t tim=0); 

    int                       size() { return rdata.size(); };
    TempRow&                  operator[] (int idx) { return rdata[idx];};

    ACE_Thread_Mutex&         get_lck() { return mtx;};

    int                       get_roaster_id() { return ruid;};
    virtual void              new_session();
    virtual void              save_new_roast();
    virtual void              start_session(time_t tim=0);
    virtual void              end_session(time_t tim=0);
    virtual void              start_loading(time_t tim=0);
    virtual void              start_unloading(time_t tim=0);
    virtual void              finish_session();
    void                      set_autosave(bool val) { autosave=val;};
    void                      clear_all();
    void                      clear_till_row(int offset);
};

class ServerStorage: public RoastStorage
{
public:
    ServerStorage(int ruid_=0, const char* roaster_="unregistered");
    ~ServerStorage();
  
    virtual void              save_new_roast();
};

#endif
// ------------------------------[EOF]-------------------------------------
