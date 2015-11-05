//# Estimate.cc: Non-linear parameter estimation using iterated least squares
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

#include <lofar_config.h>
#include <BBSKernel/Estimate.h>
#include <BBSKernel/EstimateUtil.h>
#include <BBSKernel/Expr/Timer.h>

#include <Common/lofar_math.h>
#include <Common/lofar_sstream.h>
#include <Common/Timer.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::sqrt;

namespace
{
    // Processing statistics and timers.
    class Statistics
    {
    public:
        enum Counter
        {
            C_ALL,
            C_FLAGGED,
            C_ZERO_WEIGHT,
            C_INVALID_RESIDUAL,
            C_INVALID_DERIVATIVE,
            C_INVALID_WEIGHT,
            C_OUTLIER,
            N_Counter
        };

        enum Timer
        {
            T_EVALUATE,
            T_MAKE_COEFF_MAP,
            T_PROCESS_CELL,
            T_MODIFIER,
            T_MAKE_NORM,
            N_Timer
        };

        Statistics();

        void inc(Counter counter);
        void inc(Counter counter, size_t count);
        void reset(Counter counter);

        void reset(Timer timer);
        void start(Timer timer);
        void stop(Timer timer);

        void reset();

        string counters() const;
        string timers() const;

    private:
        size_t          itsCounters[N_Counter];
        static string   theirCounterNames[N_Counter];

        NSTimer         itsTimers[N_Timer];
        static string   theirTimerNames[N_Timer];
    };

    // State kept for a single cell in the solution grid.
    struct Cell
    {
        // Flag that indicates if processing has completed.
        bool            done;

        // LSQ solver and current estimates for the coefficients.
        casa::LSQFit    solver;
        vector<double>  coeff;

        // RMS of this iteration.
        double          rms;
        // No. of visibilities used this iteration.
        size_t          count;

        // Current L1 epsilon value and index into the list of epsilon values.
        double          epsilon;
        unsigned int    epsilonIdx;

        // Flag that controls if outliers are to be flagged this iteration.
        bool            flag;
        // Current RMS threshold used for outlier detection and index in the
        // list of threshold values.
        double          threshold;
        unsigned int    thresholdIdx;
        // No. of outliers detected this iteration (if applicable).
        size_t          outliers;
    };

    // Functor to add condition equations to the LSQ solver of a cell using real
    // observed and simulated visibilities. Used when solving based on amplitude
    // only.
    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    class CellProcessorReal
    {
    public:
        typedef Cell        CellType;
        typedef Statistics  StatisticsType;

        CellProcessorReal(size_t nDerivative, flag_t mask, flag_t outlier);
        ~CellProcessorReal();

        void process(CellType &cell,
            const Interval<size_t> &freqInterval,
            const Interval<size_t> &timeInterval,
            boost::multi_array<flag_t, 4>::array_view<2>::type flagObs,
            boost::multi_array<dcomplex, 4>::const_array_view<2>::type visObs,
            boost::multi_array<double, 5>::const_array_view<2>::type covObs,
            const flag_t *flagSim, size_t flagIndex,
            const size_t (&flagStride)[2],
            const double *reSim, const double *imSim,
            const vector<double*> &reSimDerivative,
            const vector<double*> &imSimDerivative,
            size_t simIndex, const size_t (&simStride)[2],
            const vector<unsigned int> &coeffIndex,
            StatisticsType &statistics);

    private:
        double  *itsReDerivative, *itsImDerivative;
        flag_t  itsMask, itsOutlierMask;
    };

    // Functor to add condition equations to the LSQ solver of a cell using
    // complex observed and simulated visibilities. Used when solving based on
    // phase only or based on both real and imaginary components.
    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    class CellProcessorComplex
    {
    public:
        typedef Cell        CellType;
        typedef Statistics  StatisticsType;

        CellProcessorComplex(size_t nDerivative, flag_t mask, flag_t outlier);
        ~CellProcessorComplex();

        void process(CellType &cell,
            const Interval<size_t> &freqInterval,
            const Interval<size_t> &timeInterval,
            boost::multi_array<flag_t, 4>::array_view<2>::type flagObs,
            boost::multi_array<dcomplex, 4>::const_array_view<2>::type visObs,
            boost::multi_array<double, 5>::const_array_view<2>::type covObs,
            const flag_t *flagSim, size_t flagIndex,
            const size_t (&flagStride)[2],
            const double *reSim, const double *imSim,
            const vector<double*> &reSimDerivative,
            const vector<double*> &imSimDerivative,
            size_t simIndex, const size_t (&simStride)[2],
            const vector<unsigned int> &coeffIndex,
            StatisticsType &statistics);

    private:
        double  *itsReDerivative, *itsImDerivative;
        flag_t  itsMask, itsOutlierMask;
    };

    // Helper function to initialize each cell in the range [\p start, \p end).
    // \pre The range starting at \p cell should contain exactly one Cell
    // instance for each cell in the range [\p start, \p end].
    template <typename T>
    void initCells(const Location &start, const Location &end,
        const ParmGroup &solvables, size_t nCoeff,
        const EstimateOptions &options, T cell);

    // Cell counts separated by state. Used to indicate the status of all the
    // cells after performing an iteration.
    struct IterationStatus
    {
        unsigned int nActive, nConverged, nStopped, nNoReduction, nSingular;
    };

