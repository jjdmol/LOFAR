//#  BeamletAllocator.cc: Implementation of the BeamletAllocator
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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <Common/lofar_sstream.h>
#include <APLCommon/APLUtilities.h>

#include "BeamletAllocator.h"

INIT_TRACER_CONTEXT(LOFAR::GSO::BeamletAllocator,LOFARLOGGER_PACKAGE);

using namespace LOFAR::APLCommon;

namespace LOFAR
{

namespace GSO
{

BeamletAllocator::BeamletAllocator(int16 maxBeamlets) :
  m_allocation(),
  m_maxBeamlets(maxBeamlets)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
}


BeamletAllocator::~BeamletAllocator()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
}

bool BeamletAllocator::allocateBeamlets(
  const string&               vi,
  const vector<string>        stations,
  const time_t                startTime, 
  const time_t                stopTime, 
  const vector<int16>         subbands, 
  TStationBeamletAllocation&  allocation)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  // First the current allocation of a VI is removed.
  // The the new allocation is tested and added. 
  // Finally, if the new allocation does not fit, 
  // the extracted allocated is re-inserted
  
  TStation2AllocationMapPtr oldAllocationDetailsPtr(new TStation2AllocationMap);
  TStation2AllocationMapPtr newAllocationDetailsPtr(new TStation2AllocationMap);

  _extractAllocation(vi,oldAllocationDetailsPtr);
  bool allocationOk = _testAllocateBeamlets(vi,stations,startTime,stopTime,subbands,newAllocationDetailsPtr,allocation);
  if(allocationOk)
  {
    allocationOk = _mergeAllocation(newAllocationDetailsPtr);
  }
  else
  {
    _mergeAllocation(oldAllocationDetailsPtr);
  }
  return allocationOk;
}

void BeamletAllocator::deallocateBeamlets(const string& vi)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  // Deallocate a VI by removing the current allocation
  
  TStation2AllocationMapPtr oldAllocationDetailsPtr(new TStation2AllocationMap);
  _extractAllocation(vi,oldAllocationDetailsPtr);
}

void BeamletAllocator::logAllocation(bool groupByVI)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  map<string,string> logs;
  map<string,string>::iterator logsIterator;
  TStation2AllocationMap::iterator itStationAllocation;
  
  for(itStationAllocation = m_allocation.begin();itStationAllocation != m_allocation.end();++itStationAllocation)
  {
    TBeamlet2AllocationMap::iterator itBeamletAllocation;
    for(itBeamletAllocation = itStationAllocation->second.begin();itBeamletAllocation != itStationAllocation->second.end();++itBeamletAllocation)
    {
      TAllocationInfoVector::iterator itAllocationInfo;
      for(itAllocationInfo = itBeamletAllocation->second.begin();itAllocationInfo != itBeamletAllocation->second.end();++itAllocationInfo)
      {
        tm* pStartTm = localtime(&itAllocationInfo->startTime);
        char startTimeStr[100];
        sprintf(startTimeStr,"%02d-%02d-%04d %02d:%02d:%02d",pStartTm->tm_mday,pStartTm->tm_mon+1,pStartTm->tm_year+1900,pStartTm->tm_hour,pStartTm->tm_min,pStartTm->tm_sec);
        
        tm* pStopTm = localtime(&itAllocationInfo->stopTime);
        char stopTimeStr[100];
        sprintf(stopTimeStr,"%02d-%02d-%04d %02d:%02d:%02d",pStopTm->tm_mday,pStopTm->tm_mon+1,pStopTm->tm_year+1900,pStopTm->tm_hour,pStopTm->tm_min,pStopTm->tm_sec);

        char allocationStr[100];
        sprintf(allocationStr,"%03d,->,%03d",itBeamletAllocation->first,itAllocationInfo->subband);
        
        stringstream logStream;
        
        if(groupByVI)
        {
          logStream << itAllocationInfo->vi << ",";
          logStream << itStationAllocation->first << ",";
        }
        else
        {
          logStream << itStationAllocation->first << ",";
          logStream << itAllocationInfo->vi << ",";
        }
        logStream << allocationStr << ",";
        logStream << startTimeStr << ",";
        logStream << stopTimeStr << endl;
        
        string log;
        string key = itStationAllocation->first;
        if(groupByVI)
        {
          key = itAllocationInfo->vi;
        }
        logsIterator = logs.find(key);
        if(logsIterator != logs.end())
        {
          log = logsIterator->second;
        }
        logs[key] = log+logStream.str();
      }
    }
  }
  
  string logString("Current beamlet allocation:\n");
  if(groupByVI)
  {
    logString += "VI, Station, beamlet->subband, starttime, stoptime\n";
  }
  else
  {
    logString += "Station, VI, beamlet->subband, starttime, stoptime\n";
  }
  for(logsIterator=logs.begin();logsIterator!=logs.end();++logsIterator)
  {
    logString+=logsIterator->second;
  }
  LOG_DEBUG(logString.c_str());
}

