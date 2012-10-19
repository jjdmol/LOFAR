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

#include <ParmDB/ParmDBLog.h>

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

    // Check if the Mode is valid (i.e. < N_Mode).
    static bool isDefined(Mode in);

    // Convert the input argument to the corresponding Mode. If the input is
    // out of bounds, N_Mode is returned.
    static Mode asMode(unsigned int in);

    // Convert the input argument to the corresponding Mode. If the input does
    // not match any defined Mode, N_Mode is returned.
    static Mode asMode(const string &in);

    // Convert the input Mode to its string representation. N_Mode converts to
    // "<UNDEFINED>".
    static const string &asString(Mode in);

    enum Algorithm
    {
        L1,
        L2,
        N_Algorithm
    };

    // Check if the Algorithm is valid (i.e. < N_Algorithm).
    static bool isDefined(Algorithm in);

    // Convert the input argument to the corresponding Algorithm. If the input
    // is out of bounds, N_Algorithm is returned.
    static Algorithm asAlgorithm(unsigned int in);

    // Convert the input argument to the corresponding Algorithm. If the input
    // does not match any defined Algorithm, N_Algorithm is returned.
    static Algorithm asAlgorithm(const string &in);

    // Convert the input Algorithm to its string representation. N_Algorithm
    // converts to "<UNDEFINED>".
    static const string &asString(Algorithm in);

    EstimateOptions(Mode mode = COMPLEX, Algorithm algorithm = L2,
        bool robust = false, size_t chunkSize = 1, bool propagate = false,
        flag_t mask = ~flag_t(0), flag_t outlierMask = 1,
        const SolverOptions &options = SolverOptions());

    Mode mode() const;
    Algorithm algorithm() const;
    bool robust() const;
    flag_t mask() const;
    flag_t outlierMask() const;
    size_t chunkSize() const;
    bool propagate() const;

    template <typename T>
    void setEpsilon(T first, T last);
    size_t nEpsilon() const;
    double epsilon(size_t i) const;

    template <typename T>
    void setThreshold(T first, T last);
    size_t nThreshold() const;
    double threshold(size_t i) const;

    const SolverOptions &lsqOptions() const;

private:
    Mode            itsMode;
    Algorithm       itsAlgorithm;
    bool            itsRobustFlag;
    size_t          itsChunkSize;
    bool            itsPropagateFlag;
    flag_t          itsMask;
    vector<double>  itsEpsilon;
    vector<double>  itsThreshold;
    flag_t          itsOutlierMask;
    SolverOptions   itsLSQOptions;
};

// This function estimates values for the coefficients of \p solvables that
// minimize the difference between the observed visibilities in \p buffer and
// the visibilities simulated using \p model. An independent solution is
// computed for each cell in \p grid. Only those baselines and correlations
// selected in \p baselines and \p correlations will be used.
void estimate(ParmDBLog &log,
    const VisBuffer::Ptr &buffer,
    const BaselineMask &baselines,
    const CorrelationMask &correlations,
    const MeasurementExpr::Ptr &model,
    const Grid &grid,
    const ParmGroup &solvables,
    const EstimateOptions &options = EstimateOptions());

// @}

// -----------------------------------------------------------------------------
// EstimateOptions implementation.
// -----------------------------------------------------------------------------

template <typename T>
void EstimateOptions::setEpsilon(T first, T last)
{
    if(first != last)
    {
        itsEpsilon = vector<double>(first, last);
    }
}

template <typename T>
void EstimateOptions::setThreshold(T first, T last)
{
    if(first != last)
    {
        itsThreshold = vector<double>(first, last);
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
