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
               BeamletAllocator();
      virtual ~BeamletAllocator();
      
      bool allocateBeamlets(vector<string> stations, vector<int16> subbands, time_t startTime, time_t stopTime, vector<int16>& beamlets);

    protected:
      // protected copy constructor
      BeamletAllocator(const BeamletAllocator&);
      // protected assignment operator
      BeamletAllocator& operator=(const BeamletAllocator&);

    private:
      typedef struct
      {
        int16               subband;
        time_t              startTime;
        time_t              stopTime;
      } TAllocationInfo;
      typedef vector<TAllocationInfo> TAllocationInfoVector;
      typedef map<int16,  TAllocationInfoVector>  TBeamlet2AllocationMap;
      typedef map<string, TBeamlet2AllocationMap> TStation2AllocationMap;
      
      TStation2AllocationMap  m_allocation;

      ALLOC_TRACER_CONTEXT  
   };
};//GSO
};//LOFAR
#endif
