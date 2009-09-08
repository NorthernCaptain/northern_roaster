SOURCES	+= nr_admin.cxx \
	mainwin_adm.cxx \
	dataview.cxx \
	../common/configman.cxx
HEADERS	+= ../common/debug.hxx \
	../common/boundedqueue.hxx \
	../common/configman.hxx \
	mainwin_adm.hxx \
	dataview.hxx


TARGET		= nr_admin








FORMS	= infowin/infowin.ui
TEMPLATE	=app
CONFIG	+= qt warn_on debug
INCLUDEPATH	+= $(ACE_ROOT) $(BOOST_ROOT) ../common
LIBS	+= -L$(ACE_ROOT)/ace -lACE
LANGUAGE	= C++
