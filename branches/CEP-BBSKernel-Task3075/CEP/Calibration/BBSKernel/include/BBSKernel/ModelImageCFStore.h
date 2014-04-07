//# ModelImageCFStore.h: 
//#
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
//# $Id: ModelImageCFStore.h 20029 2012-04-23 15:07:23Z duscha $


#ifndef LOFAR_BBSKERNEL_MODELIMAGECFSTORE_H 
#define LOFAR_BBSKERNEL_MODELIMAGECFSTORE_H

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Utilities/CountedPtr.h>
#include <images/Images/ImageInterface.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

namespace LOFAR {   //# NAMESPACE LOFAR BEGIN
namespace BBS {     //# NAMESPACE BBS BEBGIN

//#ifndef SYNTHESIS_CFDEFS_H
//  namespace CFDefs { //# NAMESPACE CASA - BEGIN
typedef casa::Array<casa::Complex> CFType;
typedef casa::Array<casa::Double> CFTypeReal;
enum CACHETYPE {NOTCACHED=0,DISKCACHE, MEMCACHE};
enum CFARRAYSHAPE {NXPOS=0,NYPOS,NWPOS,NPOLPOS,NBASEPOS,CFNDIM};
//  } //# NAMESPACE CASA - END
//#define SYNTHESIS_CFDEFS_H
//#endif
//using namespace CFDefs

using namespace casa;   // normally shouldn't do that in header file

class CFStore
{
public:
  CFStore():data(), rdata(), coordSys(), sampling(), 
      xSupport(), ySupport(), 
      maxXSupport(-1), maxYSupport(-1),
      pa(), mosPointingPos(0) {};

  CFStore(CFType *dataPtr, CoordinateSystem& cs, Vector<Float>& samp,
    Vector<Int>& xsup, Vector<Int>& ysup, Int maxXSup, Int maxYSup,
    Quantity PA, Int mosPointing):
    data(),rdata(), coordSys(cs), sampling(samp),
    xSupport(xsup), ySupport(ysup), maxXSupport(maxXSup),
    maxYSupport(maxYSup), pa(PA), mosPointingPos(mosPointing)
  {data = new CFType(*dataPtr);};

  ~CFStore() {};

  CFStore& operator=(const CFStore& other);
  void show(const char *Mesg=NULL,ostream &os=cerr);
  Bool null() {return data.null();};
  void set(const CFStore& other)
  {
    coordSys = other.coordSys; sampling.assign(other.sampling); 
    xSupport.assign(other.xSupport); ySupport.assign(other.ySupport);
    maxXSupport=other.maxXSupport;  maxYSupport=other.maxYSupport; pa=other.pa;
    mosPointingPos = other.mosPointingPos;
  }
  void set(CFType *dataPtr, CoordinateSystem& cs, Vector<Float>& samp,
     Vector<Int>& xsup, Vector<Int>& ysup, Int maxXSup, Int maxYSup,
     Quantity PA, const Int mosPointing=0)
  {
    data=dataPtr; coordSys=cs; sampling.assign(samp); 
    xSupport.assign(xsup); ySupport.assign(ysup);
    maxXSupport=maxXSup;maxYSupport=maxYSup;
    pa=PA;
    mosPointingPos = mosPointing;
  }

  void resize(Int nw, Bool retainValues=False);
  void resize(IPosition imShape, Bool retainValues=False);


  CountedPtr<CFType> data;
  CountedPtr<CFTypeReal> rdata;
  CoordinateSystem coordSys;
  Vector<Float> sampling;
  Vector<Int> xSupport,ySupport;
  Int maxXSupport, maxYSupport;
  Quantity pa;
  Int mosPointingPos;
};

} // end BBS namespace
} // end LOFAR namespace
#endif LOFAR_BBSKERNEL_MODELIMAGECFSTORE_H