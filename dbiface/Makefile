#  nr_server - Northern Roast Server for Blues Coffee Co.
#                                
#  Copyright (C) 2004 by Leo Khramov
#  email:  leo@xnc.dubna.su
#  
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License 2 for more details.
#


# ./Make.common.in	 -*- Makefile -*-
# Makefile common definisions for XNC Project
# 

srcdir = .

DB_TYPE=ODBC
include dbtype.mk

SHELL = /bin/sh

# Installation target directories & other installation stuff
prefix = /usr
exec_prefix = ${prefix}
binprefix =
manprefix =
bindir = ${exec_prefix}/bin
mandir = ${prefix}/man/man1
manext = 1

# Tools & program stuff
CC = gcc
CXX = c++
IDL = $(ACE_ROOT)/bin/tao_idl
CPP = gcc -E
RM = /bin/rm
RMF = /bin/rm -f
CP = /bin/cp
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = /usr/bin/install -c -s -m 755
INSTALL_DATA = /usr/bin/install -c -m 644
INSTALL_SHARE_DATA = /usr/bin/install -c -m 666
INS=/usr/bin/install -c -m 755
INSM=/usr/bin/install -c  -m 644
INSD=/usr/bin/install -c -d -m 755
INSOWN=/usr/bin/install -c
INSTDIR=/usr/bin
INSTLIB=/usr/share/nr_server
INSTMAN=/usr/man
DEB=-g -DDEBUG_XNC -Wall
STRIPPING=
XINC=
XLIBS=
TAO_ROOT = $(ACE_ROOT)/TAO

CORBA_GEN = ../generated
CFLAGS = -O $(PLUGINFLAGS)
CPPFLAGS =  #-DHAVE_LIBXPM
LDFLAGS = -O
SHAREFLAGS = 
LD_SHAREFLAGS= 
PLUGINS = 
DEFS = #-DHAVE_CONFIG_H
LIBS = -lm -lz $(OTL_LIBS)
INCLUDES=$(OTL_CPPFLAGS) -I. -I./../common -I$(ACE_ROOT) -I$(BOOST_ROOT) -I$(TAO_ROOT) -I$(TAO_ROOT)/orbsvcs

# extra libraries needed by X on some systems, X library location
XLIB =   #-lX11 -L/usr/local/lib -lXpm


.SUFFIXES: .cxx .cpp .c .idl

.cxx.o:
	$(CXX) -c $(CXXINCLUDES) $(INCLUDES) $(CXXOPT) $< -o $@

.cpp.o:
	$(CXX) -c $(CXXINCLUDES) $(INCLUDES) $(CXXOPT) $< -o $@

# End of Make.common



.c.o:
	$(CC) -c $(CXXINCLUDES) $(INCLUDES) $(CXXOPT) $< -o $@

.idl.cpp:
	$(IDL) -Ge 1 -Gd  $<

           LIBOBJS = \
		otl_connect_wrapper.o \
		otl_stream_wrapper.o \
		Datetime.o \
		DBSObjects.o \
		DBQueryException.o

           OBJS = \
		otest.o \
		../common/dbglog.o \
		$(LIBOBJS)

           CXXINCLUDES=-I$(CORBA_GEN)
           CXXOPT=$(DEFS) $(DEB) $(CFLAGS)
           LOCAL_LIBRARIES=-L$(ACE_ROOT)/ace -L$(ACE_ROOT)/lib \
		-lACE \
		-licuuc \
		$(LIBS)
#		-lTAO_CosNotification \

PROGRAM = libdbiface.a

all:: $(PROGRAM)

$(PROGRAM): $(LIBOBJS)
	ar rc $(PROGRAM) $(LIBOBJS)
	ranlib $(PROGRAM)

otest: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(DEB) $(STRIPPING) $(CFLAGS) $(LDFLAGS) $(LOCAL_LIBRARIES)

clean:
	$(RM) -f *.o *.a

tags: FORCE
	etags *.[hc]xx ../common/*.[hc]xx

FORCE:
