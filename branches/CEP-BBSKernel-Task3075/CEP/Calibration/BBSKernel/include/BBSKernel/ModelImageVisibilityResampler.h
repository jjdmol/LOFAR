//# ModelImageVisibilityResampler.h: 
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
//# $Id: ModelImageVisibilityResampler.h 20029 2012-04-13 15:07:23Z duscha $

#ifndef LOFAR_BBSKERNEL_MODELIMAGEVISIBILITYRESAMPLER_H 
#define LOFAR_BBSKERNEL_MODELIMAGEVISIBILITYRESAMPLER_H

//#include <BBSKernel/ModelImageFft.h>
#include <BBSKernel/ModelImageCFStore.h>
#include <casa/Arrays/Matrix.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

namespace LOFAR {   //# NAMESPACE LOFAR BEGIN
namespace BBS {     //# NAMESPACE BBS BEBGIN

class ModelImageVisibilityResampler
{
public:
//  friend class ModelImageFft;

  ModelImageVisibilityResampler();
  ModelImageVisibilityResampler(const vector<double> &frequencies);
  ~ModelImageVisibilityResampler();

//  void DataToGrid(casa::Array<casa::DComplex>& griddedData, casa::VBStore& vbs, 
//                  casa::Matrix<casa::Double>& sumwt, const casa::Bool& dopsf);
  void DataToGrid(casa::Array<casa::DComplex>& griddedData);

  void setParams( const casa::Vector<casa::Double>& uvwScale, 
                  const casa::Vector<casa::Double>& offset,
                  const casa::Vector<casa::Double>& dphase)
  {
    uvwScale_p.reference(uvwScale);
    offset_p.reference(offset);
    dphase_p.reference(dphase);
  };

  void setMaps( const casa::Vector<casa::Int>& chanMap, 
                const casa::Vector<casa::Int>& polMap)
  {
    chanMap_p.reference(chanMap);
    polMap_p.reference(polMap);
  }

  virtual void setConvFunc(const CFStore& cfs) 
  {
    convFuncStore_p = cfs;
  };
  
  void setCFMaps( const casa::Vector<casa::Int>& cfMap, 
                  const casa::Vector<casa::Int>& conjCFMap);

  void sgrid( Int& ndim, 
    	        Double* __restrict__  pos, 
	            Int* __restrict__  loc, 
	            Int* __restrict__  off, 
    	        Complex& phasor, const Int& irow, 
	            const Double* __restrict__  uvw, 
    	        const Double& dphase, const Double& freq, 
    	        const Double* __restrict__  scale, 
	            const Double* __restrict__  offset,
    	        const Float* __restrict__  sampling);

protected:
  vector<double> itsFrequencies;
  SpectralCoordinate spectralCoord_p;
  casa::Vector<casa::Double> uvwScale_p, offset_p, dphase_p;
  CFStore convFuncStore_p;
  casa::Vector<casa::Int> chanMap_p, polMap_p;
  Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny;
};

}  //# NAMESPACE LOFAR END
}  //# NAMESPACE BBS END
#endif