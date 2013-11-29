//# CFStore.h: Definition of the CFStore class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef LOFARFT_CFSTORE_H
#define LOFARFT_CFSTORE_H

#include <LofarFT/LofarCFDefs.h>
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
  using namespace LofarCFDefs;
  class LofarCFStore
  {
  public:

    LofarCFStore():data(), rdata(),vdata(), coordSys(), sampling(), 
	      xSupport(), ySupport(), 
	      maxXSupport(-1), maxYSupport(-1),
		   pa(), mosPointingPos(0) {};

    LofarCFStore(const casa::CountedPtr<CFType>& dataPtr, casa::CoordinateSystem& cs, casa::Vector<casa::Float>& samp,
	    casa::Vector<casa::Int>& xsup, casa::Vector<casa::Int>& ysup, casa::Int maxXSup, casa::Int maxYSup,
	    casa::Quantity PA, casa::Int mosPointing):
      data(dataPtr),rdata(),vdata(), coordSys(cs), sampling(samp),
      xSupport(xsup), ySupport(ysup), maxXSupport(maxXSup),
      maxYSupport(maxYSup), pa(PA), mosPointingPos(mosPointing)
    {}

    LofarCFStore(const casa::CountedPtr<CFTypeVec>& dataPtr, casa::CoordinateSystem& cs, casa::Vector<casa::Float>& samp,
	    casa::Vector<casa::Int>& xsup, casa::Vector<casa::Int>& ysup, casa::Int maxXSup, casa::Int maxYSup,
		 casa::Quantity PA, casa::Int mosPointing, const casa::Matrix<bool>&):
      data(),rdata(),vdata(dataPtr), coordSys(cs), sampling(samp),
      xSupport(xsup), ySupport(ysup), maxXSupport(maxXSup),
      maxYSupport(maxYSup), pa(PA), mosPointingPos(mosPointing)
    {}

    ~LofarCFStore() {};

    LofarCFStore& operator=(const LofarCFStore& other);
    void show(const char *Mesg=NULL, casa::ostream &os=casa::cerr);
    casa::Bool null() {return data.null();};
    void set(const LofarCFStore& other)
    {
      coordSys = other.coordSys; sampling.assign(other.sampling); 
      xSupport.assign(other.xSupport); ySupport.assign(other.ySupport);
      maxXSupport=other.maxXSupport;  maxYSupport=other.maxYSupport; pa=other.pa;
      mosPointingPos = other.mosPointingPos;
    }

    void set(CFType *dataPtr, casa::CoordinateSystem& cs, casa::Vector<casa::Float>& samp,
	     casa::Vector<casa::Int>& xsup, casa::Vector<casa::Int>& ysup, casa::Int maxXSup, casa::Int maxYSup,
	     casa::Quantity PA, const casa::Int mosPointing=0)
    {
      data=dataPtr; coordSys=cs; sampling.assign(samp); 
      xSupport.assign(xsup); ySupport.assign(ysup);
      maxXSupport=maxXSup;maxYSupport=maxYSup;
      pa=PA;
      mosPointingPos = mosPointing;
    }

    void set(CFTypeVec *dataPtr, casa::CoordinateSystem& cs, casa::Vector<casa::Float>& samp,
	     casa::Vector<casa::Int>& xsup, casa::Vector<casa::Int>& ysup, casa::Int maxXSup, casa::Int maxYSup,
	     casa::Quantity PA, const casa::Int mosPointing=0)
    {
      vdata=dataPtr; coordSys=cs; sampling.assign(samp); 
      xSupport.assign(xsup); ySupport.assign(ysup);
      maxXSupport=maxXSup;maxYSupport=maxYSup;
      pa=PA;
      mosPointingPos = mosPointing;
    }

    void resize(casa::Int nw, casa::Bool retainValues=casa::False);
    void resize(casa::IPosition imShape, casa::Bool retainValues=casa::False);


    casa::CountedPtr<CFType> data;
    casa::CountedPtr<CFTypeReal> rdata;
    casa::CountedPtr<CFTypeVec> vdata;
    casa::CoordinateSystem coordSys;
    casa::Vector<casa::Float> sampling;
    casa::Vector<casa::Int> xSupport,ySupport;
    casa::Int maxXSupport, maxYSupport;
    casa::Quantity pa;
    casa::Int mosPointingPos;
  };

} //# NAMESPACE LOFAR - END

#endif
