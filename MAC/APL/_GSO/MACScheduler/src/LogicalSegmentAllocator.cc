//#  LogicalSegmentAllocator.cc: Implementation of the LogicalSegmentAllocator
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

#include "LogicalSegmentAllocator.h"

INIT_TRACER_CONTEXT(LOFAR::GSO::LogicalSegmentAllocator,LOFARLOGGER_PACKAGE);

using namespace LOFAR::APLCommon;

namespace LOFAR
{

namespace GSO
{

LogicalSegmentAllocator::LogicalSegmentAllocator() :
  m_allocation(),
  m_suspendedAllocation(),
  m_resumedVRs(),
  m_suspendedVRs()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
}

LogicalSegmentAllocator::~LogicalSegmentAllocator()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
}

bool LogicalSegmentAllocator::allocateVirtualRoute(
  const string&                   vr,
  const TLogicalSegmentBandwidth& lsCapacities,
  const vector<string>&           logicalSegments,
  const uint16                    priority,
  const time_t                    startTime, 
  const time_t                    stopTime, 
  const double                    requiredBandwidth, 
  set<string>&                    resumeVRs,
  set<string>&                    suspendVRs)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  
  // First the current allocation of a VR is removed.
  // The the new allocation is tested and added. 
  // Finally, if the new allocation does not fit, 
  // the extracted allocated is re-inserted

  suspendVRs.clear();
  resumeVRs.clear();
  bool allocationOk(false);
  
  TLogicalSegment2AllocationMap oldAllocationDetails;
  TLogicalSegment2AllocationMap newAllocationDetails;

  // remove existing allocation
  LOG_TRACE_FLOW("allocateVirtualRoute: Remove existing allocation");
  _extractAllocation(vr,m_allocation,oldAllocationDetails);
  
  // test if it fits
  LOG_TRACE_FLOW("allocateVirtualRoute: Test if the new allocation fits");
  allocationOk = _testAllocateLogicalSegments(vr,lsCapacities,logicalSegments,priority,startTime,stopTime,requiredBandwidth,newAllocationDetails,suspendVRs);
  if(allocationOk)
  {
    // merge the new allocation
    LOG_TRACE_FLOW("allocateVirtualRoute: Merge the new allocation");
    allocationOk = _mergeAllocation(m_allocation,newAllocationDetails);

    // deallocate the suspended VR's    
    LOG_TRACE_FLOW("allocateVirtualRoute: Deallocate the suspended VR's");
    set<string> deAllocateSuspendVRs=suspendVRs;
    for(set<string>::const_iterator it=deAllocateSuspendVRs.begin();it!=deAllocateSuspendVRs.end();++it)
    {
      set<string> tempResumeVRs;
      set<string> tempSuspendVRs;
      TLogicalSegment2AllocationMap deallocationDetails;
      _deallocateVirtualRoute(*it, lsCapacities, deallocationDetails, tempResumeVRs, tempSuspendVRs);
      // save the allocation of the suspended VR's. They can be resumed lateron
      LOG_TRACE_FLOW("allocateVirtualRoute: Save the allocation state of the suspended VR's");
      _mergeAllocation(m_suspendedAllocation,deallocationDetails);

      resumeVRs.insert(tempResumeVRs.begin(),tempResumeVRs.end());
      suspendVRs.insert(tempSuspendVRs.begin(),tempSuspendVRs.end());
    }
    
    // save the state of the VR's for later (mis)use
    m_resumedVRs.insert(resumeVRs.begin(),resumeVRs.end());
    m_suspendedVRs.insert(suspendVRs.begin(),suspendVRs.end());
  }
  else
  {
    // restore the old allocation and nothing changes.
    LOG_TRACE_FLOW("allocateVirtualRoute: Restore the old allocation");
    _mergeAllocation(m_allocation,oldAllocationDetails);
  }
  return allocationOk;
}

