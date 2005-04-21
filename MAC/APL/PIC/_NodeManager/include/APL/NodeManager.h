//#  NodeManager.h: 
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

#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_TCPPort.h>

namespace LOFAR 
{
 namespace ANM 
 {  

typedef list<string> TNodeList;

class NodeManagerInterface 
{
  protected:
    NodeManagerInterface() {}
    virtual ~NodeManagerInterface() {}
    
  public:
    virtual void nodesClaimed(TNodeList& newClaimedNodes, 
                              TNodeList& releasedNodes, 
                              TNodeList& faultyNodes) = 0;
    virtual void nodesReleased() = 0;
    
  private:
    // protected copy constructor
    NodeManagerInterface(const NodeManagerInterface&);
    // protected assignment operator
    NodeManagerInterface& operator=(const NodeManagerInterface&);
};

class NodeManager : public GCF::TM::GCFTask
{
  public:
    NodeManager (NodeManagerInterface& interface);
    virtual ~NodeManager () {}
    
  public: // member functions
    void claimNodes(TNodeList& nodesToClaim);
    void releaseNodes(TNodeList& nodesToRelease);
  
  private: // state methods
    GCF::TM::GCFEvent::TResult initial     (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    GCF::TM::GCFEvent::TResult operational (GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
        
  private: // helper methods
    
  private: // data members        
    GCF::TM::GCFTCPPort  _nmPort;
    NodeManagerInterface& _interface;

  private: // admin members
};

 } // namespace ANM
} // namespace LOFAR

#endif