    // Perform a single iteration for all cells in the range [\p start, \p end)
    // that have not yet converged or failed, updating the coefficient values
    // to the new estimates found.
    // \pre The range starting at \p cell should contain exactly one Cell
    // instance for each cell in the range [\p start, \p end].
    template <typename T>
    IterationStatus iterate(ParmDBLog &log, const Grid &grid,
        const Location &start, const Location &end, const ParmGroup &solvables,
        const EstimateOptions &options, T cell);

    // Decode casa::LSQFit ready codes and update the status counts.
    void updateIterationStatus(const Cell &cell, IterationStatus &status);

    // Write a map that maps parameter names to positions in the solution vector
    // to the solver log. This is necessary to be able to correctly interpret
    // the solution vectors in the log afterwards.
    void logCoefficientIndex(ParmDBLog &log, const ParmGroup &solvables);

    // Write the configuration of the least squares solver to the solver log.
    void logLSQOptions(ParmDBLog &log, const SolverOptions &options);

    // Log solver statistics for the given cell.
    void logCellStats(ParmDBLog &log, const Box &box, Cell &cell);

    // Function that does the actual iteration over chunk of cells and can be
    // specialised using \p T_CELL_PROCESSOR.
    template <typename T_CELL_PROCESSOR>
    void estimateImpl(ParmDBLog &log,
        const VisBuffer::Ptr &buffer,
        const BaselineMask &baselines,
        const CorrelationMask &correlations,
        const MeasurementExpr::Ptr &model,
        const Grid &grid,
        const ParmGroup &solvables,
        const EstimateOptions &options);
} //# anonymous namespace

void estimate(ParmDBLog &log,
    const VisBuffer::Ptr &buffer,
    const BaselineMask &baselines,
    const CorrelationMask &correlations,
    const MeasurementExpr::Ptr &model,
    const Grid &grid,
    const ParmGroup &solvables,
    const EstimateOptions &options)
{
    switch(options.algorithm())
    {
        case EstimateOptions::L1:
            switch(options.mode())
            {
                case EstimateOptions::AMPLITUDE:
                    estimateImpl<CellProcessorReal<SampleModifierAmplitude,
                        WeightModifierL1> >(log, buffer, baselines,
                        correlations, model, grid, solvables, options);
                    break;
                case EstimateOptions::PHASE:
                    estimateImpl<CellProcessorComplex<SampleModifierPhase,
                        WeightModifierL1> >(log, buffer, baselines,
                        correlations, model, grid, solvables, options);
                    break;
                case EstimateOptions::COMPLEX:
                    estimateImpl<CellProcessorComplex<SampleModifierComplex,
                        WeightModifierL1> >(log, buffer, baselines,
                        correlations, model, grid, solvables, options);
                    break;
                default:
                    THROW(BBSKernelException, "Invalid mode selected.");
            };
            break;

        case EstimateOptions::L2:
            switch(options.mode())
            {
                case EstimateOptions::AMPLITUDE:
                    estimateImpl<CellProcessorReal<SampleModifierAmplitude,
                        WeightModifierL2> >(log, buffer, baselines,
                        correlations, model, grid, solvables, options);
                    break;
                case EstimateOptions::PHASE:
                    estimateImpl<CellProcessorComplex<SampleModifierPhase,
                        WeightModifierL2> >(log, buffer, baselines,
                        correlations, model, grid, solvables, options);
                    break;
                case EstimateOptions::COMPLEX:
                    estimateImpl<CellProcessorComplex<SampleModifierComplex,
                        WeightModifierL2> >(log, buffer, baselines,
                        correlations, model, grid, solvables, options);
                    break;
                default:
                    THROW(BBSKernelException, "Invalid mode selected.");
            };
            break;

        default:
            THROW(BBSKernelException, "Invalid algorithm selected.");
    }
}

EstimateOptions::EstimateOptions(Mode mode, Algorithm algorithm, bool robust,
        size_t chunkSize, bool propagate, flag_t mask, flag_t outlierMask,
        const SolverOptions &options)
    :   itsMode(mode),
        itsAlgorithm(algorithm),
        itsRobustFlag(robust),
        itsChunkSize(chunkSize),
        itsPropagateFlag(propagate),
        itsMask(mask),
        itsOutlierMask(outlierMask),
        itsLSQOptions(options)
{
    double defThreshold[10] = {7.0, 5.0, 4.0, 3.5, 3.0, 2.8, 2.6, 2.4, 2.2,
        2.5};
    itsThreshold = vector<double>(defThreshold, defThreshold + 10);

    double defEpsilon[3] = {1e-4, 1e-5, 1e-6};
    itsEpsilon = vector<double>(defEpsilon, defEpsilon + 3);
}

EstimateOptions::Mode EstimateOptions::mode() const
{
    return itsMode;
}

EstimateOptions::Algorithm EstimateOptions::algorithm() const
{
    return itsAlgorithm;
}

bool EstimateOptions::robust() const
{
    return itsRobustFlag;
}

flag_t EstimateOptions::mask() const
{
    return itsMask;
}

flag_t EstimateOptions::outlierMask() const
{
    return itsOutlierMask;
}

size_t EstimateOptions::chunkSize() const
{
    return itsChunkSize;
}

bool EstimateOptions::propagate() const
{
    return itsPropagateFlag;
}

size_t EstimateOptions::nEpsilon() const
{
    return itsEpsilon.size();
}

double EstimateOptions::epsilon(size_t i) const
{
    DBGASSERT(i < itsEpsilon.size());
    return itsEpsilon[i];
}

size_t EstimateOptions::nThreshold() const
{
    return itsThreshold.size();
}

double EstimateOptions::threshold(size_t i) const
{
    DBGASSERT(i < itsThreshold.size());
    return itsThreshold[i];
}

const SolverOptions &EstimateOptions::lsqOptions() const
{
    return itsLSQOptions;
}

bool EstimateOptions::isDefined(Mode in)
{
    return in != N_Mode;
}

EstimateOptions::Mode EstimateOptions::asMode(unsigned int in)
{
    return (in < N_Mode ? static_cast<Mode>(in) : N_Mode);
}

EstimateOptions::Mode EstimateOptions::asMode(const string &in)
{
    Mode out = N_Mode;
    for(unsigned int i = 0; i < N_Mode; ++i)
    {
        if(in == asString(static_cast<Mode>(i)))
        {
            out = static_cast<Mode>(i);
            break;
        }
    }

    return out;
}

const string &EstimateOptions::asString(Mode in)
{
    //# Caution: Always keep this array of strings in sync with the enum Mode
    //# that is defined in the header.
    static const string name[N_Mode + 1] =
        {"AMPLITUDE",
        "PHASE",
        "COMPLEX",
        //# "<UNDEFINED>" should always be last.
        "<UNDEFINED>"};

    return name[in];
}

bool EstimateOptions::isDefined(Algorithm in)
{
    return in != N_Algorithm;
}

EstimateOptions::Algorithm EstimateOptions::asAlgorithm(unsigned int in)
{
    return (in < N_Algorithm ? static_cast<Algorithm>(in) : N_Algorithm);
}

EstimateOptions::Algorithm EstimateOptions::asAlgorithm(const string &in)
{
    Algorithm out = N_Algorithm;
    for(unsigned int i = 0; i < N_Algorithm; ++i)
    {
        if(in == asString(static_cast<Algorithm>(i)))
        {
            out = static_cast<Algorithm>(i);
            break;
        }
    }

    return out;
}

const string &EstimateOptions::asString(Algorithm in)
{
    //# Caution: Always keep this array of strings in sync with the enum Algorithm
    //# that is defined in the header.
    static const string name[N_Algorithm + 1] =
        {"L1",
        "L2",
        //# "<UNDEFINED>" should always be last.
        "<UNDEFINED>"};

    return name[in];
}

