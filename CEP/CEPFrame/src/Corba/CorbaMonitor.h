//  Monitor.h: Corba monitor mechanism
//
//  Copyright (C) 2000-2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//  Revision 1.2  2002/05/07 08:58:42  gvd
//  Added BUILT_SOURCES to Corba makefile.am to force idl build
//  Added BaseSim/Corba to includes of .hh files
//  Move CorbaControlClient from Corba to CorbaExe (for correct build order)
//
//  Revision 1.1  2002/05/02 12:10:50  schaaf
//  new class CorbaMonitor
//
//
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_CORBAMONITOR_H
#define BASESIM_CORBAMONITOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "corba.h"
#include "BaseSim/Corba/CorbaControl_s.hh"
class WorkHolder;

class CorbaMonitor:public POA_CorbaControl::Monitor
{
public:

  CorbaMonitor(PortableServer::POA_var         aRootPOA,
	       PortableServer::POAManager_var  aPOAManager,
	       const string&                         aMonitorname,
	       WorkHolder*                     aWorkHolder);
  virtual ~CorbaMonitor();

  CORBA::Long getValue(const char* _name);

 private:
  WorkHolder* itsWorker;
};

#endif
