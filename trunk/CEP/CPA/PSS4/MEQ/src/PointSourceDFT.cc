//# PointSourceDFT.cc: The point source DFT component for a station
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <MEQ/PointSourceDFT.h>
#include <aips/Mathematics/Constants.h>

namespace Meq {    

using namespace VellsMath;

const HIID child_labels[] = { AidSt|AidDFT|1,AidSt|AidDFT|2,AidN };
const int num_children = sizeof(child_labels)/sizeof(child_labels[0]);

PointSourceDFT::PointSourceDFT()
: Function(num_children,child_labels)
{}

PointSourceDFT::~PointSourceDFT()
{}

int PointSourceDFT::getResult (Result::Ref &resref, 
                               const std::vector<Result::Ref> &childres,
                               const Request &request, bool newreq)
{
  const int expect_nvs[] = {2,2,1};
  // Check that child results are all OK (no fails, expected # of vellsets per child)
  string fails;
  for( int i=0; i<num_children; i++ )
  {
    int nvs = childres[i]->numVellSets();
    if( nvs != expect_nvs[i] )
      Debug::appendf(fails,"child %s: expecting %d VellsSets, got %d;",
          child_labels[i].toString().c_str(),expect_nvs[i],nvs);
    if( childres[i]->hasFails() )
      Debug::appendf(fails,"child %s: has fails",child_labels[i].toString().c_str());
  }
  if( !fails.empty() )
    NodeThrow1(fails);
  // Get F0 and DF of S1 and S2.
  const Vells& vs1f0 = childres[0]->vellSet(0).getValue();
  const Vells& vs1df = childres[0]->vellSet(1).getValue();
  const Vells& vs2f0 = childres[1]->vellSet(0).getValue();
  const Vells& vs2df = childres[1]->vellSet(1).getValue();
  const Vells& vn    = childres[2]->vellSet(0).getValue();
  // For the time being we only support 1 frequency range.
  const Cells& cells = request.cells();
  Assert (cells.numSegments(FREQ) == 1);
  // Allocate a 1-plane result.
  Result &result = resref <<= new Result(1,request);
  // create a vellset for each plane.
  VellSet &vellset = result.setNewVellSet(0,0,0);

  // It is tried to compute the DFT as efficient as possible.
  // Therefore the baseline contribution is split into its antenna parts.
  // dft = exp(2i.pi(ul+vm+wn)) / n                 (with u,v,w in wavelengths)
  //     = (exp(2i.pi((u1.l+v1.m+w1.n) - (u2.l+v2.m+w2.n))/wvl))/n (u,v,w in m)
  //     = ((exp(i(u1.l+v1.m+w1.n)) / exp(i(u2.l+v2.m+w2.m))) ^ (2.pi/wvl)) / n
  // So left and right return the exp values independent of wavelength.
  // Thereafter they are scaled to the freq domain by raising the values
  // for each time to the appropriate powers.
  // Alas the rule
  //   x^(a*b) = (x^a)^b
  // which is valid for real numbers, is only valid for complex numbers
  // if b is an integer number.
  // Therefore the station calculations (in MeqStatSources) are done as
  // follows, where it should be noted that the frequencies are regularly
  // strided.
  //  f = f0 + k.df   (k = 0 ... nchan-1)
  //  s1 = (u1.l+v1.m+w1.n).2i.pi/c
  //  s2 = (u2.l+v2.m+w2.n).2i.pi/c
  //  dft = exp(s1(f0+k.df)) / exp(s2(f0+k.df)) / n
  //      = (exp(s1.f0)/exp(s2.f0)) . (exp(s1.k.df)/exp(s2.k.df)) / n
  //      = (exp(s1.f0)/exp(s2.f0)) . (exp(s1.df)/exp(s2.df))^k / n
  // In principle the power is expensive, but because the frequencies are
  // regularly strided, it is possible to use multiplication.
  // So it gets
  // dft(f0) = (exp(s1.f0)/exp(s2.f0)) / n
  // dft(fj) = dft(fj-1) * (exp(s1.df)/exp(s2.df))
  // Using a python script (tuvw.py) is is checked that this way of
  // calculation is accurate enough.
  // Another optimization can be achieved in the division of the two
  // complex numbers which can be turned into a cheaper multiplication.
  //  exp(x)/exp(y) = (cos(x) + i.sin(x)) / (cos(y) + i.sin(y))
  //                = (cos(x) + i.sin(x)) * (cos(y) - i.sin(y))

  const complex<double>* tmpl = vs1f0.complexStorage();
  const complex<double>* tmpr = vs2f0.complexStorage();
  const complex<double>* deltal = vs1df.complexStorage();
  const complex<double>* deltar = vs2df.complexStorage();
  const double* tmpnk = vn.realStorage();

  // Assume that frequency is the first axis.
  Assert(FREQ==0);

  // Set the type and shape of the result value.
  int ntime = cells.ncells(TIME);
  int nfreq = cells.ncells(FREQ);
  LoMat_dcomplex& vells = vellset.setComplex (nfreq, ntime);
  dcomplex* resdata = vells.data();

  // vn can be a scalar or an array (in time axis), so set its step to 0
  // if it is a scalar.
  int stepnk = (vn.ny() > 1  ?  1 : 0);
  int nki = 0;
  for (int i=0; i<ntime; i++) {
    dcomplex val0 = tmpr[i] * conj(tmpl[i]) / tmpnk[nki];
    nki += stepnk;
    *resdata++ = val0;
    if (nfreq > 1) {
      dcomplex dval = deltar[i] * conj(deltal[i]);
      for (int j=1; j<nfreq; j++) {
        val0 *= dval;
        *resdata++ = val0;
      }
    }
  }
  return 0;
}

} // namespace Meq
