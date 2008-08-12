//#  BeamletAllocator.h: Allocates beamlets per station
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

#ifndef BeamletAllocator_H
#define BeamletAllocator_H

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

  class BeamletAllocator
  {
    public:
      typedef map<string, vector<int16> > TStationBeamletAllocation;
      
               BeamletAllocator(int16 maxBeamlets);
      virtual ~BeamletAllocator();
      
      bool allocateBeamlets(const string&               vi,
                            const uint16                priority,
                            const vector<string>        stations, 
                            const time_t                startTime, 
                            const time_t                stopTime, 
                            const vector<int16>         subbands, 
                            TStationBeamletAllocation&  allocation,
                            map<string, TStationBeamletAllocation>& resumeVIs,
                            set<string>&                suspendVIs);
      void deallocateBeamlets(const string& vi, 
                              map<string, TStationBeamletAllocation>& resumeIVs,
                              set<string>& suspendVIs);
      void logAllocation(bool groupByVI=false);
      void logSuspendedAllocation(bool groupByVI=false);

    protected:
      // protected copy constructor
      BeamletAllocator(const BeamletAllocator&);
      // protected assignment operator
      BeamletAllocator& operator=(const BeamletAllocator&);

    private:
      struct TAllocationInfo
      {
        TAllocationInfo() :
          subband(0),vi(""),priority(0),startTime(0),stopTime(0) 
        {};
        
        TAllocationInfo(const TAllocationInfo& rhs) :
          subband(rhs.subband),vi(rhs.vi),priority(rhs.priority),startTime(rhs.startTime),stopTime(rhs.stopTime) 
        {};
        
        virtual ~TAllocationInfo() {};
        
        TAllocationInfo& operator=(const TAllocationInfo& rhs)
        {
          if(this != &rhs)
          {
            subband = rhs.subband;
            vi = rhs.vi;
            priority = rhs.priority;
            startTime = rhs.startTime;
            stopTime = rhs.stopTime;
          }
          return *this;
        };

        int16   subband;
        string  vi;
        uint16  priority;
        time_t  startTime;
        time_t  stopTime;
      };

      struct earlierStart
      {
        bool operator()(const TAllocationInfo& k1, const TAllocationInfo& k2) const
        {
          // returns true if k1 starts earlier than k2.
	  // if they have the same starttime, then it returns true if k1 has a higher priority.
          return ((k1.startTime==k2.startTime?k1.priority<k2.priority:k1.startTime<k2.startTime));
        }
      };
      
      typedef set<TAllocationInfo, earlierStart> TAllocationInfoSet;
      typedef map<int16,  TAllocationInfoSet>  TBeamlet2AllocationMap;
      typedef map<string, TBeamlet2AllocationMap> TStation2AllocationMap;
//      typedef boost::shared_ptr<TStation2AllocationMap> TStation2AllocationMapPtr;
      
      TStation2AllocationMap::iterator _addStationAllocation(TStation2AllocationMap& allocation, const string& station);
      bool _testAllocateBeamlets(const string&              vi,
                                 const uint16               priority, 
                                 const vector<string>       stations, 
                                 const time_t               startTime, 
                                 const time_t               stopTime, 
                                 const vector<int16>        subbands, 
                                 TStation2AllocationMap& newAllocationDetails, 
                                 TStationBeamletAllocation& newAllocationBeamlets,
                                 set<string>&               suspendVIs);
      bool _mergeAllocation(TStation2AllocationMap& allocation, TStation2AllocationMap& newAllocationDetails);
      void _extractAllocation(const string&                 vi, 
                              TStation2AllocationMap&       source, 
                              TStation2AllocationMap&    allocationDetails);
      void _deallocateBeamlets(const string& vi, 
                               TStation2AllocationMap& deallocatedInfo,
                               map<string, TStationBeamletAllocation>& resumeIVs,
                               set<string>& suspendVIs);
      void _logAllocation(TStation2AllocationMap& allocation, const string& title, bool groupByVI);
      
      TStation2AllocationMap  m_allocation;
      TStation2AllocationMap  m_suspendedAllocation;

      set<string>             m_resumedVIs;
      set<string>             m_suspendedVIs;
      const int16             m_maxBeamlets;

      ALLOC_TRACER_CONTEXT  
   };
};//GSO
};//LOFAR
#endif
