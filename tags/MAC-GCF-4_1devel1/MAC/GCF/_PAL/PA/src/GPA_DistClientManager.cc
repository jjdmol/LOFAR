//#  GPA_DistClientManager.cc:
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

#include "GPA_DistClientManager.h"
#include "GPA_Controller.h"
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/GCF_PVDynArr.h>
#include <GCF/GCF_PVInteger.h>
#include <Resources.hxx>

#include <strings.h>
#include <stdio.h>
#include <unistd.h>

GPADistClientManager::GPADistClientManager(GPAController& controller) :
  _controller(controller),
  _dummyPort(0, "GPADISTM", 0)
{
  dpeGet("_Connections.Api.ManNums");
  dpeGet("_DistManager.State.SystemNums");
  dpeSubscribe("_Connections.Api.ManNums");
  dpeSubscribe("_DistManager.State.SystemNums");
}

GPADistClientManager::~GPADistClientManager()
{
}

void GPADistClientManager::dpeValueGet(const string& propName, const GCFPValue& value)
{
  if (propName.find("_Connections.Api.ManNums") < propName.length())
  {
    assert(value.getType() == LPT_DYNINTEGER);
    GCFPVDynArr* pArrayValue = (GCFPVDynArr*) &value;
    const GCFPValueArray& values = pArrayValue->getValue();
    const GCFPVInteger* pValue;

    assert(GCFPVSSInfo::getLastEventSysId() > 0);

    _connectionStates[GCFPVSSInfo::getLastEventSysId()].available = true;

    for (GCFPValueArray::const_iterator iter = values.begin();
         iter != values.end(); ++iter)
    {
      pValue = (GCFPVInteger*) *iter;
      _connectionStates[GCFPVSSInfo::getLastEventSysId()].managers[pValue->getValue()] = true;
    }
  }
  else if (propName.find("_DistManager.State.SystemNums") < propName.length())
  {
    assert(value.getType() == LPT_DYNINTEGER);
    GCFPVDynArr* pArrayValue = (GCFPVDynArr*) &value;
    const GCFPValueArray& values = pArrayValue->getValue();
    const GCFPVInteger* pValue;
    
    string distConnecions;
    
    for (GCFPValueArray::const_iterator iter = values.begin();
         iter != values.end(); ++iter)
    {
      pValue = (GCFPVInteger*) *iter;
      _connectionStates[pValue->getValue()].available = true;
      distConnecions = GCFPVSSInfo::getSystemName(pValue->getValue()) +                               
                       ":_Connections.Api.ManNums";
      dpeGet(distConnecions);
      dpeSubscribe(distConnecions);
    }
  }
}

void GPADistClientManager::dpeValueChanged(const string& propName, const GCFPValue& value)
{
  list<int> numGarbage;
  if (propName.find("_Connections.Api.ManNums") < propName.length())
  {
    assert(value.getType() == LPT_DYNINTEGER);
    GCFPVDynArr* pArrayValue = (GCFPVDynArr*) &value;
    const GCFPValueArray& values = pArrayValue->getValue();
    const GCFPVInteger* pValue;
    
    // reset all manager states
    TDistSystemState* pDistSystemState = &_connectionStates[GCFPVSSInfo::getLastEventSysId()];
    TManagerStates* pManagerStates = &pDistSystemState->managers;
    for (TManagerStates::iterator iter = pManagerStates->begin();
         iter != pManagerStates->end(); ++iter)
    {
      iter->second = false;
    }
    _connectionStates[GCFPVSSInfo::getLastEventSysId()].available = true;

    // set all manager states
    for (GCFPValueArray::const_iterator iter = values.begin();
         iter != values.end(); ++iter)
    {
      pValue = (GCFPVInteger*) *iter;
      (*pManagerStates)[pValue->getValue()] = true;
    }
    
    // all not set managers are gone (disconnected)
    string goneClient;
    for (TManagerStates::iterator iter = pManagerStates->begin();
         iter != pManagerStates->end(); ++iter)
    {
      if (!iter->second)
      {
        goneClient = formatString("%s:DPA-client%s",
            GCFPVSSInfo::getSystemName(GCFPVSSInfo::getLastEventSysId()).c_str(),
            iter->first /* manNr */);
        _goneClients.push_back(goneClient);
        numGarbage.push_back(iter->first);
      }
    }
    for (list<int>::iterator iter = numGarbage.begin();
         iter != numGarbage.end(); ++iter)
    {
      pManagerStates->erase(*iter);
    }
  }
  else if (propName.find("_DistManager.State.SystemNums") < propName.length())
  {
    assert(value.getType() == LPT_DYNINTEGER);
    GCFPVDynArr* pArrayValue = (GCFPVDynArr*) &value;
    const GCFPValueArray& values = pArrayValue->getValue();
    const GCFPVInteger* pValue;
    
    // reset all manager states
    TDistSystemState* pDistSystemState;
    for (TConnectionStates::iterator iter = _connectionStates.begin();
         iter != _connectionStates.end(); ++iter)
    {
      pDistSystemState = &iter->second;
      pDistSystemState->available = false;
    }

    // set all manager states
    for (GCFPValueArray::const_iterator iter = values.begin();
         iter != values.end(); ++iter)
    {
      pValue = (GCFPVInteger*) *iter;
      _connectionStates[pValue->getValue()].available = true;
    }
    
    // all not set managers are gone (disconnected)
    string goneClient;
    TManagerStates* pManagerStates;
    for (TConnectionStates::iterator iter = _connectionStates.begin();
         iter != _connectionStates.end(); ++iter)
    {
      pDistSystemState = &iter->second;
      if (!pDistSystemState->available)
      {
        pManagerStates = &pDistSystemState->managers;
        for (TManagerStates::iterator miter = pManagerStates->begin();
             miter != pManagerStates->end(); ++miter)
        {
          goneClient = formatString("%s:DPA-client%s",
              GCFPVSSInfo::getSystemName(iter->first).c_str(),
              miter->first /* manNr */);
          _goneClients.push_back(goneClient);
        }
        numGarbage.push_back(iter->first);
      }      
    }
    for (list<int>::iterator iter = numGarbage.begin();
         iter != numGarbage.end(); ++iter)
    {
      _connectionStates.erase(*iter);
    }
  }
  if (_goneClients.size() > 0)
  {
    GCFEvent closedEvent(F_CLOSED);
    _controller.dispatch(closedEvent, _dummyPort);
  }
}

void GPADistClientManager::dpeSubscribed(const string& propName)
{
}

void GPADistClientManager::dpeUnsubscribed(const string& propName)
{
}

const string GPADistClientManager::getNextGoneClient()
{
  string nextGoneClient("");
  if (_goneClients.size() > 0)
  {
    nextGoneClient = *_goneClients.begin();
    _goneClients.pop_front();
  }
  return nextGoneClient;
}
