//# EstimateUtil.h: Helper functions and types for model parameter estimation.
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

#ifndef LOFAR_BBSKERNEL_ESTIMATEUTIL_H
#define LOFAR_BBSKERNEL_ESTIMATEUTIL_H

// \file
// Helper functions and types for model parameter estimation.

#include <BBSKernel/Types.h>
#include <BBSKernel/MeasurementExpr.h>
#include <BBSKernel/VisBuffer.h>
#include <Common/lofar_algorithm.h>
#include <Common/lofar_map.h>
#include <Common/lofar_math.h>
#include <Common/lofar_vector.h>
#include <Common/Timer.h>

#include <scimath/Fitting/LSQFit.h>

//# For the definition of SolverOptions...
#include <BBSKernel/Solver.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::min;
using LOFAR::max;
using LOFAR::sqrt;

// \addtogroup BBSKernel
// @{

// Find the range of cells that contains the intersection of \p range and the
// range covered by \p axis. If the intersection is empty,
// Interval<size_t>(1, 0) will be returned (i.e. an illegal interval).
Interval<size_t> findIntersection(const Axis::ShPtr &axis,
    const Interval<double> &range);

// Print an instance of NSTimer in human readable format.
string toString(const NSTimer &timer);

// Apply the provided options to \p solver.
void configLSQSolver(casa::LSQFit &solver, const SolverOptions &options);

// Functors that modify observed and simulated visibilities (and partial
// derivatives) for amplitude or phase only comparison. SampleModifierComplex
// does not do anything, because the default is to compare both the real and
// the imaginary component.
struct SampleModifierAmplitude;
struct SampleModifierPhase;
struct SampleModifierComplex;

// Functors that modify the weight for L1 weighting. WeightModifierL2 does not
// do anything, because the default weighting is already L2.
struct WeightModifierL1;
struct WeightModifierL2;

// Process all cells in [\p start, \p end]. This function only
// performs the iteration and calls \p T_CELL_PROCESSOR::process() to do the
// actually processing. This allows re-use of the currently quite hairy
// iteration code for different types of processing.
//
// \pre The evaluation grid of \p model should be set to exactly cover all the
// cells in the range [\p start, \p end] at the same resolution as \p buffer
// (such that evalution of \p model yields a model value for each observation
// in \p buffer).
// \pre The range starting at \p cells should contain exactly one instance of
// T_CELL_PROCESSOR::CellType for each cell in the range [\p start, \p end].
template <typename T_CELL_PROCESSOR, typename T_ITER>
void equate(const Location &start, const Location &end,
    const VisBuffer::Ptr &buffer,
    const MeasurementExpr::Ptr &model,
    const vector<pair<size_t, size_t> > &baselines,
    const vector<pair<size_t, size_t> > &correlations,
    const vector<Interval<size_t> > (&cellMap)[2],
    const map<PValueKey, unsigned int> &coeffMap,
    T_CELL_PROCESSOR &processor,
    typename T_CELL_PROCESSOR::StatisticsType &statistics,
    T_ITER cells);

// Make a map that maps from cells in axis \p from to a range of cells in axis
// \p to. The resulting intervals are writting to the output iterator \p out.
// The function returns the interval of cells on axis \p from that have a
// counterpart on axis \p to. This number of cells in this interval equals the
// number of intervals writted to the output iterator. If the overlap between
// \p from and \p to is empty, Interval<size_t>(1, 0) will be returned (i.e. an
// illegal interval).
template <typename T_OUT>
Interval<size_t> makeAxisMap(const Axis::ShPtr &from, const Axis::ShPtr &to,
    T_OUT out);

// Make a map that maps from a 0-based index to the individual coefficients of
// the solvables. The function outputs PValueKey instances to the output
// iterator \p out. Let solvables contain 4, 5, 10 and let parameter 4 be a
// scalar (one coefficient), parameter 5 a first order polynomial (two
// coefficients), and parameter 10 a third order polynomial (four coefficients).
// The following PValueKeys will be written to \p out in this case: (4, 0)
// (5, 0) (5, 1) (10, 0) (10, 1) (10, 2) (10, 3).
//
// This map is used to map simulation partial derivatives to the correct index
// in the condition equations (as each baseline concerns only two stations and
// thus a sparse subset of the set of solvables).
template <typename T>
void makeCoeffMap(const ParmGroup &solvables, T out);