BeamletAllocator::TStation2AllocationMap::iterator BeamletAllocator::_addStationAllocation(const string& station)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  // add empty allocation vectors for all beamlets
  TAllocationInfoVector emptyVector;
  TBeamlet2AllocationMap beamlet2AllocationMap;
  for(int16 beamlet=0;beamlet<m_maxBeamlets;beamlet++)
  {
    beamlet2AllocationMap[beamlet] = emptyVector;
  }
  pair<TStation2AllocationMap::iterator,bool> inserted;
  inserted = m_allocation.insert(TStation2AllocationMap::value_type(station,beamlet2AllocationMap));
  return inserted.first;
}

bool BeamletAllocator::_testAllocateBeamlets(
  const string&               vi,
  const vector<string>        stations,
  const time_t                startTime, 
  const time_t                stopTime, 
  const vector<int16>         subbands,
  TStation2AllocationMapPtr&  newAllocationDetailsPtr, 
  TStationBeamletAllocation&  newAllocationBeamlets)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  time_t timeNow = APLUtilities::getUTCtime();
  bool allocationOk(true);
  newAllocationDetailsPtr->clear();
  newAllocationBeamlets.clear();
  
  vector<string>::const_iterator itStation;
  for(itStation = stations.begin();allocationOk && itStation != stations.end(); ++itStation)
  {
    vector<int16>           beamletVector;
    TBeamlet2AllocationMap  beamletAllocationMap;
    
    TStation2AllocationMap::iterator itStationAllocation = m_allocation.find(*itStation);
    if(itStationAllocation == m_allocation.end())
    {
      LOG_TRACE_FLOW(formatString("Adding station %s to allocation data",itStation->c_str()));
      itStationAllocation = _addStationAllocation(*itStation);
    }
    if(itStationAllocation == m_allocation.end())
    {
      allocationOk = false;
      LOG_FATAL(formatString("Beamlet allocation for station %s, VI %s failed",itStation->c_str(),vi.c_str()));
    }
    else
    {
      // station found in allocation administration. 
      // Check its current allocation
      vector<int16>::const_iterator     itSubbands = subbands.begin();
      TBeamlet2AllocationMap::iterator  itBeamletAllocation = itStationAllocation->second.begin();
      while(allocationOk && itSubbands!=subbands.end() && itBeamletAllocation != itStationAllocation->second.end())
      {
        bool allocated=false;
        // search the next free beamlet
        while(!allocated && itBeamletAllocation != itStationAllocation->second.end())
        {
          // check ALL allocations for one beamlet
          bool roomForAllocation(true);
          TAllocationInfoVector::iterator itAllocationInfo = itBeamletAllocation->second.begin();
          while(roomForAllocation && itAllocationInfo != itBeamletAllocation->second.end())
          {
            // overlapping start/stoptimes not allowed
            if(startTime < itAllocationInfo->stopTime && stopTime > itAllocationInfo->startTime)
            {
              roomForAllocation=false;
            }
              
            // garbage collection on the fly: remove allocations with stoptimes before now
            if(itAllocationInfo->stopTime < timeNow)
            {
              itAllocationInfo = itBeamletAllocation->second.erase(itAllocationInfo);
            }
            else
            {
              ++itAllocationInfo;
            }
          }
          if(roomForAllocation)
          {
            // Yes, this beamlet is free for allocation
            TAllocationInfo newAllocation;
            newAllocation.vi        = vi;
            newAllocation.startTime = startTime;
            newAllocation.stopTime  = stopTime;
            newAllocation.subband   = (*itSubbands);
            
            TAllocationInfoVector allocationVector;
            allocationVector.push_back(newAllocation);
            beamletAllocationMap[itBeamletAllocation->first] = allocationVector;
            beamletVector.push_back(itBeamletAllocation->first);
            
            allocated=true;
          }
          ++itBeamletAllocation; // next round: begin with the next beamlet because the current has been allocated.
        }
        if(!allocated)
        {
          allocationOk = false;
          LOG_FATAL(formatString("Beamlet allocation for station %s, VI %s, subband %d failed",itStation->c_str(),vi.c_str(),(*itSubbands)));
        }
        ++itSubbands; // allocate next subband
      }
      if(itSubbands!=subbands.end())
      {
        allocationOk = false;
        LOG_FATAL(formatString("Beamlet allocation for station %s, VI %s failed",itStation->c_str(),vi.c_str()));
      }
    }
    if(allocationOk)
    {
      (*newAllocationDetailsPtr)[*itStation] = beamletAllocationMap;
      newAllocationBeamlets[*itStation] = beamletVector;
    }
  }  
  
  if(!allocationOk)
  {
    newAllocationDetailsPtr->clear();
    newAllocationBeamlets.clear();
  }
  return allocationOk;
}

