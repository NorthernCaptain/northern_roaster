#ifndef DBGLOG_H
#define DBGLOG_H

#include <iostream>
#include <cstdio>
#include <ace/Date_Time.h>
#include <ace/SString.h>

#define DL_USE_MUTEX

#ifdef DL_USE_MUTEX
#include <ace/Synch.h>
#define MUTEX_LOCK ACE_Guard<ACE_Recursive_Thread_Mutex> sml__( DL.stream_mutex );
#else
#define MUTEX_LOCK
#endif

#define AST( expr ) if ( !(expr) ) { MUTEX_LOCK \
    *(DL.Out()) << DebugLog::timestamp << __FILE__ << ':' << __LINE__ \
    << '('<<__FUNCTION__<<") Assertion failed on " << #expr << std::endl; DL.flush_out(); }

#define DBG( lvl, expr ) if ( lvl <= DL.debug_level() ) { MUTEX_LOCK \
    *(DL.Out()) << DebugLog::timestamp \
	<< expr << std::endl;  DL.flush_out();}

#define ERR( expr ) { MUTEX_LOCK \
    *(DL.Err()) << DebugLog::timestamp << __FILE__ << ':' << __LINE__ \
	<< '(' << __FUNCTION__  << ") " \
	<< expr << std::endl; }

#define DHEX( expr ) std::hex << expr << std::dec

#include <mcommon_exp_imp.h>

class MCOMMON_DLL_EXP_IMP DebugLog
{
private:
   int			debug_level_;
   std::ostream*	dout;
   std::ostream*	derr;

   int                  n_flushes;
   int                  max_flushes;

   std::string          file_name_prefix;
   std::string          fname;

   time_t               startup;

   void                 open_out();
   void                 close_out();

public:
#ifdef DL_USE_MUTEX
   ACE_Recursive_Thread_Mutex	stream_mutex;
#endif

   DebugLog () 
       { 
	   debug_level_ = 0; 
	   dout = &std::cout; 
	   derr = &std::cerr; 
	   max_flushes = 0; 
	   n_flushes = 0;
	   ACE_OS::time(&startup);
       }

   DebugLog (std::ostream* pout, std::ostream* perr)
       { debug_level_ = 0; dout = pout; derr = perr; }

   ~DebugLog() { if(max_flushes) close_out();};

   void Out (std::ostream* pout) { dout = pout; }
   void Err (std::ostream* perr) { derr = perr; }

   inline std::ostream* Out ( void ) { return dout; }
   inline std::ostream* Err ( void ) { return derr; }

   void debug_level ( int dl ) { debug_level_ = dl;   }
   int  debug_level ( void   ) { return debug_level_; }

   void flush_out();

   void set_log_output(const std::string& pref, int imax_flushes);

   static std::ostream& timestamp (std::ostream& s);
};

extern MCOMMON_DLL_EXP_IMP DebugLog DL;

#endif
