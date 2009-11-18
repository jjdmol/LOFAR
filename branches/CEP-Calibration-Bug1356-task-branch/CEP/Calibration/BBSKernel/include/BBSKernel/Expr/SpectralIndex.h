//# SpectralIndex.h: Frequency dependent flux.
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_BBSKERNEL_EXPR_SPECTRALINDEX_H
#define LOFAR_BBSKERNEL_EXPR_SPECTRALINDEX_H

// \file
// Frequency dependent flux.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class SpectralIndex: public Expr<Scalar>
{
public:
    typedef shared_ptr<SpectralIndex>       Ptr;
    typedef shared_ptr<const SpectralIndex> ConstPtr;

    template <typename T_ITER>
    SpectralIndex(const Expr<Scalar>::ConstPtr &refFreq,
        const Expr<Scalar>::ConstPtr &refStokes, T_ITER first, T_ITER last);

    virtual ~SpectralIndex();

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    virtual const Scalar evaluateExpr(const Request &request, Cache &cache)
        const;

    virtual const Scalar::View evaluateImpl(const Request &request,
        const Scalar::View &refFreq, const Scalar::View &refStokes,
        const vector<Scalar::View> &coeff) const;

private:
    Expr<Scalar>::ConstPtr          itsRefFreq;
    Expr<Scalar>::ConstPtr          itsRefStokes;
    vector<Expr<Scalar>::ConstPtr>  itsCoeff;
};

// @}

// -------------------------------------------------------------------------- //
// - SpectralIndex implementation                                           - //
// -------------------------------------------------------------------------- //

template <typename T_ITER>
SpectralIndex::SpectralIndex(const Expr<Scalar>::ConstPtr &refFreq,
    const Expr<Scalar>::ConstPtr &refStokes, T_ITER first, T_ITER last)
    :   itsRefFreq(refFreq),
        itsRefStokes(refStokes),
        itsCoeff(first, last)
{
    connect(itsRefFreq);
    connect(itsRefStokes);
    for(unsigned int i = 0; i < itsCoeff.size(); ++i)
    {
        connect(itsCoeff[i]);
    }
}


} //# namespace BBS
} //# namespace LOFAR

#endif
