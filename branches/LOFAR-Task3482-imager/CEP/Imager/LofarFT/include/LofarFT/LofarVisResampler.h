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
//added
#include <LofarFT/LofarATerm.h>
#include <LofarFT/LofarWTerm.h>
#include <LofarFT/LofarCFStore.h>

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
                          Int rbeg, Int rend,
                          Matrix<Double>& sumwt,
                          const Bool& dopsf, LofarCFStore& cfs)
    {DataToGridImpl_p(griddedData, vbs, rows, rbeg, rend, sumwt,dopsf,cfs);}
    void lofarDataToGrid (Array<DComplex>& griddedData, LofarVBStore& vbs,
                          const Vector<uInt>& rows,
                          Int rbeg, Int rend,
                          Matrix<Double>& sumwt,
                          const Bool& dopsf, LofarCFStore& cfs)
    {DataToGridImpl_p(griddedData, vbs, rows, rbeg, rend, sumwt,dopsf,cfs);}

    //Linear interpolation tries
    void lofarDataToGrid_linear (Array<Complex>& griddedData, LofarVBStore& vbs,
                          const Vector<uInt>& rows,
                          Int rbeg, Int rend,
                          Matrix<Double>& sumwt,
                          const Bool& dopsf, LofarCFStore& cfs)
    {DataToGridImpl_linear_p(griddedData, vbs, rows, rbeg, rend, sumwt,dopsf,cfs);}
    void lofarDataToGrid_linear (Array<DComplex>& griddedData, LofarVBStore& vbs,
                          const Vector<uInt>& rows,
                          Int rbeg, Int rend,
                          Matrix<Double>& sumwt,
                          const Bool& dopsf, LofarCFStore& cfs)
    {DataToGridImpl_linear_p(griddedData, vbs, rows, rbeg, rend, sumwt,dopsf,cfs);}

    //End Linear interpolation tries


    void lofarGridToData(LofarVBStore& vbs,
                         const Array<Complex>& grid,
                         const Vector<uInt>& rows,
                         Int rbeg, Int rend,
                         LofarCFStore& cfs);

    template <class T>
    void lofarDataToGrid_interp(Array<T>& grid,  LofarVBStore& vbs,
				const Vector<uInt>& rows,
				Matrix<Double>& sumwt,
				const Bool& dopsf,
				LofarCFStore& cfs);//,
				//vector<Float> wvec, Float wStep, Float wcf, vector<Complex> vecCorr);

    void lofarGridToData_interp(LofarVBStore& vbs,
                         const Array<Complex>& grid,
                         const Vector<uInt>& rows,
                         //Int rbeg, Int rend,
				LofarCFStore& cfs);//,
				//vector<Float> wvec, Float wStep, Float wcf, vector<Complex> vecCorr);

    void lofarGridToData_linear(LofarVBStore& vbs,
                         const Array<Complex>& grid,
                         const Vector<uInt>& rows,
                         Int rbeg, Int rend,
                         LofarCFStore& cfs0,
                         LofarCFStore& cfs1);

    Vector<uInt> ChanCFMap;
    void setChanCFMaps(Vector<uInt> ChanMap)
    {ChanCFMap=ChanMap.copy();}

    virtual void setCFMaps(const Vector<Int>& cfMap, const Vector<Int>& conjCFMap)
    {cfMap_p.assign(cfMap); conjCFMap_p.assign(conjCFMap);}

    void lofarComputeResiduals(LofarVBStore& vbs);

  void sgrid(Vector<Double>& pos, Vector<Int>& loc,
			     Vector<Int>& off, Complex& phasor,
			     const Int& irow, const Matrix<Double>& uvw,
			     const Double& dphase, const Double& freq,
			     const Vector<Double>& scale,
			     const Vector<Double>& offset,
                                const Vector<Float>& sampling);

    /*
  template <class T>
    void store2(const Matrix<T> &data, const string &name)
    {
      CoordinateSystem csys;

      Matrix<Double> xform(2, 2);
      xform = 0.0;
      xform.diagonal() = 1.0;
      Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
      Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
      Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
      csys.addCoordinate(DirectionCoordinate(MDirection::J2000, Projection(Projection::SIN),
					     refLatLon, refLatLon, incLon, incLat,
					     xform, data.shape()(0) / 2, data.shape()(1) / 2));

      Vector<Int> stokes(1);
      stokes(0) = Stokes::I;
      csys.addCoordinate(StokesCoordinate(stokes));
      csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));

      PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 1, 1)), csys, name);
      im.putSlice(data, IPosition(4, 0, 0, 0, 0));
    };
    */

  private:
    // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
    //
    template <class T>
    void DataToGridImpl_p(Array<T>& griddedData, LofarVBStore& vb,
                          const Vector<uInt>& rows,
                          Int rbeg, Int rend,
			  Matrix<Double>& sumwt,const Bool& dopsf,
                          LofarCFStore& cfs);

    template <class T>
    void DataToGridImpl_linear_p(Array<T>& griddedData, LofarVBStore& vb,
                          const Vector<uInt>& rows,
                          Int rbeg, Int rend,
			  Matrix<Double>& sumwt,const Bool& dopsf,
                          LofarCFStore& cfs);


    Vector<Int> cfMap_p, conjCFMap_p;
  };

} //# NAMESPACE CASA - END

#endif //
