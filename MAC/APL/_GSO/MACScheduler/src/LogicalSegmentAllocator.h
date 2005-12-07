//#  LogicalSegmentAllocator.h: Allocates logical segments to virtual arrays
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

#ifndef LogicalSegmentAllocator_H
#define LogicalSegmentAllocator_H

//# Includes
//#include <boost/shared_ptr.hpp>

//# GCF Includes

//# local includes

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_set.h>
#include <Common/lofar_map.h>
#include <Common/LofarLogger.h>
#include <utility> // for the pair declaration
// forward declaration

namespace LOFAR
{
  
namespace GSO
{

  class LogicalSegmentAllocator
  {
    public:
      typedef map<string, double > TLogicalSegmentBandwidth;

               LogicalSegmentAllocator();
      virtual ~LogicalSegmentAllocator();
      
      bool allocateVirtualRoute(const string&                   vr,
                                const TLogicalSegmentBandwidth& lsCapacities,
                                const vector<string>&           logicalSegments,
                                const uint16                    priority,
                                const time_t                    startTime,
                                const time_t                    stopTime,
                                const double                    requiredBandwidth,
                                set<string>&                    resumeVRs,
                                set<string>&                    suspendVRs);
      void deallocateVirtualRoute(const string&                   vr,
                                  const TLogicalSegmentBandwidth& lsCapacities, // for reallocating other VR's
                                  set<string>&                    resumeVRs,
                                  set<string>&                    suspendVRs);
      void logAllocation(bool groupByVR=false);
      void logSuspendedAllocation(bool groupByVR=false);

    protected:
      // protected copy constructor
      LogicalSegmentAllocator(const LogicalSegmentAllocator&);
      // protected assignment operator
      LogicalSegmentAllocator& operator=(const LogicalSegmentAllocator&);

    private:
      struct TAllocationInfo
      {
        TAllocationInfo() :
          vr(""),
          requiredBandwidth(0.0),
          priority(0),
          startTime(0),
          stopTime(0) 
        {};
        
        TAllocationInfo(const TAllocationInfo& rhs) :
          vr(rhs.vr),
          requiredBandwidth(rhs.requiredBandwidth),
          priority(rhs.priority),
          startTime(rhs.startTime),
          stopTime(rhs.stopTime) 
        {};
        
        virtual ~TAllocationInfo() {};
        
        TAllocationInfo& operator=(const TAllocationInfo& rhs)
        {
          if(this != &rhs)
          {
            vr = rhs.vr;
            requiredBandwidth = rhs.requiredBandwidth;
            priority = rhs.priority;
            startTime = rhs.startTime;
            stopTime = rhs.stopTime;
          }
          return *this;
        };

        string  vr;
        double  requiredBandwidth;
        uint16  priority;
        time_t  startTime;
        time_t  stopTime;
      };
//      typedef boost::shared_ptr<TAllocationInfo> TAllocationInfoPtr;

      struct earlierStart
      {
        bool operator()(const TAllocationInfo& k1, const TAllocationInfo& k2) const
        {
          // returns true if k1 starts earlier than k2.
          // if they have the same starttime, then it returns true if k1 has a higher priority.
          return ((k1.startTime==k2.startTime?k1.priority<k2.priority:k1.startTime<k2.startTime));
        }
      };
      
      typedef multiset<TAllocationInfo, earlierStart> TAllocationInfoSet;
      typedef map<time_t, double> TBandwidthUsageMap;
      struct TAllocation
      {
        TAllocation() : infoSet(), bandwidthUsage() {};
        TAllocation(const TAllocationInfoSet& _infoSet) :
          infoSet(_infoSet), bandwidthUsage() {};
        TAllocation(const TAllocationInfoSet& _infoSet,const TBandwidthUsageMap& _bandwidthUsage) :
          infoSet(_infoSet), bandwidthUsage(_bandwidthUsage) {};
        
        TAllocationInfoSet infoSet;
        TBandwidthUsageMap bandwidthUsage;
      };
      typedef map<string, TAllocation> TLogicalSegment2AllocationMap;
      
      TLogicalSegment2AllocationMap::iterator _addLogicalSegmentAllocation(TLogicalSegment2AllocationMap& allocation, 
                                                                           const string& logicalSegment);
      bool _testAllocateLogicalSegments(const string&                   vr,
                                        const TLogicalSegmentBandwidth& lsCapacities, 
                                        const vector<string>&           logicalSegments,
                                        const uint16                    priority, 
                                        const time_t                    startTime, 
                                        const time_t                    stopTime, 
                                        const double                    requiredBandwidth, 
                                        TLogicalSegment2AllocationMap&  newAllocationDetails, 
                                        set<string>&                    suspendVRs);
      bool _mergeAllocation(TLogicalSegment2AllocationMap& allocation, 
                            TLogicalSegment2AllocationMap& newAllocationDetails);
      void _extractAllocation(const string&                   vr, 
                              TLogicalSegment2AllocationMap&  source, 
                              TLogicalSegment2AllocationMap&  allocationDetails);
      void _deallocateVirtualRoute(const string&                    vr, 
                                   const TLogicalSegmentBandwidth&  lsCapacities, // for reallocating other VR's
                                   TLogicalSegment2AllocationMap&   lsDeallocatedInfo, 
                                   set<string>&                     resumeVRs,
                                   set<string>&                     suspendVRs);
      void _addBandwidthUsage(TBandwidthUsageMap& bandwidthUsage, const TAllocationInfo& allocationInfo);
      void _removeBandwidthUsage(TBandwidthUsageMap& bandwidthUsage, const TAllocationInfo& allocationInfo);
      void _logAllocation(TLogicalSegment2AllocationMap& allocation, const string& title, bool groupByVR);
      void _logAllocationInfo(TLogicalSegment2AllocationMap& allocation, const string& title, bool groupByVR);
      void _logBandwidthUsage(TLogicalSegment2AllocationMap& allocation, const string& title);
      
      TLogicalSegment2AllocationMap m_allocation;
      TLogicalSegment2AllocationMap m_suspendedAllocation;

      set<string>                   m_resumedVRs;
      set<string>                   m_suspendedVRs;

      ALLOC_TRACER_CONTEXT  
   };
};//GSO
};//LOFAR
#endif
