//  BS_Corba.h: Corba Basis functionality
//
//  Copyright (C) 20000-2002
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

#ifndef CEPFRAME_BS_CORBA_H
#define CEPFRAME_BS_CORBA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pthread.h>
#include <corba.h>
/**

*/

class BS_Corba
{
 public:
  BS_Corba();
  virtual ~BS_Corba();

  static bool                           init();
  static CORBA::ORB_var                 getOrb();
  static PortableServer::POA_var        getPOA();
  static PortableServer::POAManager_var getPOAManager();


 private:
  static void* run_orb(void *);
  static pthread_t                      itsORBThread;
  static CORBA::ORB_var                 itsORB;
  static PortableServer::POA_var        itsRootPOA;
  static PortableServer::POAManager_var itsPOAManager;

};

inline CORBA::ORB_var BS_Corba::getOrb() {return itsORB;}
inline PortableServer::POA_var BS_Corba::getPOA() {return itsRootPOA;}
inline PortableServer::POAManager_var BS_Corba::getPOAManager() {return itsPOAManager;}

#endif

