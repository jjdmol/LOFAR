//  CorbaController.h: Corba transport mechanism
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
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_CORBACONTROLLER_H
#define CEPFRAME_CORBACONTROLLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "corba.h"
#include "CEPFrame/Corba/CorbaControl_s.hh"
class VirtualMachine;

class CorbaController:public POA_CorbaControl::Controller
{
public:

  CorbaController(PortableServer::POA_var         aRootPOA,
		  PortableServer::POAManager_var  aPOAManager,
		  const string&                   aControllername,
		  VirtualMachine*                 aVM); 
  virtual ~CorbaController();

  void start();
  void stop();
  void abort();
  void connect(CORBA::Boolean _sourceside, 
	       CORBA::Long _channel,
	       CORBA::Long _ID);

 private:
  VirtualMachine* itsVM;
};

#endif
