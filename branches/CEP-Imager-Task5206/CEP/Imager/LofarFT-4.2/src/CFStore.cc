// -*- C++ -*-
//# CFStore.cc: Implementation of the CFStore class
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
#include <lofar_config.h>
#include <LofarFT/CFStore.h>

using namespace casa;

namespace LOFAR{
namespace LofarFT{

  CFStore::CFStore()
  : itsData(),
    itsRData(),
    itsVData(), 
    itsCoordSys(), 
    itsSampling(), 
    itsXSupport(), 
    itsYSupport(), 
    itsMaxXSupport(-1), 
    itsMaxYSupport(-1),
    itsPA(), 
    itsMosPointingPos(0),
    itsConjugated(False)    
  {};

  CFStore::CFStore(
    const casa::CountedPtr<CFType>& dataPtr, 
    casa::CoordinateSystem& cs, 
    casa::Vector<casa::Float>& samp,
    casa::Vector<casa::Int>& xsup, 
    casa::Vector<casa::Int>& ysup, 
    casa::Int maxXSup, 
    casa::Int maxYSup,
    casa::Quantity PA, 
    casa::Int mosPointing,
    casa::Bool conjugated)
  : itsData(dataPtr),
    itsRData(),
    itsVData(), 
    itsCoordSys(cs), 
    itsSampling(samp),
    itsXSupport(xsup), 
    itsYSupport(ysup), 
    itsMaxXSupport(maxXSup),
    itsMaxYSupport(maxYSup), 
    itsPA(PA), 
    itsMosPointingPos(mosPointing),
    itsConjugated(conjugated)
  {};

  CFStore::CFStore(
    const casa::CountedPtr<CFTypeVec>& dataPtr, 
    casa::CoordinateSystem& cs, 
    casa::Vector<casa::Float>& samp,
    casa::Vector<casa::Int>& xsup, 
    casa::Vector<casa::Int>& ysup, 
    casa::Int maxXSup, 
    casa::Int maxYSup,
    casa::Quantity PA, 
    casa::Int mosPointing,
    casa::Bool conjugated)
  : itsData(),
    itsRData(),
    itsVData(dataPtr), 
    itsCoordSys(cs), 
    itsSampling(samp),
    itsXSupport(xsup), 
    itsYSupport(ysup), 
    itsMaxXSupport(maxXSup),
    itsMaxYSupport(maxYSup), 
    itsPA(PA), 
    itsMosPointingPos(mosPointing),
    itsConjugated(conjugated)
  {};
  
  
  CFStore& CFStore::operator=(const CFStore& other)
  {
    if (&other != this)
    {
      itsData=other.itsData; 
      itsRData=other.itsRData; 
      itsVData=other.itsVData; 
      itsCoordSys=other.itsCoordSys; 
      itsSampling.assign(other.itsSampling);
      itsXSupport.assign(other.itsXSupport);
      itsYSupport.assign(other.itsYSupport);
      itsPA=other.itsPA;
      itsConjugated=other.itsConjugated;
    }
    return *this;
  };
  
  void CFStore::set(const CFStore& other)
  {
    itsCoordSys = other.itsCoordSys; 
    itsSampling.assign(other.itsSampling); 
    itsXSupport.assign(other.itsXSupport); 
    itsYSupport.assign(other.itsYSupport);
    itsMaxXSupport = other.itsMaxXSupport;  
    itsMaxYSupport = other.itsMaxYSupport; 
    itsPA=other.itsPA;
    itsMosPointingPos = other.itsMosPointingPos;
    itsConjugated = other.itsConjugated;
  };

  void CFStore::set(
    CFType *dataPtr, 
    casa::CoordinateSystem& cs, 
    casa::Vector<casa::Float>& samp,
    casa::Vector<casa::Int>& xsup, 
    casa::Vector<casa::Int>& ysup, 
    casa::Int maxXSup, 
    casa::Int maxYSup,
    casa::Quantity PA, 
    const casa::Int mosPointing,
    casa::Bool conjugated)
  {
    itsData=dataPtr;
    itsCoordSys=cs; 
    itsSampling.assign(samp); 
    itsXSupport.assign(xsup); 
    itsYSupport.assign(ysup);
    itsMaxXSupport=maxXSup;
    itsMaxYSupport=maxYSup;
    itsPA=PA;
    itsMosPointingPos = mosPointing;
    itsConjugated = conjugated;
  };

  void CFStore::set(
    CFTypeVec *dataPtr, 
    casa::CoordinateSystem& cs, 
    casa::Vector<casa::Float>& samp,
    casa::Vector<casa::Int>& xsup,
    casa::Vector<casa::Int>& ysup, 
    casa::Int maxXSup, 
    casa::Int maxYSup,
    casa::Quantity PA, 
    const casa::Int mosPointing,
    casa::Bool conjugated)
  {
    itsVData=dataPtr; 
    itsCoordSys=cs; 
    itsSampling.assign(samp); 
    itsXSupport.assign(xsup); 
    itsYSupport.assign(ysup);
    itsMaxXSupport=maxXSup;
    itsMaxYSupport=maxYSup;
    itsPA=PA;
    itsMosPointingPos = mosPointing;
    itsConjugated = conjugated;
  };

  void CFStore::show(
    const char *Mesg, 
    ostream& os)
  {
    if (!this->isNull())
    {
      if (Mesg)
      {
        os << Mesg << endl;
        os << "Data Shape: " << itsData->shape() << endl;
        os << "Sampling: " << itsSampling << endl;
        os << "xSupport: " << itsXSupport << endl;
        os << "ySupport: " << itsYSupport << endl;
        os << "PA = " << itsPA.get("deg") << endl;
      }
    }
  };
  
  void CFStore::resize(
    Int nw,  
    Bool retainValues)
  {
    itsXSupport.resize(nw,retainValues); 
    itsYSupport.resize(nw,retainValues);
    itsSampling.resize(1);
  }
  
  void CFStore::resize(
    IPosition imShape, 
    Bool retainValues)
  {
    if (imShape.nelements() > CFDefs::NWPOS)
      this->resize(imShape(CFDefs::NWPOS), retainValues);
    if ((imShape.nelements() > 0) && (!itsData.null()))
      itsData->resize(imShape,retainValues);
  }
  
} // end LofarFT namespace
} // end LOFAR namespace
