//# LofarVisResampler.cc: Implementation of the LofarVisResampler class
//#
//# Copyright (C) 2011
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
//# $Id$

#include <lofar_config.h>
#include <LofarFT/VisResampler.h>
#include <synthesis/TransformMachines/Utils.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <cassert>
#include <Common/OpenMP.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {

void VisResampler::set_chan_map(const casa::Vector<casa::Int> &map)
{
  itsChanMap.resize();
  itsChanMap = map;
}

void VisResampler::set_chan_map_CF(const casa::Vector<casa::Int> &map)
{
  itsChanMapCF.resize();
  itsChanMapCF = map;
}

void VisResampler::ComputeResiduals(VBStore& vbs)
{
  Int rbeg = vbs.beginRow();
  Int rend = vbs.endRow();
  IPosition vbDataShape = vbs.modelVisCube().shape();
  IPosition start(vbDataShape);
  IPosition last(vbDataShape);
  start=0; start(2)=rbeg;
  last(2)=rend; //last=last-1;

  for(uInt ichan = start(0); ichan < last(0); ichan++)
    for(uInt ipol = start(1); ipol < last(1); ipol++)
      for(uInt irow = start(2); irow < last(2); irow++)
        vbs.modelVisCube()(ichan,ipol,irow) -= vbs.visCube()(ichan,ipol,irow);
}

void VisResampler::sgrid(
  Vector<Double>& pos, 
  Vector<Int>& loc,
  Vector<Int>& off, 
  Complex& phasor,
  const Int& irow, 
  const Matrix<Double>& uvw,
  const Double&, 
  const Double& freq,
  const Vector<Double>& scale,
  const Vector<Double>& offset,
  const Vector<Float>& sampling)
{
  //Double phase;
  Vector<Double> uvw_l(3,0); // This allows gridding of weights
                              // centered on the uv-origin
  if (uvw.nelements() > 0) for(Int i=0;i<3;i++) uvw_l[i]=uvw(i,irow);

  pos(2)=0;//sqrt(abs(scale[2]*uvw_l(2)*freq/C::c))+offset[2];
  loc(2)=0;//SynthesisUtils::nint(pos[2]);
  off(2)=0;

  for(Int idim=0;idim<2;idim++)
    {
      pos[idim]=scale[idim]*uvw_l(idim)*freq/C::c+offset[idim];
      loc[idim]=SynthesisUtils::nint(pos[idim]);
      off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]);
    }

    phasor=Complex(1.0);
}


} // end namespace LofarFT
} // end namespace LOFAR
