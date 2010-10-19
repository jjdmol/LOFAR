//#  NodeManagerClient.h: 
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

#ifndef NODEMANAGERCLIENT_H
#define NODEMANAGERCLIENT_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <PropertyProxy.h>
#include <APL/NodeManager.h> // for the type TNodeList 

namespace LOFAR 
{
 namespace ANM 
 {  

class NodeManagerDaemon;
/**
*/

class NodeManagerClient : public GCF::TM::GCFTask
{
  public:
    NodeManagerClient (NodeManagerDaemon& daemon);
    virtual ~NodeManagerClient () {}
    
  public: // member functions
  
  private: // state methods
    GCF::TM::GCFEvent::TResult initial     (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult operational (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult claiming    (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult releasing   (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult closing     (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
        
  private: // helper methods
    
  private: // data members        
    GCF::TM::GCFTCPPort _nmcPort;
    NodeManagerDaemon&  _daemon;
    TNodeList           _curClaimedNodes;
    PropertyProxy       _propertyProxy;

  private: // admin members
    TNodeList _newClaimedNodes;
    TNodeList _nodesToRelease;
    TNodeList _releasedNodes;
    TNodeList _faultyNodes;
    unsigned int _nrOfValueGetRequests;
};
 } // namespace ANM
} // namespace LOFAR

#endif
