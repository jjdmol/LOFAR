// -*- C++ -*-
//# VBStore.h: Definition of the VBStore class
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
//# $Id: $

#ifndef LOFAR_LOFARFT_VBSTORE_H
#define LOFAR_LOFARFT_VBSTORE_H
#include <synthesis/TransformMachines/Utils.h>

namespace LOFAR {
namespace LofarFT {

  class VBStore
  {
  public:
    VBStore() : itsDoPSF(casa::False) {};
    ~VBStore() {};
    
    casa::Int nRow() const             {return itsNRow;}
    void nRow(casa::Int nrow)     {itsNRow = nrow;}
    
    casa::Int beginRow()  const        {return itsBeginRow;}
    void beginRow(casa::Int beginrow) {itsBeginRow = beginrow;}
    
    casa::Int endRow() const           {return itsEndRow;}
    void endRow(casa::Int endrow)  {itsEndRow = endrow;}

    casa::Bool dopsf() const           {return itsDoPSF;}
    void dopsf(casa::Bool do_psf)       {itsDoPSF = do_psf;}

    const casa::Vector<casa::uInt>& selection() const      {return itsSelection;};
    
    const casa::Matrix<casa::Double>& uvw() const          {return itsUVW;}
    void uvw(const casa::Matrix<casa::Double>& v)     {itsUVW.reference(v);}
    
    const casa::Vector<casa::Bool>& rowFlag() const        {return itsRowFlag;}
    void rowFlag(const casa::Vector<casa::Bool>& v)   {itsRowFlag.reference(v);}
    
    const casa::Cube<casa::Bool>& flagCube() const         {return itsFlagCube;}
    void flagCube(const casa::Cube<casa::Bool>& v)    {itsFlagCube.reference(v);}
    
    const casa::Matrix<casa::Float>& imagingWeight() const {return itsImagingWeight;}
    void imagingWeight(const casa::Matrix<casa::Float>&  v)  {itsImagingWeight.reference(v);}
    
    const casa::Cube<casa::Float>& imagingWeightCube() const {return itsImagingWeightCube;}
    void imagingWeightCube(const casa::Cube<casa::Float>&  v)  {itsImagingWeightCube.reference(v);}
    
    casa::Cube<casa::Complex>& visCube()        {return itsVisCube;}
    const casa::Cube<casa::Complex>& visCube() const       {return itsVisCube;}
    void visCube(casa::Cube<casa::Complex>& viscube) {itsVisCube.reference(viscube);}

    casa::Cube<casa::Complex>& modelVisCube() {return itsModelVisCube;}
    void modelVisCube(casa::Cube<casa::Complex>& modelviscube)    {itsModelVisCube.reference(modelviscube);}

    const casa::Vector<casa::Double>& freq() const         {return itsFreq;}
    void freq(const casa::Vector<casa::Double>& v)    {itsFreq.reference(v);}

    void reference(const VBStore& other)
    {
      itsNRow = other.itsNRow;  
      itsBeginRow = other.itsBeginRow; 
      itsEndRow = other.itsEndRow;
      itsDoPSF = other.itsDoPSF;

      itsSelection.reference(other.itsSelection);
      itsUVW.reference(other.itsUVW);
      itsRowFlag.reference(other.itsRowFlag);
      itsFlagCube.reference(other.itsFlagCube);
      itsImagingWeight.reference(other.itsImagingWeight);
      itsFreq.reference(other.itsFreq);
      itsVisCube.reference(other.itsVisCube);
      itsModelVisCube.reference(other.itsModelVisCube);
    }

  private:
    
    casa::Int itsNRow;
    casa::Int itsBeginRow;
    casa::Int itsEndRow;
    casa::Matrix<casa::Double> itsUVW;
    casa::Vector<casa::uInt> itsSelection;
    casa::Vector<casa::Bool> itsRowFlag;
    casa::Cube<casa::Bool> itsFlagCube;
    casa::Matrix<casa::Float> itsImagingWeight;
    casa::Cube<casa::Float> itsImagingWeightCube;
    casa::Cube<casa::Complex> itsVisCube;
    casa::Cube<casa::Complex> itsModelVisCube;
    casa::Vector<casa::Double> itsFreq;
    casa::Bool itsDoPSF;
  };

} // end namespace LofarFT
} // end namespace LOFAR

#endif
