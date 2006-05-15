//#  GSB_Controller.h: main class of the Property Agent
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

#ifndef GSB_CONTROLLER_H
#define GSB_CONTROLLER_H

#include <GCF/TM/GCF_Task.h>
#include <GTM_SBTCPPort.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM
  {
class GCFEvent;
class GCFPortInterface; 
  }   
  namespace SB 
  {

/**
   This is the main class of the Property Agent. It uses a number of helper 
   classes to manage PML requests, registered scopes and use counts of created 
   properties. The assigned port provider supports the possibility to accept 
   more than one connect request from different clients (PML).
*/

class GSBController : public TM::GCFTask
{
	public:
		GSBController();
		virtual ~GSBController();
  
	private: // state methods
		TM::GCFEvent::TResult initial(TM::GCFEvent& e, TM::GCFPortInterface& p);
		TM::GCFEvent::TResult operational(TM::GCFEvent& e, TM::GCFPortInterface& p);

  private: // helper methods
    typedef struct
    {
      string host;
      unsigned int portNumber;
      TM::GCFPortInterface* pPortToOwner;
    } TServiceInfo;

    void acceptConnectRequest();
    void clientGone(TM::GCFPortInterface& p);
    void readRanges();
    unsigned int claimPortNumber(const string& host);
    void cleanupGarbage();
    void freePort(const string& servicename, TServiceInfo* pServiceInfo);
    
	private: // data members
    typedef map<string /*service(task:portname)*/, TServiceInfo> TServices;
    TServices _services;
    
    typedef map<unsigned int /*portnumber*/, bool /*in use or not*/> TPortStates;
    typedef map<string /*hostname*/, TPortStates> TPortHosts;
    TPortHosts _availableHosts;

    list<TM::GCFPortInterface*> _brokerClients;		
    list<TM::GCFPortInterface*> _brokerClientsGarbage;
		GTMSBTCPPort					  _brokerProvider;

    TServiceInfo* findService(const string& servicename);
    TPortStates* findHost(const string& host);
    
  private: // admin. data members
    bool                _isBusy;
    bool                _isRegistered;
    unsigned int        _counter;  
};
  } // namespace SB
 } // namespace GCF
} // namespace LOFAR
#endif
