//# ShapeletSource.cc: Class holding the expressions defining a gauss source
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
//# $Id: ShapeletSource.cc 14789 2010-01-13 12:39:15Z zwieten $

#include <lofar_config.h>
#include <BBSKernel/Expr/ShapeletSource.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/ShapeletCoherence.h>
#include <BBSKernel/Expr/Scope.h>

#include <Common/lofar_sstream.h>
#include <ParmDB/SourceInfo.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
namespace BBS
{

ShapeletSource::ShapeletSource(const SourceInfo &source, Scope &scope)
    :   Source(source, scope)
{
    ASSERT(source.getType() == SourceInfo::SHAPELET);

    itsShapeletScaleI=source.getShapeletScaleI();
    itsShapeletScaleQ=source.getShapeletScaleQ();
    itsShapeletScaleU=source.getShapeletScaleU();
    itsShapeletScaleV=source.getShapeletScaleV();

    itsShapeletCoeffI=source.getShapeletCoeffI();
    itsShapeletCoeffQ=source.getShapeletCoeffQ();
    itsShapeletCoeffU=source.getShapeletCoeffU();
    itsShapeletCoeffV=source.getShapeletCoeffV();
}

Expr<JonesMatrix>::Ptr
ShapeletSource::coherence(const Expr<Vector<3> >::ConstPtr &uvwLHS,
    const Expr<Vector<3> >::ConstPtr &uvwRHS) const
{
    return Expr<JonesMatrix>::Ptr(new ShapeletCoherence(stokes(),
        itsShapeletScaleI, itsShapeletCoeffI, uvwLHS, uvwRHS));
}

} // namespace BBS
} // namespace LOFAR
