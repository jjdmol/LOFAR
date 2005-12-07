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
#include <APL/APLCommon/APLUtilities.h>

#include "BeamletAllocator.h"

INIT_TRACER_CONTEXT(LOFAR::GSO::BeamletAllocator,LOFARLOGGER_PACKAGE);

using namespace LOFAR::APLCommon;

namespace LOFAR
{

namespace GSO
{

BeamletAllocator::BeamletAllocator(int16 maxBeamlets) :
  m_allocation(),
  m_suspendedAllocation(),
  m_resumedVIs(),
  m_suspendedVIs(),
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
  const uint16                priority,
  const vector<string>        stations,
  const time_t                startTime, 
  const time_t                stopTime, 
  const vector<int16>         subbands, 
  TStationBeamletAllocation&  allocation,
  map<string, TStationBeamletAllocation>& resumeVIs,
  set<string>&                suspendVIs)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  // First the current allocation of a VI is removed.
  // The the new allocation is tested and added. 
  // Finally, if the new allocation does not fit, 
  // the extracted allocated is re-inserted
  
  suspendVIs.clear();
  resumeVIs.clear();
  bool allocationOk(false);
  
  TStation2AllocationMap oldAllocationDetails;
  TStation2AllocationMap newAllocationDetails;

  // remove existing allocation
  LOG_TRACE_FLOW("allocateBeamlets: Remove existing allocation");
  _extractAllocation(vi,m_allocation,oldAllocationDetails);
  // test if it fits
  LOG_TRACE_FLOW("allocateBeamlets: Test if the new allocation fits");
  allocationOk = _testAllocateBeamlets(vi,priority,stations,startTime,stopTime,subbands,newAllocationDetails,allocation, suspendVIs);
  if(allocationOk)
  {
    // merge the new allocation
    LOG_TRACE_FLOW("allocateBeamlets: Merge the new allocation");
    allocationOk = _mergeAllocation(m_allocation,newAllocationDetails);
    
    // deallocate the suspended VI's    
    LOG_TRACE_FLOW("allocateBeamlets: Deallocate the suspended VI's");
    set<string> deAllocateSuspendVIs=suspendVIs;
    for(set<string>::const_iterator it=deAllocateSuspendVIs.begin();it!=deAllocateSuspendVIs.end();++it)
    {
      map<string, TStationBeamletAllocation> tempResumeVIs;
      set<string> tempSuspendVIs;
      TStation2AllocationMap deallocationDetails;
      _deallocateBeamlets(*it, deallocationDetails, tempResumeVIs, tempSuspendVIs);
      // save the allocation of the suspended VI's. They can be resumed lateron
      LOG_TRACE_FLOW("allocateBeamlets: Save the allocation state of the suspended VI's");
      _mergeAllocation(m_suspendedAllocation,deallocationDetails);

      resumeVIs.insert(tempResumeVIs.begin(),tempResumeVIs.end());
      suspendVIs.insert(tempSuspendVIs.begin(),tempSuspendVIs.end());
    }
    
    // save the state of the VI's for later (mis)use
    for(map<string, TStationBeamletAllocation>::iterator it=resumeVIs.begin();it!=resumeVIs.end();++it)
    {
      m_resumedVIs.insert(it->first);
    }
    m_suspendedVIs.insert(suspendVIs.begin(),suspendVIs.end());
  }
  else
  {
    // restore the old allocation and nothing changes.
    LOG_TRACE_FLOW("allocateBeamlets: Restore the old allocation");
    _mergeAllocation(m_allocation,oldAllocationDetails);
  }
  return allocationOk;
}

void BeamletAllocator::deallocateBeamlets(const string& vi,
                                          map<string, TStationBeamletAllocation>& resumeVIs,
                                          set<string>& suspendVIs)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  TStation2AllocationMap ignoreDeallocationDetails;
  _deallocateBeamlets(vi,ignoreDeallocationDetails,resumeVIs,suspendVIs);
}

void BeamletAllocator::logAllocation(bool groupByVI)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  _logAllocation(m_allocation,string("Active VI's"),groupByVI);
}
  
