//# Prediffer.cc: Read and predict read visibilities
//#
//# Copyright (C) 2004
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

#include <BBSKernel/Prediffer.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/Exceptions.h>

#include <BBSKernel/MNS/MeqDomain.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqJonesResult.h>

#include <BBSKernel/MNS/MeqMatrix.h>
#include <BBSKernel/MNS/MeqMatrixRealArr.h>
#include <BBSKernel/MNS/MeqMatrixComplexArr.h>

#include <BBSKernel/MNS/MeqParm.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <BBSKernel/MNS/MeqFunklet.h>
#include <BBSKernel/MNS/MeqPhaseRef.h>

#include <Common/Timer.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/DataConvert.h>
#include <Common/lofar_algorithm.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>

#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Regex.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Quanta/MVTime.h>
#include <scimath/Fitting/LSQFit.h>

#include <measures/Measures.h>

#include <functional>
#include <stdexcept>
#include <limits>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef EXPR_GRAPH
#include <BBSKernel/MNS/MeqJonesExpr.h>
#endif

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;
using LOFAR::max;
using LOFAR::min;

string Prediffer::ThreadContext::timerNames[ThreadContext::N_Timer] =
    {"Model evaluation",
    "Process",
    "Grid look-up",
    "Pre-compute inverse perturbations",
    "Build coefficient index",
    "Compute partial derivatives",
    "casa::LSQFit::makeNorm()"};


Prediffer::Prediffer(Measurement::Pointer measurement,
        LOFAR::ParmDB::ParmDB sky,
        LOFAR::ParmDB::ParmDB instrument)
    :   itsMeasurement(measurement),
        itsSkyDb(sky),
        itsInstrumentDb(instrument)
{        
    LOG_INFO("Compile time options:");

#ifdef __SSE2__
    LOG_INFO("  SSE2: yes");
#else
    LOG_INFO("  SSE2: no");
#endif

#ifdef _OPENMP
    LOG_INFO("  OPENMP: yes");
#else
    LOG_INFO("  OPENMP: no");
#endif

#ifdef EXPR_GRAPH
    LOG_INFO("  EXPR_GRAPH: yes");
#else
    LOG_INFO("  EXPR_GRAPH: no");
#endif

//    LOG_INFO_STR("UVW source: " << (readUVW ? "read" : "computed"));
//    LOG_INFO_STR("UVW convention: " << (readUVW ? "as recorded in the input"
//        " measurement" : "ANTENNA2 - ANTENNA1 (AIPS++ convention)"));

   
    const VisDimensions &dims = itsMeasurement->getDimensions();
    itsPhaseRef = MeqPhaseRef(itsMeasurement->getPhaseCenter(),
        dims.getTimeRange().first);

    itsModel.reset(new Model(itsMeasurement->getInstrument(), itsParameters,
        &itsSkyDb, &itsPhaseRef));

    // Allocate thread private buffers.
#if defined _OPENMP
    LOG_INFO_STR("Number of threads: " << omp_get_max_threads());
    itsThreadContexts.resize(omp_get_max_threads());
#else
    itsThreadContexts.resize(1);
#endif
}


void Prediffer::attachChunk(VisData::Pointer chunk)
{
    // Check preconditions.
    DBGASSERT(chunk);

    // Set chunk.
    itsChunk = chunk;

    // Initialize the polarization map.
    initPolarizationMap();

    // Reset state and copy UVW coordinates.
    resetState();
    itsModel->setStationUVW(itsMeasurement->getInstrument(), itsChunk);

    // Reset the data selection.
    itsBaselineSelection.clear();
    itsPolarizationSelection.clear();
    
    // Fetch all parameter values that are valid for the chunk from the sky and
    // instrument parameter databases.
    loadParameterValues();
}


void Prediffer::detachChunk()
{
    // Remove the funklets from all parameters.
    for(MeqParmGroup::iterator it = itsParameters.begin();
        it != itsParameters.end();
        ++it)
    {
        it->second.removeFunklets();
    }
    
    // Clear all cached parameter values.
    itsParameterValues.clear();
    
    // Reset the data selection.
    itsBaselineSelection.clear();
    itsPolarizationSelection.clear();

    // TODO: Clear cached UVW values.
    resetState();
    
    // TODO: Clear polarization map.

    // Release chunk.
    itsChunk.reset();
}


void Prediffer::resetState()
{
    // Reset state.
    itsOperation = UNSET;

    // State related to the CONSTRUCT operation only.
    itsCellGrid = Grid();
    itsFreqIntervals.clear();
    itsTimeIntervals.clear();
    itsStartCell = Location();
    itsEndCell = Location();
    itsCoeffIndex.clear();
    itsCoeffMapping.clear();    

    // Reset solvable state of all parameters.
    resetSolvableParameters();

    // Reset model equations.
    itsModel->clearEquations();
}


