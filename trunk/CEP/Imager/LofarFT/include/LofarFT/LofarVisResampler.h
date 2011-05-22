//# LofarVisResampler.h: Convolutional AW resampler for LOFAR data
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

#ifndef LOFARFT_LOFARVISRESAMPLER_H
#define LOFARFT_LOFARVISRESAMPLER_H

#include <synthesis/MeasurementComponents/AWVisResampler.h>
#include <LofarFT/LofarCFStore.h>
#include <LofarFT/LofarVBStore.h>

using namespace casa;

namespace LOFAR { //# NAMESPACE CASA - BEGIN

  class LofarVisResampler: public AWVisResampler
  {
  public: 
    LofarVisResampler(): AWVisResampler()  {}
    LofarVisResampler(const CFStore& cfs): AWVisResampler(cfs)      {}
    virtual ~LofarVisResampler()                                    {}

    virtual VisibilityResamplerBase* clone()
    {return new LofarVisResampler(*this);}
    
    void copy(const LofarVisResampler& other) 
    {AWVisResampler::copy(other); }

    // Re-sample the griddedData on the VisBuffer (a.k.a gridding).
    void lofarDataToGrid (Array<Complex>& griddedData, LofarVBStore& vbs,
                          const Vector<uInt>& rows,
                          Matrix<Double>& sumwt,
                          const Bool& dopsf, LofarCFStore& cfs)
      {DataToGridImpl_p(griddedData, vbs, rows, sumwt,dopsf,cfs);}
    void lofarDataToGrid (Array<DComplex>& griddedData, LofarVBStore& vbs,
                          const Vector<uInt>& rows,
                          Matrix<Double>& sumwt,
                          const Bool& dopsf, LofarCFStore& cfs)
      {DataToGridImpl_p(griddedData, vbs, rows, sumwt,dopsf,cfs);}

    void lofarComputeResiduals(LofarVBStore& vbs);

  private:
    // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
    //
    template <class T>
    void DataToGridImpl_p(Array<T>& griddedData, LofarVBStore& vb,
                          const Vector<uInt>& rows,
			  Matrix<Double>& sumwt,const Bool& dopsf,
                          LofarCFStore& cfs);

  };

} //# NAMESPACE CASA - END

#endif // 
