//# MIM.h: Ionospheric disturbance of a (source, station) combination.
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BBSKERNEL_EXPR_MIM_H
#define LOFAR_BBSKERNEL_EXPR_MIM_H

// \file
// Ionospheric disturbance of a (source, station) combination.

#include <BBSKernel/Expr/Expr.h>

#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasConvert.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class MIM: public Expr<Scalar>
{
public:
    typedef shared_ptr<MIM>         Ptr;
    typedef shared_ptr<const MIM>   ConstPtr;

    template <typename T_ITER>
    MIM(const casa::MPosition &refStation, const Expr<Vector<4> >::ConstPtr &pp,
        T_ITER first, T_ITER last);

    virtual ~MIM();

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    virtual const Scalar evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;

    const Scalar::View evaluateImpl(const Grid &grid,
        const Vector<4>::View &pp, const vector<Scalar::View> &coeff) const;

private:
    casa::MPosition                 itsRefStation;
    Expr<Vector<4> >::ConstPtr      itsPiercePoint;
    vector<Expr<Scalar>::ConstPtr>  itsCoeff;
};

// @}

// -------------------------------------------------------------------------- //
// - MIM implementation                                                     - //
// -------------------------------------------------------------------------- //

template <typename T_ITER>
MIM::MIM(const casa::MPosition &refStation,
    const Expr<Vector<4> >::ConstPtr &pp, T_ITER first, T_ITER last)
    :   itsRefStation(casa::MPosition::Convert(refStation,
            casa::MPosition::ITRF)()),
        itsPiercePoint(pp),
        itsCoeff(first, last)
{
    connect(itsPiercePoint);
    for(unsigned int i = 0; i < itsCoeff.size(); ++i)
    {
        connect(itsCoeff[i]);
    }
}

} // namespace BBS
} // namespace LOFAR

#endif
