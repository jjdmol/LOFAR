//# LofarBitModeInfo.h
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_COMMON_BITMODEINFO_H
#define LOFAR_COMMON_BITMODEINFO_H


namespace LOFAR {
    
    int maxBeamletsPerRSP(unsigned bitsPerSample) {
        int max_beamlets_per_rsp;
        switch (bitsPerSample)
        {
            case 4:
            case 8: {
                max_beamlets_per_rsp = 59;
            } break;
            
            case 16: {
                max_beamlets_per_rsp = 61;
            } break;
            default: {
                max_beamlets_per_rsp = 61;
            }
        }
        return(max_beamlets_per_rsp);
    }
    
    int maxBeamlets(unsigned bitsPerSample) {
        // 4 output lanes on one station
        return(4 * maxBeamletsPerRSP(bitsPerSample));
    }
}

#endif
