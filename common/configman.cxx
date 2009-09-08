#include "debug.hxx"
#include "configman.hxx"

#include <string>

#include "boost/lexical_cast.hpp"

ConfigMan::ConfigMan(const ACE_TCHAR* fname)
{
    conf_heap=0;
    impexp=0;
    load(fname);
}

ConfigMan::~ConfigMan()
{
    save();
    delete impexp;
    delete conf_heap;
}

void ConfigMan::load(const ACE_TCHAR* fname)
{
    ACE_Guard<ACE_Thread_Mutex>   guard(mtx);
    if(impexp)
	delete impexp;
    if(conf_heap)
	delete conf_heap;

    conf_heap=new ACE_Configuration_Heap();
    conf_heap->open();

    impexp=new ACE_Registry_ImpExp(*conf_heap);

    ACE_OS::strcpy(config_name, fname);
    if(impexp->import_config(fname)==-1)
	ACE_DEBUG((LERR "can't open config [%s]: %m\n", fname));
}

void ConfigMan::save()
{
    ACE_Guard<ACE_Thread_Mutex>   guard(mtx);
    save_nolock();
}

void ConfigMan::save_nolock()
{
    impexp->export_config(config_name);
}

void ConfigMan::set_uint_value(const ACE_TCHAR* section_name,
			       const ACE_TCHAR* var,
			       u_int value)
{
    ACE_Configuration_Section_Key section;
    if(conf_heap->open_section(conf_heap->root_section(), section_name, 0, section)==-1)
	conf_heap->open_section(conf_heap->root_section(), section_name, 1, section);
    conf_heap->set_string_value(section, var, boost::lexical_cast<std::string>(value).c_str());
}

u_int ConfigMan::get_uint_value(const ACE_TCHAR* section_name,
				const ACE_TCHAR* var,
				u_int def_val)
{
    ACE_Configuration_Section_Key section;
    ACE_Guard<ACE_Thread_Mutex>   guard(mtx);
    if(conf_heap->open_section(conf_heap->root_section(), section_name, 0, section)==-1)
	return def_val;
    ACE_TString str;
    if(conf_heap->get_string_value(section, var, str)==-1)
	return def_val;
    try
    {
	 return boost::lexical_cast<u_int>(str.c_str());
    } 
    catch(...)
    {
    }

    return def_val;
}


void ConfigMan::set_int_value(const ACE_TCHAR* section_name,
			       const ACE_TCHAR* var,
			       int value)
{
    ACE_Configuration_Section_Key section;
    ACE_Guard<ACE_Thread_Mutex>   guard(mtx);
    if(conf_heap->open_section(conf_heap->root_section(), section_name, 0, section)==-1)
	conf_heap->open_section(conf_heap->root_section(), section_name, 1, section);
    conf_heap->set_string_value(section, var, boost::lexical_cast<std::string>(value).c_str());
}

int ConfigMan::get_int_value(const ACE_TCHAR* section_name,
			     const ACE_TCHAR* var,
			     int def_val)
{
    ACE_Configuration_Section_Key section;
    ACE_Guard<ACE_Thread_Mutex>   guard(mtx);
    if(conf_heap->open_section(conf_heap->root_section(), section_name, 0, section)==-1)
	return def_val;
    ACE_TString str;
    if(conf_heap->get_string_value(section, var, str)==-1)
	return def_val;
    try
    {
	 return boost::lexical_cast<int>(str.c_str());
    } 
    catch(...)
    {
    }

    return def_val;
}

int ConfigMan::get_nextval(const ACE_TCHAR* section_name,
			   const ACE_TCHAR* var,
			   int def_val)
{
    ACE_Configuration_Section_Key section;
    ACE_Guard<ACE_Thread_Mutex>   guard(mtx);
    if(conf_heap->open_section(conf_heap->root_section(), section_name, 0, section)==-1)
    {
	conf_heap->open_section(conf_heap->root_section(), section_name, 1, section);
	conf_heap->set_string_value(section, var, boost::lexical_cast<std::string>(def_val+1).c_str());
	save_nolock();
	return def_val;
    }

    ACE_TString str;
    if(conf_heap->get_string_value(section, var, str)==-1)
    {
	conf_heap->set_string_value(section, var, boost::lexical_cast<std::string>(def_val+1).c_str());
	save_nolock();
	return def_val;
    }
    try
    {
	 int val=boost::lexical_cast<int>(str.c_str());
	 conf_heap->set_string_value(section, var, boost::lexical_cast<std::string>(val+1).c_str());
	 save_nolock();
	 return val;
    } 
    catch(...)
    {
    }

    conf_heap->set_string_value(section, var, boost::lexical_cast<std::string>(def_val+1).c_str());
    save_nolock();
    return def_val;
}


void ConfigMan::set_cstring_value(const ACE_TCHAR* section_name,
				  const ACE_TCHAR* var,
				  const ACE_TCHAR* str)
{
    ACE_Configuration_Section_Key section;
    ACE_Guard<ACE_Thread_Mutex>   guard(mtx);
    if(conf_heap->open_section(conf_heap->root_section(), section_name, 0, section)==-1)
	conf_heap->open_section(conf_heap->root_section(), section_name, 1, section);
    conf_heap->set_string_value(section, var, str);
}

void ConfigMan::get_cstring_value(const ACE_TCHAR* section_name,
				  const ACE_TCHAR* var,
				  ACE_TCHAR* to_str)
{
    ACE_Configuration_Section_Key section;
    ACE_Guard<ACE_Thread_Mutex>   guard(mtx);
    if(conf_heap->open_section(conf_heap->root_section(), section_name, 0, section)==-1)
	return;
    ACE_TString str;
    if(conf_heap->get_string_value(section, var, str)==0)
	ACE_OS::strcpy(to_str, str.c_str());
}