namespace
{
    template <typename T_CELL_PROCESSOR>
    void estimateImpl(ParmDBLog &log,
        const VisBuffer::Ptr &buffer,
        const BaselineMask &baselines,
        const CorrelationMask &correlations,
        const MeasurementExpr::Ptr &model,
        const Grid &grid,
        const ParmGroup &solvables,
        const EstimateOptions &options)
    {
        ASSERTSTR(model->domain().contains(buffer->domain()), "The area in"
            " time, frequency covered by the visibility buffer extends outside"
            " the model domain.");

        ASSERTSTR(buffer->hasFlags() && buffer->hasCovariance(), "Flags and"
            " covariance information required for parameter estimation.");

        // Construct a sequence of pairs of indices of matching baselines (i.e.
        // baselines common to both buffer and model).
        vector<pair<size_t, size_t> > blMap;
        makeIndexMap(buffer->baselines(), model->baselines(), baselines,
            back_inserter(blMap));

        // Construct a sequence of pairs of indices of matching correlations
        // (i.e. correlations known by both buffer and model).
        vector<pair<size_t, size_t> > crMap;
        makeIndexMap(buffer->correlations(), model->correlations(),
            correlations, back_inserter(crMap));

        LOG_INFO_STR("Selection: Baselines: " << blMap.size() << "/"
            << buffer->nBaselines() << " Correlations: " << crMap.size() << "/"
            << buffer->nCorrelations() << " Parameters: " << solvables.size()
            << "/" << model->nParms());

        // Compute a mapping from cells of the solution grid to cell intervals
        // in the evaluation grid.
        vector<Interval<size_t> > cellMap[2];
        Interval<size_t> domain[2];
        domain[FREQ] = makeAxisMap(grid[FREQ], buffer->grid()[FREQ],
            back_inserter(cellMap[FREQ]));
        domain[TIME] = makeAxisMap(grid[TIME], buffer->grid()[TIME],
            back_inserter(cellMap[TIME]));

        // Enable the computation of the partial derivatives of the model w.r.t.
        // the solvables.
        model->setSolvables(solvables);
        ASSERT(model->solvables() == solvables);

        // Make coefficient map.
        map<PValueKey, unsigned int> coeffMap;
        makeCoeffMap(solvables, inserter(coeffMap, coeffMap.begin()));
        LOG_INFO_STR("No. of coefficients to estimate: " << coeffMap.size());

        // Assign solution grid to solvables.
        ParmManager::instance().setGrid(grid, solvables);

        // Log information that is valid for all chunks.
        logCoefficientIndex(log, solvables);
        logLSQOptions(log, options.lsqOptions());

        // ---------------------------------------------------------------------
        // Process each chunk of cells in a loop.
        // ---------------------------------------------------------------------

        // Clip chunk size to the size of the solution grid.
        size_t chunkSize = options.chunkSize() == 0 ? grid[TIME]->size()
            : std::min(options.chunkSize(), grid[TIME]->size());

        // Allocate cells.
        vector<Cell> cells(grid[FREQ]->size() * chunkSize);

        // Instantiate the requested functor (that is called to process a single
        // solution cell).
        T_CELL_PROCESSOR processor(coeffMap.size(), options.mask(),
            options.outlierMask());

        // Clear any temporary flags (safe guard against tasks that do not clean
        // up properly).
        if(options.robust())
        {
            buffer->flagsAndWithMask(~options.outlierMask());
        }

        // Compute the number of cell chunks to process.
        size_t nChunks = (grid[TIME]->size() + chunkSize - 1) / chunkSize;

        Timer::instance().reset();
        NSTimer timer;
        timer.start();

        // Process the solution grid in chunks.
        for(size_t chunk = 0; chunk < nChunks; ++chunk)
        {
            NSTimer timerChunk, timerEquate, timerIterate;
            timerChunk.start();

            // Compute cell chunk boundaries in solution grid coordinates.
            Location chunkStart(0, chunk * chunkSize);
            Location chunkEnd(grid[FREQ]->size() - 1,
                std::min(chunkStart.second + chunkSize - 1,
                grid[TIME]->size() - 1));

            // Adjust cell chunk boundaries to exclude those cells for which no
            // visibility data is available.
            chunkStart =
                Location(std::max(chunkStart.first, domain[FREQ].start),
                std::max(chunkStart.second, domain[TIME].start));
            chunkEnd =
                Location(std::min(chunkEnd.first, domain[FREQ].end),
                std::min(chunkEnd.second, domain[TIME].end));

            // If there are no cells for which visibility data is available,
            // skip the chunk.
            if(chunkStart.first > chunkEnd.first
                || chunkStart.second > chunkEnd.second)
            {
                LOG_DEBUG_STR("chunk: " << (chunk + 1) << "/" << nChunks
                    << " status: **skipped**");
                timerChunk.stop();
                continue;
            }

            // Ensure a model value is computed for all the visibility samples
            // within the chunk.
            Location reqStart(cellMap[FREQ][chunkStart.first].start,
                cellMap[TIME][chunkStart.second].start);
            Location reqEnd(cellMap[FREQ][chunkEnd.first].end,
                cellMap[TIME][chunkEnd.second].end);
            model->setEvalGrid(buffer->grid().subset(reqStart, reqEnd));

            // Initialize a cell instance for each cell in [chunkEnd,
            // chunkStart].
            initCells(chunkStart, chunkEnd, solvables, coeffMap.size(), options,
                cells.begin());

            Statistics stats;
            IterationStatus status = {0, 0, 0, 0, 0};
            unsigned int nIterations = 0;
            while(true)
            {
                // Construct normal equations from the data and an evaluation of
                // the model based on the current coefficient values.
                timerEquate.start();
                equate(chunkStart, chunkEnd, buffer, model, blMap, crMap,
                    cellMap, coeffMap, processor, stats, cells.begin());
                timerEquate.stop();

                // Perform a single iteration.
                timerIterate.start();
                status = iterate(log, grid, chunkStart, chunkEnd, solvables,
                    options, cells.begin());
                timerIterate.stop();

                // Notify model that solvables have changed.
                model->solvablesChanged();

                // Update iteration count.
                ++nIterations;

                // If no active cells remain in this chunk (i.e. all cells have
                // converged or have been stopped), then move to the next chunk
                // of cells.
                if(status.nActive == 0)
                {
                    break;
                }
            }
            timerChunk.stop();

            // Output statistics and timers.
            const size_t nCells = (chunkEnd.second - chunkStart.second + 1)
                * (chunkEnd.first - chunkStart.first + 1);
            LOG_DEBUG_STR("chunk: " << (chunk + 1) << "/" << nChunks
                << " cells: " << nCells << " iterations: " << nIterations
                << " status: " << status.nConverged << "/" << status.nStopped
                << "/" << status.nNoReduction << "/" << status.nSingular
                << " converged/stopped/noreduction/singular");
            LOG_DEBUG_STR("\t" << stats.counters());
            LOG_DEBUG_STR("\t" << stats.timers());
            LOG_DEBUG_STR("\ttimers: all: " << toString(timerChunk)
                << " equate: " << toString(timerEquate) << " iterate: "
                << toString(timerIterate) << " total/count/average");

            // Propagate solutions to the next chunk if required.
            if(options.propagate() && (chunk + 1) < nChunks)
            {
                Location srcStart(0, chunk * chunkSize);
                Location srcEnd(grid[FREQ]->size() - 1,
                    srcStart.second + chunkSize - 1);

                Location destStart(0, (chunk + 1) * chunkSize);
                Location destEnd(grid[FREQ]->size() - 1,
                    std::min(destStart.second + chunkSize - 1,
                    grid[TIME]->size() - 1));

                passCoeff(solvables, srcStart, srcEnd, destStart, destEnd);
            }
        }

        timer.stop();

        ostringstream oss;
        oss << endl << "Estimate statistics:" << endl;
        {
            const double elapsed = timer.getElapsed();
            const unsigned long long count = timer.getCount();
            double average = count > 0 ? elapsed / count : 0.0;

            oss << "TIMER s ESTIMATE ALL" << " total " << elapsed << " count "
                << count << " avg " << average << endl;
        }

        Timer::instance().dump(oss);
        LOG_DEBUG(oss.str());

        Timer::instance().reset();

        // Disable the computation of the partial derivatives of the model
        // w.r.t. the solvables.
        model->clearSolvables();

        // Clean up temporary flags.
        if(options.robust())
        {
            buffer->flagsAndWithMask(~options.outlierMask());
        }
    }

    template <typename T>
    IterationStatus iterate(ParmDBLog &log, const Grid &grid,
        const Location &start, const Location &end, const ParmGroup &solvables,
        const EstimateOptions &options, T cell)
    {
        IterationStatus status = {0, 0, 0, 0, 0};
        for(CellIterator it(start, end); !it.atEnd(); ++it, ++cell)
        {
            // If processing on the cell is already done, only update the status
            // counts and continue to the next cell.
            if(cell->done)
            {
                updateIterationStatus(*cell, status);
                continue;
            }

            // Compute RMS.
            if(cell->count > 0)
            {
                cell->rms = sqrt(cell->rms / cell->count);
            }

            // Turn outlier detection off by default. May be enabled later on.
            cell->flag = false;

            // Perform a single iteration if the cell has not yet converged or
            // failed.
            if(!cell->solver.isReady())
            {
                // LSQFit::solveLoop() only returns false if the normal
                // equations are singular. This can also be seen from the result
                // of LSQFit::isReady(), so we don't update the iteration status
                // here but do skip the update of the solvables.
                casa::uInt rank;
                if(cell->solver.solveLoop(rank, &(cell->coeff[0]),
                    options.lsqOptions().useSVD))
                {
                    // Store the updated coefficient values.
                    storeCoeff(*it, solvables, cell->coeff.begin());
                }
            }

            // Handle L1 restart with a different epsilon value.
            if(cell->solver.isReady()
                && options.algorithm() == EstimateOptions::L1
                && cell->epsilonIdx < options.nEpsilon())
            {
                // Move to the next epsilon value.
                ++cell->epsilonIdx;

                if(cell->epsilonIdx < options.nEpsilon())
                {
                    // Re-initialize LSQ solver.
                    size_t nCoeff = cell->coeff.size();
                    cell->solver =
                        casa::LSQFit(static_cast<casa::uInt>(nCoeff));
                    configLSQSolver(cell->solver, options.lsqOptions());

                    // Update epsilon value.
                    cell->epsilon = options.epsilon(cell->epsilonIdx);
                }
            }

            // Handle restart with a new RMS threshold value.
            if(cell->solver.isReady()
                && options.robust()
                && cell->thresholdIdx < options.nThreshold())
            {
                // Re-initialize LSQ solver.
                size_t nCoeff = cell->coeff.size();
                cell->solver = casa::LSQFit(static_cast<casa::uInt>(nCoeff));
                configLSQSolver(cell->solver, options.lsqOptions());

                // Reset L1 state.
                cell->epsilonIdx = 0;
                cell->epsilon = options.algorithm() == EstimateOptions::L1
                    ? options.epsilon(0) : 0.0;

                // Compute new RMS threshold and activate outlier detection.
                cell->threshold = options.threshold(cell->thresholdIdx)
                    * cell->rms;
                cell->flag = true;

                // Move to the next threshold.
                 ++cell->thresholdIdx;
            }

            // Log solution statistics.
            if(!options.robust()
                && (options.algorithm() == EstimateOptions::L2))
            {
                logCellStats(log, grid.getCell(*it), *cell);
            }

            if(cell->solver.isReady())
            {
                cell->done = true;
            }
            else
            {
                // If not yet converged or failed, reset state for the next
                // iteration.
                cell->rms = 0.0;
                cell->count = 0;
                cell->outliers = 0;
            }

            updateIterationStatus(*cell, status);
        }

        return status;
    }

    void updateIterationStatus(const Cell &cell, IterationStatus &status)
    {
        // casa::LSQFit::isReady() is incorrectly labelled non-const.
        casa::LSQFit &solver = const_cast<casa::LSQFit&>(cell.solver);

        // Decode and record the solver status.
        switch(solver.isReady())
        {
            case casa::LSQFit::NONREADY:
                ++status.nActive;
                break;

            case casa::LSQFit::SOLINCREMENT:
            case casa::LSQFit::DERIVLEVEL:
                ++status.nConverged;
                break;

            case casa::LSQFit::MAXITER:
                ++status.nStopped;
                break;

            case casa::LSQFit::NOREDUCTION:
                ++status.nNoReduction;
                break;

            case casa::LSQFit::SINGULAR:
                ++status.nSingular;
                break;

            default:
                // This assert triggers if an casa::LSQFit ready
                // code is encountered that is not covered above.
                // The most likely cause is that the
                // casa::LSQFit::ReadyCode enumeration has changes
                // in which case the code above needs to be changed
                // accordingly.
                ASSERT(false);
                break;
        }
    }

    void logCoefficientIndex(ParmDBLog &log, const ParmGroup &solvables)
    {
        size_t index = 0;
        for(ParmGroup::const_iterator it = solvables.begin(),
            end = solvables.end(); it != end; ++it)
        {
            ParmProxy::Ptr parm = ParmManager::instance().get(*it);
            const size_t count = parm->getCoeffCount();
            log.setCoeffIndex(parm->getName(), index, index + count - 1);
            index += count;
        }
    }

    void logLSQOptions(ParmDBLog &log, const SolverOptions &options)
    {
        log.setSolverKeywords(options.epsValue, options.epsDerivative,
            options.maxIter, options.colFactor, options.lmFactor);
    }

    void logCellStats(ParmDBLog &log, const Box &box, Cell &cell)
    {
        const ParmDBLoglevel level = log.getLoggingLevel();
        if(level.is(ParmDBLoglevel::NONE))
        {
            return;
        }

        // Get some statistics from the solver. Note that the chi squared is
        // valid for the _previous_ iteration. The solver cannot compute the chi
        // squared directly after an iteration, because it needs the new
        // condition equations for that and these are computed by the kernel.
        casa::uInt rank, nun, np, ncon, ner, *piv;
        casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
        cell.solver.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er,
            piv, sEq, sol, prec, nonlin);

