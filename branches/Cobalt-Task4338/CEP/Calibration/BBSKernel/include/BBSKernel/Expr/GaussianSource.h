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

    GaussianSource(const SourceInfo &source, Scope &scope);

    Expr<JonesMatrix>::Ptr coherence(const Expr<Vector<3> >::ConstPtr &uvwLHS,
        const Expr<Vector<3> >::ConstPtr &uvwRHS) const;

private:
    Expr<Vector<2> >::Ptr   itsDimensions;
    Expr<Scalar>::Ptr       itsOrientation;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
