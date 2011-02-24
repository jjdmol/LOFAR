//# EstimatorL1.h: Parameter estimation using the Levenberg-Marquardt
//# algorithm with a L1-norm weighting scheme.
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

#ifndef LOFAR_BBSKERNEL_ESTIMATORL1_H
#define LOFAR_BBSKERNEL_ESTIMATORL1_H

// \file
// Parameter estimation using the Levenberg-Marquardt algorithm with a L1-norm
// weighting scheme.

#include <BBSKernel/BaselineMask.h>
#include <BBSKernel/CorrelationMask.h>
#include <BBSKernel/MeasurementExpr.h>
#include <BBSKernel/Types.h>
#include <BBSKernel/VisBuffer.h>

//# For the definition of SolverOptions...
#include <BBSKernel/Solver.h>

#include <Common/Timer.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_vector.h>

#include <scimath/Fitting/LSQFit.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class EstimatorL1
{
public:
    typedef shared_ptr<EstimatorL1>       Ptr;
    typedef shared_ptr<const EstimatorL1> ConstPtr;

    EstimatorL1(const VisBuffer::Ptr &lhs,
        const MeasurementExpr::Ptr &rhs);

    ~EstimatorL1();

    // Set the solution grid.
    void setSolutionGrid(const Grid &grid);

    // Set the number of cells that are processed together (along the time
    // axis).
    void setCellChunkSize(size_t size);

    // Restrict processing to the baselines included in the mask.
    void setBaselineMask(const BaselineMask &mask);

    // Restrict processing to the correlations included in the mask.
    void setCorrelationMask(const CorrelationMask &mask);

    void setOptions(const SolverOptions &options);

//    // Get the set of parameters the measurement expression depends on.
//    ParmGroup parms() const;

//    // Get the set of parameters that will be solved for. The order of the
//    // parameters in the ParmGroup matches the order of the coefficients in
//    // the normal equations.
//    ParmGroup solvables() const;

    void setSolvables(const vector<string> &include,
        const vector<string> &exclude);

//    // Set the parameters to solve for. Parameters that the measurement
//    // expression does not depend on are silently ingnored. Use parms() to find
//    // out beforehand which parameters the measurement expression depends on, or
//    // use solvables() to get the set of parameters that will be solved for.
//    void setSolvables(const ParmGroup &solvables);

    // Estimate parameter values for all cells in the solution grid.
    void process();

    bool isSelectionEmpty() const;

    // Reset processing statistics.
    void clearStats();

    // Dump processing statistics to the provided output stream.
    void dumpStats(ostream &out) const;

private:
    // Nested class that holds the temporary buffers needed while processing
    // a single expression from the set and some counters and timers used to
    // gather processing statistics. This is collected in a nested class such
    // that it's easy to create a thread-private instance for each thread should
    // this code be parallelized. Also, it is a way of grouping a number of
    // members that belong together semantically.
    class ProcContext
    {
    public:
        ProcContext();

        void resize(size_t nCoeff);
        void clearStats();

        // Number of expression specific coefficients.
        size_t                  nCoeff;

        // Mapping from available partial derivatives (parameter, coefficient)
        // to coefficient indidces in the normal matrix.
        vector<unsigned int>    index;
        // References to the (appoximated) partial derivatives.
        vector<Matrix>          partial;
        // Value of the (approximated) partial derivatives.
        vector<double>          partialRe, partialIm;

        // Statistics and timers.
        size_t                  count;

        enum ProcTimer
        {
            EVAL_EXPR,
            EQUATE,
            MAKE_COEFF_MAP,
            TRANSPOSE,
            MAKE_NORM,
            N_ProcTimer
        };

        static string           timerNames[N_ProcTimer];
        NSTimer                 timers[N_ProcTimer];
    };

    struct Cell
    {
        Location        location;
        casa::LSQFit    solver;
        vector<double>  coeff;
        vector<double>  coeffPrev;
        vector<double>  coeffTmp;

        double          epsilon;
        size_t          index;
    };

//    boost::multi_array<Cell, 2>     itsCells;
    vector<Cell>    itsCells;
    vector<double>  itsEpsilon;

    // Set the (inclusive) range of cells of the solution grid to process.
    void setCellSelection(const Location &start, const Location &end);

    // Determine the evaluation grid, i.e. the part of the observation grid
    // that is completely contained in the model domain.
    void makeEvalGrid();

    // Create a mapping for each axis that maps from cells in the evaluation
    // grid to cells in the solution grid.
    void makeCellMap();

    void initSolutionCells(const Location &start, const Location &end);
    void initSolver(Cell &cell);
    void loadUnknowns(Cell &cell) const;
    void storeUnknowns(const Cell &cell) const;
    void iterate();

    // Create a mapping from (parameter, coefficient) pairs to coefficient
    // indices in the normal equations.
    void makeCoeffMap();

    // Generate a look-up table mapping the coefficients of the parameters that
    // the given element depends on to the respective index in the normal
    // equations.
    void makeElementCoeffMap(const Element &element, ProcContext &context);

    // Insanely complicated boost::multi_array types...
    typedef boost::multi_array<flag_t, 4>::index_range FRange;
    typedef boost::multi_array<flag_t, 4>::const_array_view<4>::type FSlice;
    typedef boost::multi_array<double, 5>::index_range CRange;
    typedef boost::multi_array<double, 5>::const_array_view<5>::type CSlice;
    typedef boost::multi_array<dcomplex, 4>::index_range SRange;
    typedef boost::multi_array<dcomplex, 4>::const_array_view<4>::type SSlice;

    // Generate normal equations for a single expression from the set.
    void procExpr(ProcContext &context,
        const EstimatorL1::FSlice &flagLHS,
        const EstimatorL1::CSlice &covarianceLHS,
        const EstimatorL1::SSlice &valueLHS,
        const pair<size_t, size_t> &idx);

    // Create a mapping from cells of axis "from" to cell indices on axis "to".
    // Additionally, the interval of cells of axis "from" that intersect axis
    // "to" (the domain) is returned.
    template <typename T_ITER>
    Interval<size_t> makeAxisMap(const Axis::ShPtr &from,
        const Axis::ShPtr &to, T_ITER out) const;

    // Find the range of cells on the axis that is completely contained inside
    // the given interval.
    Interval<size_t> findContainedCellRange(const Axis::ShPtr &axis,
        const Interval<double> &interval) const;

    // Observed data.
    VisBuffer::Ptr                      itsLHS;

    // Model expressions.
    MeasurementExpr::Ptr                itsRHS;

    // Evaluation grid.
    Grid                                itsEvalGrid;

    // Solution grid.
    Grid                                itsSolGrid;

    // Is the intersection between the observation grid, the model domain, and
    // the solution grid empty? This indicates if whether or not there are
    // visibilities that need to be processed.
    bool                                itsIntersectionEmpty;

    // Location in the observation grid of the start of the evaluation grid.
    Location                            itsEvalOffset;

    // Location in the solution grid of the start and end of the evaluation
    // grid.
    Location                            itsEvalStart, itsEvalEnd;

    // Location in the solution grid of the current selection (clipped against
    // the evaluation grid).
    Location                            itsSelectionStart, itsSelectionEnd;

    // The number of cells in the current selection (clipped against the
    // evaluation grid).
    size_t                              itsSelectedCellCount;

    // Location in the solution grid of the current selection relative to the
    // start (in the solution grid) of the evaluation grid.
    Location                            itsEvalSelStart, itsEvalSelEnd;

    // Location in the evaluation grid of the current selection.
    Location                            itsEvalReqStart, itsEvalReqEnd;

    // Location in the observation grid of the current selection.
    Location                            itsReqStart, itsReqEnd;

    // Mapping of cells in the evaluation grid to cells in the solution grid.
    vector<size_t>                      itsFreqMap, itsTimeMap;

    // Mapping of baselines and correlations to their respective indices in
    // both observed data and model.
    vector<pair<size_t, size_t> >       itsBlMap, itsCrMap;

    ParmGroup                           itsSolvables;

    size_t                              itsCellChunkSize;
    // Total number of coefficients to fit.
    size_t                              itsCoeffCount;

    // Mapping from (parameter, coefficient) pairs to coefficient indices in the
    // condition equations.
    map<PValueKey, unsigned int>        itsCoeffMap;

    SolverOptions                       itsOptions;

    // Timer for measuring the execution time of process().
    NSTimer                             itsProcTimer;

    // Expression processing buffers.
    ProcContext                         itsProcContext;
};

