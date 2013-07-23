//# EstimateUtil.cc: Helper functions and types for model parameter estimation.
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
#include <BBSKernel/EstimateUtil.h>
#include <Common/Timer.h>

namespace LOFAR
{
namespace BBS
{

Interval<size_t> findIntersection(const Axis::ShPtr &axis,
    const Interval<double> &range)
{
    Interval<double> overlap(max(axis->start(), range.start), min(axis->end(),
        range.end));

    if(overlap.start >= overlap.end || casa::near(overlap.start, overlap.end))
    {
        return Interval<size_t>(1, 0);
    }

    Interval<size_t> domain;
    domain.start = axis->locate(overlap.start);
    domain.end = axis->locate(overlap.end, false, domain.start);

    return domain;
}

string toString(const NSTimer &timer)
{
    double elapsed = timer.getElapsed();
    unsigned long long count = timer.getCount();
    double average = count > 0 ? elapsed / count : 0.0;

    ostringstream oss;
    oss << elapsed << "/" << count << "/" << average;
    return oss.str();
}

void configLSQSolver(casa::LSQFit &solver, const SolverOptions &options)
{
    solver.setEpsValue(options.epsValue);
    solver.setEpsDerivative(options.epsDerivative);
    solver.setMaxIter(options.maxIter);
    solver.set(options.colFactor, options.lmFactor);
    solver.setBalanced(options.balancedEq);
}

void passCoeff(const ParmGroup &solvables, const Location &srcStart,
    const Location &srcEnd, const Location &destStart, const Location &destEnd)
{
    ASSERT(srcEnd.first >= srcStart.first && srcEnd.second >= srcStart.second);
    ASSERT(destEnd.first >= destStart.first
        && destEnd.second >= destStart.second);

    // Compute overlap in frequency.
    size_t freqStart = max(srcStart.first, destStart.first);
    size_t freqEnd = min(srcEnd.first, destEnd.first);
    if(freqStart > freqEnd)
    {
        return;
    }

    // Determine the total number of coefficients.
    size_t nCoeff = 0;
    for(ParmGroup::const_iterator it = solvables.begin(), end = solvables.end();
        it != end; ++it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*it);
        nCoeff += parm->getCoeffCount();
    }

    // Allocate a temporary buffer to store the coefficients to copy.
    vector<double> coeff(nCoeff);

    // Copy coefficients from the right most frequency boundary to all
    // destination cells.
    Location src(freqStart, srcEnd.second);
    Location dest(freqStart, destStart.second);
    for(; dest.first <= freqEnd; ++dest.first, ++src.first)
    {
        loadCoeff(src, solvables, coeff.begin());

        for(dest.second = destStart.second; dest.second <= destEnd.second;
            ++dest.second)
        {
            storeCoeff(dest, solvables, coeff.begin());
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR
