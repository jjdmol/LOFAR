//# CFStore.h: Definition of the CFStore class
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

#ifndef LOFAR_LOFARFT_CFSTORE_H
#define LOFAR_LOFARFT_CFSTORE_H

#include <LofarFT/CFDefs.h>
#include <synthesis/TransformMachines/SynthesisError.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/CountedPtr.h>
#include <images/Images/ImageInterface.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/Arrays/Matrix.h>

namespace LOFAR {
namespace LofarFT {
  
using namespace CFDefs;

class CFStore
{
public:

  CFStore();

  CFStore(const casa::CountedPtr<CFType>& dataPtr, 
          casa::CoordinateSystem& cs, 
          casa::Vector<casa::Float>& samp,
          casa::Vector<casa::Int>& xsup, 
          casa::Vector<casa::Int>& ysup, 
          casa::Int maxXSup, 
          casa::Int maxYSup,
          casa::Quantity PA, 
          casa::Int mosPointing,
          casa::Bool conjugated = casa::False);

  CFStore(const casa::CountedPtr<CFTypeVec>& dataPtr, 
          casa::CoordinateSystem& cs, 
          casa::Vector<casa::Float>& samp,
          casa::Vector<casa::Int>& xsup, 
          casa::Vector<casa::Int>& ysup, 
          casa::Int maxXSup, 
          casa::Int maxYSup,
          casa::Quantity PA, 
          casa::Int mosPointing,
          casa::Bool conjugated = casa::False);

  ~CFStore() {};

  CFStore& operator=(const CFStore& other);
  
  void show(const char *Mesg=NULL, casa::ostream &os=casa::cerr);
  
  casa::Bool isNull() {return itsData.null();}
  
  casa::Bool isConjugated() {return itsConjugated;}
  
  void set(const CFStore& other);

  void set(CFType *dataPtr, 
           casa::CoordinateSystem& cs, 
           casa::Vector<casa::Float>& samp,
           casa::Vector<casa::Int>& xsup, 
           casa::Vector<casa::Int>& ysup, 
           casa::Int maxXSup, 
           casa::Int maxYSup,
           casa::Quantity PA, 
           const casa::Int mosPointing=0,
           casa::Bool conjugated = casa::False);

  void set(CFTypeVec *dataPtr, 
           casa::CoordinateSystem& cs, 
           casa::Vector<casa::Float>& samp,
           casa::Vector<casa::Int>& xsup,
           casa::Vector<casa::Int>& ysup, 
           casa::Int maxXSup, 
           casa::Int maxYSup,
           casa::Quantity PA, 
           const casa::Int mosPointing=0,
           casa::Bool conjugated=casa::False);

  void resize(casa::Int nw, 
              casa::Bool retainValues=casa::False);
  
  void resize(casa::IPosition imShape, 
              casa::Bool retainValues=casa::False);

  CFTypeVec& vdata() {return *itsVData;}
  casa::Vector<casa::Float>& sampling() {return itsSampling;}
  casa::Vector<casa::Int>& xSupport() {return itsXSupport;}
  casa::Vector<casa::Int>& ySupport() {return itsYSupport;}
  
  
  casa::CountedPtr<CFType> itsData;
  casa::CountedPtr<CFTypeReal> itsRData;
  casa::CountedPtr<CFTypeVec> itsVData;
  casa::CoordinateSystem itsCoordSys;
  casa::Vector<casa::Float> itsSampling;
  casa::Vector<casa::Int> itsXSupport;
  casa::Vector<casa::Int> itsYSupport;
  casa::Int itsMaxXSupport;
  casa::Int itsMaxYSupport;
  casa::Quantity itsPA;
  casa::Int itsMosPointingPos;
  casa::Bool itsConjugated;
};

} //# NAMESPACE LofarFT - END
} //# NAMESPACE LOFAR - END

#endif
