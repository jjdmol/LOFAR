//#  NodeManager.cc: 
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

#include <APL/NodeManager.h>
#include <GCF/LogSys/GCF_KeyValueLogger.h>
#include <GCF/GCF_PVChar.h>
#include <GCF/Utils.h>
#include <GCF/ParameterSet.h>
#include <NM_Protocol.ph>

namespace LOFAR 
{
using namespace GCF::Common;
using namespace GCF::TM;
 namespace ANM
 {

NodeManager::NodeManager(NodeManagerInterface& interface) :
  GCFTask((State)&NodeManager::initial, "APL-NM"),
  _interface(interface)
{
  // register the protocol for debugging purposes
  registerProtocol(NM_PROTOCOL, NM_PROTOCOL_signalnames);

  _nmPort.init(*this, "client", GCFPortInterface::SAP, NM_PROTOCOL);
  ParameterSet::instance()->adoptFile("NodeManager.conf");
  start();
}

GCFEvent::TResult NodeManager::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      if (!_nmPort.isConnected())
      {
        _nmPort.open();
      }
      break;

    case F_CONNECTED:
      TRAN(NodeManager::operational);
      break;

    case F_DISCONNECTED:
      p.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult NodeManager::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_DISCONNECTED:
      LOG_INFO("Connection lost to the NodeManager daemon.");
      p.close();
      break;
      
    case F_CLOSED:
      TRAN(NodeManager::initial);
      break;
      
    case NM_CLAIMED:
    {
      NMClaimedEvent inResponse(e);
      TNodeList newClaimedNodes, releasedNodes, faultyNodes;
      Utils::convStringToSet(newClaimedNodes, inResponse.newClaimedNodes);
      Utils::convStringToSet(releasedNodes, inResponse.releasedNodes);
      Utils::convStringToSet(faultyNodes, inResponse.faultyNodes);
      _interface.nodesClaimed(newClaimedNodes, releasedNodes, faultyNodes);
      break;
    }
    case NM_RELEASED:
    {
      NMReleasedEvent inResponse(e);
      
      _interface.nodesReleased();      
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void NodeManager::claimNodes(TNodeList& nodesToClaim)
{
  NMClaimEvent outRequest;
  
  Utils::convSetToString(outRequest.nodesToClaim, nodesToClaim);
  if (_nmPort.isConnected())
  {
    _nmPort.send(outRequest);
  }
}

void NodeManager::releaseNodes(TNodeList& nodesToRelease)
{
  NMReleaseEvent outRequest;
  
  Utils::convSetToString(outRequest.nodesToRelease, nodesToRelease);
  if (_nmPort.isConnected())
  {
    _nmPort.send(outRequest);
  }  
}
 } // namespace ANM
} // namespace LOFAR
