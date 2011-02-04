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
#include <Common/lofar_math.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::sqrt;

namespace
{
    struct Cell
    {
        Location        location;

        casa::LSQFit    solver;
        vector<double>  coeff;

        double          rms;
        size_t          count;

        double          epsilon;
        unsigned int    epsilonIdx;

        bool            flag;
        double          threshold;
        unsigned int    thresholdIdx;
        size_t          outliers;
    };

    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    class CellProcessorReal
    {
    public:
        typedef Cell    CellType;

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
            Statistics &statistics);

    private:
        double  *itsReDerivative, *itsImDerivative;
        flag_t  itsMask, itsOutlierMask;
    };

    template <typename T_SAMPLE_MODIFIER, typename T_WEIGHT_MODIFIER>
    class CellProcessorComplex
    {
    public:
        typedef Cell    CellType;

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
            Statistics &statistics);

    private:
        double  *itsReDerivative, *itsImDerivative;
        flag_t  itsMask, itsOutlierMask;
    };

    // Helper function to initialize the LSQ solver of each Cell, and any
    // additional state the Cell may have.
    void initCells(const Location &start, const Location &end,
        const ParmGroup &solvables, size_t nCoeff,
        const EstimateOptions &options, boost::multi_array<Cell, 2> &cells);

    struct IterationStatus
    {
        unsigned int nActive, nConverged, nStopped, nNoReduction, nSingular;
    };

    // Perform a single iteration for all cells in the selection that have not
    // yet converged or failed, updating the parameter values to the new values
    // found.
    IterationStatus iterate(const Location &start, const Location &end,
        const ParmGroup &solvables, const EstimateOptions &options,
        boost::multi_array<Cell, 2> &cells);

    template <typename T_CELL_PROCESSOR>
    void estimateImpl(const VisBuffer::Ptr &buffer,
        const BaselineMask &baselines,
        const CorrelationMask &correlations,
        const MeasurementExpr::Ptr &model,
        const Grid &grid,
        const ParmGroup &solvables,
        const EstimateOptions &options);
} //# anonymous namespace

void estimate(const VisBuffer::Ptr &buffer,
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
                        WeightModifierL1> >(buffer, baselines, correlations,
                        model, grid, solvables, options);
                    break;
                case EstimateOptions::PHASE:
                    estimateImpl<CellProcessorComplex<SampleModifierPhase,
                        WeightModifierL1> >(buffer, baselines, correlations,
                        model, grid, solvables, options);
                    break;
                case EstimateOptions::COMPLEX:
                    estimateImpl<CellProcessorComplex<SampleModifierComplex,
                        WeightModifierL1> >(buffer, baselines, correlations,
                        model, grid, solvables, options);
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
                        WeightModifierL2> >(buffer, baselines, correlations,
                        model, grid, solvables, options);
                    break;
                case EstimateOptions::PHASE:
                    estimateImpl<CellProcessorComplex<SampleModifierPhase,
                        WeightModifierL2> >(buffer, baselines, correlations,
                        model, grid, solvables, options);
                    break;
                case EstimateOptions::COMPLEX:
                    estimateImpl<CellProcessorComplex<SampleModifierComplex,
                        WeightModifierL2> >(buffer, baselines, correlations,
                        model, grid, solvables, options);
                    break;
                default:
                    THROW(BBSKernelException, "Invalid mode selected.");
            };
            break;

        default:
            THROW(BBSKernelException, "Invalid algorithm selected.");
    }
}

EstimateOptions::EstimateOptions(Algorithm algorithm, Mode mode,
        size_t chunkSize, bool propagate, flag_t mask, flag_t outlierMask,
        const SolverOptions &options)
    :   itsAlgorithm(algorithm),
        itsMode(mode),
        itsChunkSize(chunkSize),
        itsPropagateFlag(propagate),
        itsMask(mask),
        itsOutlierMask(outlierMask),
        itsLSQOptions(options)
{
    // Epsilon values for L1 weighting.
    itsEpsilon.push_back(1e-4);
    itsEpsilon.push_back(1e-5);
    itsEpsilon.push_back(1e-6);

    // Default RMS thresholds taken from the AIPS CALIB help text.
    itsThreshold.push_back(7.0);
    itsThreshold.push_back(5.0);
    itsThreshold.push_back(4.0);
    itsThreshold.push_back(3.5);
    itsThreshold.push_back(3.0);
    itsThreshold.push_back(2.8);
    itsThreshold.push_back(2.6);
    itsThreshold.push_back(2.4);
    itsThreshold.push_back(2.2);
    itsThreshold.push_back(2.5);
}

EstimateOptions::Mode EstimateOptions::mode() const
{
    return itsMode;
}

size_t EstimateOptions::chunkSize() const
{
    return itsChunkSize;
}

bool EstimateOptions::propagate() const
{
    return itsPropagateFlag;
}

const SolverOptions &EstimateOptions::lsqOptions() const
{
    return itsLSQOptions;
}

namespace
{
    template <typename T_CELL_PROCESSOR>
    void estimateImpl(const VisBuffer::Ptr &buffer,
        const BaselineMask &baselines,
        const CorrelationMask &correlations,
        const MeasurementExpr::Ptr &model,
        const Grid &grid,
        const ParmGroup &solvables,
        const EstimateOptions &options)
    {
        ASSERTSTR(model->domain().contains(buffer->domain()), "The area in time,"
            " frequency covered by the visibility buffer extends outside the model"
            " domain.");

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

        // Make sure the model computes partial derivatives for the solvables.
        model->setSolvables(solvables);
        ASSERT(model->solvables() == solvables);

        // Make coefficient map.
        map<PValueKey, unsigned int> coeffMap;
        makeCoeffMap(solvables, inserter(coeffMap, coeffMap.begin()));

        LOG_INFO_STR("Selection: Baselines: " << blMap.size() << "/"
            << buffer->nBaselines() << " Correlations: " << crMap.size() << "/"
            << buffer->nCorrelations() << " Parameters: " << solvables.size()
            << "/" << model->nParms());
        LOG_INFO_STR("No. of coefficients to estimate: " << coeffMap.size());

        // Assign solution grid to solvables.
        ParmManager::instance().setGrid(grid, solvables);

        if(options.robust())
        {
            buffer->flagsAndWithMask(~options.outlierMask());
        }

        // Compute a mapping from cells of the solution grid to cell intervals
        // in the evaluation grid.
        vector<Interval<size_t> > cellMap[2];
        Interval<size_t> domain[2];
        domain[FREQ] = makeAxisMap(grid[FREQ], buffer->grid()[FREQ],
            back_inserter(cellMap[FREQ]));
        domain[TIME] = makeAxisMap(grid[TIME], buffer->grid()[TIME],
            back_inserter(cellMap[TIME]));

        // ---------------------------------------------------------------------
        // Process each chunk of cells in a loop.
        // ---------------------------------------------------------------------

        // Clip chunk size to the size of the solution grid.
        size_t chunkSize = options.chunkSize() == 0 ? grid[TIME]->size()
            : std::min(options.chunkSize(), grid[TIME]->size());

        // Compute the number of cell chunks to process.
        size_t nChunks = (grid[TIME]->size() + chunkSize - 1) / chunkSize;

        // Allocate cells.
        boost::multi_array<Cell, 2> cells;
        cells.resize(boost::extents[chunkSize][grid[FREQ]->size()]);

        // Process the solution grid in chunks.
        T_CELL_PROCESSOR processor(coeffMap.size(), options.mask(),
            options.outlierMask);

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
            chunkStart = Location(std::max(chunkStart.first, domain[FREQ].start),
                std::max(chunkStart.second, domain[TIME].start));
            chunkEnd = Location(std::min(chunkEnd.first, domain[FREQ].end),
                std::min(chunkEnd.second, domain[TIME].end));

            // If there are no cells for which visibility data is available, skip
            // the chunk.
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

            // Initialize LSQ solvers.
            initCells(chunkStart, chunkEnd, solvables, coeffMap.size(), options,
                cells);

            Statistics stats;
            IterationStatus status = {0, 0, 0, 0, 0};
            unsigned int nIterations = 0;
            while(true)
            {
                // Construct normal equations from the data and an evaluation of
                // the model based on the current parameter values.
                timerEquate.start();
                equate(chunkStart, chunkEnd, buffer, model, blMap, crMap, cellMap,
                    coeffMap, cells, processor, stats);
                timerEquate.stop();

                // Perform a single iteration.
                timerIterate.start();
                status = iterate(chunkStart, chunkEnd, solvables, options, cells);
                timerIterate.stop();
                ++nIterations;

                // Notify model that solvables have changed.
                model->solvablesChanged();

                // If no active cells remain in this chunk (i.e. all cells have
                // converged or have been stopped), then move to the next chunk
                // of cells.
                if(status.nActive == 0)
                {
                    break;
                }
            }
            timerChunk.stop();

            const size_t nCells = (chunkEnd.second - chunkStart.second + 1)
                * (chunkEnd.first - chunkStart.first + 1);
            LOG_DEBUG_STR("chunk: " << (chunk + 1) << "/" << nChunks << " cells: "
                << nCells << " iterations: " << nIterations
                << " status: " << status.nConverged << "/" << status.nStopped
                << "/" << status.nNoReduction << "/" << status.nSingular
                << " converged/stopped/noreduction/singular");
            LOG_DEBUG_STR("\t" << stats.counters());
            LOG_DEBUG_STR("\t" << stats.timers());
            LOG_DEBUG_STR("\ttimers: all: " << toString(timerChunk) << " equate: "
                << toString(timerEquate) << " iterate: " << toString(timerIterate)
                << " total/count/average");
        }

        // Ensure the model no longer produces partial derivatives.
        model->clearSolvables();

        if(options.robust())
        {
            buffer->flagsAndWithMask(~options.outlierMask());
        }
    }


    IterationStatus iterate(const Location &start, const Location &end,
        const ParmGroup &solvables, const EstimateOptions &options,
        boost::multi_array<Cell, 2> &cells)
    {
        LOG_DEBUG_STR("================================================================================" << endl);

        IterationStatus status = {0, 0, 0, 0, 0};
        for(CellIterator it(start, end); !it.atEnd(); ++it)
        {
            Cell &cell =
                cells[it->second - start.second][it->first - start.first];

            if(cell.count > 0)
            {
                cell.rms = sqrt(cell.rms / cell.count);
            }

            LOG_DEBUG_STR("cell: (" << it->first - start.first << "," << it->second - start.second << ") rms: "
                << cell.rms << " count: " << cell.count << " flag: "
                << cell.flag << " outliers: " << cell.outliers << " threshold: "
                << cell.threshold << " (" << cell.thresholdIdx << ") epsilon: "
                << cell.epsilon << " (" << cell.epsilonIdx << ")");

//                << " sumll: " << cell.sumll << " sumw: " << cell.sumw
//            casa::uInt rank, nun, np, ncon, ner, *piv;
//            casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
//            cell.solver.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er,
//                piv, sEq, sol, prec, nonlin);
//            ASSERT(er && ner > casa::LSQFit::SUMLL);

//            LOG_DEBUG_STR("cell: (" << it->first - start.first << "," << it->second - start.second << ") chi2: "
//                << cell.solver.getChi2() << " sd: " << cell.solver.getSD() << " sdw: "
//                << cell.solver.getWeightedSD() << " nc: " << er[casa::LSQFit::NC] << " sumll: "
//                << er[casa::LSQFit::SUMLL] << " sumw: " << er[casa::LSQFit::SUMWEIGHT] << " chi2: "
//                << er[casa::LSQFit::CHI2] << " isready: " << cell.solver.isReady());

            cell.flag = false;

            if(!cell.solver.isReady())
            {
                // Perform an iteration. Note that LSQFit::solveLoop()
                // only returns false if the normal equations are singular.
                // This can also be seen from the result of LSQFit::isReady(),
                // so we don't update the iteration status here but only skip
                // the update of the solvables.
                casa::uInt rank;
                if(cell.solver.solveLoop(rank, &(cell.coeff[0]),
                    options.lsqOptions().useSVD))
                {
                    storeSolvables(*it, solvables, cell.coeff.begin(),
                        cell.coeff.end());
                }
            }

            if(cell.solver.isReady()
                && options.algorithm() == EstimateOptions::L1
                && cell.epsilonIdx < options.nEpsilon())
            {
                ++cell.epsilonIdx;
                if(cell.epsilonIdx < options.nEpsilon())
                {
//                        it->coeffTmp = it->coeff;

//                        if(it->index > 1)
//                        {
//                            // Predict starting point.
//                            double d1 = std::sqrt(itsEpsilon[it->index - 1]) - std::sqrt(itsEpsilon[it->index - 2]);
//                            double d2 = std::sqrt(itsEpsilon[it->index]) - std::sqrt(itsEpsilon[it->index - 1]);
//                            for(size_t i = 0; i < itsCoeffCount; ++i)
//                            {
//                                it->coeff[i] += (it->coeff[i] - it->coeffPrev[i]) / (d1*d2);
//                            }
//                        }

//                        // Store a copy of the solutions for this epsilon.
//                        it->coeffPrev = it->coeffTmp;

                    // Move to next epsilon.
                    cell.solver = casa::LSQFit(static_cast<casa::uInt>(cell.coeff.size()));
                    configLSQSolver(cell.solver, options.lsqOptions());

                    cell.epsilon = options.epsilon(cell.epsilonIdx);
                }
            }

            if(cell.solver.isReady()
                && options.robust()
                && cell.thresholdIdx < options.nThreshold())
            {
                cell.solver = casa::LSQFit(static_cast<casa::uInt>(cell.coeff.size()));
                configLSQSolver(cell.solver, options.lsqOptions());

                cell.epsilon = 0.0;
                if(options.algorithm() == EstimateOptions::L1)
                {
                    cell.epsilon = options.epsilon(0);
                }
                cell.epsilonIdx = 0;

                cell.threshold = cell.rms * options.threshold(cell.thresholdIdx);
                cell.flag = true;

                 ++cell.thresholdIdx;
            }

            // Handle LSQFit status codes.
            switch(cell.solver.isReady())
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

            cell.rms = 0.0;
            cell.count = 0;
            cell.outliers = 0;
        }

        return status;
    }

    void initCells(const Location &start, const Location &end,
        const ParmGroup &solvables, size_t nCoeff,
        const EstimateOptions &options, boost::multi_array<Cell, 2> &cells)
    {
        for(CellIterator it(start, end); !it.atEnd(); ++it)
        {
            Cell &cell =
                cells[it->second - start.second][it->first - start.first];
            cell.location = *it;

            cell.solver = casa::LSQFit(static_cast<casa::uInt>(nCoeff));
            configLSQSolver(cell.solver, options.lsqOptions());

            cell.coeff.resize(nCoeff);
            loadSolvables(*it, solvables, cell.coeff.begin(), cell.coeff.end());

            cell.rms = 0;
            cell.count = 0;

            cell.epsilon = 0.0;
            if(options.algorithm() == EstimateOptions::L1)
            {
                cell.epsilon = options.epsilon(0);
            }
            cell.epsilonIdx = 0;

            cell.flag = false;
            cell.threshold = numeric_limits<double>::infinity();
            cell.thresholdIdx = 0;
            cell.outliers = 0;
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
        Statistics &statistics)
    {
        // Skip cell if it is inactive (converged or failed).
        if(cell.solver.isReady())
        {
            return;
        }

        size_t nDerivative = coeffIndex.size();
        size_t nFreq = (freqInterval.end - freqInterval.start + 1);

        statistics.inc(Statistics::C_ALL,
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
                    statistics.inc(Statistics::C_FLAGGED);
                    continue;
                }

                // Skip samples with zero weight.
                double weight = 1.0 / covObs[i][j];
                if(weight == 0.0)
                {
                    statistics.inc(Statistics::C_ZERO_WEIGHT);
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
                T_SAMPLE_MODIFIER::process(weight, reObs, imObs,
                    reSimTmp, imSimTmp, &(itsReDerivative[0]),
                    &(itsImDerivative[0]), nDerivative);

                // Compute the residual.
                double residual = reObs - reSimTmp;

                // Filter out samples that are inf or nan.
                if(!isfinite(residual))
                {
                    statistics.inc(Statistics::C_INVALID_RESIDUAL);
                    continue;
                }

                if(!isfinite(weight))
                {
                    statistics.inc(Statistics::C_INVALID_WEIGHT);
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
                    statistics.inc(Statistics::C_INVALID_DERIVATIVE);
                    continue;
                }

                // Flag outliers when requested.
                double absResidualSqr = residual * residual;
                double weightedResidual = weight * sqrt(absResidualSqr);

                if(cell.flag && weightedResidual > cell.threshold)
                {
                    statistics.inc(Statistics::C_OUTLIER);

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
                statistics.start(Statistics::T_MAKE_NORM);
                cell.solver.makeNorm(nDerivative, &(coeffIndex[0]),
                    &(itsReDerivative[0]), weight, residual);
                statistics.stop(Statistics::T_MAKE_NORM);
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
        Statistics &statistics)
    {
        // Skip cell if it is inactive (converged or failed).
        if(cell.solver.isReady())
        {
            return;
        }

        size_t nDerivative = coeffIndex.size();
        size_t nFreq = (freqInterval.end - freqInterval.start + 1);

        statistics.inc(Statistics::C_ALL,
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
                    statistics.inc(Statistics::C_FLAGGED);
                    continue;
                }

                // Skip samples with zero weight.
                double weight = 1.0 / covObs[i][j];
                if(weight == 0.0)
                {
                    statistics.inc(Statistics::C_ZERO_WEIGHT);
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
                T_SAMPLE_MODIFIER::process(weight, reObs, imObs,
                    reSimTmp, imSimTmp, &(itsReDerivative[0]),
                    &(itsImDerivative[0]), nDerivative);

                // Compute the residual.
                double reResidual = reObs - reSimTmp;
                double imResidual = imObs - imSimTmp;

                // Filter out samples that are inf or nan.
                if(!isfinite(reResidual) || !isfinite(imResidual))
                {
                    statistics.inc(Statistics::C_INVALID_RESIDUAL);
                    continue;
                }

                if(!isfinite(weight))
                {
                    statistics.inc(Statistics::C_INVALID_WEIGHT);
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
                    statistics.inc(Statistics::C_INVALID_DERIVATIVE);
                    continue;
                }

                // Flag outliers when requested.
                double absResidualSqr = reResidual * reResidual
                    + imResidual * imResidual;
                double weightedResidual = weight * sqrt(absResidualSqr);

                if(cell.flag && weightedResidual > cell.threshold)
                {
                    statistics.inc(Statistics::C_OUTLIER);

                    flagObs[i][j] |= itsOutlierMask;
                    ++cell.outliers;
                    continue;
                }

                // Update RMS.
                ++cell.count;
                cell.rms += weightedResidual * weightedResidual;

//                    cell.rms += weight * reResidual * reResidual;
//                    cell.rms += weight * imResidual * imResidual;
//                    cell.sumll += weight * reResidual * reResidual;
//                    cell.sumll += weight * imResidual * imResidual;
//                    cell.sumw += weight;
//                    cell.sumw += weight;

                // Modify weight (L1 regularization).
                T_WEIGHT_MODIFIER::process(weight, absResidualSqr, cell);

                // Add condition equations.
                statistics.start(Statistics::T_MAKE_NORM);
                cell.solver.makeNorm(nDerivative, &(coeffIndex[0]),
                    &(itsReDerivative[0]), weight, reResidual);
                cell.solver.makeNorm(nDerivative, &(coeffIndex[0]),
                    &(itsImDerivative[0]), weight, imResidual);
                statistics.stop(Statistics::T_MAKE_NORM);
            }

            // Move cursors.
            flagIndex -= nFreq * flagStride[FREQ];
            flagIndex += flagStride[TIME];

            simIndex -= nFreq * simStride[FREQ];
            simIndex += simStride[TIME];
        }
    }
} //# anonymous namespace

} //# namespace BBS
} //# namespace LOFAR
