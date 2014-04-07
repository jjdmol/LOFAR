//# VisResampler.h: Convolutional AW resampler for LOFAR data
//# Copyright (C) 2011
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

#ifndef LOFAR_LOFARFT_VISRESAMPLER_H
#define LOFAR_LOFARFT_VISRESAMPLER_H

#include <synthesis/TransformMachines/AWVisResampler.h>
#include <LofarFT/CFStore.h>
#include <LofarFT/VBStore.h>
//added
#include <LofarFT/ATerm.h>
#include <LofarFT/WTerm.h>
#include <LofarFT/CFStore.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/ArrayMath.h>
#include <images/Images/PagedImage.h>
#include <casa/Utilities/Assert.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaParse.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <measures/Measures/MeasTable.h>

#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <stdio.h>
#include <stdlib.h>
#include <casa/vector.h>
#include <casa/OS/Directory.h>

//=========

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
namespace LofarFT {
  
class VisResampler: public casa::AWVisResampler
{
public:
  VisResampler(): AWVisResampler()  {}
  virtual ~VisResampler()                                    {}

  virtual VisibilityResamplerBase* clone()
  {return new VisResampler(*this);}

  void copy(const VisResampler& other)
  {AWVisResampler::copy(other); }
  
  void set_chan_map(const casa::Vector<casa::Int> &map);

  void set_chan_map_CF(const casa::Vector<casa::Int> &map);

  void set_pol_map(const casa::Vector<casa::Int> &map);
  
  // Re-sample the griddedData on the VisBuffer (a.k.a gridding).
  virtual void DataToGrid (
    casa::Array<casa::Complex>& griddedData, 
    VBStore& vbs,
    const casa::Vector<casa::uInt>& rows,
    casa::Int rbeg, 
    casa::Int rend,
    casa::Matrix<casa::Double>& sumwt,
    const casa::Bool& dopsf, 
    CFStore& cfs)
  {
    DataToGridImpl_p(griddedData, vbs, rows, rbeg, rend, sumwt,dopsf,cfs);
  }
  
  virtual void DataToGrid (
    casa::Array<casa::DComplex>& griddedData, 
    VBStore& vbs,
    const casa::Vector<casa::uInt>& rows,
    casa::Int rbeg, 
    casa::Int rend,
    casa::Matrix<casa::Double>& sumwt,
    const casa::Bool& dopsf, 
    CFStore& cfs)
  {
    DataToGridImpl_p(griddedData, vbs, rows, rbeg, rend, sumwt,dopsf,cfs);
  }

  virtual void GridToData(
    VBStore& vbs,
    const casa::Array<casa::Complex>& grid,
    const casa::Vector<casa::uInt>& rows,
    casa::Int rbeg, 
    casa::Int rend,
    CFStore& cfs);

  casa::Vector<casa::uInt> ChanCFMap;
  
  void ComputeResiduals(VBStore& vbs);

  void sgrid(
    casa::Vector<casa::Double>& pos, 
    casa::Vector<casa::Int>& loc,
    casa::Vector<casa::Int>& off, 
    casa::Complex& phasor,
    const casa::Int& irow, 
    const casa::Matrix<casa::Double>& uvw,
    const casa::Double& dphase, 
    const casa::Double& freq,
    const casa::Vector<casa::Double>& scale,
    const casa::Vector<casa::Double>& offset,
    const casa::Vector<casa::Float>& sampling);

protected:
  casa::Vector<casa::Int> itsChanMap;
  casa::Vector<casa::Int> itsChanMapCF;
  casa::Vector<casa::Int> itsPolMap;
  
private:
  
  // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
  //
  template <class T>
  void DataToGridImpl_p(casa::Array<T>& griddedData, VBStore& vb,
                        const casa::Vector<casa::uInt>& rows,
                        casa::Int rbeg, casa::Int rend,
                        casa::Matrix<casa::Double>& sumwt,const casa::Bool& dopsf,
                        CFStore& cfs);

  casa::Vector<casa::Int> cfMap_p, conjCFMap_p;

};

} // end namespace LofarFT
} // end namespace LOFAR

#endif //
