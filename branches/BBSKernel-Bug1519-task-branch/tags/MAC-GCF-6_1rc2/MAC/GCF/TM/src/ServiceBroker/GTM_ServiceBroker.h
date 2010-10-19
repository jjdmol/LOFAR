//#  GTM_ServiceBroker.h: singleton class; bridge between controller application 
//#                    and Property Agent
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

#ifndef GTM_SERVICEBROKER_H
#define GTM_SERVICEBROKER_H

#include <GCF/TM/GCF_Task.h>
#include "GTM_SBTCPPort.h"
#include <GCF/TM/GCF_Handler.h>

namespace LOFAR 
{
 namespace GCF 
 {  
  namespace SB 
  {

/**
*/

class GTMSBHandler;

class GTMServiceBroker : public TM::GCFTask
{
  public:
    ~GTMServiceBroker ();
    static GTMServiceBroker* instance(bool temporary = false);
    static void release();

  public: // member functions
    void registerService(TM::GCFTCPPort& servicePort);
    void unregisterService(TM::GCFTCPPort& servicePort);
    void getServiceinfo (TM::GCFTCPPort& clientPort, const string& remoteServiceName);
    void deletePort(TM::GCFTCPPort& port);
  
  private:
    friend class GTMSBHandler;
    GTMServiceBroker ();

  private: // state methods
    GCFEvent::TResult initial   (TM::GCFEvent& e, TM::GCFPortInterface& p);
    GCFEvent::TResult operational (TM::GCFEvent& e, TM::GCFPortInterface& p);
        
  private: // helper methods
    typedef struct Action
    {
      unsigned short action;
      TM::GCFTCPPort* pPort;
      string servicename;
      Action& operator= (const Action& other)
      {        
        if (this != &other)
        {
          action = other.action;
          pPort = other.pPort;
          servicename.replace(0, string::npos, other.servicename);          
        }
        return *this;
      }      
    } TAction;
    unsigned short registerAction (TAction action);

  private: // data members        
    GTMSBTCPPort _serviceBroker;

  private: // admin members  
    typedef map<unsigned short /*seqnr*/, TAction>  TActionSeqList;
    TActionSeqList _actionSeqList;    
    typedef map<string /*remoteservicename*/, list<TM::GCFTCPPort*> >  TServiceClients;
    TServiceClients _serviceClients;    
};

class GTMSBHandler : public TM::GCFHandler
{
  public:
    
    ~GTMSBHandler() { _pInstance = 0; }
    void workProc() {}
    void stop () {}
    
  private:
    friend class GTMServiceBroker;
    GTMSBHandler();

    static GTMSBHandler* _pInstance;
    GTMServiceBroker _controller;
};
  } // namespace SB
 } // namespace GCF
} // namespace LOFAR
#endif
