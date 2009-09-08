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

SHELL = /bin/sh

DB_TYPE=ODBC
include ../dbiface/dbtype.mk

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
LIBS = -lcomedi -lm -lz
INCLUDES=-I. -I./../common -I$(ACE_ROOT) -I$(BOOST_ROOT) -I$(TAO_ROOT) -I$(TAO_ROOT)/orbsvcs

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
	$(IDL) -Gd  $<

           COMMON_OBJS = \
		../common/dbglog.o \
		../common/configman.o \
		../common/timing.o \
		../common/nr_dataflow.o \
		../common/NameService.o \
		../common/ncorba.o \
		../common/ncorba_srv.o \
		../common/nc_server_control_impl.o \
		../common/StringUtils.o \
		../common/coffeesorts.o

           CORBA_OBJS = \
		$(CORBA_GEN)/nc_server_controlC.o \
		$(CORBA_GEN)/nc_server_controlS.o \
		$(CORBA_GEN)/nr_ifaceC.o \
		$(CORBA_GEN)/nr_ifaceS.o

           OBJS = \
		$(CORBA_OBJS) \
		main.o \
		ac2_watchdog.o \
		lig_watchdog.o \
		irt42_watchdog.o \
		irt42tcp_watchdog.o \
		irt42tcpdaq_watchdog.o \
		adv1751_watchdog.o \
		dev_watchdog.o \
		emu_watchdog.o \
		dev_manager.o \
		dev_task.o \
		data_task.o \
		data_manager.o \
		nr_iface_impl.o \
		storagedb.o \
		$(COMMON_OBJS) \
		../dbiface/libdbiface.a

           CXXINCLUDES=-I$(CORBA_GEN) -I../dbiface
           CXXOPT=$(DEFS) $(DEB) $(CFLAGS)
           LOCAL_LIBRARIES=-L$(ACE_ROOT)/ace -L$(ACE_ROOT)/lib \
		-lACE \
		-lTAO \
		-lTAO_PortableServer \
		-lTAO_CosNaming \
		-lTAO_IORTable \
		-lTAO_Svc_Utils \
		-lTAO_DynamicAny \
		-lTAO_Strategies \
		-licuuc \
		$(OTL_LIBS) \
		$(LIBS)
#		-lTAO_CosNotification \

PROGRAM =../nr_srv

all:: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(DEB) $(STRIPPING) $(CFLAGS) $(LDFLAGS) $(LOCAL_LIBRARIES)


$(CORBA_GEN)/nc_server_controlC.cpp: ../common/nc_server_control.idl
	$(IDL) -Gd  $< -o $(CORBA_GEN)

$(CORBA_GEN)/nc_server_controlS.cpp: ../common/nc_server_control.idl
	$(IDL) -Gd  $< -o $(CORBA_GEN)

$(CORBA_GEN)/nc_server_controlS_T.cpp: ../common/nc_server_control.idl
	$(IDL) -Gd  $< -o $(CORBA_GEN)

$(CORBA_GEN)/nr_ifaceC.cpp: ../corba/nr_iface.idl
	$(IDL) -Gd  $< -o $(CORBA_GEN)

$(CORBA_GEN)/nr_ifaceS.cpp: ../corba/nr_iface.idl
	$(IDL) -Gd  $< -o $(CORBA_GEN)

$(CORBA_GEN)/nr_ifaceS_T.cpp: ../corba/nr_iface.idl
	$(IDL) -Gd  $< -o $(CORBA_GEN)

../dbiface/libdbiface.a:
	cd ../dbiface && make

clean:
	$(RM) -f *.o
	$(RM) -f */*.o
	$(RM) -f ../common/*.o

tags: FORCE
	etags *.[hc]xx ../common/*.[hc]xx

FORCE:
