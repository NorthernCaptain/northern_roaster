// docm_prefix(///)
/****************************************************************************
*  Copyright (C) 2002 by Leo Khramov
*  email:   leo@xnc.dubna.su
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
 ****************************************************************************/
// $Id: commonfuncs.cxx,v 1.1 2009/09/08 19:07:40 leo Exp $

#include "commonfuncs.h"
#include <unistd.h>
#include <fcntl.h>
#include "configman.hxx"
#include "boost/lexical_cast.hpp"

//===========================================================================
/// global find_full_path_for_file(char* fname, char* ret_full_pathname, FMode mode)
///     find file in global and local dirs and return full pathname
/// tags common_funcs
bool find_full_path_for_file(const char* name, char* ret_full_pathname, FMode mode)
{
#ifndef WIN32
  int    fd;
  char   *home=getenv("HOME");
  mode_t fmode=O_RDONLY;
  char   theme_buf[128]=".";

  CFGMAN::instance()->get_cstring_value("common", "visual_theme", theme_buf);
  CFGMAN::instance()->set_cstring_value("common", "visual_theme", theme_buf);

  switch(mode)
  {
  case ReadOnly:
    fmode=O_RDONLY;
    break;
  case ReadWrite:
    fmode=O_RDWR;
    break;
  }

  do 
  {    
    sprintf(ret_full_pathname,"%s/%s/%s",GLOBAL_SEARCH,theme_buf,name); //First check in GLOBAL_SEARCH dir
    fd=open(ret_full_pathname,fmode);
    if(fd>0)
      break;

    sprintf(ret_full_pathname,"%s/%s/%s/%s",home,LOCAL_SEARCH,theme_buf,name); //Then in LOCAL one
    fd=open(ret_full_pathname,fmode);
    if(fd>0)
      break;

    sprintf(ret_full_pathname,"./%s/%s",theme_buf,name); //Then in current dir
    fd=open(ret_full_pathname,fmode);
    if(fd>0)
      break;

    sprintf(ret_full_pathname,"./data/%s/%s",theme_buf,name); //And finally in ./data
    fd=open(ret_full_pathname,fmode);
    if(fd>0)
      break;

    fprintf(stderr,"ERROR: Can't find file %s in:\n\t./\n\t./data\n\t%s/%s\n\t%s\n",
	    name,home,LOCAL_SEARCH,GLOBAL_SEARCH);
    return false;
  } while(0);

  close(fd);
  return true;
#else
  FILE *fp;
  sprintf(ret_full_pathname,"data\\%s",name); //And finally in ./data
  fp=fopen(ret_full_pathname, "r");
  if(fp)
  {
      fclose(fp);
      return true;
  }
  return false;
#endif
}


//===========================================================================
/// global get_next_token(char* from, char* to)
///     cut buffer by tokens
/// tags Score
char* get_next_token(char* from, char* to)
{
  int  delta=0;
  char *orig_to=to;
  *to=0;
  if(*from==';')
    from++;
  while(*from==' ')
    from++;
  while(*from!=0 && *from!=';' && *from!='\n' && *from!='\r' && *from!=0)
    *to++=*from++;
  while(to!=orig_to && *to==' ')
  {
    to--;
    delta=1;
  }
  to+=delta;
  *to=0;
  return from;
}

void generate_uname(const std::string& from, std::string& uname)
{
    std::size_t    len = from.size(), i;
    len = len < 6 ? len : 6;

    uname.clear();

    for(i=0;i<len;i++)
    {
	char ch = char(from[i] & 0x7f);
	if(ch <= ' ')
	    ch='_';
	uname += ch;
    }
    uname += '-';

    int seq=CFGMAN::instance()->get_nextval("session", "sequence_id", 100);
    uname += boost::lexical_cast<std::string>(seq);
}


void curdate2str(char* buf)
{
    time_t t;
    struct tm tmstr;
    time(&t);
    ACE_OS::localtime_r(&t, &tmstr);
    ACE_OS::strftime(buf, 128, "%d %b %Y %H:%M", &tmstr);
}

void time2str(time_t t, char* buf)
{
    struct tm tmstr;
    ACE_OS::localtime_r(&t, &tmstr);
    ACE_OS::strftime(buf, 128, "%H:%M:%S", &tmstr);
}


void difftime2str(time_t t, char* buf)
{
    struct tm tmstr;
    ACE_OS::gmtime_r(&t, &tmstr);
    ACE_OS::strftime(buf, 128, "%H:%M:%S", &tmstr);
}