bool BeamletAllocator::_mergeAllocation(TStation2AllocationMapPtr newAllocationDetailsPtr)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");

  // merge the new allocation with the existing allocation
  
  bool allocationOk = true;
  
  for(TStation2AllocationMap::iterator itNewStationAllocation = newAllocationDetailsPtr->begin();itNewStationAllocation != newAllocationDetailsPtr->end();++itNewStationAllocation)
  {
    TStation2AllocationMap::iterator itStationAllocation    = m_allocation.find(itNewStationAllocation->first);
    if(itStationAllocation == m_allocation.end())
    {
      allocationOk = false;
      LOG_FATAL(formatString("Beamlet allocation for station %s failed",itNewStationAllocation->first.c_str()));
    }
    else
    {
      // station found in allocation administration; go through all beamlets. 
      TBeamlet2AllocationMap::iterator itNewBeamletAllocation = itNewStationAllocation->second.begin();
      while(allocationOk && itNewBeamletAllocation != itNewStationAllocation->second.end())
      {
        TBeamlet2AllocationMap::iterator itBeamletAllocation = itStationAllocation->second.find(itNewBeamletAllocation->first);
        if(itBeamletAllocation == itStationAllocation->second.end())
        {        
          allocationOk = false;
          LOG_FATAL(formatString("Beamlet allocation for station %s, beamlet %d failed",itNewStationAllocation->first.c_str(),itNewBeamletAllocation->first));
        }
        else
        {
          // merge the new allocation into the existing
          itBeamletAllocation->second.insert(itBeamletAllocation->second.end(),
                                             itNewBeamletAllocation->second.begin(),
                                             itNewBeamletAllocation->second.end());
          ++itNewBeamletAllocation;
        }
      }
    }
  }
  return allocationOk;
}

void BeamletAllocator::_extractAllocation(const string& vi, TStation2AllocationMapPtr& allocationDetailsPtr)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  // Extract an allocation for a VI from the allocation administration.
  // The extracted allocation details are returned to the caller
  
  allocationDetailsPtr->clear();
  TStation2AllocationMap::iterator itStationAllocation;
  for(itStationAllocation = m_allocation.begin();itStationAllocation != m_allocation.end();++itStationAllocation)
  {
    TBeamlet2AllocationMap  beamletAllocationMap;
    TBeamlet2AllocationMap::iterator itBeamletAllocation;
    for(itBeamletAllocation = itStationAllocation->second.begin();itBeamletAllocation != itStationAllocation->second.end();++itBeamletAllocation)
    {
      TAllocationInfoVector::iterator itAllocationInfo = itBeamletAllocation->second.begin();
      while(itAllocationInfo != itBeamletAllocation->second.end())
      {
        if(itAllocationInfo->vi == vi)
        {
          TAllocationInfo allocation;
          allocation.vi        = itAllocationInfo->vi;
          allocation.startTime = itAllocationInfo->startTime;
          allocation.stopTime  = itAllocationInfo->stopTime;
          allocation.subband   = itAllocationInfo->subband;
          
          TAllocationInfoVector allocationVector;
          allocationVector.push_back(allocation);
          beamletAllocationMap[itBeamletAllocation->first] = allocationVector;
          
          itAllocationInfo = itBeamletAllocation->second.erase(itAllocationInfo);
        }
        else
        {
          ++itAllocationInfo;
        }
      }
    }
    (*allocationDetailsPtr)[itStationAllocation->first] = beamletAllocationMap;
  }
}


};
};
