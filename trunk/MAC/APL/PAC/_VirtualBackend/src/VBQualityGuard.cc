//#  VBQualityGuard.cc: Implementation of the Virtual VBQualityGuard task
//#
//#  Copyright (C) 2002-2004
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

#include "VBQualityGuard.h"
#include "VirtualBackendLD.h"
#include "VBFuncStateProperty.h"
#include <APL/NMUtilities.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVChar.h>

namespace LOFAR
{
using namespace GCF::PAL;
using namespace GCF::TM;
using namespace GCF::Common;
using namespace ANM;

  namespace AVB
  {
INIT_TRACER_CONTEXT(VBQualityGuard, LOFARLOGGER_PACKAGE);

void VBQAnswer::handleAnswer (GCFEvent& answer)
{
  switch (answer.signal)
  {
    case F_VCHANGEMSG:
    case F_VGETRESP:
    {
      GCFPropValueEvent& vcEvent = (GCFPropValueEvent&) answer;
      string propName(vcEvent.pPropName);
      DBGASSERT(vcEvent.pValue->getType() == LPT_CHAR);
      unsigned char val = ((GCFPVChar*)(vcEvent.pValue))->getValue();
      DBGASSERT(val <= 100);
      _vbqg.valueChanged(propName, val);
      break;
    }
    default:
      _vbqg.answer(answer);
      break;
  }
}

VBQualityGuard::VBQualityGuard(VirtualBackendLD& ld) :
    _vbLD(ld),
    _currQuality(0),
    _cepAppProperties(ld.getName().c_str(), "TCEPApp", &_answer),
    _answer(*this),
    _nodeManager(*this),
    _nrOfNotMonitoredNodes(0),
    _nrOfPendingSubscriptions(0),
    _lowQualityReported(false),
    _state(S_IDLE)
{ 
}


VBQualityGuard::~VBQualityGuard()
{
  for (TFuncStateProps::iterator iter = _functStateProps.begin();
       iter != _functStateProps.end(); ++iter)
  {
    delete iter->second;
  }  
}

void VBQualityGuard::monitorNodes(TNodeList& nodesToMonitor)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, _vbLD.getName().c_str());
  DBGASSERT(_state == S_IDLE || _state == S_OPERATIONAL);
  _lowQualityReported = false;
  _state = S_CLAIMING;
  _nodeManager.claimNodes(nodesToMonitor);
  //_cepAppProperties.load();
}

void VBQualityGuard::stopMonitoring()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, _vbLD.getName().c_str());
  
  if (_state != S_OPERATIONAL) return;
  
  TNodeList nodesToRelease;
  VBFuncStateProperty* pProp;
  for (TFuncStateProps::iterator iter = _functStateProps.begin();
       iter != _functStateProps.end(); ++iter)
  {
    pProp = iter->second;
    delete pProp;
    nodesToRelease.insert(iter->first);
  }
  _functStateProps.clear();
  _state = S_RELEASING;
  _nodeManager.releaseNodes(nodesToRelease);
  _currQuality = 0;  
  _lowQualityReported = false;
}

void VBQualityGuard::valueChanged(const string& propName, char newValue)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, _vbLD.getName().c_str());

  if (propName.find(":PIC_CEP_") < string::npos)
  {
    VBFuncStateProperty* pChangedProp = findFuncStatePropObject(propName);
    DBGASSERT(pChangedProp);
    if (newValue == 0)
    {
       // complitly malfunctioning
      _faultyNodes.insert(NMUtilities::extractNodeName(propName));
    }
    _currQuality -= pChangedProp->getCurrentValue() - newValue;
    _vbLD.qualityChanged();
    checkQuality();
  }
  else
  {
    
  }
}