void LogicalSegmentAllocator::deallocateVirtualRoute(
  const string&                   vr,
  const TLogicalSegmentBandwidth& lsCapacities, // for reallocating other VR's
  set<string>&                    resumeVRs,
  set<string>&                    suspendVRs)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  resumeVRs.clear();
  suspendVRs.clear();
  
  TLogicalSegment2AllocationMap deallocatedInfo; // ignore this
  _deallocateVirtualRoute(vr, lsCapacities, deallocatedInfo, resumeVRs, suspendVRs);
}

void LogicalSegmentAllocator::logAllocation(bool groupByVR)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  _logAllocation(m_allocation,string("Active VR's"),groupByVR);
}
  
void LogicalSegmentAllocator::logSuspendedAllocation(bool groupByVR)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  _logAllocation(m_suspendedAllocation,string("Suspended VR's"),groupByVR);
}
  
LogicalSegmentAllocator::TLogicalSegment2AllocationMap::iterator LogicalSegmentAllocator::_addLogicalSegmentAllocation(
  TLogicalSegment2AllocationMap& allocation, 
  const string& logicalSegment)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  
  // add empty allocation 
  TAllocation emptyAllocation;
  pair<TLogicalSegment2AllocationMap::iterator,bool> inserted;
  inserted = allocation.insert(TLogicalSegment2AllocationMap::value_type(logicalSegment,emptyAllocation));
  return inserted.first;
}

