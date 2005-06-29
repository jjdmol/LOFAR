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
#include <APLCommon/APLUtilities.h>

#include "BeamletAllocator.h"

INIT_TRACER_CONTEXT(LOFAR::GSO::BeamletAllocator,LOFARLOGGER_PACKAGE);

using namespace LOFAR::GCF::Common;
using namespace LOFAR::APLCommon;

namespace LOFAR
{

namespace GSO
{

BeamletAllocator::BeamletAllocator() :
  m_allocation()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}


BeamletAllocator::~BeamletAllocator()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

bool BeamletAllocator::allocateBeamlets(
  string station,
  time_t startTime,
  time_t stopTime,
  vector<int16> subbands,
  vector<int16>& beamlets)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  time_t timeNow = APLUtilities::getUTCtime();
  
  bool result(true);
  TStation2AllocationMap::iterator itStationAllocation = m_allocation.find(station);
  if(itStationAllocation != m_allocation.end())
  {
    vector<int16>::iterator           itSubbands = subbands.begin();
    TBeamlet2AllocationMap::iterator  itBeamletAllocation = itStationAllocation.second.begin();
    while(result && itSubbands!=subbands.end() && itBeamletAllocation != itStationAllocation.second.end())
    {
      bool allocated=false;
      // search the next free beamlet
      while(!allocated && itBeamletAllocation != itStationAllocation.second.end())
      {
        // check ALL allocations for one beamlet
        bool roomForAllocation(true);
        TAllocationInfoVector::iterator itAllocationInfo = itBeamletAllocation.second.begin();
        while(roomForAllocation && itAllocationInfo != itBeamletAllocation.second.end())
        {
          // overlapping start/stoptimes not allowed
          if(startTime <= itAllocationInfo->stopTime && stopTime >= itAllocationInfo->startTime)
            roomForAllocation=false;
            
          // garbage collection on the fly: remove allocations with stoptimes before now
          TAllocationInfoVector::iterator itAllocationInfoGarbage = itAllocationInfo;
          ++itAllocationInfo;
          if(itAllocationInfoGarbage->stopTime < timeNow)
            itBeamletAllocation.second.erase(itAllocationInfoGarbage);
        }
        if(roomForAllocation)
        {
          beamlets.push_back(itBeamletAllocation.first);
          
          TAllocationInfo newAllocation;
          newAllocation.startTime = startTime;
          newAllocation.stopTime = stopTime;
          newAllocation.subband = (*itSubbands);
          itBeamletAllocation.second.push_back(newAllocation);
          
          allocated=true
        }
        ++itBeamletAllocation; // next round: begin with the next beamlet because the current has been allocated.
      }
      if(!allocated)
      {
        result = false;
      }
      ++itSubbands; // allocate next subband
    }
  }
  else
  {
    // add new allocation
  }
  
  return result;
}

};
};
