//#  GPA_DistClientManager.h: manages the properties with its use count
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef GPA_DISTCLIENTMANAGER_H
#define GPA_DISTCLIENTMANAGER_H

#include <GPA_Defines.h>
#include <GSA_Service.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <GCF/TM/GCF_Fsm.h>
/**
   This class manages the properties with its use count, which are created 
   (resp. deleted) by means of the base class GSAService.
*/

class GPAController;

class GPADistClientManager : public GSAService
{
  public:
  	GPADistClientManager(GPAController& controller);
  	virtual ~GPADistClientManager();

  protected:
    void dpCreated(const string& /*propName*/) {};
    void dpDeleted(const string& /*propName*/) {};
    void dpeValueGet(const string& propName, const GCFPValue& value); 
    void dpeValueChanged(const string& propName, const GCFPValue& value);
    void dpeSubscribed(const string& propName);
    void dpeUnsubscribed(const string& propName);
    const string getNextGoneClient();
    
  private: // helper methods

  private: // data members
    GPAController&	  _controller;
    
  private: // admin. data members
    typedef map<int /* manNr */, bool /* available */> TManagerStates;
    typedef struct
    {
      bool available;
      TManagerStates managers;
    } TDistSystemState;
    typedef map<int /*sysnr*/, TDistSystemState> TConnectionStates;
    TConnectionStates _connectionStates;
    list<string>      _goneClients;
    GCFDummyPort      _dummyPort;
};
#endif