bool LogicalSegmentAllocator::_testAllocateLogicalSegments(
  const string&                   vr,
  const TLogicalSegmentBandwidth& lsCapacities, 
  const vector<string>&           logicalSegments,
  const uint16                    priority, 
  const time_t                    startTime, 
  const time_t                    stopTime, 
  const double                    requiredBandwidth,
  TLogicalSegment2AllocationMap&  newAllocationDetails, 
  set<string>&                    suspendVRs)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  
  time_t timeNow = APLUtilities::getUTCtime();
  bool allocationOk(true);

  newAllocationDetails.clear();
  suspendVRs.clear();

  // test the allocation for each logical segment
  vector<string>::const_iterator itLS;
  for(itLS = logicalSegments.begin();allocationOk && itLS != logicalSegments.end(); ++itLS)
  {
    TLogicalSegment2AllocationMap::iterator itLSAllocation = m_allocation.find(*itLS);
    if(itLSAllocation == m_allocation.end())
    {
      LOG_TRACE_FLOW(formatString("_testAllocateLogicalSegments: Adding logical segment %s to allocation data",itLS->c_str()));
      itLSAllocation = _addLogicalSegmentAllocation(m_allocation,*itLS);
    }
    if(itLSAllocation == m_allocation.end())
    {
      allocationOk = false;
      LOG_FATAL(formatString("Allocation for Logical Segment %s, VR %s failed",itLS->c_str(),vr.c_str()));
    }
    else
    {
      // LogicalSegment found in allocation administration. 
      // Check its current allocation without looking at priorities
      bool roomForAllocation(true);
      TLogicalSegmentBandwidth::const_iterator lsFreeIt=lsCapacities.find(*itLS);
      if(lsFreeIt != lsCapacities.end())
      {
        // first: check if the LS has enough capacity
        roomForAllocation = (lsFreeIt->second >= requiredBandwidth);
        bool exitLoop(false);
        TBandwidthUsageMap::iterator itBWusage1=itLSAllocation->second.bandwidthUsage.begin();
        TBandwidthUsageMap::iterator itBWusage2;
        while(!exitLoop && roomForAllocation && itBWusage1 != itLSAllocation->second.bandwidthUsage.end())
        {
          itBWusage2 = itBWusage1;
          ++itBWusage2;
          if(itBWusage2 != itLSAllocation->second.bandwidthUsage.end())
          {
            if(itBWusage1->first > stopTime)
            {
              exitLoop=true;
            }
            else if(itBWusage2->first >= startTime)
            {
              double bwInUse(itBWusage1->second);
              // capacity - usage = allocatable bandwidth
              roomForAllocation = (lsFreeIt->second - bwInUse >= requiredBandwidth); // there must be enough bandwidth

              LOG_TRACE_FLOW(formatString("_testAllocateLogicalSegments: bandwidth usage = capacity-usage = %f-%f = %f. Required: %f",
                lsFreeIt->second, bwInUse, lsFreeIt->second-bwInUse, requiredBandwidth));
            }
            // garbage collection on the fly: remove allocations with stoptimes before now
            if(itBWusage2->first < timeNow)
            {
              TBandwidthUsageMap::iterator eraseIt = itBWusage1++;
              LOG_TRACE_FLOW(formatString("_testAllocateLogicalSegments: removing bandwidth usage point %d -> %f",eraseIt->first,eraseIt->second));
              itLSAllocation->second.bandwidthUsage.erase(eraseIt);
            }
            else
            {
              ++itBWusage1;
            }
          }
          else
          {
            exitLoop=true;
          }
        }
      }
      TAllocationInfoSet::iterator itAllocationInfo = itLSAllocation->second.infoSet.begin();
      while(itAllocationInfo != itLSAllocation->second.infoSet.end())
      {
        // garbage collection on the fly: remove allocations with stoptimes before now
        if((*itAllocationInfo).stopTime < timeNow)
        {
          TAllocationInfoSet::iterator eraseIt = itAllocationInfo++;
          itLSAllocation->second.infoSet.erase(eraseIt);
        }
        else
        {
          ++itAllocationInfo;
        }
      }
      if(roomForAllocation)
      {
        // Yes, this LogicalSegment is free for allocation
        LOG_TRACE_FLOW(formatString("_testAllocateLogicalSegments: allocation on LogicalSegment %s for VR %s succeeded: %f,%d,%d,%d",itLS->c_str(),vr.c_str(),requiredBandwidth,priority,startTime,stopTime));
        TAllocationInfo newAllocationInfo;
        newAllocationInfo.vr                = vr;
        newAllocationInfo.requiredBandwidth = requiredBandwidth;
        newAllocationInfo.priority          = priority;
        newAllocationInfo.startTime         = startTime;
        newAllocationInfo.stopTime          = stopTime;
      
        TAllocationInfoSet allocationSet;
        allocationSet.insert(newAllocationInfo);
        TAllocation newAllocation(allocationSet);
        newAllocationDetails[*itLS] = newAllocation;
      }
      else
      {
        LOG_DEBUG("No room without stopping lower priority VR's");

        // no room without stopping lower priority VR's
        // the list of allocations is sorted by startTime
        // search the first allocation that has: 
        // 1. requestedStarttime >= starttime
        // 2. requestedStoptime <= stoptime
        // 3. priority>requestedPriority
        // 4. requiredBandwidth >= requestedBandwidth
      
        // the above means that a simple policy is used to suspend lower priority LD's:
        // the lower priority allocation is replaced only if the timespan of the 
        // higher priority allocation totally fits in the lower priority allocation.
        
        string suspendVR;
        TAllocationInfoSet::iterator itAllocationInfo = itLSAllocation->second.infoSet.begin();
        while(!roomForAllocation && itAllocationInfo != itLSAllocation->second.infoSet.end())
        {
          LOG_TRACE_FLOW(formatString("_testAllocateLogicalSegments: Checking priority and times:\nbandwidth:\t%02f <= %02f = %s\npriority:\t%d < %d = %s\nstarttime\t%d >= %d = %s\nstoptime\t%d <= %d = %s",
            requiredBandwidth,(*itAllocationInfo).requiredBandwidth,(requiredBandwidth<=(*itAllocationInfo).requiredBandwidth?"TRUE":"FALSE"),
            priority,(*itAllocationInfo).priority,(priority<(*itAllocationInfo).priority?"TRUE":"FALSE"),
            startTime,(*itAllocationInfo).startTime,(startTime >= (*itAllocationInfo).startTime?"TRUE":"FALSE"),
            stopTime,(*itAllocationInfo).stopTime,(stopTime <= (*itAllocationInfo).stopTime?"TRUE":"FALSE")));
          if(requiredBandwidth <= (*itAllocationInfo).requiredBandwidth &&
             priority < (*itAllocationInfo).priority &&
             startTime >= (*itAllocationInfo).startTime &&
             stopTime <= (*itAllocationInfo).stopTime)
          {
            roomForAllocation=true;
            suspendVR = (*itAllocationInfo).vr;
          }
            
          ++itAllocationInfo;
        }
        if(roomForAllocation)
        {
          // store the VR that's in the way
          if(suspendVRs.find(suspendVR) == suspendVRs.end())
          {
            LOG_DEBUG(formatString("Suspending VR:%s",suspendVR.c_str()));
            suspendVRs.insert(suspendVR);
          }
          // Yes, this LogicalSegment has room for allocation
          LOG_TRACE_FLOW(formatString("_testAllocateLogicalSegments: allocation on LogicalSegment %s for VR %s succeeded: %f,%d,%d,%d",itLS->c_str(),vr.c_str(),requiredBandwidth,priority,startTime,stopTime));
          TAllocationInfo newAllocationInfo;
          newAllocationInfo.vr        = vr;
          newAllocationInfo.requiredBandwidth = requiredBandwidth;
          newAllocationInfo.priority  = priority;
          newAllocationInfo.startTime = startTime;
          newAllocationInfo.stopTime  = stopTime;
          
          TAllocationInfoSet allocationSet;
          allocationSet.insert(newAllocationInfo);
          TAllocation newAllocation(allocationSet);
          newAllocationDetails[*itLS] = newAllocation;
        }
        else
        {
          allocationOk=false;
          LOG_FATAL(formatString("Allocation for Logical Segment %s, VR %s failed",itLS->c_str(),vr.c_str()));
        }
      }
    }
  }  
  
  if(!allocationOk)
  {
    newAllocationDetails.clear();
    suspendVRs.clear();
  }
  return allocationOk;
}

