//# PatchExpr.cc: Factory classes that construct expressions for the
//# visibilities of a patch on a given baseline, sharing station bound
//# sub-expressions.
//#
//# Copyright (C) 2011
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
#include <BBSKernel/MeasurementExprLOFARUtil.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/PatchExpr.h>
#include <BBSKernel/Expr/ExprVisData.h>
#include <BBSKernel/Expr/EquatorialCentroid.h>
#include <BBSKernel/Expr/MatrixSum.h>
#include <BBSKernel/Expr/PhaseShift.h>
#include <BBSKernel/Expr/ScalarMatrixMul.h>

namespace LOFAR
{
namespace BBS
{

PatchExprBase::~PatchExprBase()
{
}

PatchExpr::PatchExpr(Scope &scope, SourceDB &sourceDB, const string &name,
    const casa::MDirection &refPhase)
    :   itsName(name)
{
    initSourceList(scope, sourceDB, name);
    initPositionExpr(itsSourceList);
    initLMNExpr(itsSourceList, refPhase);
}

const string &PatchExpr::name() const
{
    return itsName;
}

Expr<Vector<2> >::Ptr PatchExpr::position() const
{
    return itsPosition;
}

Expr<JonesMatrix>::Ptr PatchExpr::coherence(const baseline_t &baseline,
    const Expr<Vector<3> >::Ptr &uvwLHS,
    const Expr<Vector<3> >::Ptr &uvwRHS) const
{
    if(nSources() == 1)
    {
        // Phase shift (incorporates geometry and fringe stopping).
        Expr<Vector<2> >::Ptr exprShiftLHS =
            makeStationShiftExpr(baseline.first, 0, uvwLHS);
        Expr<Vector<2> >::Ptr exprShiftRHS =
            makeStationShiftExpr(baseline.second, 0, uvwRHS);
        Expr<Scalar>::Ptr exprShift(new PhaseShift(exprShiftLHS,
            exprShiftRHS));

        // Source coherence.
        Expr<JonesMatrix>::Ptr exprCoherence =
            itsSourceList[0]->coherence(uvwLHS, uvwRHS);

        // Phase shift the source coherence to the correct position.
        return Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(exprShift,
            exprCoherence));
    }

    MatrixSum::Ptr sum(new MatrixSum());
    for(size_t i = 0; i < nSources(); ++i)
    {
        // Phase shift (incorporates geometry and fringe stopping).
        Expr<Vector<2> >::Ptr exprShiftLHS =
            makeStationShiftExpr(baseline.first, i, uvwLHS);
        Expr<Vector<2> >::Ptr exprShiftRHS =
            makeStationShiftExpr(baseline.second, i, uvwRHS);
        Expr<Scalar>::Ptr exprShift(new PhaseShift(exprShiftLHS,
            exprShiftRHS));

        // Source coherence.
        Expr<JonesMatrix>::Ptr exprCoherence =
            itsSourceList[i]->coherence(uvwLHS, uvwRHS);

        // Phase shift the source coherence to the correct position.
        exprCoherence =
            Expr<JonesMatrix>::Ptr(new ScalarMatrixMul(exprShift,
                exprCoherence));
        sum->connect(exprCoherence);
    }

    return sum;
}

size_t PatchExpr::nSources() const
{
    return itsSourceList.size();
}

void PatchExpr::initSourceList(Scope &scope, SourceDB &sourceDB,
    const string &name)
{
    vector<SourceInfo> sources(sourceDB.getPatchSources(name));

    if(sources.size() == 0)
    {
        THROW(BBSKernelException, "Patch " << name << " does not contain any"
            " sources");
    }

    itsSourceList.reserve(sources.size());
    for(vector<SourceInfo>::const_iterator it = sources.begin(),
        end = sources.end(); it != end; ++it)
    {
        itsSourceList.push_back(Source::create(*it, scope));
    }
}

void PatchExpr::initPositionExpr(const vector<Source::Ptr> &sources)
{
    if(sources.size() == 1)
    {
        itsPosition = sources.front()->position();
        return;
    }

    EquatorialCentroid::Ptr centroid(new EquatorialCentroid());
    for(vector<Source::Ptr>::const_iterator it = sources.begin(),
        end = sources.end(); it != end; ++it)
    {
        centroid->connect((*it)->position());
    }

    itsPosition = centroid;
}

void PatchExpr::initLMNExpr(const vector<Source::Ptr> &sources,
    const casa::MDirection &refPhase)
{
    itsLMN.reserve(sources.size());
    for(vector<Source::Ptr>::const_iterator it = sources.begin(),
        end = sources.end(); it != end; ++it)
    {
        itsLMN.push_back(makeLMNExpr(refPhase, (*it)->position()));
    }
}

Expr<Vector<2> >::Ptr PatchExpr::makeStationShiftExpr(unsigned int station,
    unsigned int source, const Expr<Vector<3> >::Ptr &uvw) const
{
    const size_t index = station * nSources() + source;
    if(index >= itsShift.size())
    {
        itsShift.resize(index + 1);
    }

    if(!itsShift[index])
    {
        itsShift[index] = LOFAR::BBS::makeStationShiftExpr(uvw, itsLMN[source]);
    }

    return itsShift[index];
}

StoredPatchExpr::StoredPatchExpr(const string &name,
    const VisBuffer::Ptr &buffer)
    :   itsName(name),
        itsBuffer(buffer),
        itsPosition(makeDirectionExpr(buffer->getPhaseReference()))
{
}

const string &StoredPatchExpr::name() const
{
    return itsName;
}

Expr<Vector<2> >::Ptr StoredPatchExpr::position() const
{
    return itsPosition;
}

Expr<JonesMatrix>::Ptr StoredPatchExpr::coherence(const baseline_t &baseline,
    const Expr<Vector<3> >::Ptr&,
    const Expr<Vector<3> >::Ptr&) const
{
    return Expr<JonesMatrix>::Ptr(new ExprVisData(itsBuffer, baseline, false));
}

} //# namespace BBS
} //# namespace LOFAR
