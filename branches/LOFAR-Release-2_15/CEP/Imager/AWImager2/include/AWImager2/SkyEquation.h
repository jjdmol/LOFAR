//# SkyEquation.h: SkyEquation definition
//# Copyright (C) 2007
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $
#ifndef LOFAR_LOFARFT_SKYEQUATION_H
#define LOFAR_LOFARFT_SKYEQUATION_H

#include <synthesis/MeasurementEquations/SkyEquation.h>

//Forward
namespace casa 
{    
  class ROVisibilityIterator;
  template <class T> class ImageInterface;
  template <class T> class TempImage;
  template <class T> class SubImage;
  template <class T> class Block;
}

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
namespace LofarFT { //# NAMESPACE LOFAR - BEGIN
  
class FTMachine;
class VisibilityIterator;
class VisBuffer;

class SkyEquation : public casa::SkyEquation
{
  public:
  //Read only iterator...hence no scratch col
  SkyEquation(
    casa::SkyModel& sm, 
    VisibilityIterator& vi, 
    FTMachine& ft, 
    casa::ComponentFTMachine& cft, 
    casa::Bool noModelCol = casa::False);

  virtual ~SkyEquation();
  
  virtual void predict(
    casa::Bool incremental = casa::False, 
    casa::MS::PredefinedColumns Type = casa::MS::MODEL_DATA);
  
  virtual void gradientsChiSquared(casa::Bool incremental, casa::Bool commitModel=casa::False);

  void makeApproxPSF(casa::PtrBlock<casa::ImageInterface<casa::Float> * >& psfs);

  private:

  casa::Block<casa::CountedPtr<casa::FTMachine> > ftm_p;
  casa::Block<casa::CountedPtr<casa::FTMachine> > iftm_p;

  FTMachine * itsFTMachine;
  VisibilityIterator * rvi_p;
  VisibilityIterator * wvi_p;
};

} //# NAMESPACE LOFARFT
} //# NAMESPACE LOFAR 

#endif