// Query the parameter database for the current values of (the coefficients of)
// \p solvables for the solution cell at location \p cell, and store these
// starting at \p first.
//
// \pre The range starting at \p first should be large enough to contain all
// (non-masked) coefficient values.
template <typename T>
T loadCoeff(const Location &cell, const ParmGroup &solvables, T first);

// Starting from \p first, store the coefficients of each parameter in
// \p solvables for the solution cell at location \cell into the parameter
// database.
//
// \pre The range starting at \p first should contain a value for each
// (non-masked) coefficient of \p solvables.
template <typename T>
T storeCoeff(const Location &cell, const ParmGroup &solvables, T first);

// Copy coefficient values of \p solvables from the right (frequency) boundary
// of the chunk of cells [srcStart, srcEnd] to all the cells in the chunk of
// cells [destStart, destEnd].
void passCoeff(const ParmGroup &solvables, const Location &srcStart,
    const Location &srcEnd, const Location &destStart, const Location &destEnd);

// Returns true if \p x is not infinite or nan.
template <typename T>
inline bool isfinite(T x);

// -----------------------------------------------------------------------------
// Implementation.
// -----------------------------------------------------------------------------

struct SampleModifierAmplitude
{
    static inline void process(double&, double &reObs, double &imObs,
        double &reSim, double &imSim, double *reDerivative,
        double *imDerivative, unsigned int nDerivative)
    {
        const double normObs = sqrt(reObs * reObs + imObs * imObs);
        reObs = normObs;

        const double normSim = sqrt(reSim * reSim + imSim * imSim);
        for(size_t i = 0; i < nDerivative; ++i)
        {
            reDerivative[i] =
                (reSim * reDerivative[i] + imSim * imDerivative[i]) / normSim;
        }
        reSim = normSim;
    }
};

struct SampleModifierComplex
{
    static inline void process(double&, double&, double&, double&, double&,
        double*, double*, unsigned int)
    {
    }
};

struct SampleModifierPhase
{
    static inline void process(double &weight, double &reObs, double &imObs,
        double &reSim, double &imSim, double *reDerivative,
        double *imDerivative, unsigned int nDerivative)
    {
        const double normObs = sqrt(reObs * reObs + imObs * imObs);
        reObs /= normObs;
        imObs /= normObs;
        weight *= normObs;

        const double normSim = sqrt(reSim * reSim + imSim * imSim);
        const double normSim3 = normSim * normSim * normSim;

        double dRe, dIm;
        for(size_t i = 0; i < nDerivative; ++i)
        {
            dRe = reDerivative[i];
            dIm = imDerivative[i];
            const double common = (reSim * dRe + imSim * dIm) / normSim3;

            reDerivative[i] = dRe / normSim - reSim * common;
            imDerivative[i] = dIm / normSim - imSim * common;
        }
        reSim /= normSim;
        imSim /= normSim;
    }
};

struct WeightModifierL1
{
    template <typename T_CELL>
    static inline void process(double &weight, double absResidualSqr,
        const T_CELL &cell)
    {
        weight /= sqrt(absResidualSqr + cell.epsilon);
    }
};

struct WeightModifierL2
{
    template <typename T_CELL>
    static inline void process(double&, double, const T_CELL&)
    {
    }
};

