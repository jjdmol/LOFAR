//#  NodeManagerClient.cc: 
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

#include <lofar_config.h>

#include <NodeManagerClient.h>
#include <NodeManagerDaemon.h>
#include <GCF/LogSys/GCF_KeyValueLogger.h>
#include <GCF/GCF_PVInteger.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/Utils.h>
#include <NM_Protocol.ph>
#include <APL/NodeManager/NMUtilities.h>
#include <NMDefines.h>

#define FULL_RS_DP(rsname) formatString("%s%s.state", NM_NODE_SCOPE.c_str(), (rsname).c_str())

enum 
{
  RS_DEFECT = -4,
  RS_SUSPECT,
  RS_VERIFY,
  RS_OFFLINE,
  RS_IDLE,
  RS_BUSY,
};

namespace LOFAR 
{
using namespace GCF::Common;
using namespace GCF::TM;
using namespace GCF::PAL;
 namespace ANM
 {
NodeManagerClient::NodeManagerClient(NodeManagerDaemon& daemon) :
  GCFTask((State)&NodeManagerClient::initial, NMC_TASK_NAME),
  _daemon(daemon),
  _propertyProxy(*this),
  _nrOfValueGetRequests(0)
{
  _nmcPort.init(*this, NMC_PORT_NAME, GCFPortInterface::SPP, NM_PROTOCOL);
  _daemon.getPortProvider().accept(_nmcPort);
}

GCFEvent::TResult NodeManagerClient::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      break;

    case F_CONNECTED:
      _daemon.clientConnected(*this);
      TRAN(NodeManagerClient::operational);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult NodeManagerClient::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static TNodeList claimNodes;
  
