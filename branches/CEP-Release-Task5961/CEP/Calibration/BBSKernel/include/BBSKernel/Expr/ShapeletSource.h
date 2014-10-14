//# ShapeletSource.h: Class holding the expressions defining a shapelet source.
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
//# $Id: ShapeletSource.h 16068 2010-07-23 19:53:58Z zwieten $

#ifndef LOFAR_BBSKERNEL_EXPR_SHAPELETSOURCE_H
#define LOFAR_BBSKERNEL_EXPR_SHAPELETSOURCE_H

// \file
// Class holding the expressions defining a shapelet source.

#include <BBSKernel/Expr/Source.h>
#include <Common/lofar_complex.h>

#include <casa/Arrays.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class ShapeletSource: public Source
{
public:
    typedef shared_ptr<ShapeletSource>       Ptr;
    typedef shared_ptr<const ShapeletSource> ConstPtr;

    ShapeletSource(const SourceInfo &source, Scope &scope);

    Expr<JonesMatrix>::Ptr coherence(const Expr<Vector<3> >::ConstPtr &uvwLHS,
        const Expr<Vector<3> >::ConstPtr &uvwRHS) const;

private:
    double                  itsShapeletScaleI;
    double                  itsShapeletScaleQ;
    double                  itsShapeletScaleU;
    double                  itsShapeletScaleV;

    casa::Array<double>     itsShapeletCoeffI;  // shapelet coefficients I-flux
    casa::Array<double>     itsShapeletCoeffQ;
    casa::Array<double>     itsShapeletCoeffU;
    casa::Array<double>     itsShapeletCoeffV;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