bool LogicalSegmentAllocator::_mergeAllocation(
  TLogicalSegment2AllocationMap& allocation, 
  TLogicalSegment2AllocationMap& newAllocationDetails)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");

  // merge the new allocation with the existing allocation
  bool allocationOk = true;

  for(TLogicalSegment2AllocationMap::iterator itNewLSAllocation = newAllocationDetails.begin();itNewLSAllocation != newAllocationDetails.end();++itNewLSAllocation)
  {
    TLogicalSegment2AllocationMap::iterator itLSAllocation = allocation.find(itNewLSAllocation->first);
    if(itLSAllocation == allocation.end())
    {
      itLSAllocation = _addLogicalSegmentAllocation(allocation,itNewLSAllocation->first);
    }    
    if(itLSAllocation == allocation.end())
    {
      allocationOk = false;
      LOG_FATAL(formatString("Bandwidth allocation for Logical Segment %s failed",itNewLSAllocation->first.c_str()));
    }
    else
    {
      if(itNewLSAllocation->second.infoSet.size() > 0)
      {
        // merge the new allocation into the existing
        TAllocationInfoSet mergedSet;
        std::merge(itLSAllocation->second.infoSet.begin(),
                   itLSAllocation->second.infoSet.end(),
                   itNewLSAllocation->second.infoSet.begin(),
                   itNewLSAllocation->second.infoSet.end(),
                   inserter(mergedSet, mergedSet.begin()),earlierStart());
        
        itLSAllocation->second.infoSet = mergedSet;                   
        // and add the bandwidth usage
        _addBandwidthUsage(itLSAllocation->second.bandwidthUsage,*itNewLSAllocation->second.infoSet.begin());
      }
    }
  }
  return allocationOk;
}