template <typename T_ITER>
Interval<size_t> EstimatorL1::makeAxisMap(const Axis::ShPtr &from,
    const Axis::ShPtr &to, T_ITER out) const
{
    Interval<double> overlap(std::max(from->start(), to->start()),
        std::min(from->end(), to->end()));

    if(overlap.start >= overlap.end || casa::near(overlap.start, overlap.end))
    {
        return Interval<size_t>(1, 0);
    }

    Interval<size_t> domain;
    domain.start = from->locate(overlap.start);
    domain.end = from->locate(overlap.end, false, domain.start);

    // Intervals are inclusive by convention.
    const size_t nCells = domain.end - domain.start + 1;
    ASSERT(nCells >= 1);

    // Special case for the first domain cell: lower and possibly upper boundary
    // may be located outside of the overlap between the "from" and "to" axis.
    Interval<size_t> interval;
    interval.start = to->locate(std::max(from->lower(domain.start),
        overlap.start));
    interval.end = to->locate(std::min(from->upper(domain.start), overlap.end),
        false, interval.start);
    for(size_t i = interval.start; i <= interval.end; ++i)
    {
        *out++ = 0;
    }

    for(size_t i = 1; i < nCells - 1; ++i)
    {
        interval.start = to->locate(from->lower(domain.start + i), true,
            interval.end);
        interval.end = to->locate(from->upper(domain.start + i), false,
            interval.end);
        for(size_t j = interval.start; j <= interval.end; ++j)
        {
            *out++ = i;
        }
    }

    if(nCells > 1)
    {
        // Special case for the last domain cell: upper boundary may be located
        // outside of the overlap between the "from" and "to" axis.
        interval.start = to->locate(from->lower(domain.end), true,
            interval.end);
        interval.end = to->locate(std::min(from->upper(domain.end),
            overlap.end), false, interval.end);
        for(size_t i = interval.start; i <= interval.end; ++i)
        {
            *out++ = nCells - 1;
        }
    }

    return domain;
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
