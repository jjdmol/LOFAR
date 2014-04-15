//# VisResamplerMatrix.h: Convolutional AW resampler for LOFAR data
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
//# $Id: VisResampler.h 28512 2014-03-05 01:07:53Z vdtol $

#ifndef LOFAR_LOFARFT_VISRESAMPLERMATRIX_H
#define LOFAR_LOFARFT_VISRESAMPLERMATRIX_H

#include <LofarFT/VisResampler.h>

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN
namespace LofarFT {
  
class VisResamplerMatrix: public VisResampler
{
public:
  VisResamplerMatrix(): VisResampler()  {}
  virtual ~VisResamplerMatrix()                                    {}

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

private:
  
  // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
  //
  template <class T>
  void DataToGridImpl_p(
    casa::Array<T>& griddedData, VBStore& vb,
    const casa::Vector<casa::uInt>& rows,
    casa::Int rbeg, 
    casa::Int rend,
    casa::Matrix<casa::Double>& sumwt,
    const casa::Bool& dopsf,
    CFStore& cfs);

};

} // end namespace LofarFT
} // end namespace LOFAR

#endif //
