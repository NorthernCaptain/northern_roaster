#ifndef NCORBA_HEADER_HXX
#define NCORBA_HEADER_HXX

#define TAO_ORB

#ifdef TAO_ORB
#include "ace/OS.h"
#include <tao/ORB.h>
#include <orbsvcs/orbsvcs/CosNamingC.h>
#endif

#ifdef OMNI_ORB
#include <omniORB4/CORBA.h>
#include <omniORB4/Naming.hh>
#endif

#endif

