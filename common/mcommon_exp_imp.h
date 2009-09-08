#ifndef MCOMMON_EXP_IMP_H
#define MCOMMON_EXP_IMP_H

#ifdef __MINGW32__
# ifdef BUILD_MCOMMON_DLL
#  define MCOMMON_DLL_EXP_IMP ACE_Proper_Export_Flag
# else
#  define MCOMMON_DLL_EXP_IMP ACE_Proper_Import_Flag
# endif
#else
# define MCOMMON_DLL_EXP_IMP
#endif

#endif