bool Prediffer::setSelection(const string &filter,
    const vector<string> &stations1,
    const vector<string> &stations2,
    const vector<string> &polarizations)
{
    // Check preconditions.
    ASSERT(itsChunk);
    
    if(stations1.size() != stations2.size())
    {
        LOG_ERROR("Baselines.Station1 and Baselines.Station2 should always have"
            " the same length. Please correct configuration file.");
        return false;
    }
    
    // Clear baseline selection.
    itsBaselineSelection.clear();
    
    // Select baselines.
    set<baseline_t> blSelection;
    if(stations1.empty())
    {
        // If no station groups are speficied, select all the baselines
        // available in the chunk that match the baseline filter.
        const VisDimensions &dims = itsChunk->getDimensions();
        const vector<baseline_t> &baselines = dims.getBaselines();
        for(vector<baseline_t>::const_iterator it = baselines.begin(),
            end = baselines.end();
            it != end;
            ++it)
        {
            if(filter.empty()
                || (it->first == it->second && filter == "AUTO")
                || (it->first != it->second && filter == "CROSS"))
            {
                blSelection.insert(*it);
            }
        }
    }
    else
    {
        const VisDimensions &dims = itsChunk->getDimensions();
        const Instrument &instrument = itsMeasurement->getInstrument();

        vector<string>::const_iterator baseline_it1 = stations1.begin();
        vector<string>::const_iterator baseline_it2 = stations2.begin();

        while(baseline_it1 != stations1.end())
        {
            // Find the indices of all the stations of which the name matches
            // the regex specified in the context.
            set<size_t> stationGroup1, stationGroup2;
            casa::Regex regex1 = casa::Regex::fromPattern(*baseline_it1);
            casa::Regex regex2 = casa::Regex::fromPattern(*baseline_it2);

            for(size_t i = 0; i < instrument.stations.size(); ++i)
            {
                casa::String stationName(instrument.stations[i].name);

                if(stationName.matches(regex1))
                {
                    stationGroup1.insert(i);
                }

                if(stationName.matches(regex2))
                {
                    stationGroup2.insert(i);
                }
            }

            // Generate all possible baselines (pairs) from the two groups of
            // station indices. If a baseline is available in the chunk _and_
            // matches the baseline filter, select it for processing.
            for(set<size_t>::const_iterator it1 = stationGroup1.begin();
                it1 != stationGroup1.end();
                ++it1)
            {
                for(set<size_t>::const_iterator it2 = stationGroup2.begin();
                it2 != stationGroup2.end();
                ++it2)
                {
                    if(filter.empty()
                        || (*it1 == *it2 && filter == "AUTO")
                        || (*it1 != *it2 && filter == "CROSS"))
                    {
                        baseline_t baseline(*it1, *it2);

                        if(dims.hasBaseline(baseline))
                        {
                            blSelection.insert(baseline);
                        }
                    }
                }
            }

            ++baseline_it1;
            ++baseline_it2;
        }
    }

    // Verify that at least one baseline is selected.
    if(blSelection.empty())
    {
        LOG_ERROR("Baseline selection did not match any baselines in the"
            " observation");
        return false;
    }

    // Copy selected baselines.
    itsBaselineSelection.resize(blSelection.size());
    copy(blSelection.begin(), blSelection.end(), itsBaselineSelection.begin());

    // Clear polarization selection.
    itsPolarizationSelection.clear();

    // Select polarizations by name.
    set<size_t> polSelection;
    if(polarizations.empty())
    {
        for(size_t i = 0; i < itsPolarizationMap.size(); ++i)
        {
            if(itsPolarizationMap[i] >= 0)
            {
                polSelection.insert(i);
            }
        }
    }
    else
    {
        const VisDimensions &dims = itsMeasurement->getDimensions();
        const vector<string> &available = dims.getPolarizations();

        for(size_t i = 0; i < available.size(); ++i)
        {
            // Only consider known polarizations.
            if(itsPolarizationMap[i] >= 0)
            {
                // Check if this polarization needs to be processed.
                for(size_t j = 0; j < polarizations.size(); ++j)
                {
                    if(available[i] == polarizations[j])
                    {
                        polSelection.insert(i);
                        break;
                    }
                }
            }
        }
    }

    // Verify that at least one polarization is selected.
    if(polSelection.empty())
    {
        LOG_ERROR("Polarization selection did not match any polarizations in"
            " the obervation.");
        return false;
    }

    // Copy selected polarizations.
    itsPolarizationSelection.resize(polSelection.size());
    copy(polSelection.begin(), polSelection.end(),
        itsPolarizationSelection.begin());

    return true;
}


void Prediffer::setModelConfig(OperationType operation,
    const ModelConfig &config)
{
    // Reset state.
    resetState();
    itsOperation = operation;
    
    Model::EquationType type = Model::UNSET;
    switch(operation)
    {
        case SIMULATE:
        case SUBTRACT:
        case CONSTRUCT:
            type = Model::SIMULATE;
            break;

        case CORRECT:
            type = Model::CORRECT;
            break;

        default:
            break;
    }
    ASSERT(type != Model::UNSET);

    // Construct model equations.
    itsModel->makeEquations(type, config, itsBaselineSelection, itsParameters,
        &itsInstrumentDb, &itsPhaseRef, itsChunk);

    // Initialize all funklets that intersect the chunk.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();

    Box bbox(visGrid.getBoundingBox());
    MeqDomain domain(bbox.start.first, bbox.end.first, bbox.start.second,
        bbox.end.second);
    
    for(MeqParmGroup::iterator it = itsParameters.begin();
        it != itsParameters.end();
        ++it)
    {
        it->second.fillFunklets(itsParameterValues, domain);
    }
}   


bool Prediffer::setSolvableParameters(const vector<string> &include,
        const vector<string> &exclude)
{
    ASSERT(itsOperation == CONSTRUCT);
    
    // Convert patterns to AIPS++ regular expression objects.
    vector<casa::Regex> includeRegex(include.size());
    vector<casa::Regex> excludeRegex(exclude.size());

    try
    {
        transform(include.begin(), include.end(), includeRegex.begin(),
            ptr_fun(casa::Regex::fromPattern));

        transform(exclude.begin(), exclude.end(), excludeRegex.begin(),
            ptr_fun(casa::Regex::fromPattern));
    }
    catch(casa::AipsError &ex)
    {
        LOG_ERROR_STR("Error parsing Parms/ExclParms pattern (exception: "
            << ex.what() << ")");
        return false;
    }

    // Clear the solvable flag of any parameter marked for solving and empty
    // the parameter selection.
    resetSolvableParameters();
    
    // Find all parameters matching context.unknowns, exclude those that
    // match context.excludedUnknowns.
    for(MeqParmGroup::iterator parm_it = itsParameters.begin();
        parm_it != itsParameters.end();
        ++parm_it)
    {
        casa::String name(parm_it->first);

        // Loop through all regex-es until a match is found.
        for(vector<casa::Regex>::iterator include_it = includeRegex.begin();
            include_it != includeRegex.end();
            ++include_it)
        {
            if(name.matches(*include_it))
            {
                bool includeFlag = true;

                // Test if excluded.
                for(vector<casa::Regex>::const_iterator exclude_it =
                    excludeRegex.begin();
                    exclude_it != excludeRegex.end();
                    ++exclude_it)
                {
                    if(name.matches(*exclude_it))
                    {
                        includeFlag = false;
                        break;
                    }
                }

                if(includeFlag)
                {
                    itsSolvableParameters.push_back(parm_it->second);
                }
                
                break;
            }
        }
    }

    return !itsSolvableParameters.empty();
}


void Prediffer::resetSolvableParameters()
{
    itsSolvableParameters.clear();

    for (MeqParmGroup::iterator it = itsParameters.begin();
        it != itsParameters.end();
        ++it)
    {
        it->second.unsetSolvable();
    }
}


bool Prediffer::setCellGrid(const Grid &cellGrid)
{
    ASSERT(itsOperation == CONSTRUCT);

    // Compute intersection of the global cell grid and the current chunk.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();

    Box intersection = visGrid.getBoundingBox() & cellGrid.getBoundingBox();
    if(intersection.empty())
    {
        LOG_ERROR("No intersection between global cell grid and current"
            " chunk.");
        return false;
    }

    // Keep a copy of the global cell grid.
    // TODO: use clone()?
    itsCellGrid = cellGrid;

    // Find the first and last solution cell that intersect the current chunk.
    itsStartCell = itsCellGrid.locate(intersection.start);
    itsEndCell = itsCellGrid.locate(intersection.end, false);

    // The end cell is inclusive by convention.
    const size_t nFreqCells = itsEndCell.first - itsStartCell.first + 1;
    const size_t nTimeCells = itsEndCell.second - itsStartCell.second + 1;
    const size_t nCells = nFreqCells * nTimeCells;

    LOG_DEBUG_STR("Cells in chunk: [(" << itsStartCell.first << ","
        << itsStartCell.second << "),(" << itsEndCell.first << ","
        << itsEndCell.second << ")]; " << nCells << " cell(s) in total");

    // Map cells to inclusive sample intervals (frequency axis).
    Axis::Pointer visAxis(visGrid[FREQ]);
    Axis::Pointer cellAxis(cellGrid[FREQ]);

    // DEBUG DEBUG
    ostringstream oss;
    oss << "Intervals FREQ: ";
    // DEBUG DEBUG

    Interval interval;
    itsFreqIntervals.resize(nFreqCells);
    for(size_t i = 0; i < nFreqCells; ++i)
    {
        interval.start =
            visAxis->locate(std::max(cellAxis->lower(itsStartCell.first + i),
                intersection.start.first));
        interval.end =
            visAxis->locate(std::min(cellAxis->upper(itsStartCell.first + i),
                intersection.end.first), false);
        itsFreqIntervals[i] = interval;

        // DEBUG DEBUG
        oss << "[" << interval.start << "," << interval.end << "] ";
        // DEBUG DEBUG
    }

    // DEBUG DEBUG
    LOG_DEBUG(oss.str());
    // DEBUG DEBUG
    
    // Map cells to inclusive sample intervals (time axis).
    visAxis = visGrid[TIME];
    cellAxis = itsCellGrid[TIME];
    
    // DEBUG DEBUG
    oss.str("");
    oss << "Intervals TIME: ";
    // DEBUG DEBUG

    interval = Interval();
    itsTimeIntervals.resize(nTimeCells);
    for(size_t i = 0; i < nTimeCells; ++i)
    {
        interval.start =
            visAxis->locate(std::max(cellAxis->lower(itsStartCell.second + i),
                intersection.start.second));
        interval.end =
            visAxis->locate(std::min(cellAxis->upper(itsStartCell.second + i),
                intersection.end.second), false);
        itsTimeIntervals[i] = interval;
        
        // DEBUG DEBUG
        oss << "[" << interval.start << "," << interval.end << "] ";
        // DEBUG DEBUG
    }
    
    // DEBUG DEBUG
    LOG_DEBUG(oss.str());
    // DEBUG DEBUG

    // Initialize model parameters selected for solving and construct
    // coefficient index.
    vector<MeqDomain> domains;
    for(size_t t = itsStartCell.second; t <= itsEndCell.second; ++t)
    {
        for(size_t f = itsStartCell.first; f <= itsEndCell.first; ++f)
        {
            Box domain = itsCellGrid.getCell(Location(f, t));
            domains.push_back(MeqDomain(domain.start.first, domain.end.first,
                domain.start.second, domain.end.second));
        }
    }

    itsCoeffIndex.clear();
    for(vector<MeqPExpr>::iterator it = itsSolvableParameters.begin();
        it != itsSolvableParameters.end();
        ++it)
    {
        // NOTE: initDomains() should come before setSolvable(), because
        // it can modify the number of funklets of a Parm.
        it->initDomain(domains);

        int nCoeff = it->setSolvable(itsCoeffIndex.getCoeffCount());
        itsCoeffIndex.insert(it->getName(), nCoeff);
    }

//    LOG_DEBUG_STR("CoeffIndex: " << endl << itsCoeffIndex);
    return (itsCoeffIndex.getCoeffCount() > 0);
}


const CoeffIndex &Prediffer::getCoeffIndex() const
{
    ASSERT(itsOperation == CONSTRUCT);
    return itsCoeffIndex;
}


void Prediffer::setCoeffIndex(const CoeffIndex &global)
{
    ASSERT(itsOperation == CONSTRUCT);

    itsCoeffMapping.resize(itsSolvableParameters.size());
    for(size_t i = 0; i < itsSolvableParameters.size(); ++i)
    {
        const MeqPExpr &parm = itsSolvableParameters[i];
        
        // Note: log(N) look-up.
        CoeffIndex::const_iterator it = global.find(parm.getName());
        ASSERT(it != global.end());
        
        itsCoeffMapping[i] = it->second;
    }
}


void Prediffer::getCoeff(const Location &cell, vector<double> &coeff) const
{
    // Check preconditions.
    ASSERT(itsOperation == CONSTRUCT);

    // Get local cell id.
    const size_t localId = getLocalCellId(cell);

    // Copy the coefficient values for every selected parameter.
    coeff.reserve(itsCoeffIndex.getCoeffCount());
    for(size_t i = 0; i < itsSolvableParameters.size(); ++i)
    {
        const MeqPExpr &parm = itsSolvableParameters[i];
        ASSERT(localId < parm.getFunklets().size());

        const MeqFunklet *funklet = parm.getFunklets()[localId];
        const MeqMatrix &funkCoeff = funklet->getCoeff();
        const double *funkCoeffValues = funkCoeff.doubleStorage();
        const vector<bool> &funkCoeffMask = funklet->getSolvMask();
        
        for(size_t j = 0; j < static_cast<size_t>(funkCoeff.nelements()); ++j)
        {
            if(funkCoeffMask[j])
            {
                coeff.push_back(funkCoeffValues[j]);
            }
        }
    }
}


void Prediffer::getCoeff(Location start, Location end, vector<CellCoeff> &local)
    const
{    
    ASSERT(itsOperation == CONSTRUCT);

    LOG_DEBUG_STR("Request coeff for [(" << start.first << "," << start.second
        << "),(" << end.first << "," << end.second << ")]");

    // Check preconditions.
    ASSERT(start.first <= end.first && start.second <= end.second);
    ASSERT(start.first <= itsEndCell.first && end.first >= itsStartCell.first);
    ASSERT(start.second <= itsEndCell.second
        && end.second >= itsStartCell.second);

    // Clip against chunk.
    start.first = max(start.first, itsStartCell.first);
    start.second = max(start.second, itsStartCell.second);
    end.first = min(end.first, itsEndCell.first);
    end.second = min(end.second, itsEndCell.second);
    
    LOG_DEBUG_STR("Getting coeff for [(" << start.first << "," << start.second
        << "),(" << end.first << "," << end.second << ")]");

    size_t nCells = (end.first - start.first + 1)
        * (end.second - start.second + 1);
        
    local.resize(nCells);

    // Loop over all requested cells and copy the coefficient values for
    // each parameter.
    Location cell;
    size_t idx = 0;
    for(cell.second = start.second; cell.second <= end.second; ++cell.second)
    {
        for(cell.first = start.first; cell.first <= end.first; ++cell.first)
        {
            // Get global cell id.
            local[idx] = CellCoeff(itsCellGrid.getCellId(cell));
            
            // Get coefficients.
            getCoeff(cell, local[idx].coeff);

            // Move to next cell.
            ++idx;
        }
    }
}    


void Prediffer::setCoeff(const Location &cell, const vector<double> &coeff)
{
    // Check preconditions.
    ASSERT(itsOperation == CONSTRUCT);

    // Get local cell id.
    const size_t localId = getLocalCellId(cell);
    LOG_DEBUG_STR("Local cell id: " << localId);

    for(size_t j = 0; j < itsSolvableParameters.size(); ++j)
    {
        MeqPExpr &parm = itsSolvableParameters[j];
        parm.update(localId, coeff, itsCoeffMapping[j].start);
    }
}


void Prediffer::setCoeff(const vector<CellSolution> &global)
{
    ASSERT(itsOperation == CONSTRUCT);

    for(size_t i = 0; i < global.size(); ++i)
    {
        const CellSolution &solution = global[i];

        // Skip solutions for cells outside the chunk.
        Location cell = itsCellGrid.getCellLocation(solution.id);
        if(cell.first > itsEndCell.first
            || cell.first < itsStartCell.first
            || cell.second > itsEndCell.second
            || cell.second < itsStartCell.second)
        {
            continue;
        }            
        
        setCoeff(cell, solution.coeff);
    }
}


void Prediffer::simulate()
{
    ASSERT(itsOperation == SIMULATE);

    resetTimers();

    const VisDimensions &dims = itsChunk->getDimensions();
    Location visStart(0, 0);
    Location visEnd(dims.getChannelCount() - 1, dims.getTimeslotCount() - 1);

    LOG_DEBUG_STR("Processing visbility domain: [ (" << visStart.first << ","
        << visStart.second << "), (" << visEnd.first << "," << visEnd.second
        << ") ]");

    itsProcessTimer.start();
    process(false, true, visStart, visEnd, &Prediffer::copyBl);
    itsProcessTimer.stop();
    
    printTimers("SIMULATE");
}


void Prediffer::subtract()
{
    ASSERT(itsOperation == SUBTRACT);
    
    resetTimers();

    const VisDimensions &dims = itsChunk->getDimensions();
    Location visStart(0, 0);
    Location visEnd(dims.getChannelCount() - 1, dims.getTimeslotCount() - 1);

    LOG_DEBUG_STR("Processing visbility domain: [ (" << visStart.first << ","
        << visStart.second << "), (" << visEnd.first << "," << visEnd.second
        << ") ]");

    itsProcessTimer.start();
    process(false, true, visStart, visEnd, &Prediffer::subtractBl);
    itsProcessTimer.stop();
    
    printTimers("SUBTRACT");
}


void Prediffer::correct()
{
    ASSERT(itsOperation == CORRECT);
    
    resetTimers();

    const VisDimensions &dims = itsChunk->getDimensions();
    Location visStart(0, 0);
    Location visEnd(dims.getChannelCount() - 1, dims.getTimeslotCount() - 1);

    LOG_DEBUG_STR("Processing visbility domain: [ (" << visStart.first << ","
        << visStart.second << "), (" << visEnd.first << "," << visEnd.second
        << ") ]");

    itsProcessTimer.start();
    process(false, true, visStart, visEnd, &Prediffer::copyBl);
    itsProcessTimer.stop();
    
    printTimers("CORRECT");
}


void Prediffer::construct(Location start, Location end,
    vector<CellEquation> &local)
{
    ASSERT(itsOperation == CONSTRUCT);

    // Check preconditions.
    ASSERT(start.first <= end.first && start.second <= end.second);
    ASSERT(start.first <= itsEndCell.first && end.first >= itsStartCell.first);
    ASSERT(start.second <= itsEndCell.second
        && end.second >= itsStartCell.second);

    resetTimers();

    // Clip request against chunk.
    start.first = max(start.first, itsStartCell.first);
    start.second = max(start.second, itsStartCell.second);
    end.first = min(end.first, itsEndCell.first);
    end.second = min(end.second, itsEndCell.second);

    const size_t nCells = (end.first - start.first + 1)
        * (end.second - start.second + 1);

    LOG_DEBUG_STR("Constructing equations for " << nCells << " cell(s).");

    // Allocate the result.
    local.resize(nCells);

    const uint32 nCoeff = itsCoeffIndex.getCoeffCount();

    size_t idx = 0;
    for(size_t t = start.second; t <= end.second; ++t)
    {
        for(size_t f = start.first; f <= end.first; ++f)
        {
            local[idx].id = itsCellGrid.getCellId(Location(f, t));
            local[idx].equation.set(nCoeff);
            ++idx;
        }
    }

    // Allocate thread-private contexts for multi-threaded execution.
    const size_t nThreads = itsThreadContexts.size();

    for(size_t i = 0; i < nThreads; ++i)
    {
        ThreadContext &context = itsThreadContexts[i];        
        context.equations.resize(nCells);
        context.coeffIndex.resize(nCoeff);
        context.perturbedRe.resize(nCoeff);
        context.perturbedIm.resize(nCoeff);
        context.partialRe.resize(nCoeff);
        context.partialIm.resize(nCoeff);
    }

    for(size_t i = 0; i < nCells; ++i)
    {
        itsThreadContexts[0].equations[i] = &(local[i].equation);
    }

#ifdef _OPENMP
    boost::multi_array<casa::LSQFit, 2> threadEquations;

    if(nThreads > 1)
    {
        threadEquations.resize(boost::extents[nThreads - 1][nCells]);

        for(size_t i = 1; i < nThreads; ++i)
        {
            ThreadContext &context = itsThreadContexts[i];
            
            for(size_t j = 0; j < nCells; ++j)
            {
                context.equations[j] = &(threadEquations[i - 1][j]);
                context.equations[j]->set(nCoeff);
            }        
        }
    }
#endif

    // Compute local start and end cell.
    start.first -= itsStartCell.first;
    start.second -= itsStartCell.second;
    end.first -= itsStartCell.first;
    end.second -= itsStartCell.second;

    Location localCells[2];
    localCells[0] = start;
    localCells[1] = end;

    // Compute start and end cell on the visibility grid
    Location visStart(itsFreqIntervals[start.first].start,
        itsTimeIntervals[start.second].start);
    Location visEnd(itsFreqIntervals[end.first].end,
        itsTimeIntervals[end.second].end);
    
    LOG_DEBUG_STR("Processing visbility domain: [ (" << visStart.first
        << "," << visStart.second << "), (" << visEnd.first << ","
        << visEnd.second << ") ]");

    // Generate equations.
    itsProcessTimer.start();
    process(true, true, visStart, visEnd, &Prediffer::constructBl,
        static_cast<void*>(&localCells[0]));
    itsProcessTimer.stop();
    
#ifdef _OPENMP
    if(nThreads > 1)
    {
        // Merge thread equations into the result.
        // TODO: Re-implement the following in O(log(N)) steps.
        ThreadContext &main = itsThreadContexts[0];

      	for(size_t i = 1; i < nThreads; ++i)
    	{
            ThreadContext &context = itsThreadContexts[i];
            
            for(size_t j = 0; j < nCells; ++j)
            {
                main.equations[j]->merge(*context.equations[j]);
            }
        }
    }
#endif

    printTimers("CONSTRUCT");

    for(size_t i = 0; i < nThreads; ++i)
    {
        itsThreadContexts[i] = ThreadContext();
    }
}


//-----------------------[ Computation ]-----------------------//
void Prediffer::process(bool, bool precalc, const Location &start,
    const Location &end, BlProcessor processor, void *arguments)
{
    ASSERT(start.first <= end.first);
    ASSERT(start.second <= end.second);
    
    // Get frequency / time grid information.
    const size_t nChannels = end.first - start.first + 1;
    const size_t nTimeslots = end.second - start.second + 1;

//    cout << "nChannels: " << nChannels << " nTimeslots: " << nTimeslots << endl;

    // Determine if perturbed values have to be calculated.
    const int nPerturbedValues =
        (itsOperation == CONSTRUCT) ? itsCoeffIndex.getCoeffCount() : 0;

    // NOTE: Temporary vector; should be removed after MNS overhaul.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();
    vector<double> timeAxis(nTimeslots + 1);
    for(size_t i = 0; i < nTimeslots; ++i)
    {
        timeAxis[i] = visGrid[TIME]->lower(start.second + i);
    }
    timeAxis[nTimeslots] = visGrid[TIME]->upper(end.second);

    // TODO: Fix memory pool implementation.    
    // Initialize the ComplexArr pool with the most frequently used size.
//    uint64 defaultPoolSize = nChannels * nTimeslots;
//    MeqMatrixComplexArr::poolDeactivate();
//    MeqMatrixRealArr::poolDeactivate();
//    MeqMatrixComplexArr::poolActivate(defaultPoolSize);
//    MeqMatrixRealArr::poolActivate(defaultPoolSize);

    // Create a request.
    MeqDomain reqDomain(visGrid[FREQ]->lower(start.first),
        visGrid[FREQ]->upper(end.first),
        visGrid[TIME]->lower(start.second),
        visGrid[TIME]->upper(end.second));

    MeqRequest request(reqDomain, nChannels, timeAxis, nPerturbedValues);
    request.setOffset(start);

    // Precalculate if required.
    if(precalc)
    {
        itsPrecalcTimer.start();
        itsModel->precalculate(request);
        itsPrecalcTimer.stop();
    }
    
    // Process all selected baselines.
    ASSERT(itsBaselineSelection.size()
           <= static_cast<size_t>(std::numeric_limits<int>::max()));

#pragma omp parallel for schedule(dynamic)
    for(int i = 0; i < static_cast<int>(itsBaselineSelection.size()); ++i)
    {
#ifdef _OPENMP
        (this->*processor)(omp_get_thread_num(), itsBaselineSelection[i], start,
            request, arguments);
#else
        (this->*processor)(0, itsBaselineSelection[i], start, request,
            arguments);
#endif
    }
}


void Prediffer::copyBl(size_t threadNr, const baseline_t &baseline,
    const Location &offset, const MeqRequest &request, void*)
{
    // Get thread context.
    ThreadContext &context = itsThreadContexts[threadNr];
    
    // Evaluate the model.
    context.timers[ThreadContext::MODEL_EVALUATION].start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    context.timers[ThreadContext::MODEL_EVALUATION].stop();

    // Process the result.
    context.timers[ThreadContext::PROCESS].start();

    // Put the results in a single array for easier handling.
    const double *modelVisRe[4], *modelVisIm[4];
    jresult.getResult11().getValue().dcomplexStorage(modelVisRe[0],
        modelVisIm[0]);
    jresult.getResult12().getValue().dcomplexStorage(modelVisRe[1],
        modelVisIm[1]);
    jresult.getResult21().getValue().dcomplexStorage(modelVisRe[2],
        modelVisIm[2]);
    jresult.getResult22().getValue().dcomplexStorage(modelVisRe[3],
        modelVisIm[3]);

    // Get information about request grid.
    const size_t nChannels = request.nx();
    const size_t nTimeslots = request.ny();

    // Update statistics.
    context.visCount += nChannels * nTimeslots;
    
    // Check preconditions.
    const VisDimensions &dims = itsChunk->getDimensions();
    DBGASSERT(offset.first + nChannels <= dims.getChannelCount());
    DBGASSERT(offset.second + nTimeslots <= dims.getTimeslotCount());

    // Get baseline index.
    const size_t bl = dims.getBaselineIndex(baseline);

    // Copy visibilities.
    for(size_t ts = offset.second; ts < offset.second + nTimeslots; ++ts)
    {
        for(size_t pol = 0; pol < itsPolarizationSelection.size(); ++pol)
        {
            // External (Measurement) polarization index.
            const size_t ext_pol = itsPolarizationSelection[pol];

            // Internal (MNS) polarization index.
            const int int_pol = itsPolarizationMap[ext_pol];

            const double *re = modelVisRe[int_pol];
            const double *im = modelVisIm[int_pol];
            for(size_t ch = 0; ch < nChannels; ++ch)
            {
                itsChunk->vis_data[bl][ts][offset.first + ch][ext_pol] =
                    makefcomplex(re[ch], im[ch]);
            }

            modelVisRe[int_pol] += nChannels;
            modelVisIm[int_pol] += nChannels;
        }
    }

    context.timers[ThreadContext::PROCESS].stop();
}


void Prediffer::subtractBl(size_t threadNr, const baseline_t &baseline,
    const Location &offset, const MeqRequest &request,
    void*)
{
    // Get thread context.
    ThreadContext &context = itsThreadContexts[threadNr];
    
    // Evaluate the model.
    context.timers[ThreadContext::MODEL_EVALUATION].start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    context.timers[ThreadContext::MODEL_EVALUATION].stop();

    // Process the result.
    context.timers[ThreadContext::PROCESS].start();

    // Put the results in a single array for easier handling.
    const double *modelVisRe[4], *modelVisIm[4];
    jresult.getResult11().getValue().dcomplexStorage(modelVisRe[0],
        modelVisIm[0]);
    jresult.getResult12().getValue().dcomplexStorage(modelVisRe[1],
        modelVisIm[1]);
    jresult.getResult21().getValue().dcomplexStorage(modelVisRe[2],
        modelVisIm[2]);
    jresult.getResult22().getValue().dcomplexStorage(modelVisRe[3],
        modelVisIm[3]);

    // Get information about request grid.
    const size_t nChannels = request.nx();
    const size_t nTimeslots = request.ny();

    // Update statistics.
    context.visCount += nChannels * nTimeslots;
    
    // Check preconditions.
    const VisDimensions &dims = itsChunk->getDimensions();
    DBGASSERT(offset.first + nChannels <= dims.getChannelCount());
    DBGASSERT(offset.second + nTimeslots <= dims.getTimeslotCount());

    // Get baseline index.
    const size_t bl = dims.getBaselineIndex(baseline);
    
    // Subtract visibilities.
    for(size_t ts = offset.second; ts < offset.second + nTimeslots; ++ts)
    {
        for(size_t pol = 0; pol < itsPolarizationSelection.size(); ++pol)
        {
            // External (Measurement) polarization index.
            const size_t ext_pol = itsPolarizationSelection[pol];

            // Internal (MNS) polarization index.
            const int int_pol = itsPolarizationMap[ext_pol];

            const double *re = modelVisRe[int_pol];
            const double *im = modelVisIm[int_pol];
            for(size_t ch = 0; ch < nChannels; ++ch)
            {
                itsChunk->vis_data[bl][ts][offset.first + ch][ext_pol] -=
                    makefcomplex(re[ch], im[ch]);
            }

            modelVisRe[int_pol] += nChannels;
            modelVisIm[int_pol] += nChannels;
        }
    }

    context.timers[ThreadContext::PROCESS].stop();
}


void Prediffer::constructBl(size_t threadNr, const baseline_t &baseline,
    const Location &offset, const MeqRequest &request, void *arguments)
{
    // Get thread context.
    ThreadContext &context = itsThreadContexts[threadNr];

    // Evaluate the model.
    context.timers[ThreadContext::MODEL_EVALUATION].start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    context.timers[ThreadContext::MODEL_EVALUATION].stop();

    // Process the result.
    context.timers[ThreadContext::PROCESS].start();
        
    // Get cells to process.
    const Location start = static_cast<Location*>(arguments)[0];
    const Location end = static_cast<Location*>(arguments)[1];

//    LOG_DEBUG_STR("Offset: (" << offset.first << "," << offset.second << ")");
//    LOG_DEBUG_STR("Processing cells [(" << start.first << "," << start.second
//        << "),(" << end.first << "," << end.second << ")]");

    // Get baseline index.
    const VisDimensions &dims = itsChunk->getDimensions();
    const size_t bl = dims.getBaselineIndex(baseline);

    // Get information about request grid.
    const size_t nChannels = request.nx();
//    const size_t nTimeslots = request.ny();
    
    // Put the results in a single array for easier handling.
    const MeqResult *modelVis[4];
    modelVis[0] = &(jresult.getResult11());
    modelVis[1] = &(jresult.getResult12());
    modelVis[2] = &(jresult.getResult21());
    modelVis[3] = &(jresult.getResult22());

    // Pre-allocate memory for inverse perturbations.
    vector<double> invDelta(itsCoeffIndex.getCoeffCount());

    // Construct equations.
    for(size_t pol = 0; pol < itsPolarizationSelection.size(); ++pol)
    {
        // External (Measurement) polarization index.
        size_t ext_pol = itsPolarizationSelection[pol];
        
        // Internal (MNS) polarization index.
        int int_pol = itsPolarizationMap[ext_pol];

        // Get the result for this polarization.
        const MeqResult &result = *modelVis[int_pol];
        
        // Get pointers to the main value.
        const double *predictRe, *predictIm;
        result.getValue().dcomplexStorage(predictRe, predictIm);

        // Determine which parameters have perturbed values (e.g. when
        // solving for station-bound parameters, only a few parameters
        // per baseline are relevant). Note that this information could be
        // different for each solution cell. However, for the moment, we will
        // assume it is the same for every cell.
        context.timers[ThreadContext::BUILD_INDEX].start();

        size_t nCoeff = 0;
        for(size_t i = 0; i < itsCoeffIndex.getCoeffCount(); ++i)
        {
            // TODO: Remove O(log(N)) look-up.
            if(result.isDefined(i))
            {
                context.coeffIndex[nCoeff] = i;
                
                const MeqMatrix &perturbed = result.getPerturbedValue(i);
                perturbed.dcomplexStorage(context.perturbedRe[nCoeff],
                    context.perturbedIm[nCoeff]);
                    
                ++nCoeff;
            }
        }

        context.timers[ThreadContext::BUILD_INDEX].stop();

        if(nCoeff == 0)
        {
            continue;
        }

        // Loop over all solution cells.
        Location cell;
        size_t eqIndex = 0;
        for(cell.second = start.second; cell.second <= end.second;
            ++cell.second)
        {
            for(cell.first = start.first; cell.first <= end.first; ++cell.first)
            {
                context.timers[ThreadContext::GRID_LOOKUP].start();
                
                const size_t cellId =
                    cell.second * (itsEndCell.first - itsStartCell.first + 1)
                        + cell.first;

                const Interval chInterval = itsFreqIntervals[cell.first];
                const Interval tsInterval = itsTimeIntervals[cell.second];

                size_t visOffset = (tsInterval.start - offset.second)
                    * nChannels + (chInterval.start - offset.first);
                
                casa::LSQFit *equation = context.equations[eqIndex];

//                cout << "[" << cellId << "] ";
//                cout << "BL: " << bl << " POL: " << ext_pol << endl;
//                cout << "(" << chStart << "," << tsStart << ")-(" << chEnd
//                    << "," << tsEnd << ") " << visOffset << endl;

                context.timers[ThreadContext::GRID_LOOKUP].stop();

                context.timers[ThreadContext::INV_DELTA].start();
                // Pre-compute inverse perturbations (cell specific).
                for(size_t i = 0; i < nCoeff; ++i)
                {
                    // TODO: Remove O(log(N)) look-up.
                    invDelta[i] =  1.0 / 
                        result.getPerturbation(context.coeffIndex[i],
                            cellId);
//                    const MeqParmFunklet *parm = result.getPerturbedParm(context.coeffIndex[i]);
//                    cout << parm->getName() << ": " << parm->getFunklets()[cellId]->getCoeff() << endl;
                }
                context.timers[ThreadContext::INV_DELTA].stop();
                
//                double sumRe = 0.0, sumIm = 0.0;

                for(size_t ts = tsInterval.start; ts <= tsInterval.end; ++ts)
                {
                    // Skip timeslot if flagged.
                    if(itsChunk->tslot_flag[bl][ts])
                    {
                        visOffset += nChannels;
                        continue;
                    }

                    // Construct two equations for each unflagged visibility.
                    for(size_t ch = chInterval.start; ch <= chInterval.end;
                        ++ch)
                    {
                        if(!itsChunk->vis_flag[bl][ts][ch][ext_pol])
                        {
                            // Update statistics.
                            ++context.visCount;

                            context.timers[ThreadContext::DERIVATIVES].start();
                            // Compute right hand side of the equation pair.
                            const sample_t obsVis =
                                itsChunk->vis_data[bl][ts][ch][ext_pol];

                            const double modelVisRe = predictRe[visOffset];
                            const double modelVisIm = predictIm[visOffset];

//                            sumRe += abs(real(obsVis) - modelVisRe);
//                            sumIm += abs(imag(obsVis) - modelVisIm);
                            
                            // Approximate partial derivatives (forward
                            // differences).
                            // TODO: Remove this transpose.                            
                            for(size_t i = 0; i < nCoeff; ++i)
                            {
                                context.partialRe[i] = 
                                    (context.perturbedRe[i][visOffset] -
                                        modelVisRe) * invDelta[i];
                                
                                context.partialIm[i] = 
                                    (context.perturbedIm[i][visOffset] -
                                        modelVisIm) * invDelta[i];
                            }

                            context.timers[ThreadContext::DERIVATIVES].stop();

                            context.timers[ThreadContext::MAKE_NORM].start();

                            equation->makeNorm(nCoeff,
                                &(context.coeffIndex[0]),
                                &(context.partialRe[0]),
                                1.0,
                                real(obsVis) - modelVisRe);
                                
                            equation->makeNorm(nCoeff,
                                &(context.coeffIndex[0]),
                                &(context.partialIm[0]),
                                1.0,
                                imag(obsVis) - modelVisIm);

                            context.timers[ThreadContext::MAKE_NORM].stop();
                        } // !itsChunk->vis_flag[bl][ts][ch][ext_pol]

                        // Move to next channel.
                        ++visOffset;
                    } // for(size_t ch = chStart; ch < chEnd; ++ch)
                    
                    // Move to next timeslot.
                    visOffset +=
                        nChannels - (chInterval.end - chInterval.start + 1);
                } // for(size_t ts = tsStart; ts < tsEnd; ++ts) 
                
//                LOG_DEBUG_STR("[" << cellId << "] " << setprecision(20) << sumRe
//                    << " " << sumIm);
                    
                // Move to next solution cell.
                ++eqIndex;
            } // for(cell.first = ...; cell.first < ...; ++cell.first)
        } // cell.second = ...; cell.second < ...; ++cell.second
    } // for(size_t pol = 0; pol < itsPolarizationSelection.size(); ++pol)

    context.timers[ThreadContext::PROCESS].stop();
}


/*
// NOTE: Experimental version, only works for 256 x 1 cells.
void Prediffer::constructBl(size_t threadNr, const baseline_t &baseline,
    const Location &offset, const MeqRequest &request, void *arguments)
{
    // Get thread context.
    ThreadContext &context = itsThreadContexts[threadNr];

    // Evaluate the model.
    context.timers[ThreadContext::MODEL_EVALUATION].start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    context.timers[ThreadContext::MODEL_EVALUATION].stop();

    // Process the result.
    context.timers[ThreadContext::PROCESS].start();
        
    // Get cells to process.
    const Location &start = static_cast<Location*>(arguments)[0];
    const Location &end = static_cast<Location*>(arguments)[1];

//    LOG_DEBUG_STR("Offset: (" << offset.first << "," << offset.second << ")");
//    LOG_DEBUG_STR("Processing cells (" << start.first << "," << start.second
//        << ") - (" << end.first << "," << end.second << ")");

    // Get baseline index.
    const size_t bl = itsChunk->dims.getBaselineIndex(baseline);

    // Get information about request grid.
    const size_t nChannels = request.nx();
    const size_t nTimeslots = request.ny();
    
    // Put the results in a single array for easier handling.
    const MeqResult *modelVis[4];
    modelVis[0] = &(jresult.getResult11());
    modelVis[1] = &(jresult.getResult12());
    modelVis[2] = &(jresult.getResult21());
    modelVis[3] = &(jresult.getResult22());

    // Construct equations.
    for(size_t pol = 0; pol < itsPolarizationSelection.size(); ++pol)
    {
        // External (Measurement) polarization index.
        size_t ext_pol = itsPolarizationSelection[pol];
        
        // Internal (MNS) polarization index.
        int int_pol = itsPolarizationMap[ext_pol];

        // Get the result for this polarization.
        const MeqResult &result = *modelVis[int_pol];
        
        // Get pointers to the main value.
        const double *predictRe, *predictIm;
        result.getValue().dcomplexStorage(predictRe, predictIm);

        // Determine which parameters have perturbed values (e.g. when
        // solving for station-bound parameters, only a few parameters
        // per baseline are relevant). Note that this information could be
        // different for each solution cell. However, for the moment, we will
        // assume it is the same for every cell.
        context.timers[ThreadContext::BUILD_INDEX].start();

        size_t nCoeff = 0;
        for(size_t i = 0; i < itsCoeffCount; ++i)
        {
            // TODO: Remove log(N) look-up in isDefined()!
            if(result.isDefined(i))
            {
                context.coeffIndex[nCoeff] = i;
                
                const MeqMatrix &perturbed = result.getPerturbedValue(i);
                perturbed.dcomplexStorage(context.perturbedRe[nCoeff],
                    context.perturbedIm[nCoeff]);
                    
                ++nCoeff;
            }
        }

        context.timers[ThreadContext::BUILD_INDEX].stop();

        if(nCoeff == 0)
        {
            continue;
        }

        size_t visOffset = 0;
        for(size_t ts = offset.second; ts < offset.second + nTimeslots; ++ts)
        {
            // Skip timeslot if flagged.
            if(itsChunk->tslot_flag[bl][ts])
            {
                visOffset += nChannels;
                continue;
            }

            size_t cellIdTime = ts;
            size_t solverIdTime = ts - offset.second;
            cellIdTime *= 256;
            solverIdTime *= 256;

            // Construct two equations for each unflagged visibility.
            for(size_t ch = offset.first; ch < offset.first + nChannels; ++ch)
            {
                if(!itsChunk->vis_flag[bl][ts][ch][ext_pol])
                {
                    size_t cellId = ch;
                    size_t solverId = ch - offset.first;
                    cellId += cellIdTime;
                    solverId += solverIdTime;

                    // Update statistics.
                    ++context.visCount;

                    context.timers[ThreadContext::DERIVATIVES].start();
                    // Compute right hand side of the equation pair.
                    const sample_t obsVis =
                        itsChunk->vis_data[bl][ts][ch][ext_pol];

                    const double modelVisRe = predictRe[visOffset];
                    const double modelVisIm = predictIm[visOffset];

                    const double diffRe = real(obsVis) - modelVisRe;
                    const double diffIm = imag(obsVis) - modelVisIm;

//                            sumRe += abs(real(obsVis) - modelVisRe);
//                            sumIm += abs(imag(obsVis) - modelVisIm);
                    
                    // Approximate partial derivatives (forward
                    // differences).
                    // TODO: Remove this transpose.                            
                    for(size_t i = 0; i < nCoeff; ++i)
                    {
                        double invDelta = 1.0 /
                            result.getPerturbation(context.coeffIndex[i],
                                cellId);

                        context.partialRe[i] = 
                            (context.perturbedRe[i][visOffset] -
                                modelVisRe) * invDelta;
                        
                        context.partialIm[i] = 
                            (context.perturbedIm[i][visOffset] -
                                modelVisIm) * invDelta;
                    }

                    context.timers[ThreadContext::DERIVATIVES].stop();

                    context.timers[ThreadContext::MAKE_NORM].start();

                    context.equations[solverId]->makeNorm(nCoeff,
                        &(context.coeffIndex[0]),
                        &(context.partialRe[0]),
                        1.0,
                        diffRe);
                        
                    context.equations[solverId]->makeNorm(nCoeff,
                        &(context.coeffIndex[0]),
                        &(context.partialIm[0]),
                        1.0,
                        diffIm);

                    context.timers[ThreadContext::MAKE_NORM].stop();
                } // !itsChunk->vis_flag[bl][ts][ch][ext_pol]

                // Move to next channel.
                ++visOffset;
            } // for(size_t ch = chStart; ch < chEnd; ++ch)

            // Move to next timeslot.
            //visOffset += nChannels - (chEnd - chStart);
        } // for(size_t ts = tsStart; ts < tsEnd; ++ts) 
    } // for(size_t pol = 0; pol < itsPolarizationSelection.size(); ++pol)

    context.timers[ThreadContext::PROCESS].stop();
}
*/


/*
void Prediffer::updateSolvableParms()
{
  // Iterate through all parms.
  for (MeqParmGroup::iterator iter = itsParmGroup.begin();
       iter != itsParmGroup.end();
       ++iter)
  {
    if (iter->second.isSolvable()) {
      iter->second.updateFromTable();

    //       streamsize prec = cout.precision();
    //       cout.precision(10);
    //       cout << "****Read: " << iter->second.getCoeffValues().getDouble()
    //     << " for parameter " << iter->second.getName() << endl;
    //       cout.precision (prec);
    }
  }
}
*/


#ifdef EXPR_GRAPH
void Prediffer::writeExpressionGraph(const string &fileName,
    baseline_t baseline)
{
    MeqJonesExpr& expr = itsExpr[baseline];
    ASSERTSTR(!expr.isNull(), "Trying to process an unknown baseline.");

    ofstream os(fileName.c_str());
    os << "digraph MeasurementEquation" << endl;
    os << "{" << endl;
    os << "node [shape=box];" << endl;
    expr.writeExpressionGraph(os);
    os << "}" << endl;
}
#endif


void Prediffer::resetTimers()
{
    itsPrecalcTimer.reset();
    itsProcessTimer.reset();
    
    for(size_t i = 0; i < itsThreadContexts.size(); ++i)
    {
        for(size_t j = 0; j < ThreadContext::N_Timer; ++j)
        {
            itsThreadContexts[i].visCount = 0;
            itsThreadContexts[i].timers[j].reset();
        }
    }
}


void Prediffer::printTimers(const string &operation)
{
    LOG_DEBUG_STR("Timings for " << operation);

    unsigned long long count = 0;
    for(size_t i = 0; i < itsThreadContexts.size(); ++i)
    {
        count += itsThreadContexts[i].visCount;
    }
    LOG_DEBUG_STR("Processing speed: " << (count / itsProcessTimer.getElapsed())
        << " vis/s");

    LOG_DEBUG_STR("Total time: " << itsProcessTimer.getElapsed() << " s");
    LOG_DEBUG_STR("> Precalculation: " << itsPrecalcTimer.getElapsed() * 1000.0
        << " ms");

    for(size_t j = 0; j < ThreadContext::N_Timer; ++j)
    {
        unsigned long long count = 0;
        double sum = 0.0;

        // Merge thread-local timers.
        for(size_t i = 0; i < itsThreadContexts.size(); ++i)
        {
            // Convert from s to ms.
            sum += itsThreadContexts[i].timers[j].getElapsed() * 1000.0;
            count += itsThreadContexts[i].timers[j].getCount();
        }
    
        if(count == 0)
        {
            LOG_DEBUG_STR("> " << ThreadContext::timerNames[j]
                << ": timer not used.");
        }
        else
        {
            LOG_DEBUG_STR("> " << ThreadContext::timerNames[j] << ": total: "
                << sum << " ms, count: " << count << ", avg: " << sum / count
                << " ms");
        }
    }
}


void Prediffer::initPolarizationMap()
{
    // Create a look-up table that maps external (measurement) polarization
    // indices to internal polarization indices. The internal indices are:
    // 0 - XX
    // 1 - XY
    // 2 - YX
    // 3 - YY
    // No assumptions can be made regarding the external polarization indices,
    // which is why we need this map in the first place.
    
    const VisDimensions &dims = itsChunk->getDimensions();
    const vector<string> &polarizations = dims.getPolarizations();
        
    itsPolarizationMap.resize(polarizations.size());
    for(size_t i = 0; i < polarizations.size(); ++i)
    {
        if(polarizations[i] == "XX")
        {
            itsPolarizationMap[i] = 0;
        }
        else if(polarizations[i] == "XY")
        {
            itsPolarizationMap[i] = 1;
        }
        else if(polarizations[i] == "YX")
        {
            itsPolarizationMap[i] = 2;
        }
        else if(polarizations[i] == "YY")
        {
            itsPolarizationMap[i] = 3;
        }
        else
        {
            LOG_WARN_STR("Don't know how to process polarization "
                << polarizations[i] << "; will be skipped.");
            itsPolarizationMap[i] = -1;
        }
    }
}


void Prediffer::loadParameterValues()
{
    // Remove the funklets from all parameters.
    for(MeqParmGroup::iterator it = itsParameters.begin();
        it != itsParameters.end();
        ++it)
    {
        it->second.removeFunklets();
    }

    // Clear all cached parameter values.
    itsParameterValues.clear();

    // Get all parameters that intersect the chunk.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();
    const Box bbox(visGrid.getBoundingBox());
    const LOFAR::ParmDB::ParmDomain domain(bbox.start.first, bbox.end.first,
        bbox.start.second, bbox.end.second);

    itsInstrumentDb.getValues(itsParameterValues, vector<string>(), domain);
    itsSkyDb.getValues(itsParameterValues, vector<string>(), domain);
}


void Prediffer::storeParameterValues()
{
    try
    {    
        itsSkyDb.lock(true);
        itsInstrumentDb.lock(true);
        
        for(size_t i = 0; i < itsSolvableParameters.size(); ++i)
        {
            itsSolvableParameters[i].save();
        }

        itsSkyDb.unlock();
        itsInstrumentDb.unlock();
    }
    catch(...)
    {
        itsSkyDb.unlock();
        itsInstrumentDb.unlock();
        throw;
    }                
}


size_t Prediffer::getLocalCellId(const Location &globalCell) const
{
    // Check preconditions.
    ASSERT(globalCell.first >= itsStartCell.first
        && globalCell.first <= itsEndCell.first);
    ASSERT(globalCell.second >= itsStartCell.second
        && globalCell.second <= itsEndCell.second);

    return (globalCell.second - itsStartCell.second)
        * (itsEndCell.first - itsStartCell.first + 1)
        + (globalCell.first - itsStartCell.first);
}

} // namespace BBS
} // namespace LOFAR