void LogicalSegmentAllocator::_extractAllocation(
  const string& vr, 
  TLogicalSegment2AllocationMap& source, 
  TLogicalSegment2AllocationMap& allocationDetails)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");

  // Extract an allocation for a VR from the allocation administration.
  // The extracted allocation details are returned to the caller
  
  allocationDetails.clear();
  TLogicalSegment2AllocationMap::iterator itLSAllocation;
  for(itLSAllocation = source.begin();itLSAllocation != source.end();++itLSAllocation)
  {
    TAllocationInfoSet::iterator itAllocationInfo = itLSAllocation->second.infoSet.begin();
    while(itAllocationInfo != itLSAllocation->second.infoSet.end())
    {
      if((*itAllocationInfo).vr == vr)
      {
        TAllocationInfo allocationInfo(*itAllocationInfo); // create a copy
        TAllocationInfoSet allocationSet;
        allocationSet.insert(allocationInfo);
        TAllocation allocation(allocationSet);
        LOG_TRACE_FLOW(formatString("_extractAllocation: saving %s allocation for %s",vr.c_str(),itLSAllocation->first.c_str()));
        allocationDetails[itLSAllocation->first] = allocation;
        
        _removeBandwidthUsage(itLSAllocation->second.bandwidthUsage,allocationInfo);

        TAllocationInfoSet::iterator eraseIt = itAllocationInfo++;
        itLSAllocation->second.infoSet.erase(eraseIt);
      }
      else
      {
        ++itAllocationInfo;
      }
    }
  }
}

//
// Deallocation of Logical Segments. 
// Important side effect: If a high priority LS is deallocated, some suspended lower priority
// LS's may be reallocated.
//
void LogicalSegmentAllocator::_deallocateVirtualRoute(
  const string&                   vr,
  const TLogicalSegmentBandwidth& lsCapacities, // for reallocating other VR's
  TLogicalSegment2AllocationMap&  deallocatedInfo,
  set<string>&                    resumeVRs,
  set<string>&                    suspendVRs)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  
  LOG_TRACE_VAR(formatString("_deallocateVirtualRoute(%s): deallocatedInfo size:%d",vr.c_str(),deallocatedInfo.size()));
  deallocatedInfo.clear();
  suspendVRs.clear();
  resumeVRs.clear();
  
  // Deallocate a VR by removing the current allocation
  LOG_TRACE_FLOW(formatString("_deallocateVirtualRoute(%s): Remove %s allocation",vr.c_str(),vr.c_str()));
  _extractAllocation(vr,m_allocation,deallocatedInfo);
  LOG_TRACE_VAR(formatString("_deallocateVirtualRoute(%s): deallocatedInfo size:%d",vr.c_str(),deallocatedInfo.size()));
  m_resumedVRs.erase(vr);
  m_suspendedVRs.erase(vr);
  
  // now try to reallocate the VR's that are suspended
  LOG_TRACE_FLOW(formatString("_deallocateVirtualRoute(%s): Re-allocating the VR's that are suspended",vr.c_str()));
  TLogicalSegment2AllocationMap currentSuspendedAllocation=m_suspendedAllocation;
  m_suspendedAllocation.clear();
  set<string> currentSuspendedVRs(m_suspendedVRs);

  set<string>::iterator suspendedVRit=currentSuspendedVRs.begin();
  while(suspendedVRit!=currentSuspendedVRs.end())
  {
    TLogicalSegment2AllocationMap suspendedAllocationDetails;
  
    string suspendVR(*suspendedVRit);
    // get an allocation of a suspended VR. The LS2Allocation map contains
    // allocations of LogicalSegments, so the first thing to do is to gather all allocations
    // of one VR
    _extractAllocation(suspendVR,currentSuspendedAllocation,suspendedAllocationDetails);
    if(suspendedAllocationDetails.size() > 0)
    {
      uint16          priority;
      vector<string>  logicalSegments;
      time_t          startTime;
      time_t          stopTime;
      double          requiredBandwidth;
      set<string>     tempResumeVRs;
      set<string>     tempSuspendVRs;
      
      // collect allocation information. 
      TLogicalSegment2AllocationMap::iterator itLSAllocation;
      for(itLSAllocation = suspendedAllocationDetails.begin();itLSAllocation != suspendedAllocationDetails.end();++itLSAllocation)
      {
        logicalSegments.push_back(itLSAllocation->first);

        // For all logical segments, the priorities, start/stop times    
        // and requiredbandwidth are the same, so it is sufficient to get that information
        // for the first station only
        if(itLSAllocation == suspendedAllocationDetails.begin())
        {
          TAllocationInfoSet::iterator itAllocationInfo = itLSAllocation->second.infoSet.begin();
          if(itAllocationInfo != itLSAllocation->second.infoSet.end())
          {
            requiredBandwidth = (*itAllocationInfo).requiredBandwidth;
            priority  = (*itAllocationInfo).priority;
            startTime = (*itAllocationInfo).startTime;
            stopTime  = (*itAllocationInfo).stopTime;
          }
        }
      }
      
      // try to reallocate
      LOG_INFO(formatString("Trying to reallocate VR %s",suspendVR.c_str()));
      if(allocateVirtualRoute(suspendVR, lsCapacities, logicalSegments, priority, startTime, stopTime, requiredBandwidth, tempResumeVRs, tempSuspendVRs))
      {
        m_suspendedVRs.erase(suspendVR);
        suspendVRs.insert(tempSuspendVRs.begin(),tempSuspendVRs.end());
        resumeVRs.insert(tempResumeVRs.begin(),tempResumeVRs.end());
      }
      else
      {
        // allocation unsuccessful. reinstall the suspended allocation.
        _mergeAllocation(m_suspendedAllocation,suspendedAllocationDetails);
      }
    }
    // try the next suspended VR
    currentSuspendedVRs.erase(suspendedVRit);
    suspendedVRit = currentSuspendedVRs.begin();
  }
  LOG_TRACE_VAR(formatString("_deallocateVirtualRoute(%s): deallocatedInfo size:%d",vr.c_str(),deallocatedInfo.size()));
}

