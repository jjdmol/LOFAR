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

#if defined _OPENMP
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
using LOFAR::ParmDB::ParmDB;
using LOFAR::ParmDB::ParmDBMeta;

string ThreadContext::timerNames[ThreadContext::N_Timer] = {"Model evaluation",
    "Process",
    "Grid look-up",
    "Build coefficient index",
    "Compute partial derivatives",
    "casa::LSQFit::makeNorm()"};


Prediffer::Prediffer(uint32 id,
        Measurement::Pointer measurement,
        ParmDB::ParmDB sky,
        ParmDB::ParmDB instrument)
    :   itsId(id),
        itsMeasurement(measurement),
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

   
    // Initialize model.
//    casa::MEpoch startTimeMeas = itsMeasurement->getTimeRange().first;
//    casa::Quantum<casa::Double> startTime = startTimeMeas.get("s");
//    itsPhaseRef = MeqPhaseRef(itsMeasurement->getPhaseCenter(),
//        startTime.getValue("s"));

    itsPhaseRef = MeqPhaseRef(itsMeasurement->getPhaseCenter(),
        measurement->getTimeRange().first);

    itsModel.reset(new Model(itsMeasurement->getInstrument(), itsParameters,
        &itsSkyDb, &itsPhaseRef));

    // Allocate thread private buffers.
#if defined _OPENMP
    LOG_INFO_STR("Number of threads used: " << omp_get_max_threads());
    itsThreadContexts.resize(omp_get_max_threads());
#else
    itsThreadContexts.resize(1);
#endif
}


Prediffer::~Prediffer()
{
    LOG_TRACE_FLOW( "Prediffer destructor" );

    // Clean up the matrix allocation pool.
    MeqMatrixComplexArr::poolDeactivate();
    MeqMatrixRealArr::poolDeactivate();
}


void Prediffer::attachChunk(VisData::Pointer chunk)
{
    // Check preconditions.
    DBGASSERT(chunk);

    // Set chunk.
    itsChunk = chunk;

    // Initialize the polarization map.
    initPolarizationMap();

    // Reset the model and copy UVW coordinates.
    itsOperation = UNSET;
    itsModel->clearEquations();
    itsModel->setStationUVW(itsMeasurement->getInstrument(), itsChunk);

    // Reset the data selection.
    itsSelection = Selection();
    
    // Fetch all parameter values that are valid for the chunk from the sky and
    // instrument parameter databases.
    loadParameterValues();
}


void Prediffer::detachChunk()
{
    itsChunk.reset();
}


bool Prediffer::setSelection(const string &filter,
    const vector<string> &stations1,
    const vector<string> &stations2,
    const vector<string> &polarizations)
{
    // Check preconditions.
    DBGASSERT(itsChunk);
    DBGASSERT(stations1.size() == stations2.size());
    
    // Clear baseline selection.
    itsSelection.baselines.clear();
    
    // Select baselines.
    set<baseline_t> selection;
    if(stations1.empty())
    {
        // If no station groups are speficied, select all the baselines
        // available in the chunk that match the baseline filter.
        const vector<baseline_t> &baselines = itsChunk->dims.getBaselines();
        for(vector<baseline_t>::const_iterator it = baselines.begin(),
            end = baselines.end();
            it != end;
            ++it)
        {
            if(filter.empty()
                || (it->first == it->second && filter == "AUTO")
                || (it->first != it->second && filter == "CROSS"))
            {
                selection.insert(*it);
            }
        }
    }
    else
    {
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

            for(size_t i = 0; i < instrument.getStationCount(); ++i)
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

                        if(itsChunk->dims.hasBaseline(baseline))
                        {
                            selection.insert(baseline);
                        }
                    }
                }
            }

            ++baseline_it1;
            ++baseline_it2;
        }
    }

    itsSelection.baselines.resize(selection.size());
    copy(selection.begin(), selection.end(), itsSelection.baselines.begin());

    // Verify that at least one baseline is selected.
    if(selection.empty())
    {
        LOG_ERROR("Baseline selection did not match any baselines in the"
            " observation");
        return false;
    }

    // Clear polarization selection.
    itsSelection.polarizations.clear();

    // Select polarizations on name.
    if(polarizations.empty())
    {
        for(size_t i = 0; i < itsPolarizationMap.size(); ++i)
        {
            if(itsPolarizationMap[i] >= 0)
            {
                itsSelection.polarizations.insert(i);
            }
        }
    }
    else
    {
        const vector<string> &available = itsMeasurement->getPolarizations();
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
                        itsSelection.polarizations.insert(i);
                        break;
                    }
                }
            }
        }
    }

    // Verify that at least one polarization is selected.
    if(itsSelection.polarizations.empty())
    {
        LOG_ERROR("Polarization selection did not match any polarizations in"
            " the obervation.");
        return false;
    }

    return true;
}


void Prediffer::setModelConfig(OperationType operation,
    const vector<string> &components,
    const vector<string> &sources)
{
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

    DBGASSERT(type != Model::UNSET);

    itsModel->makeEquations(type, components, itsSelection.baselines, sources,
        itsParameters, &itsInstrumentDb, &itsPhaseRef, itsChunk);

    const Grid<double> &sampleGrid = itsChunk->dims.getGrid();
    Box<double> bbox = sampleGrid.getBoundingBox();
    MeqDomain domain(bbox.start.first, bbox.end.first, bbox.start.second,
        bbox.end.second);
            
    for(MeqParmGroup::iterator it = itsParameters.begin();
        it != itsParameters.end();
        ++it)
    {
        it->second.fillFunklets(itsParameterValues, domain);
    }
}   


bool Prediffer::setSolutionGrid(const Grid<double> &solutionGrid)
{
    const Grid<double> &sampleGrid = itsChunk->dims.getGrid();
    Box<double> bbox =
        sampleGrid.getBoundingBox() & solutionGrid.getBoundingBox();

    if(bbox.empty())
    {
        return false;
    }

    LOG_DEBUG_STR("BBox: (" << setprecision(15) << bbox.start.first << ","
        << bbox.start.second << ") - (" << bbox.end.first << ","
        << bbox.end.second << ")");
        
    itsContext.solutionGrid = solutionGrid;

    pair<Location, bool> result;
    result = solutionGrid.locate(bbox.start, true);
    DBGASSERT(result.second);
    Location start = result.first;

    result = solutionGrid.locate(bbox.end, false);
    DBGASSERT(result.second);
    Location end = result.first;

    itsContext.chunkStart = start;
    itsContext.chunkEnd = end;

    LOG_DEBUG_STR("Chunk start: (" << start.first << "," << start.second << ")");
    LOG_DEBUG_STR("Chunk end  : (" << end.first << "," << end.second << ")");

    const size_t nFreqDomains = (end.first - start.first + 1);
    const size_t nTimeDomains = (end.second - start.second + 1);
    const size_t nDomains = nFreqDomains * nTimeDomains;

    LOG_DEBUG_STR("Domain count: " << nDomains);

    // Map domain boundaries to sample boundaries (frequency axis).
    Axis<double>::Pointer domainAxis, sampleAxis;
    vector<size_t> boundaries(nFreqDomains + 1);

    domainAxis = solutionGrid[FREQ];
    sampleAxis = sampleGrid[FREQ];
    boundaries.front() = sampleAxis->locate(bbox.start.first, true);
    DBGASSERT(boundaries.front() < sampleAxis->size());
    boundaries.back() = sampleAxis->locate(bbox.end.first, true);
    DBGASSERT(boundaries.back() > boundaries.front() &&
        boundaries.back() <= sampleAxis->size());

    for(size_t i = 1; i < nFreqDomains; ++i)
    {
        boundaries[i] =
            sampleAxis->locate(domainAxis->lower(start.first + i), true);
        DBGASSERT(boundaries[i] < sampleAxis->size());
    }
    LOG_DEBUG_STR("Boundaries FREQ: " << boundaries);
    Axis<size_t>::Pointer fAxis(new IrregularAxis<size_t>(boundaries));

    // Map domain boundaries to sample boundaries (time axis).
    domainAxis = solutionGrid[TIME];
    sampleAxis = sampleGrid[TIME];
    boundaries.resize(nTimeDomains + 1);
    boundaries.front() = sampleAxis->locate(bbox.start.second, true);
    DBGASSERT(boundaries.front() < sampleAxis->size());
    boundaries.back() = sampleAxis->locate(bbox.end.second, true);
    DBGASSERT(boundaries.back() > boundaries.front() &&
        boundaries.back() <= sampleAxis->size());

    for(size_t i = 1; i < nTimeDomains; ++i)
    {
        boundaries[i] =
            sampleAxis->locate(domainAxis->lower(start.second + i), true);
        DBGASSERT(boundaries[i] < sampleAxis->size());
    }
    LOG_DEBUG_STR("Boundaries TIME: " << boundaries);
    Axis<size_t>::Pointer tAxis(new IrregularAxis<size_t>(boundaries));

    itsContext.domainGrid = Grid<size_t>(fAxis, tAxis);

    // Initialize model parameters marked for fitting.
    vector<MeqDomain> domains;
    for(size_t t = start.second; t <= end.second; ++t)
    {
        for(size_t f = start.first; f <= end.first; ++f)
        {
            Box<double> domain = solutionGrid.getCell(Location(f, t));
            domains.push_back(MeqDomain(domain.start.first, domain.end.first,
                domain.start.second, domain.end.second));
        }
    }

    int nCoefficients = 0;

    // Temporary vector (required by MeqParmFunklet::initDomain() but not
    // needed here).
    vector<int> tmp(nDomains, 0);
    for(MeqParmGroup::iterator it = itsParameters.begin();
        it != itsParameters.end();
        ++it)
    {
        // Determine maximal number of coefficients over all solution
        // domains for this parameter.
        it->second.initDomain(domains, nCoefficients, tmp);

/*
        if(it->second.isSolvable())
        {
            for(size_t i = 0; i < nDomains; ++i)
            {
                ....
            }
        }
*/        
    }
    itsContext.coefficientCount = nCoefficients;

    LOG_DEBUG_STR("nCoefficients: " << nCoefficients);
    return nCoefficients > 0;
}


bool Prediffer::setParameterSelection(const vector<string> &include,
        const vector<string> &exclude)
{
    DBGASSERT(itsOperation == CONSTRUCT);
    
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
    catch(std::exception &ex)
    {
        LOG_ERROR_STR("Error parsing Parms/ExclParms pattern (exception: "
            << ex.what() << ")");
        return false;
    }

    // Clear the list of parameters selected for fitting.
    itsContext.localParmSelection.clear();
    
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
                    itsContext.localParmSelection.push_back(parm_it->second);
                    parm_it->second.setSolvable(true);
                }
                
                break;
            }
        }
    }
    
    return true;
}


void Prediffer::clearParameterSelection()
{
    for (MeqParmGroup::iterator it = itsParameters.begin();
        it != itsParameters.end();
        ++it)
    {
        it->second.setSolvable(false);
    }
}


CoeffIndexMsg::Pointer Prediffer::getCoefficientIndex() const
{
    CoeffIndexMsg::Pointer msg(new CoeffIndexMsg(itsId));
    CoefficientIndex &dest = msg->getContents();
    
    Location localCell;
    for(localCell.second = 0;
        localCell.second < itsContext.domainGrid[TIME]->size();
        ++localCell.second)
    {
        for(localCell.first = 0;
            localCell.first < itsContext.domainGrid[FREQ]->size();
            ++localCell.first)
        {
            Location globalCell(localCell.first + itsContext.chunkStart.first,
                localCell.second + itsContext.chunkStart.second);
                
            const size_t localId = itsContext.domainGrid.getCellId(localCell);
            const size_t globalId =
                itsContext.solutionGrid.getCellId(globalCell);

            // Note: log(N) look-up.
            CellCoeffIndex &cell = dest[globalId];

            for(MeqParmGroup::const_iterator it = itsParameters.begin(),
                end = itsParameters.end();
                it != end;
                ++it)
            {
                if(it->second.isSolvable())
                {
                    // Note: log(N) look-up.
                    const uint32 parmId = (dest.insertParm(it->first)).first;
                    const MeqFunklet *funklet =
                        it->second.getFunklets()[localId];
                        
                    cell.setInterval(parmId, CoeffInterval(funklet->scidInx(),
                        funklet->ncoeff()));
                }
            }
        }
    }
    
    return msg;
}


void Prediffer::setCoefficientIndex(CoeffIndexMsg::Pointer msg)
{
    CoefficientIndex &index = msg->getContents();

    vector<bool> parmMask(index.getParmCount(), false);
    vector<uint32> parmMapping(index.getParmCount(), 0);
    for(size_t i = 0; i < itsContext.localParmSelection.size(); ++i)
    {
        const MeqPExpr &parm = itsContext.localParmSelection[i];
        
        // Note: log(N) look-up.
        pair<uint32, bool> result = index.findParm(parm.getName());
        parmMask[result.first] = result.second;
        parmMapping[result.first] = i;
    }
    cout << "Parameter mapping: " << parmMapping << endl;
    
    itsContext.localCoeffIndices.clear();
    for(CoefficientIndex::CoeffIndexType::const_iterator it = index.begin(),
        end = index.end();
        it != end;
        ++it)
    {
        const uint32 cellId = it->first;

        Location location = itsContext.solutionGrid.getCellLocation(cellId);
        if(location.first >= itsContext.chunkStart.first
            && location.first <= itsContext.chunkEnd.first
            && location.second >= itsContext.chunkStart.second
            && location.second <= itsContext.chunkEnd.second)
        {
            const CellCoeffIndex &global = it->second;
            // Note: log(N) look-up.
            CellCoeffIndex &local = itsContext.localCoeffIndices[cellId];

            for(CellCoeffIndex::const_iterator intIt = global.begin(),
                intEnd = global.end();
                intIt != intEnd;
                ++intIt)
            {
                DBGASSERT(intIt->first < index.getParmCount());
                if(parmMask[intIt->first])
                {
                    local.setInterval(parmMapping[intIt->first], intIt->second);
                }
            }
        }
    }    
}

CoefficientMsg::Pointer Prediffer::getCoefficients(const Location &start,
    const Location &end) const
{
    return CoefficientMsg::Pointer(new CoefficientMsg(itsId));
}    

void Prediffer::setCoefficients(SolutionMsg::Pointer msg)
{
}

void Prediffer::storeParameterValues()
{
}

/*
CoefficientMsg::Pointer Prediffer::getCoefficients(const Location &start,
    const Location &end) const
{    
    // Check preconditions.
    DBGASSERT(start.first <= end.first && start.second <= end.second);
    DBGASSERT(start.first >= itsContext.chunkStart.first
        && start.first <= itsContext.chunkEnd.first
        && start.second >= itsContext.chunkStart.second
        && start.second <= itsContext.chunkEnd.second);
    DBGASSERT(end.first >= itsContext.chunkStart.first
        && end.first <= itsContext.chunkEnd.first
        && end.second >= itsContext.chunkStart.second
        && end.second <= itsContext.chunkEnd.second);

    size_t nCells = (end.second - start.second + 1)
        * (end.first - start.first + 1);
        
    CoefficientMsg::Pointer result(new CoefficientMsg(itsId));
    vector<CellCoeff> &cells = result->getContents();

    cells.resize(nCells);
    size_t index = 0;
    for(size_t t = start.second; t <= end.second; ++t)
    {
        for(size_t f = start.first; f <= end.first; ++f)
        {
            const uint32 cellId =
                itsContext.solutionGrid.getCellId(Location(f, t));

            cells[index].id = cellId;

            const Location location(f - itsContext.chunkStart.first,
                t - itsContext.chunkStart.second);

            const uint32 localCellId =
                itsContext.domainGrid.getCellId(location);
        
            for(size_t i = 0; i < itsContext.localParmSelection.size(); ++i)
            {
                const MeqPExpr &parm = itsContext.localParmSelection[i];
                DBGASSERT(localCellId < parm.getFunklets().size());

                const MeqFunklet *funklet = parm.getFunklets()[localCellId];
                const MeqMatrix &localCoeff = funklet->getCoeff();
                const double *localCoeffValues = localCoeff.doubleStorage();
                const vector<bool> &localMask = funklet->getSolvMask();
                
                cells[index].coeff.reserve(funklet->nscid());
                for(size_t j = 0; j < localCoeff.nelements(); ++j)
                {
                    if(localMask[j])
                    {
                        cells[index].coeff.push_back(localCoeffValues[j]);
                    }
                }
            }

            ++index;
        }            
    }
    
    return result;
}


void Prediffer::setCoefficients(CoefficientMsg::Pointer msg)
{
    const vector<CellCoeff> &cells = msg->getContents();

    for(vector<CellCoeff>::const_iterator cellIt = cells.begin(),
        cellEnd = cells.end();
        cellIt != cellEnd;
        ++cellIt)
    {        
        const uint32 cellId = cellIt->id;
        Location location = itsContext.solutionGrid.getCellLocation(cellId);

        DBGASSERT(location.first >= itsContext.chunkStart.first
            && location.second >= itsContext.chunkStart.second);
        location.first -= itsContext.chunkStart.first;
        location.second -= itsContext.chunkStart.second;
        
        const uint32 localCellId = itsContext.domainGrid.getCellId(location);
        DBGASSERT(localCellId < itsContext.domainGrid.getCellCount());

        // TODO: Change this into look-up on local cell id.
        const CellCoefficientIndex &local =
            itsContext.localCoeffIndices[cellId];
            
        for(CellCoefficientIndex::const_iterator it = local.begin(),
            end = local.end();
            it != end;
            ++it)
        {
            DBGASSERT(it->first < itsContext.localParmSelection.size());

            const MeqPExpr &funklet = itsContext.localParmSelection[it->first];
//            funklet.update(localCellId, it->second.start,
//                srcCoeff.itsCoefficients);
        }
    }
}
*/