template <typename T_CELL_PROCESSOR, typename T_ITER>
void equate(const Location &start, const Location &end,
    const VisBuffer::Ptr &buffer,
    const MeasurementExpr::Ptr &model,
    const vector<pair<size_t, size_t> > &baselines,
    const vector<pair<size_t, size_t> > &correlations,
    const vector<Interval<size_t> > (&cellMap)[2],
    const map<PValueKey, unsigned int> &coeffMap,
    T_CELL_PROCESSOR &processor,
    typename T_CELL_PROCESSOR::StatisticsType &statistics,
    T_ITER cells)
{
    typedef typename T_CELL_PROCESSOR::CellType CellType;
    typedef typename T_CELL_PROCESSOR::StatisticsType StatisticsType;

    typedef vector<pair<size_t, size_t> >::const_iterator IteratorType;
    for(IteratorType blIt = baselines.begin(), blEnd = baselines.end();
        blIt != blEnd; ++blIt)
    {
        // Evaluate model.
        statistics.start(StatisticsType::T_EVALUATE);
        const JonesMatrix valueSim = model->evaluate(blIt->second);
        const FlagArray flagSim(valueSim.hasFlags() ? valueSim.flags()
            : FlagArray(flag_t(0)));
        statistics.stop(StatisticsType::T_EVALUATE);

        // If all the model visibilities are flagged, skip this baseline.
        if(flagSim.rank() == 0 && (*flagSim.begin()) != 0)
        {
            continue;
        }

        for(IteratorType crIt = correlations.begin(),
            crEnd = correlations.end(); crIt != crEnd; ++crIt)
        {
            const Element element = valueSim.element(crIt->second);
            if(element.size() <= 1)
            {
                continue;
            }

            typedef boost::multi_array<flag_t, 4>::index_range FRange;
            boost::multi_array<flag_t, 4>::array_view<2>::type flagObs =
                buffer->flags[boost::indices[blIt->first][FRange()][FRange()]
                [crIt->first]];

            typedef boost::multi_array<dcomplex, 4>::index_range VRange;
            boost::multi_array<dcomplex, 4>::const_array_view<2>::type visObs =
                buffer->samples[boost::indices[blIt->first][VRange()][VRange()]
                [crIt->first]];

            typedef boost::multi_array<double, 5>::index_range CRange;
            boost::multi_array<double, 5>::const_array_view<2>::type covObs =
                buffer->covariance[boost::indices[blIt->first][CRange()]
                [CRange()][crIt->first][crIt->first]];

            // -----------------------------------------------------------------
            // Setup pointers and strides to access the model value and
            // derivatives.
            // TODO: Use array cursors once we move to a single array type.
            // -----------------------------------------------------------------
            statistics.start(StatisticsType::T_MAKE_COEFF_MAP);
            Matrix simulation(element.value());
            DBGASSERT(simulation.isComplex());
            double *reSim, *imSim;
            simulation.dcomplexStorage(reSim, imSim);

            int nFreq = simulation.nx();
            DBGASSERT(simulation.nx() > 0 && simulation.ny() > 0);

            size_t simStride[2];
            simStride[FREQ] = simulation.isArray() ? 1 : 0;
            simStride[TIME] = simulation.isArray() ? nFreq : 0;

            const flag_t *flag = flagSim.begin();
            size_t flagStride[2];
            flagStride[FREQ] = flagSim.rank() > 0 ? 1 : 0;
            flagStride[TIME] = flagSim.rank() > 0 ? nFreq : 0;

            const size_t nDerivative = element.size() - 1;
            vector<unsigned int> coeffIndex(nDerivative);
            vector<double*> reSimDerivative(nDerivative);
            vector<double*> imSimDerivative(nDerivative);

            size_t i = 0;
            for(Element::const_iterator elIt = element.begin(),
                elEnd = element.end(); elIt != elEnd; ++elIt, ++i)
            {
                // Look-up coefficient index for this coefficient.
                map<PValueKey, unsigned int>::const_iterator coeffIt =
                    coeffMap.find(elIt->first);
                DBGASSERT(coeffIt != coeffMap.end());
                coeffIndex[i] = coeffIt->second;

                // Get pointers to the real and imaginary part of the partial
                // derivarive of the model with respect to this coefficient.
                Matrix derivative(elIt->second);
                DBGASSERT(derivative.isComplex());
                DBGASSERT(derivative.nx() == simulation.nx());
                DBGASSERT(derivative.ny() == simulation.ny());
                derivative.dcomplexStorage(reSimDerivative[i],
                    imSimDerivative[i]);
            }
            statistics.stop(StatisticsType::T_MAKE_COEFF_MAP);
            // -----------------------------------------------------------------

            size_t offset[2];
            offset[FREQ] = cellMap[FREQ][start.first].start;
            offset[TIME] = cellMap[TIME][start.second].start;

            T_ITER cell = cells;
            for(CellIterator it(start, end); !it.atEnd(); ++it, ++cell)
            {
                const Interval<size_t> &freqInterval =
                    cellMap[FREQ][it->first];
                const Interval<size_t> &timeInterval =
                    cellMap[TIME][it->second];

                size_t simIndex = (timeInterval.start - offset[TIME])
                    * simStride[TIME] + (freqInterval.start - offset[FREQ])
                    * simStride[FREQ];
                size_t flagIndex = (timeInterval.start - offset[TIME])
                    * flagStride[TIME] + (freqInterval.start - offset[FREQ])
                    * flagStride[FREQ];

                statistics.start(StatisticsType::T_PROCESS_CELL);
                processor.process(*cell,
                    freqInterval, timeInterval,
                    flagObs, visObs, covObs,
                    flag, flagIndex, flagStride,
                    reSim, imSim, reSimDerivative, imSimDerivative,
                    simIndex, simStride, coeffIndex, statistics);
                statistics.stop(StatisticsType::T_PROCESS_CELL);
            } // loop over cells
        } // loop over correlations
    } // loop over baselines
}

template <typename T_OUT>
Interval<size_t> makeAxisMap(const Axis::ShPtr &from, const Axis::ShPtr &to,
    T_OUT out)
{
    Interval<double> overlap(max(from->start(), to->start()), min(from->end(),
        to->end()));

    if(overlap.start >= overlap.end || casa::near(overlap.start, overlap.end))
    {
        return Interval<size_t>(1, 0);
    }

    Interval<size_t> domain;
    domain.start = from->locate(overlap.start);
    domain.end = from->locate(overlap.end, false, domain.start);

    // Intervals are inclusive by convention.
    const size_t nCells = domain.end - domain.start + 1;
    DBGASSERT(nCells >= 1);

    // Special case for the first domain cell: lower and possibly upper boundary
    // may be located outside of the overlap between the "from" and "to" axis.
    Interval<size_t> interval;
    interval.start = to->locate(max(from->lower(domain.start), overlap.start));
    interval.end = to->locate(min(from->upper(domain.start), overlap.end),
        false, interval.start);
    *out++ = interval;

    for(size_t i = 1; i < nCells - 1; ++i)
    {
        interval.start = to->locate(from->lower(domain.start + i), true,
            interval.end);
        interval.end = to->locate(from->upper(domain.start + i), false,
            interval.end);
        *out++ = interval;
    }

    if(nCells > 1)
    {
        // Special case for the last domain cell: upper boundary may be located
        // outside of the overlap between the "from" and "to" axis.
        interval.start = to->locate(from->lower(domain.end), true,
            interval.end);
        interval.end = to->locate(min(from->upper(domain.end), overlap.end),
            false, interval.end);
        *out++ = interval;
    }

    return domain;
}

template <typename T>
void makeCoeffMap(const ParmGroup &solvables, T out)
{
    size_t index = 0;
    for(ParmGroup::const_iterator it = solvables.begin(), end = solvables.end();
        it != end; ++it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*it);
        const size_t count = parm->getCoeffCount();
        for(size_t i = 0; i < count; ++i)
        {
            *out++ = make_pair(PValueKey(*it, i), index++);
        }
    }
}

template <typename T>
T loadCoeff(const Location &cell, const ParmGroup &solvables, T first)
{
    for(ParmGroup::const_iterator it = solvables.begin(), end = solvables.end();
        it != end; ++it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*it);
        vector<double> tmp = parm->getCoeff(cell);
        first = copy(tmp.begin(), tmp.end(), first);
    }

    return first;
}

template <typename T>
T storeCoeff(const Location &cell, const ParmGroup &solvables, T first)
{
    for(ParmGroup::const_iterator it = solvables.begin(), end = solvables.end();
        it != end; ++it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*it);
        parm->setCoeff(cell, &(*first), parm->getCoeffCount());
        first += parm->getCoeffCount();
    }

    return first;
}

template <typename T>
inline bool isfinite(T x)
{
    return !isnan(x) && !isinf(x);
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
