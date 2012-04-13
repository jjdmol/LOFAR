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

#include <casa/Arrays/Matrix.h>

namespace LOFAR {   //# NAMESPACE LOFAR BEGIN
namespace BBS {     //# NAMESPACE BBS BEBGIN
class ModelImageVisibilityResampler
{
public:

  ModelImageVisibilityResampler();
  ~ModelImageVisibilityResampler();

  void DataToGrid(Array<DComplex>& griddedData, VBStore& vbs, 
                  Matrix<Double>& sumwt, const Bool& dopsf);

  void DataToGrid(Array<Complex>& griddedData, VBStore& vbs, 
         			    Matrix<Double>& sumwt, const Bool& dopsf);

  void setParams(const Vector<Double>& uvwScale, const Vector<Double>& offset,
       const Vector<Double>& dphase)
  {
    // SynthesisUtils::SETVEC(uvwScale_p, uvwScale); 
    // SynthesisUtils::SETVEC(offset_p, offset);
    // SynthesisUtils::SETVEC(dphase_p, dphase);
    uvwScale_p.reference(uvwScale);
    offset_p.reference(offset);
    dphase_p.reference(dphase);
  };

  void setMaps(const Vector<Int>& chanMap, const Vector<Int>& polMap)
  {
    chanMap_p.reference(chanMap);
    polMap_p.reference(polMap);
  }

  virtual void setConvFunc(const CFStore& cfs) 
  {
    convFuncStore_p = cfs;
  };
  
  void setCFMaps(const Vector<Int>& cfMap, const Vector<Int>& conjCFMap);

private:

};

}  //# NAMESPACE LOFAR END
}  //# NAMESPACE BBS END
#endif