void BeamletAllocator::logSuspendedAllocation(bool groupByVI)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  _logAllocation(m_suspendedAllocation,string("Suspended VI's"),groupByVI);
}
  
BeamletAllocator::TStation2AllocationMap::iterator BeamletAllocator::_addStationAllocation(TStation2AllocationMap& allocation, const string& station)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  // add empty allocation vectors for all beamlets
  TAllocationInfoSet emptySet;
  TBeamlet2AllocationMap beamlet2AllocationMap;
  for(int16 beamlet=0;beamlet<m_maxBeamlets;beamlet++)
  {
    beamlet2AllocationMap[beamlet] = emptySet;
  }
  pair<TStation2AllocationMap::iterator,bool> inserted;
  inserted = allocation.insert(TStation2AllocationMap::value_type(station,beamlet2AllocationMap));
  return inserted.first;
}

bool BeamletAllocator::_testAllocateBeamlets(
  const string&               vi,
  const uint16                priority, 
  const vector<string>        stations,
  const time_t                startTime, 
  const time_t                stopTime, 
  const vector<int16>         subbands,
  TStation2AllocationMap&  newAllocationDetails, 
  TStationBeamletAllocation&  newAllocationBeamlets,
  set<string>&                suspendVIs)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  time_t timeNow = APLUtilities::getUTCtime();
  bool allocationOk(true);

  newAllocationDetails.clear();
  newAllocationBeamlets.clear();
  suspendVIs.clear();

  // test the allocation for each station  
  vector<string>::const_iterator itStation;
  for(itStation = stations.begin();allocationOk && itStation != stations.end(); ++itStation)
  {
    vector<int16>           beamletVector;
    TBeamlet2AllocationMap  beamletAllocationMap;
    
    TStation2AllocationMap::iterator itStationAllocation = m_allocation.find(*itStation);
    if(itStationAllocation == m_allocation.end())
    {
      LOG_TRACE_FLOW(formatString("_testAllocateBeamlets: Adding station %s to allocation data",itStation->c_str()));
      itStationAllocation = _addStationAllocation(m_allocation,*itStation);
    }
    if(itStationAllocation == m_allocation.end())
    {
      allocationOk = false;
      LOG_FATAL(formatString("Beamlet allocation for station %s, VI %s failed",itStation->c_str(),vi.c_str()));
    }
    else
    {
      // station found in allocation administration. 
      // Check its current allocation without looking at priorities
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
          TAllocationInfoSet::iterator itAllocationInfo = itBeamletAllocation->second.begin();
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
              TAllocationInfoSet::iterator eraseIt = itAllocationInfo++;
              itBeamletAllocation->second.erase(eraseIt);
            }
            else
            {
              ++itAllocationInfo;
            }
          }
          if(roomForAllocation)
          {
            // Yes, this beamlet is free for allocation
            LOG_TRACE_FLOW(formatString("_testAllocateBeamlets: allocation on station %s for VI %s succeeded: %d,%d,%d,%d->%d",itStation->c_str(),vi.c_str(),priority,startTime,stopTime,itBeamletAllocation->first,*itSubbands));
            TAllocationInfo newAllocation;
            newAllocation.vi        = vi;
            newAllocation.priority  = priority;
            newAllocation.startTime = startTime;
            newAllocation.stopTime  = stopTime;
            newAllocation.subband   = (*itSubbands);
            
            TAllocationInfoSet allocationSet;
            allocationSet.insert(newAllocation);
            beamletAllocationMap[itBeamletAllocation->first] = allocationSet;
            beamletVector.push_back(itBeamletAllocation->first);
            
            allocated=true;
          }
          ++itBeamletAllocation; // next round: begin with the next beamlet because the current has been allocated.
        }
        ++itSubbands; // allocate next subband
      }
      if(itSubbands!=subbands.end())
      {
        LOG_DEBUG("No room without stopping lower priority VI's");

        beamletVector.clear();
        beamletAllocationMap.clear();

            // no room without stopping lower priority VI's
            // the list of allocations is sorted by startTime
            // search the first allocation that has: 
            // 1. requestedStarttime >= starttime
            // 2. requestedStoptime <= stoptime
            // 3. priority>requestedPriority
          
            // the above means that a simple policy is used to suspend lower priority LD's:
            // the lower priority allocation is replaced only if the timespan of the 
            // higher priority allocation totally fits in the lower priority allocation.
            
        itSubbands = subbands.begin();
        itBeamletAllocation = itStationAllocation->second.begin();
        while(allocationOk && itSubbands!=subbands.end() && itBeamletAllocation != itStationAllocation->second.end())
        {
          bool allocated=false;
          // search the next free beamlet
          while(!allocated && itBeamletAllocation != itStationAllocation->second.end())
          {
            // check allocations for one beamlet
            bool roomForAllocation(false);
            string suspendVI;
            TAllocationInfoSet::iterator itAllocationInfo = itBeamletAllocation->second.begin();
            while(!roomForAllocation && itAllocationInfo != itBeamletAllocation->second.end())
            {
              LOG_TRACE_FLOW(formatString("_testAllocateBeamlets: Checking priority and times:\npriority:\t%d < %d = %s\nstarttime\t%d >= %d = %s\nstoptime\t%d <= %d = %s",priority,itAllocationInfo->priority,(priority<itAllocationInfo->priority?"TRUE":"FALSE"),startTime,itAllocationInfo->startTime,(startTime >= itAllocationInfo->startTime?"TRUE":"FALSE"),stopTime,itAllocationInfo->stopTime,(stopTime <= itAllocationInfo->stopTime?"TRUE":"FALSE")));
              if(priority < itAllocationInfo->priority &&
                 startTime >= itAllocationInfo->startTime &&
                 stopTime <= itAllocationInfo->stopTime)
              {
                roomForAllocation=true;
                suspendVI = itAllocationInfo->vi;
              }
                
              ++itAllocationInfo;
            }
            if(roomForAllocation)
            {
              // store the VI that's in the way
              if(suspendVIs.find(suspendVI) == suspendVIs.end())
              {
                LOG_DEBUG(formatString("Suspending VI:%s",suspendVI.c_str()));
                suspendVIs.insert(suspendVI);
              }
              // Yes, this beamlet is free for allocation
              LOG_TRACE_FLOW(formatString("_testAllocateBeamlets: allocation on station %s for VI %s succeeded: %d,%d,%d,%d->%d",itStation->c_str(),vi.c_str(),priority,startTime,stopTime,itBeamletAllocation->first,*itSubbands));
              TAllocationInfo newAllocation;
              newAllocation.vi        = vi;
              newAllocation.priority  = priority;
              newAllocation.startTime = startTime;
              newAllocation.stopTime  = stopTime;
              newAllocation.subband   = (*itSubbands);
              
              TAllocationInfoSet allocationSet;
              allocationSet.insert(newAllocation);
              beamletAllocationMap[itBeamletAllocation->first] = allocationSet;
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
      }
    }
    if(allocationOk)
    {
      newAllocationDetails[*itStation] = beamletAllocationMap;
      newAllocationBeamlets[*itStation] = beamletVector;
    }
  }  
  
  if(!allocationOk)
  {
    newAllocationDetails.clear();
    newAllocationBeamlets.clear();
    suspendVIs.clear();
  }
  return allocationOk;
}

