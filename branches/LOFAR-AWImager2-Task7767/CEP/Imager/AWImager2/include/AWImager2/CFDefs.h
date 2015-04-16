//# CFDefs.h: Convolution Function Defs
//#
//# Copyright (C) 2011
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
//# $Id: $

#ifndef LOFAR_LOFARFT_CFDEFS_H
#define LOFAR_LOFARFT_CFDEFS_H
#include <casa/Arrays/Array.h>
#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>

namespace LOFAR {
namespace LofarFT {
namespace CFDefs { //# NAMESPACE CFDefs - BEGIN
  
// freq, mueller_x, mueller_y
typedef vector< vector< vector < casa::Matrix<casa::Complex> > > > CFTypeVec ;
typedef casa::Array<casa::Complex> CFType ;
typedef casa::Array<casa::Double> CFTypeReal ;
enum CACHETYPE {NOTCACHED=0,DISKCACHE, MEMCACHE};
enum CFARRAYSHAPE {NXPOS=0,NYPOS,NWPOS,NPOLPOS,NBASEPOS,CFNDIM};
    
} //# NAMESPACE CFDefs - END
} //# NAMESPACE LofarFT - END
} //# NAMESPACE LOFAR - END
#endif
