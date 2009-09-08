#ifndef COMMONFUNCS_H
#define COMMONFUNCS_H

#include "globals.h"

#include <string>

enum FMode { ReadOnly, ReadWrite };

extern bool find_full_path_for_file(const char* fname, char* ret_full_pathname, 
				    FMode fm=ReadOnly);

extern char* get_next_token(char* from, char* to);

extern void generate_uname(const std::string& from, std::string& uname);

/*** random number generator ***/

/* insert your favorite */
extern void SetRNG(long);
extern long LongRNG();
#define SRAND(X) SetRNG((long) X)
#define LRAND() LongRNG()

#define NRAND(X) ((int)(LRAND()%(X)))

void time2str(time_t t, char* buf);
void curdate2str(char* buf);
void difftime2str(time_t t, char* buf);

#endif
