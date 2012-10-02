//# LofarBitModeInfo.h
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: LofarBitModeInfo.h 21167 2012-06-08 13:36:34Z mol $

#ifndef LOFAR_COMMON_BITMODEINFO_H_
#define LOFAR_COMMON_BITMODEINFO_H_

#include <Common/LofarConstants.h>

namespace LOFAR {
    //int maxBeamletsPerRSP(int bitsPerSample);
    //int maxBeamlets(int bitsPerSample);
    
    inline static int maxDataslotsPerRSP(int bitsPerSample) {
		assert(bitsPerSample >= MIN_BITS_PER_SAMPLE && bitsPerSample <= MAX_BITS_PER_SAMPLE && bitsPerSample%2==0);
		return(61);
	}


    inline static int maxBeamletsPerRSP(int bitsPerSample) {
		assert(bitsPerSample >= MIN_BITS_PER_SAMPLE && bitsPerSample <= MAX_BITS_PER_SAMPLE && bitsPerSample%2==0);
        switch (bitsPerSample) {
          case 4:
              return(4*maxDataslotsPerRSP(bitsPerSample));
          case 8: 
              return(2*maxDataslotsPerRSP(bitsPerSample));
          default: 
              return(maxDataslotsPerRSP(bitsPerSample));
		}
    }
    
    inline static int maxBeamletsPerBank(int bitsPerSample) {
        // 4 output lanes on one station
        return(4 * maxDataslotsPerRSP(bitsPerSample));
    }
    
    inline static int maxBeamletsPerPlane(int bitsPerSample) {
        return(maxBeamletsPerBank(bitsPerSample));
    }
    
    
    inline static int maxBeamlets(int bitsPerSample) {
        // 4 output lanes on one station
        return(4 * maxBeamletsPerRSP(bitsPerSample));
    }
}

#endif
