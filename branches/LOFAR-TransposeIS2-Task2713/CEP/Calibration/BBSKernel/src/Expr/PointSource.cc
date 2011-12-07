//# PointSource.cc: Class holding the expressions defining a point source.
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

#include <lofar_config.h>
#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/PointCoherence.h>
#include <BBSKernel/Expr/Scope.h>

#include <Common/lofar_sstream.h>
#include <ParmDB/SourceInfo.h>

namespace LOFAR
{
namespace BBS
{

PointSource::PointSource(const SourceInfo &source, Scope &scope)
    :   Source(source, scope)
{
    ASSERT(source.getType() == SourceInfo::POINT);
}

Expr<JonesMatrix>::Ptr
PointSource::coherence(const Expr<Vector<3> >::ConstPtr&,
    const Expr<Vector<3> >::ConstPtr&) const
{
    if(!itsCoherence)
    {
        itsCoherence = Expr<JonesMatrix>::Ptr(new PointCoherence(stokes()));
    }

    return itsCoherence;
}

} // namespace BBS
} // namespace LOFAR
