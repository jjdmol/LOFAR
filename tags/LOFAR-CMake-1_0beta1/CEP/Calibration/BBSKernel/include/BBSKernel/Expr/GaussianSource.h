//# GaussianSource.h: Class holding the expressions defining a gaussian
//# source.
//#
//# Copyright (C) 2002
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

#ifndef LOFAR_BBSKERNEL_EXPR_GAUSSIANSOURCE_H
#define LOFAR_BBSKERNEL_EXPR_GAUSSIANSOURCE_H

// \file
// Class holding the expressions defining a gaussian source.

#include <BBSKernel/Expr/Source.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class GaussianSource: public Source
{
public:
    typedef shared_ptr<GaussianSource>       Ptr;
    typedef shared_ptr<const GaussianSource> ConstPtr;

    GaussianSource(const string &name,
        const Expr<Vector<2> >::ConstPtr &position,
        const Expr<Vector<4> >::ConstPtr &stokes,
        const Expr<Vector<2> >::ConstPtr &dimensions,
        const Expr<Scalar>::ConstPtr &orientation);

    Expr<Vector<4> >::ConstPtr getStokesVector() const;
    Expr<Vector<2> >::ConstPtr getDimensions() const;
    Expr<Scalar>::ConstPtr getOrientation() const;

private:
    Expr<Vector<4> >::ConstPtr  itsStokesVector;
    Expr<Vector<2> >::ConstPtr  itsDimensions;
    Expr<Scalar>::ConstPtr      itsOrientation;
};

// @}

// -------------------------------------------------------------------------- //
// - GaussianSource implementation                                          - //
// -------------------------------------------------------------------------- //

inline Expr<Vector<4> >::ConstPtr GaussianSource::getStokesVector() const
{
    return itsStokesVector;
}

inline Expr<Vector<2> >::ConstPtr GaussianSource::getDimensions() const
{
    return itsDimensions;
}

inline Expr<Scalar>::ConstPtr GaussianSource::getOrientation() const
{
    return itsOrientation;
}

} // namespace BBS
} // namespace LOFAR

#endif