/*
CoefficientSet::Pointer Prediffer::getCoefficients(const Location &start,
    const Location &end) const
{    
    // Check preconditions.
    DBGASSERT(start.first <= end.first && start.second <= end.second);
    DBGASSERT(start.first >= itsContext.chunkStart.first
        && start.first <= itsContext.chunkEnd.first
        && start.second >= itsContext.chunkStart.second
        && start.second <= itsContext.chunkEnd.second);
    DBGASSERT(end.first >= itsContext.chunkStart.first
        && end.first <= itsContext.chunkEnd.first
        && end.second >= itsContext.chunkStart.second
        && end.second <= itsContext.chunkEnd.second);

    size_t nCells = (end.second - start.second + 1)
        * (end.first - start.first + 1);
        
    CoefficientSet::Pointer result(new CoefficientSet(itsId, nCells));
    CoefficientSet::iterator cellIt = result->begin();

    for(size_t t = start.second; t <= end.second; ++t)
    {
        for(size_t f = start.first; f <= end.first; ++f)
        {
            const uint32 cellId =
                itsContext.solutionGrid.getCellId(Location(f, t));

            cellIt->id = cellId;

            const Location location(f - itsContext.chunkStart.first,
                t - itsContext.chunkStart.second);

            const uint32 localCellId =
                itsContext.domainGrid.getCellId(location);
        
            for(size_t i = 0; i < itsContext.localParmSelection.size(); ++i)
            {
                const MeqPExpr &parm = itsContext.localParmSelection[i];
                DBGASSERT(localCellId < parm.getFunklets().size());

                const MeqFunklet *funklet = parm.getFunklets()[localCellId];
                const MeqMatrix &localCoeff = funklet->getCoeff();
                const double *localCoeffValues = localCoeff.doubleStorage();
                const vector<bool> &localMask = funklet->getSolvMask();
                
                cellIt->coeff.reserve(funklet->nscid());
                for(size_t j = 0; j < localCoeff.nelements(); ++j)
                {
                    if(localMask[j])
                    {
                        cellIt->coeff.push_back(localCoeffValues[j]);
                    }
                }
            }

            ++cellIt;
        }            
    }
    
    return result;
}


void Prediffer::setCoefficients(CoefficientSet::Pointer coeffSet)
{
    for(CoefficientSet::iterator cellIt = coeffSet->begin(),
        cellEnd = coeffSet->end();
        cellIt != cellEnd;
        ++cellIt)
    {        
        const uint32 cellId = cellIt->id;
        Location location = itsContext.solutionGrid.getCellLocation(cellId);

        DBGASSERT(location.first >= itsContext.chunkStart.first
            && location.second >= itsContext.chunkStart.second);
        location.first -= itsContext.chunkStart.first;
        location.second -= itsContext.chunkStart.second;
        
        const uint32 localCellId = itsContext.domainGrid.getCellId(location);
        DBGASSERT(localCellId < itsContext.domainGrid.getCellCount());

        // TODO: Change this into look-up on local cell id.
        const CellCoefficientIndex &local =
            itsContext.localCoeffIndices[cellId];
            
        for(CellCoefficientIndex::const_iterator it = local.begin(),
            end = local.end();
            it != end;
            ++it)
        {
            DBGASSERT(it->first < itsContext.localParmSelection.size());

            const MeqPExpr &funklet = itsContext.localParmSelection[it->first];
//            funklet.update(localCellId, it->second.start,
//                srcCoeff.itsCoefficients);
        }
    }
}
*/

void Prediffer::simulate()
{
//    resetTimers();
//    itsProcessTimer.start();
    
    DBGASSERT(itsOperation == SIMULATE);

//    process(false, true, false, make_pair(0, 0),
//        make_pair(itsChunk->grid.freq.size() - 1,
//            itsChunk->grid.time.size() - 1),
//        &Prediffer::copyBaseline, 0);
    
//    itsProcessTimer.stop();
//    printTimers("Copy");
}


void Prediffer::subtract()
{
//    resetTimers();
//    itsProcessTimer.start();

    DBGASSERT(itsOperation == SUBTRACT);
    
//    process(false, true, false, make_pair(0, 0),
//        make_pair(itsChunk->grid.freq.size() - 1,
//            itsChunk->grid.time.size() - 1),
//        &Prediffer::subtractBaseline, 0);
    
//    itsProcessTimer.stop();
//    printTimers("Subtract");
}


void Prediffer::correct()
{
//    resetTimers();
//    itsProcessTimer.start();
    
    DBGASSERT(itsOperation == CORRECT);
    
//    process(false, true, false, make_pair(0, 0),
//        make_pair(itsChunk->grid.freq.size() - 1,
//            itsChunk->grid.time.size() - 1),
//        &Prediffer::copyBaseline, 0);
    
//    itsProcessTimer.stop();
//    printTimers("Correct");
}

/*
EquationMsg::Pointer Prediffer::construct(const Location &start,
    const Location &end)
{
    DBGASSERT(itsOperation == CONSTRUCT);

    DBGASSERT(start.first <= end.first && start.second <= end.second);
    DBGASSERT(start.first <= itsContext.chunkEnd.first
        && end.first >= itsContext.chunkStart.first);
    DBGASSERT(start.second <= itsContext.chunkEnd.second
        && end.second >= itsContext.chunkStart.second);

//    resetTimers();

    // Clip request against chunk grid.
    Location clipStart, clipEnd;
    clipStart.first = max(start.first, itsContext.chunkStart.first);
    clipStart.second = max(start.second, itsContext.chunkStart.second);
    clipEnd.first = min(end.first, itsContext.chunkEnd.first);
    clipEnd.second = min(end.second, itsContext.chunkEnd.second);

    size_t nCells = (clipEnd.first - clipStart.first + 1)
        * (clipEnd.second - clipStart.second + 1);

    LOG_DEBUG_STR("Constructing equations for " << nCells << " cells.");


    // Compute local cell id of start and end cell.
    Location localStart, localEnd;
    localStart.first = clipStart.first - itsContext.chunkStart.first;
    localStart.second = clipStart.second - itsContext.chunkStart.second; 
    localEnd.first = clipEnd.first - itsContext.chunkStart.first;
    localEnd.second = clipEnd.second - itsContext.chunkStart.second;


    uint32 localStartId = localStart.second
        * (itsContext.chunkEnd.first - itsContext.chunkStart.first + 1)
        + localStart.first;

    uint32 localEndId = localEnd.second
        * (itsContext.chunkEnd.first - itsContext.chunkStart.first + 1)
        + localEnd.first;

    DBGASSERT(localStartId < itsContext.solutionCells.size()
        && localEndId < itsContext.solutionCells.size());



    // TODO: introduce Box class template.
    Location vstart(itsContext.domainGrid[FREQ]->lower(localStart.first),
        itsContext.domainGrid[TIME]->lower(localStart.second));
    Location vend(itsContext.domainGrid[FREQ]->upper(localEnd.first),
        itsContext.domainGrid[TIME]->upper(localEnd.second));

    itsContext.domainGrid[FREQ]->lower(localEnd.first);
    Location startCell = 
    const pair<GridLocation, GridLocation> &startCell = itsContext.solutionCells[localStartId];
    const pair<GridLocation, GridLocation> &endCell = itsContext.solutionCells[localEndId];
    Location vstart(startCell.first.first, startCell.first.second);
    Location vend(endCell.second.first, endCell.second.second);

    LOG_DEBUG_STR("Processing visbility domain: [ (" << vstart.first << ","
        << vstart.second << "), (" << vend.first << "," << vend.second << ") ]"
        << endl);



    // CONVERT TO LOCAL
    GridLocation *startend[2];
    startend[0] = new GridLocation();
    startend[1] = new GridLocation();
    *startend[0] = localStart;
    *startend[1] = localEnd;

    // Allocate contexts for multi-threaded execution.
    for(size_t thread = 0; thread < itsThreadContexts.size(); ++thread)
    {
        ThreadContext &context = itsThreadContexts[thread];

        context.equations.resize(nCells);
        size_t i = 0;
        for(uint32 tcell = clipStart.second; tcell <= clipEnd.second; ++tcell)
        {
            for(uint32 fcell = clipStart.first; fcell <= clipEnd.first; ++fcell)
            {
//                uint32 cellId = itsContext.globalGrid.getCellId(GridLocation(fcell, tcell));
                const uint32 localCellId =
                    (tcell - itsContext.chunkStart.second)
                    * (itsContext.chunkEnd.first - itsContext.chunkStart.first + 1)
                    + fcell - itsContext.chunkStart.first;

                cout << "Init local cell " << localCellId << " " << itsContext.cellCoefficientCount[localCellId] << endl;
                context.equations[i].set(itsContext.cellCoefficientCount[localCellId]);
//            context.equations[i].set(static_cast<int>(itsContext.unknownCount));
                ++i;
            }
        }
        
        context.unknownIndex.resize(itsContext.unknownCount);
        context.perturbedRe.resize(itsContext.unknownCount);
        context.perturbedIm.resize(itsContext.unknownCount);
        context.partialRe.resize(itsContext.unknownCount);
        context.partialIm.resize(itsContext.unknownCount);
    }
    
    // Generate equations.
    itsProcessTimer.start();
    process(true, true, true, vstart, vend, &Prediffer::generateBaseline, &startend);
    itsProcessTimer.stop();

    delete startend[0];
    delete startend[1];
    	
    // Merge thread equations into main equations.
    // TODO: Re-implement the following in O(log(N)) steps.
    ThreadContext &main = itsThreadContexts[0];
    	for(size_t thread = 1; thread < itsThreadContexts.size(); ++thread)
    	{
        ThreadContext &context = itsThreadContexts[thread];
            
        for(size_t i = 0; i < context.equations.size(); ++i)
        {
            main.equations[i].merge(context.equations[i]);
    }

    	context.equations.clear();
    }

    size_t i = 0;
    equations.resize(nCells);
    for(uint32 tcell = clipStart.second; tcell <= clipEnd.second; ++tcell)
    {
        for(uint32 fcell = clipStart.first; fcell <= clipEnd.first; ++fcell)
        {
            equations[i].itsCellId = 
                itsContext.globalGrid.getCellId(GridLocation(fcell, tcell));
            equations[i].itsEquations = main.equations[i];
            ++i;
    	}
    }
    
    printTimers("Generate");

    EquationMsg::Pointer msg(new EquationMsg(itsId));
    return msg;
}

*/

