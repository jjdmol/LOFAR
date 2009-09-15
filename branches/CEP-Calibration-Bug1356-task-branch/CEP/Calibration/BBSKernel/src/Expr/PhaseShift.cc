//# PhaseShift.cc: Phase delay due to baseline geometry with respect to a
//#     direction on the sky.
//#
//# Copyright (C) 2009
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

#include <lofar_config.h>

#include <BBSKernel/Expr/PhaseShift.h>
//#include <BBSKernel/Expr/MatrixTmp.h>
//#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{

PhaseShift::PhaseShift(const Expr<Vector<2> >::ConstPtr &lhs,
    const Expr<Vector<2> >::ConstPtr &rhs)
    :   BasicBinaryExpr<Vector<2>, Vector<2>, Scalar>(lhs, rhs)
{
}

// Compute the baseline phase shift for a specific direction on the sky.
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
// Therefore the station calculations (in StatSources) are done as
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
// exp(x)/exp(y) = (cos(x) + i.sin(x)) / (cos(y) + i.sin(y))
//               = (cos(x) + i.sin(x)) * (cos(y) - i.sin(y))
const Scalar::View PhaseShift::evaluateImpl(const Request &request,
    const Vector<2>::View &lhs, const Vector<2>::View &rhs) const
{
    Scalar::View result;

    size_t nChannels = request[FREQ]->size();
    size_t nTimeslots = request[TIME]->size();

    Matrix shift(makedcomplex(0.0, 0.0), nChannels, nTimeslots, false);
    for(size_t ts = 0; ts < nTimeslots; ++ts)
    {
        dcomplex stationA = lhs(0).getDComplex(0, ts);
        dcomplex stationB = rhs(0).getDComplex(0, ts);

        dcomplex delta = makedcomplex(0.0, 0.0);
        if(nChannels > 1)
        {
            dcomplex deltaA = lhs(1).getDComplex(0, ts);
            dcomplex deltaB = rhs(1).getDComplex(0, ts);
            delta = deltaB * conj(deltaA);
        }

        shift.fillRowWithProducts(stationB * conj(stationA), delta, ts);
    }

    result.assign(shift);
    return result;
}

// -------------------------------------------------------------------------- //

//PhaseShift::PhaseShift()
//    :   ExprStatic<PhaseShift::N_Inputs>()
//{
//}

//ValueSet::ConstPtr PhaseShift::evaluateImpl(const Request &request,
//    const ValueSet::ConstPtr (&inputs)[PhaseShift::N_Inputs]) const
//{
//    // Let (u . l) = u * l + v * m + w * (n - 1.0), then:
//    //
//    // shift = exp(2.0 * pi * i * (u . l) * f / c)

//    ValueSet::ConstPtr uvw = inputs[UVW];
//    ValueSet::ConstPtr lmn = inputs[LMN];

//    Matrix phase = casa::C::_2pi
//        * (uvw->value(0) * lmn->value(0)
//        + uvw->value(1) * lmn->value(1)
//        + uvw->value(2) * (lmn->value(2) - 1.0));
//
//    ValueSet::Ptr result(new ValueSet());
//    result->assign(tocomplex(cos(phase), sin(phase)));
//    return result;

////    ASSERT(request[FREQ]->isRegular());
////    const real_t l = lmn->value(0)(0, 0);
////    const real_t m = lmn->value(1)(0, 0);
////    const real_t n = lmn->value(2)(0, 0) - 1.0;
////
////    ARRAY(real_t) inner(uvw->value(0) * l
////        + uvw->value(1) * m
////        + uvw->value(2) * n);

////    const int nFreq = (int) request[FREQ]->size();
////    const int nTime = (int) request[TIME]->size();

////    const double scale0 = casa::C::_2pi * request[FREQ]->center(0) / casa::C::c;
////    const double scaleDelta = casa::C::_2pi * request[FREQ]->width(0) / casa::C::c;
////
////    ResultType::Ptr result(new ResultType());
////    ARRAY(complex_t) shift(nTime, nFreq);
////    complex_t *data = shift.data();
////
////    for(int t = 0; t < nTime; ++t)
////    {
////        const double phase0 = scale0 * inner(0, t);
////        const double phaseDelta = scaleDelta * inner(0, t);
////        complex_t curr(cos(phase0), sin(phase0));
////        const complex_t delta(cos(phaseDelta), sin(phaseDelta));

////        for(int f = 0; f < nFreq; ++f)
////        {
////            *data++ = curr;
////            curr *= delta;
////        }
////    }
////
////    result->assign(shift);
////    return result;
//}

} // namespace BBS
} // namespace LOFAR