bool BeamletAllocator::_mergeAllocation(TStation2AllocationMap& allocation, TStation2AllocationMap& newAllocationDetails)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");

  // merge the new allocation with the existing allocation
  
  bool allocationOk = true;

  for(TStation2AllocationMap::iterator itNewStationAllocation = newAllocationDetails.begin();itNewStationAllocation != newAllocationDetails.end();++itNewStationAllocation)
  {
    TStation2AllocationMap::iterator itStationAllocation    = allocation.find(itNewStationAllocation->first);
    if(itStationAllocation == allocation.end())
    {
      itStationAllocation = _addStationAllocation(allocation,itNewStationAllocation->first);
    }    
    if(itStationAllocation == allocation.end())
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
          itStationAllocation->second.insert(TBeamlet2AllocationMap::value_type(itNewBeamletAllocation->first,itNewBeamletAllocation->second));
          LOG_WARN(formatString("Added beamlet %d for station %s",itNewBeamletAllocation->first,itNewStationAllocation->first.c_str()));
        }
        else
        {
          // merge the new allocation into the existing
          itBeamletAllocation->second.insert(itNewBeamletAllocation->second.begin(),
                                             itNewBeamletAllocation->second.end());
          ++itNewBeamletAllocation;
        }
      }
    }
  }
  return allocationOk;
}