/*
void Prediffer::generate(pair<size_t, size_t> start, pair<size_t, size_t> end,
    vector<casa::LSQFit> &solvers)
{
    resetTimers();

    ASSERT(start.first <= end.first
        && end.first < itsContext.domainCount.first);
    ASSERT(start.second <= end.second
        && end.second < itsContext.domainCount.second);

    size_t nFreqDomains = end.first - start.first + 1;
    size_t nTimeDomains = end.second - start.second + 1;
    size_t nDomains = nFreqDomains * nTimeDomains;
    ASSERT(solvers.size() >= nDomains);

    // Pre-allocate buffers for parallel execution.
    for(size_t thread = 0; thread < itsThreadContexts.size(); ++thread)
    {
        ThreadContext &context = itsThreadContexts[thread];

        context.solvers.resize(nDomains);
        context.unknownIndex.resize(itsContext.unknownCount);
        context.perturbedRe.resize(itsContext.unknownCount);
        context.perturbedIm.resize(itsContext.unknownCount);
        context.partialRe.resize(itsContext.unknownCount);
        context.partialIm.resize(itsContext.unknownCount);
    }
    
    // Initialize thread specific solvers.
    for(size_t domain = 0; domain < nDomains; ++domain)
    {
    	solvers[domain].set(static_cast<int>(itsContext.unknownCount));
    	itsThreadContexts[0].solvers[domain] = &solvers[domain];
    	
    	for(size_t thread = 1; thread < itsThreadContexts.size(); ++thread)
    	{
        	itsThreadContexts[thread].solvers[domain] =
        	    new casa::LSQFit(itsContext.unknownCount);
    	}
    }

    // Convert from solve domain 'coordinates' to sample numbers (i.e. channel
    // number, timeslot number).
    pair<size_t, size_t> vstart(start.first * itsContext.domainSize.first,
        start.second * itsContext.domainSize.second);
    pair<size_t, size_t> vend
        ((end.first + 1) * itsContext.domainSize.first - 1,
        (end.second + 1) * itsContext.domainSize.second - 1);
    ASSERT(vstart.first < itsChunk->grid.freq.size());
    ASSERT(vstart.second < itsChunk->grid.time.size());

    // Clip against data boundary.
    if(vend.first >= itsChunk->grid.freq.size())
        vend.first = itsChunk->grid.freq.size() - 1;
    if(vend.second >= itsChunk->grid.time.size())
        vend.second = itsChunk->grid.time.size() - 1;

//    LOG_DEBUG_STR("Processing visbility domain: " << vstart.first << ", "
//        << vstart.second << " - " << vend.first << ", " << vend.second <<
//endl);
    // Generate equations.
    itsProcessTimer.start();
    process(true, true, true, vstart, vend, &Prediffer::generateBaseline, 0);
    itsProcessTimer.stop();

    // Merge thead specific solvers back into the main ones.
    for(size_t thread = 1; thread < itsThreadContexts.size(); ++thread)
    {
        for(size_t domain = 0; domain < nDomains; ++domain)
        {
    	    solvers[domain].merge(*itsThreadContexts[thread].solvers[domain]);
    	    delete itsThreadContexts[thread].solvers[domain];
    	    itsThreadContexts[thread].solvers[domain] = 0;
    	}
    }
    
    printTimers("Generate");
}


//-----------------------[ Parameter Access ]-----------------------//

void Prediffer::readParms()
{
    vector<string> emptyvec;

    // Clear all parameter values.
    itsParmValues.clear();

    // Get all parameters that intersect the work domain.
    LOFAR::ParmDB::ParmDomain pdomain(itsWorkDomain.startX(),
        itsWorkDomain.endX(), itsWorkDomain.startY(), itsWorkDomain.endY());
    itsInstrumentDb.getValues(itsParmValues, emptyvec, pdomain);
    itsSkyDb.getValues(itsParmValues, emptyvec, pdomain);

    // Remove the funklets from all parms.
    for (MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        it->second.removeFunklets();
    }
}


void Prediffer::clearSolvableParms()
{
    LOG_TRACE_FLOW( "clearSolvableParms" );
    for (MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        it->second.setSolvable(false);
    }
}


//-----------------------[ Computation ]-----------------------//
void Prediffer::process(bool useFlags, bool precalc, bool derivatives,
    pair<size_t, size_t> start, pair<size_t, size_t> end,
    BaselineProcessor processor, void *arguments)
{
    // Determine if perturbed values have to be calculated.
    int nPerturbedValues = derivatives ? itsContext.unknownCount : 0;

    // Get frequency / time grid information.
    int nChannels = end.first - start.first + 1;
    int nTimeslots = end.second - start.second + 1;

    // NOTE: Temporary vector; should be removed after MNS overhaul.
    vector<double> timeAxis(nTimeslots + 1);
    for(size_t i = 0; i < nTimeslots; ++i)
        timeAxis[i] = itsChunk->grid.time.lower(start.second + i);
    timeAxis[nTimeslots] = itsChunk->grid.time.upper(end.second);

    // Initialize the ComplexArr pool with the most frequently used size.
    uint64 defaultPoolSize = itsMeasurement->getChannelCount() * nTimeslots;
    MeqMatrixComplexArr::poolDeactivate();
    MeqMatrixRealArr::poolDeactivate();
    MeqMatrixComplexArr::poolActivate(defaultPoolSize);
    MeqMatrixRealArr::poolActivate(defaultPoolSize);

    // Create a request.
    MeqDomain domain(itsChunk->grid.freq.lower(start.first),
        itsChunk->grid.freq.upper(end.first),
        itsChunk->grid.time.lower(start.second),
        itsChunk->grid.time.upper(end.second));

    MeqRequest request(domain, nChannels, timeAxis, nPerturbedValues);
    request.setOffset(start);

    // Precalculate if required.
    if(precalc)
    {
        itsPrecalcTimer.start();
        itsModel->precalculate(request);
        itsPrecalcTimer.stop();
    }
    
#pragma omp parallel
    {
#pragma omp for schedule(dynamic)
        // Process all selected baselines.
        for(int i = 0; i < itsContext.baselines.size(); ++i)
        {
#if defined _OPENMP
            size_t thread = omp_get_thread_num();
#else
            size_t thread = 0;
#endif
            // Process baseline.
            (this->*processor)
                (thread, arguments, itsChunk, start, request, itsContext.baselines[i], false);
        }
    } // omp parallel
}


void Prediffer::copyBaseline(int threadnr, void*, VisData::Pointer chunk,
    pair<size_t, size_t> offset, const MeqRequest& request, baseline_t baseline,
    bool showd)
{
    // Get thread context.
    ThreadContext &threadContext = itsThreadContexts[threadnr];
    
    // Evaluate the model.
    threadContext.evaluationTimer.start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    threadContext.evaluationTimer.stop();

    // Process the result.
    threadContext.operationTimer.start();

    // Put the results in a single array for easier handling.
    const double *resultRe[4], *resultIm[4];
    jresult.getResult11().getValue().dcomplexStorage(resultRe[0],
        resultIm[0]);
    jresult.getResult12().getValue().dcomplexStorage(resultRe[1],
        resultIm[1]);
    jresult.getResult21().getValue().dcomplexStorage(resultRe[2],
        resultIm[2]);
    jresult.getResult22().getValue().dcomplexStorage(resultRe[3],
        resultIm[3]);

    // Get information about request grid.
    size_t nChannels = request.nx();
    size_t nTimeslots = request.ny();

    // Check preconditions.
    ASSERT(offset.first + nChannels <= chunk->grid.freq.size());
    ASSERT(offset.second + nTimeslots <= chunk->grid.time.size());

    // Get baseline index.
    size_t basel = chunk->getBaselineIndex(baseline);

    // Copy the data to the right location.
    for(size_t tslot = offset.second;
        tslot < offset.second + nTimeslots;
        ++tslot)
    {
        for(set<size_t>::const_iterator it = itsContext.polarizations.begin();
            it != itsContext.polarizations.end();
            ++it)
        {
            // External (Measurement) polarization index.
            size_t ext_pol = *it;
            // Internal (MNS) polarization index.
            int int_pol = itsPolarizationMap[ext_pol];

            const double *re = resultRe[int_pol];
            const double *im = resultIm[int_pol];
            for(size_t chan = 0; chan < nChannels; ++chan)
            {
                chunk->vis_data[basel][tslot][offset.first + chan][ext_pol] =
                    makefcomplex(re[chan], im[chan]);
            }

            resultRe[int_pol] += nChannels;
            resultIm[int_pol] += nChannels;
        }
    }
    threadContext.operationTimer.stop();
}


void Prediffer::subtractBaseline(int threadnr, void*, VisData::Pointer chunk,
    pair<size_t, size_t> offset, const MeqRequest& request, baseline_t baseline,
    bool showd)
{
    // Get thread context.
    ThreadContext &threadContext = itsThreadContexts[threadnr];
    
    // Evaluate the model.
    threadContext.evaluationTimer.start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    threadContext.evaluationTimer.stop();

    // Process the result.
    threadContext.operationTimer.start();

    // Put the results in a single array for easier handling.
    const double *resultRe[4], *resultIm[4];
    jresult.getResult11().getValue().dcomplexStorage(resultRe[0],
        resultIm[0]);
    jresult.getResult12().getValue().dcomplexStorage(resultRe[1],
        resultIm[1]);
    jresult.getResult21().getValue().dcomplexStorage(resultRe[2],
        resultIm[2]);
    jresult.getResult22().getValue().dcomplexStorage(resultRe[3],
        resultIm[3]);

    // Get information about request grid.
    size_t nChannels = request.nx();
    size_t nTimeslots = request.ny();
    
    // Check preconditions.
    ASSERT(offset.first + nChannels <= chunk->grid.freq.size());
    ASSERT(offset.second + nTimeslots <= chunk->grid.time.size());

    // Get baseline index.
    size_t basel = chunk->getBaselineIndex(baseline);
    
    // Copy the data to the right location.
    for(size_t tslot = offset.second;
        tslot < offset.second + nTimeslots;
        ++tslot)
    {
        for(set<size_t>::const_iterator it = itsContext.polarizations.begin();
            it != itsContext.polarizations.end();
            ++it)
        {
            // External (Measurement) polarization index.
            size_t ext_pol = *it;
            // Internal (MNS) polarization index.
            int int_pol = itsPolarizationMap[ext_pol];

            const double *re = resultRe[int_pol];
            const double *im = resultIm[int_pol];
            for(size_t chan = 0; chan < nChannels; ++chan)
            {
                chunk->vis_data[basel][tslot][offset.first + chan][ext_pol] -=
                    makefcomplex(re[chan], im[chan]);
            }

            resultRe[int_pol] += nChannels;
            resultIm[int_pol] += nChannels;
        }
    }
    threadContext.operationTimer.stop();
}


void Prediffer::generateBaseline(int threadnr, void *arguments,
    VisData::Pointer chunk, pair<size_t, size_t> offset,
    const MeqRequest& request, baseline_t baseline, bool showd)
{
    // Check precondition.
    ASSERT(offset.first % itsContext.domainSize.first == 0);
    ASSERT(offset.second % itsContext.domainSize.second == 0);

    // Get thread context.
    ThreadContext &threadContext = itsThreadContexts[threadnr];
    
    // Evaluate the model.
    threadContext.evaluationTimer.start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    threadContext.evaluationTimer.stop();

    // Process the result.
    threadContext.operationTimer.start();
        
    // Put the results in a single array for easier handling.
    const MeqResult *predictResults[4];
    predictResults[0] = &(jresult.getResult11());
    predictResults[1] = &(jresult.getResult12());
    predictResults[2] = &(jresult.getResult21());
    predictResults[3] = &(jresult.getResult22());

    // Get information about request grid.
    size_t nChannels = request.nx();
    size_t nTimeslots = request.ny();

    // Check preconditions.
    ASSERT(offset.first + nChannels <= chunk->grid.freq.size());
    ASSERT(offset.second + nTimeslots <= chunk->grid.time.size());

    pair<size_t, size_t> relDomainOffset = make_pair
        (offset.first / itsContext.domainSize.first,
        offset.second / itsContext.domainSize.second);

    pair<size_t, size_t> relDomainCount = make_pair
        (ceil(nChannels / static_cast<double>(itsContext.domainSize.first)),
        ceil(nTimeslots / static_cast<double>(itsContext.domainSize.second)));

//    LOG_DEBUG_STR("relDomainOffset: " << relDomainOffset.first << ", "
//        << relDomainOffset.second);
//    LOG_DEBUG_STR("relDomainCount: " << relDomainCount.first << ", "
//        << relDomainCount.second);

    // Get baseline index.
    size_t basel = chunk->getBaselineIndex(baseline);

    // To avoid having to use large temporary arrays, step through the
    // data by timeslot and correlation.
    for(set<size_t>::const_iterator it = itsContext.polarizations.begin();
        it != itsContext.polarizations.end();
        ++it)
    {
        // External (Measurement) polarization index.
        size_t ext_pol = *it;
        // Internal (MNS) polarization index.
        int int_pol = itsPolarizationMap[ext_pol];

        // Get the result for this polarization.
        const MeqResult &result = *predictResults[int_pol];

        // Get pointers to the main value.
        const double *predictRe, *predictIm;
        const MeqMatrix &predict = result.getValue();
        predict.dcomplexStorage(predictRe, predictIm);

        // Determine which parameters have derivatives and keep that info.
        // E.g. when solving for station parameters, only a few parameters
        // per baseline have derivatives.
        // Note that this is the same for the entire work domain.
        // Also get pointers to the perturbed values.
        int nUnknownsFound = 0;
        for(size_t idx = 0; idx < itsContext.unknownCount; ++idx)
        {
            if(result.isDefined(idx))
            {
                threadContext.unknownIndex[nUnknownsFound] = idx;
                
                const MeqMatrix &perturbed = result.getPerturbedValue(idx);
                perturbed.dcomplexStorage
                    (threadContext.perturbedRe[nUnknownsFound],
                    threadContext.perturbedIm[nUnknownsFound]);
                    
                ++nUnknownsFound;
            }        	
        }

//        LOG_DEBUG_STR("nUnknownsFound: " << nUnknownsFound);

        if(nUnknownsFound > 0)
        {
            for(size_t tslot = offset.second;
                tslot < offset.second + nTimeslots;
                ++tslot)
            {
                // Skip timeslot if flagged.
                if(chunk->tslot_flag[basel][tslot])
                {
                    // Move pointers to the next timeslot.
                    for(size_t idx = 0; idx < nUnknownsFound; ++idx)
                    {
                        threadContext.perturbedRe[idx] += nChannels;
                        threadContext.perturbedIm[idx] += nChannels;
                    }
                    predictRe += nChannels;
                    predictIm += nChannels;
                    continue;
                }


                size_t domainIdxTime = tslot / itsContext.domainSize.second;
                size_t solverIdxTime = domainIdxTime - relDomainOffset.second;
                domainIdxTime *= itsContext.domainCount.first;
                solverIdxTime *= relDomainCount.first;

                // Construct two equations for each unflagged visibility.
                for(int ch = offset.first;
                    ch < offset.first + nChannels;
                    ++ch)
                {
                    if(!chunk->vis_flag[basel][tslot][ch][ext_pol])
                    {
                        size_t domainIdx = ch / itsContext.domainSize.first;
                        size_t solverIdx = domainIdx - relDomainOffset.first;
                        domainIdx += domainIdxTime;
                        solverIdx += solverIdxTime;
                            
                        // Compute right hand side of the equation pair.
                        sample_t vis =
                            chunk->vis_data[basel][tslot][ch][ext_pol];

                        double diffRe = real(vis) - *predictRe;
                        double diffIm = imag(vis) - *predictIm;

//                        LOG_DEBUG_STR("ch: " << ch);
//                        LOG_DEBUG_STR("predictRe: " << *predictRe << " diffRe: "
//                            << diffRe);
//                        LOG_DEBUG_STR("predictIm: " << *predictIm << " diffIm: "
//                            << diffIm);

                        // Approximate partial derivatives (forward
                        // differences).
                        for(int idx = 0; idx < nUnknownsFound; ++idx)
                        {
                            double invPerturbation = 1.0 /
                                result.getPerturbation
                                    (threadContext.unknownIndex[idx],
                                    domainIdx);
                                
                            threadContext.partialRe[idx] =
                                (*(threadContext.perturbedRe[idx]) - *predictRe)
                                    * invPerturbation;
                            threadContext.partialIm[idx] =
                                (*(threadContext.perturbedIm[idx]) - *predictIm)
                                    * invPerturbation;
                        }

                        // Add condition equations to the solver.
                        if(nUnknownsFound != itsContext.unknownCount)
                        {
//                            LOG_DEBUG_STR("solverIdx: " << solverIdx);
                            threadContext.solvers[solverIdx]->makeNorm
                                (nUnknownsFound,
                                &(threadContext.unknownIndex[0]),
                                &(threadContext.partialRe[0]),
                                1.0,
                                diffRe);
                                
                            threadContext.solvers[solverIdx]->makeNorm
                                (nUnknownsFound,
                                &(threadContext.unknownIndex[0]),
                                &(threadContext.partialIm[0]),
                                1.0,
                                diffIm);
                        }
                        else
                        {
                            threadContext.solvers[solverIdx]->makeNorm
                                (&(threadContext.partialRe[0]),
                                1.0,
                                diffRe);
                                
                            threadContext.solvers[solverIdx]->makeNorm
                                (&(threadContext.partialIm[0]),
                                1.0,
                                diffIm);
                        }
                    } // !chunk->vis_flag[basel][tslot][ch][ext_pol]

                    // Move pointers to the next channel.
                    for(size_t idx = 0; idx < nUnknownsFound; ++idx)
                    {
                        ++threadContext.perturbedRe[idx];
                        ++threadContext.perturbedIm[idx];
                    }
                    ++predictRe;
                    ++predictIm;
                }
            }
        }
    }

    threadContext.operationTimer.stop();
}



void Prediffer::updateUnknowns(size_t domain, const vector<double> &unknowns)
{
    for(MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        if(it->second.isSolvable())
        {
            it->second.update(domain, unknowns);
        }
    }
}


void Prediffer::updateSolvableParms (const ParmDataInfo& parmDataInfo)
{
  const vector<ParmData>& parmData = parmDataInfo.parms();
  // Iterate through all parms.
  for (MeqParmGroup::iterator iter = itsParmGroup.begin();
       iter != itsParmGroup.end();
       ++iter)
  {
    if (iter->second.isSolvable()) {
      const string& pname = iter->second.getName();
      // Update the parameter matching the name.
      for (vector<ParmData>::const_iterator iterpd = parmData.begin();
       iterpd != parmData.end();
       iterpd++) {
    if (iterpd->getName() == pname) {
      iter->second.update (*iterpd);
      break;
    }
    // A non-matching name is ignored.
      }
    }
  }
}

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


void Prediffer::writeParms(size_t domain)
{
    for(MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        if(it->second.isSolvable())
        {
            it->second.save(domain);
        }
    }

}


void Prediffer::writeParms()
{
//  BBSTest::ScopedTimer saveTimer("P:write-parm");
  //  saveTimer.start();
  for (MeqParmGroup::iterator iter = itsParmGroup.begin();
       iter != itsParmGroup.end();
       ++iter)
  {
    if (iter->second.isSolvable()) {
      iter->second.save();
    }
  }
  //  saveTimer.stop();
  //  BBSTest::Logger::log("write-parm", saveTimer);
//  cout << "wrote timeIndex=" << itsTimeIndex
       //<< " nrTimes=" << itsNrTimes << endl;
}

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
        itsThreadContexts[i].evaluationTimer.reset();
        itsThreadContexts[i].operationTimer.reset();
    }
}


void Prediffer::printTimers(const string &operationName)
{
    LOG_DEBUG_STR("Process: " << itsProcessTimer);
    LOG_DEBUG_STR("-Precalculation: " << itsPrecalcTimer);

    // Merge thread local timers.
    unsigned long long count_e = 0, count_o = 0;
    double sum_e = 0.0, sum_o = 0.0, sumsqr_e = 0.0, sumsqr_o = 0.0;
    for(size_t i = 0; i < itsThreadContexts.size(); ++i)
    {
        double time;
        
        time = itsThreadContexts[i].evaluationTimer.getElapsed() * 1000.0;
        sum_e += time;
        sumsqr_e += time * time;
        count_e += itsThreadContexts[i].evaluationTimer.getCount();

        time = itsThreadContexts[i].operationTimer.getElapsed() * 1000.0;
        sum_o += time;
        sumsqr_o += time * time;
        count_o += itsThreadContexts[i].operationTimer.getCount();
    }
    
    // Print merged timings.
    if(count_e == 0)
    {
        LOG_DEBUG("-Model evaluation: timer not used");
    }
    else
    {
        double avg = sum_e / count_e;
        double std = std::sqrt(std::max(sumsqr_e / count_e - avg * avg, 0.0));
        LOG_DEBUG_STR("-Model evaluation: count: " << count_e << ", total: "
            << sum_e << " ms, avg: " << avg << " ms, std: " << std << " ms");
    }

    if(count_o == 0)
    {
        LOG_DEBUG_STR("-" << operationName << ": timer not used");
    }
    else
    {
        double avg = sum_o / count_o;
        double std = std::sqrt(std::max(sumsqr_o / count_o - avg * avg, 0.0));
        LOG_DEBUG_STR("-" << operationName << ": count: " << count_o
            << ", total: " << sum_o << " ms, avg: " << avg << " ms, std: "
            << std << " ms");
    }
}
*/

void Prediffer::initPolarizationMap()
{
    // Create a lookup table that maps external (measurement) polarization
    // indices to internal polarization indices. The internal indices are:
    // 0 - XX
    // 1 - XY
    // 2 - YX
    // 3 - YY
    // No assumptions can be made regarding the external polarization indices,
    // which is why we need this map in the first place.
    
    const vector<string> &polarizations = itsChunk->dims.getPolarizations();
        
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
    vector<string> empty;

    // Clear all cached parameter values.
    itsParameterValues.clear();

    // Get all parameters that intersect the chunk.
    const Grid<double> &sampleGrid = itsChunk->dims.getGrid();
    const Box<double> bbox = sampleGrid.getBoundingBox();
    LOFAR::ParmDB::ParmDomain domain(bbox.start.first, bbox.end.first,
        bbox.start.second, bbox.end.second);

    itsInstrumentDb.getValues(itsParameterValues, empty, domain);
    itsSkyDb.getValues(itsParameterValues, empty, domain);

    // Remove the funklets from all parameters.
    for (MeqParmGroup::iterator it = itsParameters.begin();
        it != itsParameters.end();
        ++it)
    {
        it->second.removeFunklets();
    }
}

} // namespace BBS
} // namespace LOFAR
