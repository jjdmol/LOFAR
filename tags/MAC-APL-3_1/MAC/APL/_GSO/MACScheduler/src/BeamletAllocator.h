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
#include <boost/shared_ptr.hpp>

//# GCF Includes

//# local includes

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <Common/LofarLogger.h>

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
                            const vector<string>        stations, 
                            const time_t                startTime, 
                            const time_t                stopTime, 
                            const vector<int16>         subbands, 
                            TStationBeamletAllocation&  allocation);
      void deallocateBeamlets(const string& vi);
      void logAllocation(bool groupByVI=false);

    protected:
      // protected copy constructor
      BeamletAllocator(const BeamletAllocator&);
      // protected assignment operator
      BeamletAllocator& operator=(const BeamletAllocator&);

    private:
      typedef struct
      {
        int16   subband;
        string  vi;
        time_t  startTime;
        time_t  stopTime;
      } TAllocationInfo;
      typedef vector<TAllocationInfo> TAllocationInfoVector;
      typedef map<int16,  TAllocationInfoVector>  TBeamlet2AllocationMap;
      typedef map<string, TBeamlet2AllocationMap> TStation2AllocationMap;
      typedef boost::shared_ptr<TStation2AllocationMap> TStation2AllocationMapPtr;
      
      TStation2AllocationMap::iterator _addStationAllocation(const string& station);
      bool _testAllocateBeamlets(const string&              vi,
                                 const vector<string>       stations, 
                                 const time_t               startTime, 
                                 const time_t               stopTime, 
                                 const vector<int16>        subbands, 
                                 TStation2AllocationMapPtr& newAllocationDetailsPtr, 
                                 TStationBeamletAllocation& newAllocationBeamlets);
      bool _mergeAllocation(TStation2AllocationMapPtr newAllocationDetailsPtr);
      void _extractAllocation(const string& vi, TStation2AllocationMapPtr& allocationDetailsPtr);
      
      TStation2AllocationMap  m_allocation;
      const int16             m_maxBeamlets;

      ALLOC_TRACER_CONTEXT  
   };
};//GSO
};//LOFAR
#endif
