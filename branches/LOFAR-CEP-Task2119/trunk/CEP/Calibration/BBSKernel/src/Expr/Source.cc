//# Source.cc: Class holding the expressions defining a  source
//#
//# Copyright (C) 2006
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
#include <BBSKernel/Expr/Source.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/MeasurementExprLOFARUtil.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/ExprVisData.h>
#include <BBSKernel/Expr/GaussianSource.h>
#include <BBSKernel/Expr/ShapeletSource.h>
#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/Expr/Scope.h>

#include <ParmDB/SourceInfo.h>

namespace LOFAR
{
namespace BBS
{

Source::Ptr Source::create(const SourceInfo &source, Scope &scope)
{
    switch(source.getType())
    {
    case SourceInfo::POINT:
        return Source::Ptr(new PointSource(source, scope));
    case SourceInfo::GAUSSIAN:
        return Source::Ptr(new GaussianSource(source, scope));
    case SourceInfo::SHAPELET:
        return Source::Ptr(new ShapeletSource(source, scope));
    default:
        THROW(BBSKernelException, "Unsupported source type: "
            << source.getType() << " for source: " << source.getName());
    }
}

Source::Ptr Source::create(const string &name, const VisBuffer::Ptr &buffer)
{
    return Source::Ptr(new PrecomputedSource(name, buffer));
}

Source::Source(const string &name, const Expr<Vector<2> >::Ptr &position)
    :   itsName(name),
        itsPosition(position)
{
}

Source::Source(const SourceInfo &source, Scope &scope)
    :   itsName(source.getName())
{
    // Position.
    ExprParm::Ptr ra = scope(SKY, "Ra:" + name());
    ExprParm::Ptr dec = scope(SKY, "Dec:" + name());

    AsExpr<Vector<2> >::Ptr position(new AsExpr<Vector<2> >());
    position->connect(0, ra);
    position->connect(1, dec);
    itsPosition = position;
}

Source::~Source()
{
}

const string &Source::name() const
{
    return itsName;
}

Expr<Vector<2> >::Ptr Source::position() const
{
    return itsPosition;
}

PrecomputedSource::PrecomputedSource(const string &name,
    const VisBuffer::Ptr &buffer)
    :   Source(name, makeDirectionExpr(buffer->getPhaseReference())),
        itsBuffer(buffer)
{
}

Expr<JonesMatrix>::Ptr PrecomputedSource::coherence(const baseline_t &baseline,
    const Expr<Vector<3> >::ConstPtr&, const Expr<Vector<3> >::ConstPtr&) const
{
    return Expr<JonesMatrix>::Ptr(new ExprVisData(itsBuffer, baseline,
        false));
}

} // namespace BBS
} // namespace LOFAR
