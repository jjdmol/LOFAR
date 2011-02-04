//# Estimate.h: Non-linear parameter estimation using iterated least squares
//# (Levenberg-Marquardt).
//#
//# Copyright (C) 2010
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

#ifndef LOFAR_BBSKERNEL_ESTIMATE_H
#define LOFAR_BBSKERNEL_ESTIMATE_H

// \file Non-linear parameter estimation using iterated least squares
// (Levenberg-Marquardt).

#include <BBSKernel/BaselineMask.h>
#include <BBSKernel/CorrelationMask.h>
#include <BBSKernel/MeasurementExpr.h>
#include <BBSKernel/VisBuffer.h>

//# For the definition of SolverOptions...
#include <BBSKernel/Solver.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class EstimateOptions
{
public:
    enum Mode
    {
        AMPLITUDE,
        PHASE,
        COMPLEX,
        N_Mode
    };

    enum Algorithm
    {
        L1,
        L2,
        L1R,
        L2R,
        N_Algorithm
    };

    EstimateOptions(Algorithm algorithm = L2, Mode mode = COMPLEX,
        size_t chunkSize = 1, bool propagate = false, flag_t mask = ~flag_t(0),
        flag_t outlierMask = 1, const SolverOptions &options = SolverOptions());

    Mode mode() const;
    Algorithm algorithm() const
    {
        return itsAlgorithm;
    }

    bool robust() const
    {
        return itsAlgorithm == L1R || itsAlgorithm == L2R;
    }

    flag_t mask() const
    {
        return itsMask;
    }

    flag_t outlierMask() const
    {
        return itsOutlierMask;
    }

    size_t chunkSize() const;
    bool propagate() const;

    template <typename T>
    void setEpsilon(T first, T last)
    {
        itsEpsilon = vector<double>(first, last);
    }

    size_t nEpsilon() const
    {
        return itsEpsilon.size();
    }

    double epsilon(size_t i) const
    {
        DBGASSERT(i < itsEpsilon.size());
        return itsEpsilon[i];
    }

    template <typename T>
    void setThreshold(T first, T last)
    {
        itsThreshold = vector<double>(first, last);
    }

    size_t nThreshold() const
    {
        return itsThreshold.size();
    }

    double threshold(size_t i) const
    {
        DBGASSERT(i < itsThreshold.size());
        return itsThreshold[i];
    }

    const SolverOptions &lsqOptions() const;

private:
    Algorithm       itsAlgorithm;
    Mode            itsMode;
    size_t          itsChunkSize;
    bool            itsPropagateFlag;
    flag_t          itsMask;
    vector<double>  itsEpsilon;
    vector<double>  itsThreshold;
    flag_t          itsOutlierMask;
    SolverOptions   itsLSQOptions;
};

void estimate(const VisBuffer::Ptr &buffer,
    const BaselineMask &baselines,
    const CorrelationMask &correlations,
    const MeasurementExpr::Ptr &model,
    const Grid &grid,
    const ParmGroup &solvables,
    const EstimateOptions &options = EstimateOptions());

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
