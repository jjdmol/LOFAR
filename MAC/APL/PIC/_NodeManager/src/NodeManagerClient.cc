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

#include "NodeManagerClient.h"
#include "NodeManagerDaemon.h"
#include <GCF/LogSys/GCF_KeyValueLogger.h>
#include <GCF/GCF_PVInteger.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/Utils.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <NM_Protocol.ph>

#define FULL_RS_DP(rsname) formatString("PIC_CEP_%s.state", (rsname).c_str())
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
  GCFTask((State)&NodeManagerClient::initial, "APL-NMC"),
  _daemon(daemon),
  _propertyProxy(*this),
  _nrOfValueGetRequests(0)
{
  _nmcPort.init(*this, "nmd-client", GCFPortInterface::SPP, NM_PROTOCOL);
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

  static list<string> claimNodes;
  
  switch (e.signal)
  {
    case F_DISCONNECTED:
      LOG_INFO("Connection lost to a NodeManager client.");
      p.close();
      break;
      
    case F_CLOSED:
      _daemon.clientClosed(*this);
      break;
      
    case NM_CLAIM:
    {
      NMClaimEvent inRequest(e);
      
      claimNodes.clear();
      _nodesToRelease.clear();
      _faultyNodes.clear();
      _newClaimedNodes.clear();
      _releasedNodes.clear();
      
      Utils::convStringToList(claimNodes, inRequest.nodesToClaim);
      _nodesToRelease = _curClaimedNodes;
      list<string>::iterator curClaimedNode;
      for (list<string>::iterator nodeToClaim = claimNodes.begin();
          nodeToClaim != claimNodes.end(); ++nodeToClaim)
      {
        for (curClaimedNode = _curClaimedNodes.begin();
             curClaimedNode != _curClaimedNodes.end(); ++curClaimedNode)
        {
          if (curClaimedNode == nodeToClaim)
          {
            _nodesToRelease.remove(*curClaimedNode);
            break;
          }
        }
        if (curClaimedNode == _curClaimedNodes.end())
        {
          _newClaimedNodes.push_back(*nodeToClaim);
        }
        
        if (_propertyProxy.requestPropValue(FULL_RS_DP(*nodeToClaim)) != GCF_NO_ERROR)
        {
          _faultyNodes.push_back(*nodeToClaim);
        }
        else
        {
          _nrOfValueGetRequests++;
        }        
      }
      for (list<string>::iterator nodeToRelease = _nodesToRelease.begin();
           nodeToRelease != _nodesToRelease.end(); ++nodeToRelease)
      {
        if (_propertyProxy.requestPropValue(FULL_RS_DP(*nodeToRelease)) != GCF_NO_ERROR)
        {
          _faultyNodes.push_back(*nodeToRelease);
        }
        else
        {
          _nrOfValueGetRequests++;
        }        
      }
      if (_nrOfValueGetRequests == 0)
      {
        NMClaimedEvent outResponse;
        Utils::convListToString(outResponse.newClaimedNodes, _newClaimedNodes);
        Utils::convListToString(outResponse.releasedNodes, _releasedNodes);
        Utils::convListToString(outResponse.faultyNodes, _faultyNodes);
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
      list<string> releaseNodes;
      Utils::convStringToList(releaseNodes, inRequest.nodesToRelease);
      for (list<string>::iterator nodeToRelease = releaseNodes.begin();
          nodeToRelease != releaseNodes.end(); ++nodeToRelease)
      {        
        if (_propertyProxy.requestPropValue(FULL_RS_DP(*nodeToRelease)) != GCF_NO_ERROR)
        {
          _faultyNodes.push_back(*nodeToRelease);
        }
        else
        {
          _nrOfValueGetRequests++;
        }      
      }
      if (_nrOfValueGetRequests == 0)
      {
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
      _daemon.clientClosed(*this);
      break;
      
    case F_VGETRESP:
    {
      GCFPropValueEvent& getResp = (GCFPropValueEvent&) e;
      GCFPVInteger& value = (GCFPVInteger&) (*getResp.pValue);
      string resName(getResp.pPropName);
      resName.erase(0, GCFPVSSInfo::getLocalSystemName().length() + strlen(":PIC_CEP_"));
      resName.erase(resName.length() - strlen(".state"));
      list<string>::iterator nodeToRelease;
      for (nodeToRelease = _nodesToRelease.begin();
           nodeToRelease != _nodesToRelease.end(); ++nodeToRelease)
      {
        if (*nodeToRelease == resName) 
        {
          if (value.getValue() > RS_IDLE)
          {
            value.setValue(value.getValue() - 1); // decrease usecount
            _propertyProxy.setPropValue(getResp.pPropName, value);
            _releasedNodes.push_back(*nodeToRelease);
          }
          _curClaimedNodes.remove(*nodeToRelease);
          break;
        }
      }
      
      if (nodeToRelease == _nodesToRelease.end())
      {
        // not in releasedNodes list, so it must be claimed
        if (value.getValue() < RS_IDLE)
        {
          _faultyNodes.push_back(resName);
        }
        else 
        {
          value.setValue(value.getValue() + 1); // increase usecount
          _propertyProxy.setPropValue(getResp.pPropName, value);
          _curClaimedNodes.push_back(resName);
        }        
      }
      
      _nrOfValueGetRequests--;
      if (_nrOfValueGetRequests == 0)
      {
        NMClaimedEvent outResponse;
        Utils::convListToString(outResponse.newClaimedNodes, _newClaimedNodes);
        Utils::convListToString(outResponse.releasedNodes, _releasedNodes);
        Utils::convListToString(outResponse.faultyNodes, _faultyNodes);
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
      _daemon.clientClosed(*this);
      break;
      
    case F_VGETRESP:
    {
      GCFPropValueEvent& getResp = (GCFPropValueEvent&) e;
      GCFPVInteger& value = (GCFPVInteger&) (*getResp.pValue);
      string resName(getResp.pPropName);
      resName.erase(0, GCFPVSSInfo::getLocalSystemName().length() + strlen(":PIC_CEP_"));
      resName.erase(resName.length() - strlen(".state"));
      if (value.getValue() > RS_IDLE)
      {
        value.setValue(value.getValue() - 1); // decrease usecount
        _propertyProxy.setPropValue(getResp.pPropName, value);
      }
      _curClaimedNodes.remove(resName);
            
      _nrOfValueGetRequests--;
      if (_nrOfValueGetRequests == 0)
      {
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
 } // namespace ANM
} // namespace LOFAR