void BeamletAllocator::_extractAllocation(const string& vi, TStation2AllocationMap& source, TStation2AllocationMap& allocationDetails)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");

  // Extract an allocation for a VI from the allocation administration.
  // The extracted allocation details are returned to the caller
  
  allocationDetails.clear();
  TStation2AllocationMap::iterator itStationAllocation;
  for(itStationAllocation = source.begin();itStationAllocation != source.end();++itStationAllocation)
  {
    TBeamlet2AllocationMap  beamletAllocationMap;
    TBeamlet2AllocationMap::iterator itBeamletAllocation;
    for(itBeamletAllocation = itStationAllocation->second.begin();itBeamletAllocation != itStationAllocation->second.end();++itBeamletAllocation)
    {
      TAllocationInfoSet::iterator itAllocationInfo = itBeamletAllocation->second.begin();
      while(itAllocationInfo != itBeamletAllocation->second.end())
      {
        if(itAllocationInfo->vi == vi)
        {
          TAllocationInfo allocation(*itAllocationInfo);
          
          TAllocationInfoSet allocationSet;
          allocationSet.insert(allocation);
          beamletAllocationMap[itBeamletAllocation->first] = allocationSet;
          
          TAllocationInfoSet::iterator eraseIt = itAllocationInfo++;
          itBeamletAllocation->second.erase(eraseIt);
        }
        else
        {
          ++itAllocationInfo;
        }
      }
    }
    if(beamletAllocationMap.size()>0)
    {
      LOG_TRACE_FLOW(formatString("_extractAllocation: saving %s allocation for %s",vi.c_str(),itStationAllocation->first.c_str()));
      allocationDetails[itStationAllocation->first] = beamletAllocationMap;
    }
  }
}

