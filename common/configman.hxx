#ifndef CONFIGMAN_HXX
#define CONFIGMAN_HXX

#include "ace/OS.h"
#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Singleton.h"

const int MAX_CONFIG_NAME=1024;

class ConfigMan
{
    ACE_Configuration_Heap    *conf_heap;
    ACE_Registry_ImpExp       *impexp;

    ACE_TCHAR                  config_name[MAX_CONFIG_NAME];
    ACE_Thread_Mutex           mtx;

    void                       save_nolock();

public:
    ConfigMan(const ACE_TCHAR* fname="nc_config.cfg");
    ~ConfigMan();

    void      load(const ACE_TCHAR* fname="nc_config.cfg");
    void      save();
    void      set_uint_value(const ACE_TCHAR* section, 
			     const ACE_TCHAR* var,
			     u_int value);
    u_int     get_uint_value(const ACE_TCHAR* section_name,
			     const ACE_TCHAR* var,
			     u_int def_val=0);

    void      set_int_value(const ACE_TCHAR* section, 
			    const ACE_TCHAR* var,
			    int value);
    int       get_int_value(const ACE_TCHAR* section_name,
			    const ACE_TCHAR* var,
			    int def_val=0);
    int       get_nextval(const ACE_TCHAR* section_name,
			  const ACE_TCHAR* var,
			  int def_val=1);

    void      set_cstring_value(const ACE_TCHAR* section, 
				const ACE_TCHAR* var,
				const ACE_TCHAR* str);
    void      get_cstring_value(const ACE_TCHAR* section, 
				const ACE_TCHAR* var,
				ACE_TCHAR* to_str);
};

typedef ACE_Singleton<ConfigMan, ACE_Thread_Mutex>     CFGMAN;

#endif
