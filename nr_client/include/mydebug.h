#ifndef XNC_DEBUG_H
#define XNC_DEBUG_H

#include <config.h>
#ifdef HAVE_SYS_SIGEVENT_H
#include <sys/sigevent.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include "dbglog.h"

#ifdef __cplusplus
extern "C" int debprintf(const char *format,...);
#else
extern int debprintf(const char *format,...);
#endif

#ifdef DEBUG_XNC
#ifdef __FILE__
#ifdef __LINE__
#define __DEBUG()                fprintf(stderr, "%-17s:%4d ", __FILE__, __LINE__)
#endif
#endif

#ifndef __DEBUG
#define __DEBUG()                ((void)0)
#endif

#  define dbgprintf(x)           DBG(5,x)
#else
#  define dbgprintf(x)           ((void)0)
#endif

#endif