void BeamletAllocator::_deallocateBeamlets(const string& vi,
                                          TStation2AllocationMap& deallocatedInfo,
                                          map<string, TStationBeamletAllocation>& resumeVIs,
                                          set<string>& suspendVIs)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  LOG_TRACE_VAR(formatString("_deallocateBeamlets(%s): deallocatedInfo size:%d",vi.c_str(),deallocatedInfo.size()));
  deallocatedInfo.clear();
  suspendVIs.clear();
  resumeVIs.clear();
  
  // Deallocate a VI by removing the current allocation
  LOG_TRACE_FLOW(formatString("_deallocateBeamlets(%s): Remove %s allocation",vi.c_str(),vi.c_str()));
  _extractAllocation(vi,m_allocation,deallocatedInfo);
  LOG_TRACE_VAR(formatString("_deallocateBeamlets(%s): deallocatedInfo size:%d",vi.c_str(),deallocatedInfo.size()));
  m_resumedVIs.erase(vi);
  m_suspendedVIs.erase(vi);
  
  // now try to resume the VI's that are suspended
  LOG_TRACE_FLOW(formatString("_deallocateBeamlets(%s): Resume the VI's that are suspended",vi.c_str()));
  TStation2AllocationMap currentSuspendedAllocation=m_suspendedAllocation;
  m_suspendedAllocation.clear();
  set<string> currentSuspendedVIs(m_suspendedVIs);

  set<string>::iterator suspendedVIit=currentSuspendedVIs.begin();
  while(suspendedVIit!=currentSuspendedVIs.end())
  {
    TStation2AllocationMap suspendedAllocationDetails;
  
    string suspendVI(*suspendedVIit);
    // get an allocation of a suspended VI. The Station2Allocation map contains
    // allocations of stations, so the first thing to do is to gather all allocations
    // of one VI
    _extractAllocation(suspendVI,currentSuspendedAllocation,suspendedAllocationDetails);
    if(suspendedAllocationDetails.size() > 0)
    {
      uint16                    priority;
      vector<string>            stations;
      time_t                    startTime;
      time_t                    stopTime;
      vector<int16>             subbands;
      TStationBeamletAllocation allocation;
      map<string, TStationBeamletAllocation> tempResumeVIs;
      set<string>               tempSuspendVIs;
      
      // collect allocation information. 
      TStation2AllocationMap::iterator itStationAllocation;
      for(itStationAllocation = suspendedAllocationDetails.begin();itStationAllocation != suspendedAllocationDetails.end();++itStationAllocation)
      {
        stations.push_back(itStationAllocation->first);

        // For all stations, the priorities, start/stop times    
        // and subbands are the same, so it is sufficient to get that information
        // for the first station only
        if(itStationAllocation == suspendedAllocationDetails.begin())
        {
          TBeamlet2AllocationMap  beamletAllocationMap;
          TBeamlet2AllocationMap::iterator itBeamletAllocation;
          for(itBeamletAllocation = itStationAllocation->second.begin();itBeamletAllocation != itStationAllocation->second.end();++itBeamletAllocation)
          {
            TAllocationInfoSet::iterator itAllocationInfo;
            for(itAllocationInfo = itBeamletAllocation->second.begin();itAllocationInfo != itBeamletAllocation->second.end(); ++itAllocationInfo)
            {
              priority  = itAllocationInfo->priority;
              startTime = itAllocationInfo->startTime;
              stopTime  = itAllocationInfo->stopTime;
              subbands.push_back(itAllocationInfo->subband);
            }
          }
        }
      }
      
      // try to reallocate:
      if(allocateBeamlets(suspendVI,priority,stations,startTime,stopTime,subbands,allocation,tempResumeVIs,tempSuspendVIs))
      {
        m_suspendedVIs.erase(suspendVI);
        suspendVIs.insert(tempSuspendVIs.begin(),tempSuspendVIs.end());
        resumeVIs.insert(tempResumeVIs.begin(),tempResumeVIs.end());
      }
      else
      {
        // allocation unsuccessful. reinstall the suspended allocation.
        m_suspendedAllocation.insert(suspendedAllocationDetails.begin(),suspendedAllocationDetails.end());
      }
    }
    // try the next suspended VI
    currentSuspendedVIs.erase(suspendedVIit);
    suspendedVIit = currentSuspendedVIs.begin();
  }
  LOG_TRACE_VAR(formatString("_deallocateBeamlets(%s): deallocatedInfo size:%d",vi.c_str(),deallocatedInfo.size()));
}

void BeamletAllocator::_logAllocation(TStation2AllocationMap& allocation, const string& title, bool groupByVI)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"BeamletAllocator");
  
  map<string,string> logs;
  map<string,string>::iterator logsIterator;
  TStation2AllocationMap::iterator itStationAllocation;
  
  for(itStationAllocation = allocation.begin();itStationAllocation != allocation.end();++itStationAllocation)
  {
    TBeamlet2AllocationMap::iterator itBeamletAllocation;
    for(itBeamletAllocation = itStationAllocation->second.begin();itBeamletAllocation != itStationAllocation->second.end();++itBeamletAllocation)
    {
      TAllocationInfoSet::iterator itAllocationInfo;
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
          logStream << itStationAllocation->first << ",";  // station
        }
        else
        {
          logStream << itStationAllocation->first << ",";  // station
          logStream << itAllocationInfo->vi << ",";
        }
        logStream << itAllocationInfo->priority << ",";
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
  
  string logString("Allocation of ");
  logString += title;
  logString += string(":\n");
  if(groupByVI)
  {
    logString += "VI, Station, priority, beamlet->subband, starttime, stoptime\n";
  }
  else
  {
    logString += "Station, VI, priority, beamlet->subband, starttime, stoptime\n";
  }
  for(logsIterator=logs.begin();logsIterator!=logs.end();++logsIterator)
  {
    logString+=logsIterator->second;
  }
  LOG_DEBUG(logString.c_str());
}

};
};