void LogicalSegmentAllocator::_addBandwidthUsage(TBandwidthUsageMap& bandwidthUsage, const TAllocationInfo& allocationInfo)
{
  // add bandwidth usage
  TBandwidthUsageMap::iterator startIt;
  TBandwidthUsageMap::iterator stopIt;
  double currentUsage(0.0);

  // find existing allocation at the start-time, or create a new entry
  LOG_TRACE_VAR(formatString("_addBandwidthUsage: usage map size:%d",bandwidthUsage.size()));
  startIt = bandwidthUsage.find(allocationInfo.startTime);
  if(startIt == bandwidthUsage.end())
  {
    pair<TBandwidthUsageMap::iterator,bool> insertIt = bandwidthUsage.insert(TBandwidthUsageMap::value_type(allocationInfo.startTime,0.0));
    if(insertIt.second)
    {
      startIt = insertIt.first;
      if(startIt != bandwidthUsage.begin())
      {
        // calc current usage of the resource
        TBandwidthUsageMap::iterator prevIt=startIt;
        --prevIt;
        currentUsage = prevIt->second;
      }
      LOG_TRACE_VAR(formatString("_addBandwidthUsage: inserted usage startpoint:%d -> %f",startIt->first,startIt->second));
    }
  }
  else
  {
    // add to existing usage value
    currentUsage = startIt->second;
  }
  if(startIt != bandwidthUsage.end())
  {
    // find existing allocation at the stop-time, or create a new entry
    stopIt = bandwidthUsage.find(allocationInfo.stopTime);
    if(stopIt == bandwidthUsage.end())
    {
      pair<TBandwidthUsageMap::iterator,bool> insertIt = bandwidthUsage.insert(TBandwidthUsageMap::value_type(allocationInfo.stopTime,0.0));
      if(insertIt.second)
      {
        stopIt = insertIt.first;
        // fill this new stop entry with the value of the previous in the list (if any)
        TBandwidthUsageMap::iterator prevIt=stopIt;
        --prevIt;
        if(prevIt != bandwidthUsage.end())
        {
          stopIt->second = prevIt->second;
        }
        LOG_TRACE_VAR(formatString("_addBandwidthUsage: inserted usage endpoint:%d -> %f",stopIt->first,stopIt->second));
      }
    }
    if(stopIt != bandwidthUsage.end())
    {
      bool exitLoop(false);
      TBandwidthUsageMap::iterator loopIt=startIt;
      while(!exitLoop)
      {
        if(loopIt!=stopIt)
        {
          loopIt->second = currentUsage + allocationInfo.requiredBandwidth;
          LOG_TRACE_VAR(formatString("_addBandwidthUsage: updated usage point:%d -> %f",loopIt->first,loopIt->second));
          ++loopIt;
          if(loopIt!= bandwidthUsage.end())
          {
            currentUsage = loopIt->second;
          }
        }
        else
        {
          // stoptime: set old usage value
          exitLoop = true;
          loopIt->second = currentUsage;
        }
      }
    }
  }
}

