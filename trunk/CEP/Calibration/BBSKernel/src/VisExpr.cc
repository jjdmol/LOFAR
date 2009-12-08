//# VisExpr.cc: ExprSet interface to observed visibility data.
//#
//# Copyright (C) 2009
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/VisExpr.h>

#include <BBSKernel/Expr/DFTPS.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/ExprVisData.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/Mul.h>
#include <BBSKernel/Expr/PhaseShift.h>
#include <BBSKernel/Expr/Resampler.h>
#include <BBSKernel/Expr/StatUVW.h>
#include <BBSKernel/Exceptions.h>

#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>

namespace LOFAR
{
namespace BBS
{

VisExpr::VisExpr(const Instrument &instrument,
    const casa::MDirection &reference, const VisData::Ptr &chunk,
    const vector<baseline_t> &baselines, bool shift,
    const casa::MDirection &target, bool resample, double flagDensityThreshold)
        :   itsChunk(chunk),
            itsResampleFlag(resample)
{
    // Make a list of all the stations that are used in the given baseline
    // selection.
    const vector<unsigned int> stations = makeUsedStationList(baselines);
    if(stations.empty())
    {
        THROW(BBSKernelException, "Baseline selection is empty.");
    }

    casa::Vector<Expr<Vector<2> >::Ptr> exprStationShift;
    if(shift)
    {
        if(target.isModel())
        {
            THROW(BBSKernelException, "The target direction to phase shift to"
                " cannot be time variable.");
        }

//        // TODO: Make source object first?
//        ExprParm::Ptr refRa(new ExprParm(ParmManager::instance().get(SKY, "Ra:"
//            + dirName)));
//        ExprParm::Ptr refDec(new ExprParm(ParmManager::instance().get(SKY,
//            "Dec:" + dirName)));

        casa::MDirection targetJ2000(casa::MDirection::Convert(target,
            casa::MDirection::J2000)());
        casa::Quantum<casa::Vector<casa::Double> > angles =
            targetJ2000.getAngle();
        Literal::Ptr raTarget(new Literal(angles.getBaseValue()(0)));
        Literal::Ptr decTarget(new Literal(angles.getBaseValue()(1)));

        AsExpr<Vector<2> >::Ptr exprTargetDir(new AsExpr<Vector<2> >());
        exprTargetDir->connect(0, raTarget);
        exprTargetDir->connect(1, decTarget);
        Expr<Vector<3> >::Ptr exprTargetLMN(new LMN(reference, exprTargetDir));

        exprStationShift = casa::Vector<Expr<Vector<2> >::Ptr>(stations.size());
        for(size_t i = 0; i < stations.size(); ++i)
        {
            const casa::MPosition &position =
                instrument[stations[i]].position();
            Expr<Vector<3> >::Ptr exprUVW =
                Expr<Vector<3> >::Ptr(new StatUVW(position,
                    instrument.position(), reference));
            exprStationShift(i) = Expr<Vector<2> >::Ptr(new DFTPS(exprUVW,
                exprTargetLMN));
        }
    }

    // Create an expression tree for each baseline.
    itsExpr.reserve(baselines.size());
    for(size_t i = 0; i < baselines.size(); ++i)
    {
        const baseline_t &baseline = baselines[i];

        // Find left and right hand side station index.
        vector<unsigned int>::const_iterator it;

        it = find(stations.begin(), stations.end(), baseline.first);
        unsigned int lhs = distance(stations.begin(), it);

        it = find(stations.begin(), stations.end(), baseline.second);
        unsigned int rhs = distance(stations.begin(), it);

        ASSERT(lhs < stations.size() && rhs < stations.size());

        Expr<JonesMatrix>::Ptr exprVisData(new ExprVisData(chunk, baseline));

        if(shift)
        {
            PhaseShift::Ptr shift(new PhaseShift(exprStationShift(rhs),
                exprStationShift(lhs)));

            // Phase shift the source coherence.
            exprVisData = Expr<JonesMatrix>::Ptr(new Mul(shift, exprVisData));
        }

        if(itsResampleFlag)
        {
            exprVisData =
                Expr<JonesMatrix>::Ptr(new Resampler(exprVisData, 1,
                    flagDensityThreshold));
        }

        itsExpr.push_back(exprVisData);
    }
}

unsigned int VisExpr::size() const
{
    return itsExpr.size();
}

Box VisExpr::domain() const
{
    return itsChunk->getDimensions().getGrid().getBoundingBox();
}

ParmGroup VisExpr::getParms() const
{
    return ParmGroup();
}

ParmGroup VisExpr::getSolvableParms() const
{
    return ParmGroup();
}

void VisExpr::setSolvableParms(const ParmGroup&)
{
}

void VisExpr::clearSolvableParms()
{
}

void VisExpr::setEvalGrid(const Grid &grid)
{
    const Grid &visGrid = itsChunk->getDimensions().getGrid();
    ASSERT(visGrid.getBoundingBox().contains(grid.getBoundingBox()));
    itsRequest = Request(grid);

    if(itsResampleFlag)
    {
        // Check preconditions.
        size_t start = visGrid[FREQ]->locate(grid[FREQ]->start());
        size_t end = visGrid[FREQ]->locate(grid[FREQ]->end(), false,
            start);
        ASSERT(casa::near(visGrid[FREQ]->lower(start), grid[FREQ]->start())
            && casa::near(visGrid[FREQ]->upper(end), grid[FREQ]->end()));
        Axis::ShPtr freqAxis(visGrid[FREQ]->subset(start, end));
        ASSERT(freqAxis->size() == end - start + 1);

        start = visGrid[TIME]->locate(grid[TIME]->start());
        end = visGrid[TIME]->locate(grid[TIME]->end(), false, start);
        ASSERT(casa::near(visGrid[TIME]->lower(start), grid[TIME]->start())
            && casa::near(visGrid[TIME]->upper(end), grid[TIME]->end()));
        Axis::ShPtr timeAxis(visGrid[TIME]->subset(start, end));
        ASSERT(timeAxis->size() == end - start + 1);

        // Create and append high resolution grid.
        itsRequest.append(Grid(freqAxis, timeAxis));
    }

    LOG_DEBUG_STR("" << itsCache);
    itsCache.clearStats();
    itsCache.clear();
}

const JonesMatrix VisExpr::evaluate(unsigned int i)
{
    // Evaluate the model.
    ASSERT(i < itsExpr.size());
    return itsExpr[i]->evaluate(itsRequest, itsCache, 0);
}

vector<unsigned int>
VisExpr::makeUsedStationList(const vector<baseline_t> &baselines) const
{
    set<unsigned int> used;
    for(vector<baseline_t>::const_iterator it = baselines.begin(),
        end = baselines.end();
        it != end; ++it)
    {
        used.insert(it->first);
        used.insert(it->second);
    }

    return vector<unsigned int>(used.begin(), used.end());
}

} //# namespace BBS
} //# namespace LOFAR