        if(level.is(ParmDBLoglevel::PERITERATION)
            || (!cell.solver.isReady()
            && level.is(ParmDBLoglevel::PERITERATION_CORRMATRIX))
            || (cell.solver.isReady() && level.is(ParmDBLoglevel::PERSOLUTION)))
        {
        	log.add(box.lower().first, box.upper().first, box.lower().second,
                box.upper().second, cell.solver.nIterations(),
                cell.solver.isReady(), rank, cell.solver.getDeficiency(),
                cell.solver.getChi(), nonlin, cell.coeff,
                cell.solver.readyText());
        }
        else if(cell.solver.isReady()
            && (level.is(ParmDBLoglevel::PERSOLUTION_CORRMATRIX)
            || level.is(ParmDBLoglevel::PERITERATION_CORRMATRIX)))
        {
            casa::Array<double> corrMatrix(casa::IPosition(1,
                cell.solver.nUnknowns() * cell.solver.nUnknowns()));

            bool status = cell.solver.getCovariance(corrMatrix.data());
            ASSERT(status);

            log.add(box.lower().first, box.upper().first, box.lower().second,
                box.upper().second, cell.solver.nIterations(),
                cell.solver.isReady(), rank, cell.solver.getDeficiency(),
                cell.solver.getChi(), nonlin, cell.coeff,
                cell.solver.readyText(), corrMatrix);
        }
    }

    template <typename T>
    void initCells(const Location &start, const Location &end,
        const ParmGroup &solvables, size_t nCoeff,
        const EstimateOptions &options, T cell)
    {
        for(CellIterator it(start, end); !it.atEnd(); ++it, ++cell)
        {
            // Processing has not completed yet.
            cell->done = false;

            // Initalize LSQ solver.
            cell->solver = casa::LSQFit(static_cast<casa::uInt>(nCoeff));
            configLSQSolver(cell->solver, options.lsqOptions());

            // Initialize coefficients.
            cell->coeff.resize(nCoeff);
            loadCoeff(*it, solvables, cell->coeff.begin());

            // Clear RMS and sample counts.
            cell->rms = 0;
            cell->count = 0;

            // Initialize L1 epsilon value.
            cell->epsilonIdx = 0;
            cell->epsilon = options.algorithm() == EstimateOptions::L1
                ? options.epsilon(0) : 0.0;

            // Initialize RMS threshold and deactivate outlier detection.
            cell->flag = false;
            cell->thresholdIdx = 0;
            cell->threshold = numeric_limits<double>::infinity();
            cell->outliers = 0;
        }
    }

    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    CellProcessorReal<T_SAMPLE_MODIFIER,
        T_WEIGHT_MODIFIER>::CellProcessorReal(size_t nDerivative,
        flag_t mask, flag_t outlier)
        :   itsReDerivative(0),
            itsImDerivative(0),
            itsMask(mask),
            itsOutlierMask(outlier)
    {
        itsReDerivative = new double[nDerivative];
        itsImDerivative = new double[nDerivative];
    }

    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    CellProcessorReal<T_SAMPLE_MODIFIER,
        T_WEIGHT_MODIFIER>::~CellProcessorReal()
    {
        delete[] itsReDerivative;
        delete[] itsImDerivative;
    }

    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    void CellProcessorReal<T_SAMPLE_MODIFIER,
        T_WEIGHT_MODIFIER>::process(typename CellProcessorReal<T_SAMPLE_MODIFIER,
            T_WEIGHT_MODIFIER>::CellType &cell,
        const Interval<size_t> &freqInterval,
        const Interval<size_t> &timeInterval,
        boost::multi_array<flag_t, 4>::array_view<2>::type flagObs,
        boost::multi_array<dcomplex, 4>::const_array_view<2>::type visObs,
        boost::multi_array<double, 5>::const_array_view<2>::type covObs,
        const flag_t *flagSim, size_t flagIndex,
        const size_t (&flagStride)[2],
        const double *reSim, const double *imSim,
        const vector<double*> &reSimDerivative,
        const vector<double*> &imSimDerivative,
        size_t simIndex, const size_t (&simStride)[2],
        const vector<unsigned int> &coeffIndex,
        typename CellProcessorReal<T_SAMPLE_MODIFIER,
            T_WEIGHT_MODIFIER>::StatisticsType &statistics)
    {
        // Skip cell if it is inactive (converged or failed).
        if(cell.solver.isReady())
        {
            return;
        }

        size_t nDerivative = coeffIndex.size();
        size_t nFreq = (freqInterval.end - freqInterval.start + 1);

        statistics.inc(StatisticsType::C_ALL,
            (timeInterval.end - timeInterval.start + 1) * nFreq);

        for(size_t i = timeInterval.start; i <= timeInterval.end; ++i)
        {
            for(size_t j = freqInterval.start; j <= freqInterval.end; ++j,
                flagIndex += flagStride[FREQ], simIndex += simStride[FREQ])
            {
                // Unflag previously flagged outliers. They will be
                // reconsidered in this round of flagging.
                if(cell.flag)
                {
                    flagObs[i][j] &= ~itsOutlierMask;
                }

                // Skip flagged samples.
                if((flagObs[i][j] | flagSim[flagIndex]) & itsMask)
                {
                    statistics.inc(StatisticsType::C_FLAGGED);
                    continue;
                }

                // Skip samples with zero weight.
                double weight = 1.0 / covObs[i][j];
                if(weight == 0.0)
                {
                    statistics.inc(StatisticsType::C_ZERO_WEIGHT);
                    continue;
                }

                // Gather all values and derivatives from the various data
                // arrays into local variables.
                double reObs = real(visObs[i][j]);
                double imObs = imag(visObs[i][j]);
                double reSimTmp = reSim[simIndex];
                double imSimTmp = imSim[simIndex];
                for(size_t k = 0; k < nDerivative; ++k)
                {
                    itsReDerivative[k] = reSimDerivative[k][simIndex];
                    itsImDerivative[k] = imSimDerivative[k][simIndex];
                }

                // Modify the observed and simulated data depending on the
                // solving mode (complex, phase only, amplitude only).
                statistics.start(StatisticsType::T_MODIFIER);
                T_SAMPLE_MODIFIER::process(weight, reObs, imObs,
                    reSimTmp, imSimTmp, &(itsReDerivative[0]),
                    &(itsImDerivative[0]), nDerivative);
                statistics.stop(StatisticsType::T_MODIFIER);

                // Compute the residual.
                double residual = reObs - reSimTmp;

                // Filter out samples that are inf or nan.
                if(!isfinite(residual))
                {
                    statistics.inc(StatisticsType::C_INVALID_RESIDUAL);
                    continue;
                }

                if(!isfinite(weight))
                {
                    statistics.inc(StatisticsType::C_INVALID_WEIGHT);
                    continue;
                }

                bool finite = true;
                for(size_t k = 0; k < nDerivative; ++k)
                {
                    if(!(finite = isfinite(itsReDerivative[k])))
                    {
                        break;
                    }
                }

                if(!finite)
                {
                    statistics.inc(StatisticsType::C_INVALID_DERIVATIVE);
                    continue;
                }

                // Flag outliers when requested.
                double absResidualSqr = residual * residual;
                double weightedResidual = weight * sqrt(absResidualSqr);

                if(cell.flag && weightedResidual > cell.threshold)
                {
                    statistics.inc(StatisticsType::C_OUTLIER);

                    flagObs[i][j] |= itsOutlierMask;
                    ++cell.outliers;
                    continue;
                }

                // Update RMS.
                ++cell.count;
                cell.rms += weightedResidual * weightedResidual;

                // Modify weight.
                T_WEIGHT_MODIFIER::process(weight, absResidualSqr, cell);

                // Add condition equations.
                statistics.start(StatisticsType::T_MAKE_NORM);
                cell.solver.makeNorm(nDerivative, &(coeffIndex[0]),
                    &(itsReDerivative[0]), weight, residual);
                statistics.stop(StatisticsType::T_MAKE_NORM);
            }

            // Move cursors.
            flagIndex -= nFreq * flagStride[FREQ];
            flagIndex += flagStride[TIME];

            simIndex -= nFreq * simStride[FREQ];
            simIndex += simStride[TIME];
        }
    }

    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    CellProcessorComplex<T_SAMPLE_MODIFIER,
        T_WEIGHT_MODIFIER>::CellProcessorComplex(size_t nDerivative,
        flag_t mask, flag_t outlier)
        :   itsReDerivative(0),
            itsImDerivative(0),
            itsMask(mask),
            itsOutlierMask(outlier)
    {
        itsReDerivative = new double[nDerivative];
        itsImDerivative = new double[nDerivative];
    }

    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    CellProcessorComplex<T_SAMPLE_MODIFIER,
        T_WEIGHT_MODIFIER>::~CellProcessorComplex()
    {
        delete[] itsReDerivative;
        delete[] itsImDerivative;
    }

    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    void CellProcessorComplex<T_SAMPLE_MODIFIER,
        T_WEIGHT_MODIFIER>::process(typename CellProcessorComplex<T_SAMPLE_MODIFIER,
            T_WEIGHT_MODIFIER>::CellType &cell,
        const Interval<size_t> &freqInterval,
        const Interval<size_t> &timeInterval,
        boost::multi_array<flag_t, 4>::array_view<2>::type flagObs,
        boost::multi_array<dcomplex, 4>::const_array_view<2>::type visObs,
        boost::multi_array<double, 5>::const_array_view<2>::type covObs,
        const flag_t *flagSim, size_t flagIndex,
        const size_t (&flagStride)[2],
        const double *reSim, const double *imSim,
        const vector<double*> &reSimDerivative,
        const vector<double*> &imSimDerivative,
        size_t simIndex, const size_t (&simStride)[2],
        const vector<unsigned int> &coeffIndex,
        typename CellProcessorComplex<T_SAMPLE_MODIFIER,
            T_WEIGHT_MODIFIER>::StatisticsType &statistics)
    {
        // Skip cell if it is inactive (converged or failed).
        if(cell.solver.isReady())
        {
            return;
        }

        size_t nDerivative = coeffIndex.size();
        size_t nFreq = (freqInterval.end - freqInterval.start + 1);

        statistics.inc(StatisticsType::C_ALL,
            (timeInterval.end - timeInterval.start + 1) * nFreq);

        for(size_t i = timeInterval.start; i <= timeInterval.end; ++i)
        {
            for(size_t j = freqInterval.start; j <= freqInterval.end; ++j,
                flagIndex += flagStride[FREQ], simIndex += simStride[FREQ])
            {
                // Unflag previously flagged outliers. They will be
                // reconsidered in this round of flagging.
                if(cell.flag)
                {
                    flagObs[i][j] &= ~itsOutlierMask;
                }

                // Skip flagged samples.
                if((flagObs[i][j] | flagSim[flagIndex]) & itsMask)
                {
                    statistics.inc(StatisticsType::C_FLAGGED);
                    continue;
                }

                // Skip samples with zero weight.
                double weight = 1.0 / covObs[i][j];
                if(weight == 0.0)
                {
                    statistics.inc(StatisticsType::C_ZERO_WEIGHT);
                    continue;
                }

                // Gather all values and derivatives from the various data
                // arrays into local variables.
                double reObs = real(visObs[i][j]);
                double imObs = imag(visObs[i][j]);
                double reSimTmp = reSim[simIndex];
                double imSimTmp = imSim[simIndex];
                for(size_t k = 0; k < nDerivative; ++k)
                {
                    itsReDerivative[k] = reSimDerivative[k][simIndex];
                    itsImDerivative[k] = imSimDerivative[k][simIndex];
                }

                // Modify the observed and simulated data depending on the
                // solving mode (complex, phase only, amplitude only).
                statistics.start(StatisticsType::T_MODIFIER);
                T_SAMPLE_MODIFIER::process(weight, reObs, imObs,
                    reSimTmp, imSimTmp, &(itsReDerivative[0]),
                    &(itsImDerivative[0]), nDerivative);
                statistics.stop(StatisticsType::T_MODIFIER);

                // Compute the residual.
                double reResidual = reObs - reSimTmp;
                double imResidual = imObs - imSimTmp;

                // Filter out samples that are inf or nan.
                if(!isfinite(reResidual) || !isfinite(imResidual))
                {
                    statistics.inc(StatisticsType::C_INVALID_RESIDUAL);
                    continue;
                }

                if(!isfinite(weight))
                {
                    statistics.inc(StatisticsType::C_INVALID_WEIGHT);
                    continue;
                }

                bool finite = true;
                for(size_t k = 0; k < nDerivative; ++k)
                {
                    if(!(finite = isfinite(itsReDerivative[k])
                        && isfinite(itsImDerivative[k])))
                    {
                        break;
                    }
                }

                if(!finite)
                {
                    statistics.inc(StatisticsType::C_INVALID_DERIVATIVE);
                    continue;
                }

                // Flag outliers when requested.
                double absResidualSqr = reResidual * reResidual
                    + imResidual * imResidual;
                double weightedResidual = weight * sqrt(absResidualSqr);

                if(cell.flag && weightedResidual > cell.threshold)
                {
                    statistics.inc(StatisticsType::C_OUTLIER);

                    flagObs[i][j] |= itsOutlierMask;
                    ++cell.outliers;
                    continue;
                }

                // Update RMS.
                ++cell.count;
                cell.rms += weightedResidual * weightedResidual;

                // Modify weight (L1 regularization).
                T_WEIGHT_MODIFIER::process(weight, absResidualSqr, cell);

                // Add condition equations.
                statistics.start(StatisticsType::T_MAKE_NORM);
                cell.solver.makeNorm(nDerivative, &(coeffIndex[0]),
                    &(itsReDerivative[0]), weight, reResidual);
                cell.solver.makeNorm(nDerivative, &(coeffIndex[0]),
                    &(itsImDerivative[0]), weight, imResidual);
                statistics.stop(StatisticsType::T_MAKE_NORM);
            }

            // Move cursors.
            flagIndex -= nFreq * flagStride[FREQ];
            flagIndex += flagStride[TIME];

            simIndex -= nFreq * simStride[FREQ];
            simIndex += simStride[TIME];
        }
    }

    Statistics::Statistics()
    {
        fill(itsCounters, itsCounters + N_Counter, 0);
    }

    inline void Statistics::inc(Statistics::Counter counter)
    {
        ++itsCounters[counter];
    }

    inline void Statistics::inc(Statistics::Counter counter, size_t count)
    {
        itsCounters[counter] += count;
    }

    inline void Statistics::reset(Statistics::Counter counter)
    {
        itsCounters[counter] = 0;
    }

    inline void Statistics::reset(Statistics::Timer timer)
    {
        itsTimers[timer].reset();
    }

    inline void Statistics::start(Statistics::Timer timer)
    {
        itsTimers[timer].start();
    }

    inline void Statistics::stop(Statistics::Timer timer)
    {
        itsTimers[timer].stop();
    }

    void Statistics::reset()
    {
        fill(itsCounters, itsCounters + N_Counter, 0);

        for(size_t i = 0; i < N_Timer; ++i)
        {
            itsTimers[i].reset();
        }
    }

    string Statistics::counters() const
    {
        ostringstream oss;
        oss << "counters:";
        for(size_t i = 0; i < N_Counter; ++i)
        {
            oss << " " << theirCounterNames[i] << ": " << itsCounters[i];
        }
        return oss.str();
    }

    string Statistics::timers() const
    {
        ostringstream oss;
        oss << "timers:";
        for(size_t i = 0; i < N_Timer; ++i)
        {
            oss << " " << theirTimerNames[i] << ": " << toString(itsTimers[i]);
        }
        oss << " total/count/average";
        return oss.str();
    }

    string Statistics::theirCounterNames[Statistics::N_Counter] =
        {"all",
         "flagged",
         "zero weight",
         "invalid residual",
         "invalid derivative",
         "invalid weight",
         "outlier"};

    string Statistics::theirTimerNames[Statistics::N_Timer] =
        {"evaluate",
        "coeff map",
        "process cell",
        "modify sample",
        "condition eq"};

} //# anonymous namespace

} //# namespace BBS
} //# namespace LOFAR