void VBQualityGuard::answer(GCFEvent& answer)
{
  GCFPropAnswerEvent& aEvent = (GCFPropAnswerEvent&) answer;
  string propName(aEvent.pPropName);
  VBFuncStateProperty* pAProp = findFuncStatePropObject(propName);

  DBGASSERT(pAProp);

  switch (answer.signal)
  {
    case F_SUBSCRIBED:
      if (propName.find(":PIC_CEP_") < string::npos)
      {
        DBGASSERT(_state == S_SUBSCRIBING);
        _currQuality -= (100 - pAProp->getCurrentValue());
        _nrOfPendingSubscriptions--;
        if (_nrOfPendingSubscriptions == 0)
        {
          _vbLD.qualityChanged();
          _state = S_OPERATIONAL;
          checkQuality();
          if (!_lowQualityReported)
          {
            _vbLD.qualityGuardStarted();
          }
        }
      }
      break;

    case F_UNSUBSCRIBED: // subscription lost
      if (propName.find(":PIC_CEP_") < string::npos)
      {
        unsigned int oldQuality(_currQuality);

        _currQuality -= pAProp->getCurrentValue();

        if (_currQuality != oldQuality) _vbLD.qualityChanged();

        _nrOfNotMonitoredNodes++;

        _faultyNodes.insert(NMUtilities::extractNodeName(propName));

        checkQuality();
      }
      break;
  }
}  

void VBQualityGuard::nodesClaimed(TNodeList& newClaimedNodes, 
                                  TNodeList& releasedNodes,
                                  TNodeList& faultyNodes)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, _vbLD.getName().c_str());

  DBGASSERT(_state == S_CLAIMING);
  
  _faultyNodes = faultyNodes;
  _nrOfNotMonitoredNodes = faultyNodes.size();
  _nrOfPendingSubscriptions = 0;
  VBFuncStateProperty* pAProp;
  for (TNodeList::iterator iter = newClaimedNodes.begin();
       iter != newClaimedNodes.end(); ++iter)
  {
    pAProp = new VBFuncStateProperty(formatString("PIC_CEP_%s.funcState", (*iter).c_str()), _answer);
    _functStateProps[*iter] = pAProp;
    _currQuality += 100; // the actual quality of this node will be set in the 
                         // valueChanged method
  }
  
  TFuncStateProps::iterator foundNode;
  for (TNodeList::iterator iter = releasedNodes.begin();
       iter != releasedNodes.end(); ++iter)
  {   
    foundNode = _functStateProps.find(*iter);
    if (foundNode != _functStateProps.end())
    {
      pAProp = foundNode->second;
      _currQuality -= pAProp->getCurrentValue();
      delete pAProp;
      _functStateProps.erase(foundNode);        
    }
  }    

  for (TNodeList::iterator iter = newClaimedNodes.begin();
       iter != newClaimedNodes.end(); ++iter)
  {
    foundNode = _functStateProps.find(*iter);
    if (foundNode != _functStateProps.end())
    {
      pAProp = foundNode->second;
      _nrOfPendingSubscriptions++;
      pAProp->startMonitoring();
    }
  }
  if (_nrOfPendingSubscriptions == 0)
  {
    _vbLD.qualityChanged();
    _state = S_OPERATIONAL;
    checkQuality();
    if (!_lowQualityReported)
    {
      _vbLD.qualityGuardStarted();
    }
  }
  else
  {
    _state = S_SUBSCRIBING;
  }
}

void VBQualityGuard::nodesReleased()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, _vbLD.getName().c_str());

  DBGASSERT(_state == S_RELEASING);
  _state = S_IDLE;
  _vbLD.qualityGuardStopped();  
}

bool VBQualityGuard::isQualityLow() const
{
  return (_currQuality < (90.0 * NR_OF_NODES));
}

void VBQualityGuard::checkQuality()
{
  if (!_lowQualityReported && isQualityLow())
  {
    _lowQualityReported = true;
    _vbLD.lowQuality(_faultyNodes);
  }
}

VBFuncStateProperty* VBQualityGuard::findFuncStatePropObject(const string& propName)
{
  string nodeName(NMUtilities::extractNodeName(propName));
  TFuncStateProps::iterator foundNode = _functStateProps.find(nodeName);
  return (foundNode != _functStateProps.end() ? foundNode->second : 0);    
}
  } // namespace AVB
} // namespace LOFAR