void LogicalSegmentAllocator::_removeBandwidthUsage(TBandwidthUsageMap& bandwidthUsage, const TAllocationInfo& allocationInfo)
{
  // subtract bandwidth usage
  bool exitLoop(false);
  time_t timeNow = APLUtilities::getUTCtime();
  TBandwidthUsageMap::iterator bwUsageIt=bandwidthUsage.find(allocationInfo.startTime);
  while(!exitLoop && bwUsageIt!=bandwidthUsage.end())
  {
    // adjust all usage figures between the start and stop times.
    if(bwUsageIt->first >= allocationInfo.stopTime)
    {
      exitLoop=true;
    }
    else if(bwUsageIt->first >= allocationInfo.startTime)
    {
      if(bwUsageIt->second < allocationInfo.requiredBandwidth)
      {
        // erase only if it is information of the past.
        if(bwUsageIt->first < timeNow)
        {
          LOG_TRACE_VAR(formatString("_removeBandwidthUsage: removing historical usage point:%d -> %f",bwUsageIt->first,bwUsageIt->second));
          TBandwidthUsageMap::iterator bwEraseIt=bwUsageIt++;
          bandwidthUsage.erase(bwEraseIt);
        }
        else
        {
          bwUsageIt->second = 0.0;
          LOG_TRACE_VAR(formatString("_removeBandwidthUsage: updated usage point:%d -> %f",bwUsageIt->first,bwUsageIt->second));
          ++bwUsageIt;
        }
      }
      else
      {
        bwUsageIt->second -= allocationInfo.requiredBandwidth;
        LOG_TRACE_VAR(formatString("_removeBandwidthUsage: updated usage point:%d -> %f",bwUsageIt->first,bwUsageIt->second));
        ++bwUsageIt;
      }
    }
  }
}

void LogicalSegmentAllocator::_logAllocation(
  TLogicalSegment2AllocationMap& allocation, 
  const string& title, 
  bool groupByVR)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  
  _logAllocationInfo(allocation,title,groupByVR);
  _logBandwidthUsage(allocation,title);
}