  switch (e.signal)
  {
    case F_DISCONNECTED:
      LOG_INFO("Connection lost to a NodeManager client.");
      p.close();
      break;
      
    case F_CLOSED:
      TRAN(NodeManagerClient::closing);
      break;
      
    case NM_CLAIM:
    {
      NMClaimEvent inRequest(e);
      
      claimNodes.clear();
      _nodesToRelease.clear();
      _faultyNodes.clear();
      _newClaimedNodes.clear();
      _releasedNodes.clear();
      
      convStringToSet(claimNodes, inRequest.nodesToClaim);
      
      _nodesToRelease = _curClaimedNodes;
      TNodeList::iterator curClaimedNode;
      // find out whether nodes must be claimed or released
      for (TNodeList::iterator nodeToClaim = claimNodes.begin();
          nodeToClaim != claimNodes.end(); ++nodeToClaim)
      {
        curClaimedNode = _curClaimedNodes.find(*nodeToClaim);

        if (curClaimedNode == _curClaimedNodes.end())
        {
          // node not yet claimed 
          
          // get current state value
          if (_propertyProxy.requestPropValue(FULL_RS_DP(*nodeToClaim)) != GCF_NO_ERROR)
          {
            _faultyNodes.insert(*nodeToClaim);
          }
          else
          {
            _newClaimedNodes.insert(*nodeToClaim);
            _nrOfValueGetRequests++;
          }        
        }
        else
        {
          _nodesToRelease.erase(*curClaimedNode);
        }            
        
      }
      // now we have a list with which are currently claimed but not needed anymore
      // so they have to release
      for (TNodeList::iterator nodeToRelease = _nodesToRelease.begin();
           nodeToRelease != _nodesToRelease.end(); ++nodeToRelease)
      {
        if (_propertyProxy.requestPropValue(FULL_RS_DP(*nodeToRelease)) != GCF_NO_ERROR)
        {
          // could not be released in the right way so pretend this is already 
          // done here
          _releasedNodes.insert(*nodeToRelease);
        }
        else
        {
          _nrOfValueGetRequests++;
        }        
      }
      if (_nrOfValueGetRequests == 0)
      {
        // no state value has to be updated so the response can be sent here
        LOG_INFO("Ready with claiming. All requested nodes are already claimed!");
        NMClaimedEvent outResponse;
        convSetToString(outResponse.newClaimedNodes, _newClaimedNodes);
        convSetToString(outResponse.releasedNodes, _releasedNodes);
        convSetToString(outResponse.faultyNodes, _faultyNodes);
        _nmcPort.send(outResponse);
      }
      else
      {
        TRAN(NodeManagerClient::claiming);
      }
      break;
    }
    case NM_RELEASE:
    {
      NMReleaseEvent inRequest(e);
      TNodeList releaseNodes;
      convStringToSet(releaseNodes, inRequest.nodesToRelease);
      for (TNodeList::iterator nodeToRelease = releaseNodes.begin();
          nodeToRelease != releaseNodes.end(); ++nodeToRelease)
      {        
        if (_propertyProxy.requestPropValue(FULL_RS_DP(*nodeToRelease)) != GCF_NO_ERROR)
        {
          // could not be released in the right way so pretend this is already 
          // done here
          _releasedNodes.insert(*nodeToRelease);
        }
        else
        {
          _nrOfValueGetRequests++;
        }      
      }
      if (_nrOfValueGetRequests == 0)
      {
        // no state value has to be updated so the response can be sent here
        LOG_INFO("Ready with releasing. All requested nodes are already (automatically) released!");
        NMReleasedEvent outResponse;
        _nmcPort.send(outResponse);
      }
      else
      {
        TRAN(NodeManagerClient::releasing);
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


GCFEvent::TResult NodeManagerClient::claiming(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_DISCONNECTED:
      LOG_INFO("Connection lost to a NodeManager client.");
      p.close();
      break;
      
    case F_CLOSED:
      TRAN(NodeManagerClient::closing);
      break;
      
    case F_VGETRESP:
    {
      GCFPropValueEvent& getResp = (GCFPropValueEvent&) e;
      GCFPVInteger& value = (GCFPVInteger&) (*getResp.pValue);
      
      string resName(NMUtilities::extractNodeName(getResp.pPropName));
      
      TNodeList::iterator nodeToRelease;
      for (nodeToRelease = _nodesToRelease.begin();
           nodeToRelease != _nodesToRelease.end(); ++nodeToRelease)
      {
        if (*nodeToRelease == resName) 
        {
          // node was selected to be released
          if (value.getValue() > RS_IDLE)
          {
            value.setValue(value.getValue() - 1); // decrease usecount
            LOG_INFO(formatString(
                "Decreases the usecount of '%s' to %d", 
                resName.c_str(),
                value.getValue()));            
            _propertyProxy.setPropValue(getResp.pPropName, value);
          }
          else
          {
            LOG_INFO(formatString(
                "Could not decrease the usecount of '%s' (state == %d)", 
                resName.c_str(),
                value.getValue()));           
          }
          _releasedNodes.insert(*nodeToRelease);
          _curClaimedNodes.erase(*nodeToRelease);
          break;
        }
      }
      
      if (nodeToRelease == _nodesToRelease.end())
      {
        // not in releasedNodes list, so it must be claimed
        if (value.getValue() < RS_IDLE)
        {
          LOG_INFO(formatString(
              "Could not increase the usecount of '%s' (state == %d)", 
              resName.c_str(),
              value.getValue()));
          // could not be claimed due to the malfunctioning state of the node
          _faultyNodes.insert(resName);
          _newClaimedNodes.erase(resName);
        }
        else 
        {
          value.setValue(value.getValue() + 1); // increase usecount
          LOG_INFO(formatString(
              "Increases the usecount of '%s' to %d", 
              resName.c_str(),
              value.getValue()));
          
          _propertyProxy.setPropValue(getResp.pPropName, value);
          _curClaimedNodes.insert(resName);
        }        
      }
      
      _nrOfValueGetRequests--;
      if (_nrOfValueGetRequests == 0)
      {
        LOG_INFO("Ready with claiming.");
        NMClaimedEvent outResponse;
        convSetToString(outResponse.newClaimedNodes, _newClaimedNodes);
        convSetToString(outResponse.releasedNodes, _releasedNodes);
        convSetToString(outResponse.faultyNodes, _faultyNodes);
        _nmcPort.send(outResponse);
        TRAN(NodeManagerClient::operational);
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult NodeManagerClient::releasing(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_DISCONNECTED:
      LOG_INFO("Connection lost to a NodeManager client.");
      p.close();
      break;
      
    case F_CLOSED:
      TRAN(NodeManagerClient::closing);
      break;
      
    case F_VGETRESP:
    {
      GCFPropValueEvent& getResp = (GCFPropValueEvent&) e;
      GCFPVInteger& value = (GCFPVInteger&) (*getResp.pValue);

      // extract the resource (node) name from the propName
      string resName(NMUtilities::extractNodeName(getResp.pPropName));

      if (value.getValue() > RS_IDLE)
      {
        value.setValue(value.getValue() - 1); // decrease usecount
        LOG_INFO(formatString(
            "Decreases the usecount of '%s' to %d", 
            resName.c_str(),
            value.getValue()));
        _propertyProxy.setPropValue(getResp.pPropName, value);
      }
      else
      {
        LOG_INFO(formatString(
            "Could not decrease the usecount of '%s' (state == %d)", 
            resName.c_str(),
            value.getValue()));           
      }
      _curClaimedNodes.erase(resName);
            
      _nrOfValueGetRequests--;
      if (_nrOfValueGetRequests == 0)
      {
        LOG_INFO("Ready with releasing.");
        NMReleasedEvent outResponse;
        _nmcPort.send(outResponse);
        TRAN(NodeManagerClient::operational);
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult NodeManagerClient::closing(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("Client gone. Start autoreleasing of all currently claimed nodes.");
      for (TNodeList::iterator nodeToRelease = _curClaimedNodes.begin();
           nodeToRelease != _curClaimedNodes.end(); ++nodeToRelease)
      {
        if (_propertyProxy.requestPropValue(FULL_RS_DP(*nodeToRelease)) == GCF_NO_ERROR)
        {
          _nrOfValueGetRequests++;
        }        
      }
      if (_nrOfValueGetRequests == 0)
      {
        _daemon.clientClosed(*this);
      }
      break;     
    }
    case F_VGETRESP:
    {
      GCFPropValueEvent& getResp = (GCFPropValueEvent&) e;
      GCFPVInteger& value = (GCFPVInteger&) (*getResp.pValue);

      // extract the resource (node) name from the propName
      string resName(NMUtilities::extractNodeName(getResp.pPropName));

      if (value.getValue() > RS_IDLE)
      {
        value.setValue(value.getValue() - 1); // decrease usecount
        LOG_INFO(formatString(
            "Decreases the usecount of '%s' to %d", 
            resName.c_str(),
            value.getValue()));        
        _propertyProxy.setPropValue(getResp.pPropName, value);
      }
            
      _nrOfValueGetRequests--;
      if (_nrOfValueGetRequests == 0)
      {
        LOG_INFO("Ready with autoreleasing.");
        _daemon.clientClosed(*this);
      }
      break;
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}
 } // namespace ANM
} // namespace LOFAR
