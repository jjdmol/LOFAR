//#  MISDaemon.h: 
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

#ifndef MISDAEMON_H
#define MISDAEMON_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <MISPolicyHandler.h>

namespace LOFAR 
{
 namespace AMI 
 {  

class MISSession;

/**
*/

class MISDaemon : public GCF::TM::GCFTask
{
  public:
    MISDaemon ();
    virtual ~MISDaemon () {}
    
  public: // member functions
    GCF::TM::GCFTCPPort& getPortProvider();
    MISPolicyHandler& getPolicyHandler();
    void clientClosed(MISSession& client);
  
  private: // state methods
    GCF::TM::GCFEvent::TResult initial   (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult accepting (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
        
  private: // helper methods
    
  private: // data members        
    GCF::TM::GCFTCPPort  _misdPortProvider;
    MISPolicyHandler _policyHandler;

  private: // admin members
    typedef list<MISSession*> TSessions;
    TSessions        _sessionsGarbage;    
};

inline GCF::TM::GCFTCPPort& MISDaemon::getPortProvider()
{
  return _misdPortProvider;
}

inline MISPolicyHandler& MISDaemon::getPolicyHandler()
{
  return _policyHandler;
}

 } // namespace AMI
} // namespace LOFAR

#endif