void LogicalSegmentAllocator::_logAllocationInfo(
  TLogicalSegment2AllocationMap& allocation, 
  const string& title, 
  bool groupByVR)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  map<string,string> logs;
  map<string,string>::iterator logsIterator;
  TLogicalSegment2AllocationMap::iterator itLSAllocation;
  
  for(itLSAllocation = allocation.begin();itLSAllocation != allocation.end();++itLSAllocation)
  {
    TAllocationInfoSet::iterator itAllocationInfo;
    for(itAllocationInfo = itLSAllocation->second.infoSet.begin();itAllocationInfo != itLSAllocation->second.infoSet.end();++itAllocationInfo)
    {
      tm* pStartTm = localtime(&(*itAllocationInfo).startTime);
      char startTimeStr[100];
      sprintf(startTimeStr,"%02d-%02d-%04d %02d:%02d:%02d",pStartTm->tm_mday,pStartTm->tm_mon+1,pStartTm->tm_year+1900,pStartTm->tm_hour,pStartTm->tm_min,pStartTm->tm_sec);
      
      tm* pStopTm = localtime(&(*itAllocationInfo).stopTime);
      char stopTimeStr[100];
      sprintf(stopTimeStr,"%02d-%02d-%04d %02d:%02d:%02d",pStopTm->tm_mday,pStopTm->tm_mon+1,pStopTm->tm_year+1900,pStopTm->tm_hour,pStopTm->tm_min,pStopTm->tm_sec);

      stringstream logStream;
      
      if(groupByVR)
      {
        logStream << (*itAllocationInfo).vr << ",";
        logStream << itLSAllocation->first << ",";  // LogicalSegment
      }
      else
      {
        logStream << itLSAllocation->first << ",";  // LogicalSegment
        logStream << (*itAllocationInfo).vr << ",";
      }
      logStream << (*itAllocationInfo).requiredBandwidth << ",";
      logStream << (*itAllocationInfo).priority << ",";
      logStream << startTimeStr << ",";
      logStream << stopTimeStr << endl;
      
      string log;
      string key = itLSAllocation->first;
      if(groupByVR)
      {
        key = (*itAllocationInfo).vr;
      }
      logsIterator = logs.find(key);
      if(logsIterator != logs.end())
      {
        log = logsIterator->second;
      }
      logs[key] = log+logStream.str();
    }
  }
  
  string logString("Allocation of ");
  logString += title;
  logString += string(":\n");
  if(groupByVR)
  {
    logString += "VR, LogSegment, Req.Bandwidth, priority, starttime, stoptime\n";
  }
  else
  {
    logString += "LogSegment, VR, Req.Bandwidth, priority, starttime, stoptime\n";
  }
  for(logsIterator=logs.begin();logsIterator!=logs.end();++logsIterator)
  {
    logString+=logsIterator->second;
  }
  LOG_DEBUG(logString.c_str());
}

void LogicalSegmentAllocator::_logBandwidthUsage(
  TLogicalSegment2AllocationMap& allocation, 
  const string& title)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalSegmentAllocator");
  map<string,string> logs;
  map<string,string>::iterator logsIterator;
  TLogicalSegment2AllocationMap::iterator itLSAllocation;
  
  for(itLSAllocation = allocation.begin();itLSAllocation != allocation.end();++itLSAllocation)
  {
    TBandwidthUsageMap::iterator itAllocation;
    for(itAllocation = itLSAllocation->second.bandwidthUsage.begin();itAllocation != itLSAllocation->second.bandwidthUsage.end();++itAllocation)
    {
      tm* pTm = localtime(&itAllocation->first);
      char timeStr[100];
      sprintf(timeStr,"%02d-%02d-%04d %02d:%02d:%02d",pTm->tm_mday,pTm->tm_mon+1,pTm->tm_year+1900,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);
      
      stringstream logStream;
      
      logStream << itLSAllocation->first << ",";  // LogicalSegment
      logStream << timeStr << ",";
      logStream << itAllocation->second << endl;
      
      string log;
      string key = itLSAllocation->first;
      logsIterator = logs.find(key);
      if(logsIterator != logs.end())
      {
        log = logsIterator->second;
      }
      logs[key] = log+logStream.str();
    }
  }
  
  string logString("Bandwidth usage of ");
  logString += title;
  logString += string(":\n");
  logString += "LogSegment, time, Bandwidth usage\n";
  for(logsIterator=logs.begin();logsIterator!=logs.end();++logsIterator)
  {
    logString+=logsIterator->second;
  }
  LOG_DEBUG(logString.c_str());
}

};
};
