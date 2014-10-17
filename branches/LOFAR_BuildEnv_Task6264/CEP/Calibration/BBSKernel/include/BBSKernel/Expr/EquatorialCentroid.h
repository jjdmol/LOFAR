//# EquatorialCentroid.h: Compute the centroid of a list of positions in
//# spherical coordinates.
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

#ifndef LOFAR_BBSKERNEL_EXPR_SPHERICALCENTROID_H
#define LOFAR_BBSKERNEL_EXPR_SPHERICALCENTROID_H

// \file
// Compute the centroid of a list of positions in spherical coordinates.

#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class EquatorialCentroid: public Expr<Vector<2> >
{
public:
    typedef shared_ptr<EquatorialCentroid>       Ptr;
    typedef shared_ptr<const EquatorialCentroid> ConstPtr;

    EquatorialCentroid();

    template <typename T_ITER>
    EquatorialCentroid(T_ITER first, T_ITER last);

    virtual ~EquatorialCentroid();

    void connect(const Expr<Vector<2> >::ConstPtr &arg);

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;

    virtual const Vector<2> evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;

    const Vector<2>::View evaluateImpl(const Grid &grid,
        const vector<Vector<2>::View> &args) const;

private:
    vector<Expr<Vector<2> >::ConstPtr>  itsArgs;
};

// @}

// -------------------------------------------------------------------------- //
// - EquatorialCentroid implementation                                       - //
// -------------------------------------------------------------------------- //

template <typename T_ITER>
EquatorialCentroid::EquatorialCentroid(T_ITER first, T_ITER last)
{
    for(; first != last; ++first)
    {
        connect(*first);
        itsArgs.push_back(*first);
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
