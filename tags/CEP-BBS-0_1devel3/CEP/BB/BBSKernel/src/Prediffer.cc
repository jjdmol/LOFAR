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

Prediffer::Prediffer(const string &measurement, size_t subband,
    const string &inputColumn, const string &skyDBase,
    const string &instrumentDBase, const string &historyDBase, bool readUVW)
    :   itsInputColumn          (inputColumn),
        itsReadUVW              (readUVW),
        itsChunkSize            (0),
        itsChunkPosition        (0)
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

    // Open measurement.
    try
    {
        LOG_INFO_STR("Input: " << measurement << "::" << itsInputColumn);
        itsMeasurement.reset(new MeasurementAIPS(measurement, 0, subband, 0));
        
        // Initialize the polarization map.
        initPolarizationMap();
    }
    catch(casa::AipsError &_ex)
    {
        THROW(BBSKernelException, "Failed to open measurement: "
            << measurement << endl << "(AipsError: "
            << _ex.what() << ")");
    }

    // Open sky model parmdb.
    try
    {
        ParmDBMeta skyDBaseMeta("aips", skyDBase);
        LOG_INFO_STR("Sky model database: " << skyDBaseMeta.getTableName());
        itsSkyDBase.reset(new ParmDB(skyDBaseMeta));
    }
    catch(casa::AipsError &_ex)
    {
        THROW(BBSKernelException, "Failed to open sky parameter db: "
            << skyDBase << endl << "(AipsError: " << _ex.what() << ")");
    }

    // Open instrument model parmdb.
    try
    {
        ParmDBMeta instrumentDBaseMeta("aips", instrumentDBase);
        LOG_INFO_STR("Instrument model database: "
            << instrumentDBaseMeta.getTableName());
        itsInstrumentDBase.reset(new ParmDB(instrumentDBaseMeta));
    }
    catch(casa::AipsError &_ex)
    {
        THROW(BBSKernelException, "Failed to open instrument parameter db: "
            << instrumentDBase << endl << "(AipsError: " << _ex.what() << ")");
    }

    // Open history db.
    if(!historyDBase.empty())
    {
        try
        {
            ParmDBMeta historyDBaseMeta("aips", historyDBase);
            LOG_INFO_STR("History DB: " << historyDBaseMeta.getTableName());
            itsHistoryDBase.reset(new ParmDB(historyDBaseMeta));
        }
        catch (casa::AipsError &_ex)
        {
            THROW(BBSKernelException, "Failed to open history db: "
                << historyDBase << endl << "(AipsError: " << _ex.what() << ")");
        }
    }
    else
        LOG_INFO_STR("History DB: -");

    LOG_INFO_STR("UVW source: " << (readUVW ? "read" : "computed"));
    LOG_INFO_STR("UVW convention: " << (readUVW ? "as recorded in the input"
        " measurement" : "ANTENNA2 - ANTENNA1 (AIPS++ convention)"));

    // Initialize model.
    casa::MEpoch startTimeMeas = itsMeasurement->getTimeRange().first;
    casa::Quantum<casa::Double> startTime = startTimeMeas.get("s");
    itsPhaseRef = MeqPhaseRef(itsMeasurement->getPhaseCenter(),
        startTime.getValue("s"));
    itsModel.reset(new Model(itsMeasurement->getInstrument(), itsParmGroup,
        itsSkyDBase.get(), &itsPhaseRef));

    // Allocate thread private buffers.
#if defined _OPENMP
    itsThreadContexts.resize(omp_get_max_threads());
#else
    itsThreadContexts.resize(1);
#endif
}


Prediffer::~Prediffer()
{
    LOG_TRACE_FLOW( "Prediffer destructor" );

    // clear up the matrix pool
    MeqMatrixComplexArr::poolDeactivate();
    MeqMatrixRealArr::poolDeactivate();
}


void Prediffer::initPolarizationMap()
{
    // Create a lookup table that maps external (measurement) polarization
    // indices to internal polarization indices. The internal indices are:
    // 0 - XX or LL
    // 1 - XY or LR
    // 2 - YX or RL
    // 3 - YY or RR
    // No assumptions can be made regarding the external polarization indices,
    // which is why we need this table in the first place.
    
    const vector<string> &polarizations = itsMeasurement->getPolarizations();
        
    itsPolarizationMap.resize(polarizations.size());
    for(size_t i = 0; i < polarizations.size(); ++i)
    {
        if(polarizations[i] == "XX" || polarizations[i] == "LL")
        {
            itsPolarizationMap[i] = 0;
        }
        else if(polarizations[i] == "XY" || polarizations[i] == "LR")
        {
            itsPolarizationMap[i] = 1;
        }
        else if(polarizations[i] == "YX" || polarizations[i] == "RL")
        {
            itsPolarizationMap[i] = 2;
        }
        else if(polarizations[i] == "YY" || polarizations[i] == "RR")
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


void Prediffer::setSelection(VisSelection selection)
{
    itsChunkSelection = selection;
    itsGrid = itsMeasurement->grid(selection);
}


void Prediffer::setChunkSize(size_t size)
{
    itsChunkSize = size;
    itsChunkPosition = 0;
}


bool Prediffer::nextChunk()
{
    // Exit if the MS has no overlap in frequency with the specified work
    // domain.
//    if(startChannel >= itsMeasurement->getChannelCount())
//        return false;
    ASSERT(itsChunkPosition <= itsGrid.time.size());

    if(itsChunkPosition == itsGrid.time.size())
        return false;

    if(itsChunkSize == 0)
    {
        itsChunkPosition = itsGrid.time.size();
    }
    else
    {
        // Clear time selection.
        itsChunkSelection.clear(VisSelection::TIME_START);
        itsChunkSelection.clear(VisSelection::TIME_END);

        double start = itsGrid.time.lower(itsChunkPosition);
        itsChunkPosition += itsChunkSize;
        if(itsChunkPosition > itsGrid.time.size())
        {
            itsChunkPosition = itsGrid.time.size();
        }
        double end = itsGrid.time.upper(itsChunkPosition - 1);
        itsChunkSelection.setTimeRange(start, end);
    }

    // Clear equations.
    itsModel->clearEquations();

    // Deallocate chunk.
    ASSERTSTR(itsChunkData.use_count() == 0 || itsChunkData.use_count() == 1,
        "itsChunkData shoud be unique (or uninitialized) by now.");
    itsChunkData.reset();

    // Read data.
    LOG_DEBUG("Reading chunk...");
    itsChunkData = 
        itsMeasurement->read(itsChunkSelection, itsInputColumn, itsReadUVW);
    if(itsReadUVW)
        itsModel->setStationUVW(itsMeasurement->getInstrument(), itsChunkData);

/*
    LOG_DEBUG_STR("Times:");
    for(size_t i = 0; i < itsChunkData->grid.time.size(); ++i)
        LOG_DEBUG_STR("i: " << i << " " << setprecision(15) <<
        itsChunkData->grid.time(i));
*/

    // Set work domain.
    pair<double, double> freqRange = itsChunkData->grid.freq.range();
    pair<double, double> timeRange = itsChunkData->grid.time.range();
    itsWorkDomain = MeqDomain(freqRange.first, freqRange.second,
        timeRange.first, timeRange.second);

    // Read all parameter values that are part of the work domain.
    readParms();

    // Display chunk meta data.
    LOG_INFO_STR("Chunk: ");
    LOG_INFO_STR("  Frequency: "
        << setprecision(3) << freqRange.first / 1000.0 << " kHz"
        << " - "
        << setprecision(3) << freqRange.second / 1000.0 << " kHz");
    LOG_INFO_STR("  Bandwidth: "
        << setprecision(3) << (freqRange.second - freqRange.first) / 1000.0
        << "kHz (" << itsChunkData->grid.freq.size() << " channels of "
        << setprecision(3)
        << (freqRange.second - freqRange.first) / itsChunkData->grid.freq.size()
        << " Hz)");
    LOG_INFO_STR("  Time:      "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeRange.first, "s"))
        << " - "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeRange.second, "s")));
    LOG_INFO_STR("  Duration:  "
        << setprecision(3) << (timeRange.second - timeRange.first) / 3600.0
        << " hours (" << itsChunkData->grid.time.size() << " samples of "
        << setprecision(3)
        << (timeRange.second - timeRange.first) / itsChunkData->grid.time.size()
        << " s on average)");

    return true;
}


bool Prediffer::setContext(const Context &context)
{
    // Clear context.
    itsContext = ProcessingContext();

    // Select polarizations.
    if(context.correlation.type.empty())
    {
        for(size_t i = 0; i < itsPolarizationMap.size(); ++i)
        {
            if(itsPolarizationMap[i] >= 0)
            {
                itsContext.polarizations.insert(i);
            }
        }
    }
    else
    {
        const vector<string> &requested = context.correlation.type;
        const vector<string> &available = itsMeasurement->getPolarizations();

        for(size_t i = 0; i < available.size(); ++i)
        {
            // Skip all unknown polarizations.
            if(itsPolarizationMap[i] >= 0)
            {
                // Check if this polarization needs to be processed.
                for(size_t j = 0; j < requested.size(); ++j)
                {
                    if(available[i] == requested[j])
                    {
                        itsContext.polarizations.insert(i);
                        break;
                    }
                }
            }
        }
    }

    // Verify that at least one polarization is selected.
    if(itsContext.polarizations.empty())
    {
        LOG_ERROR("At least one polarization should be selected.");
        return false;
    }

    // Select baselines.
    if(context.baselines.station1.empty())
    {
        // If no baselines are speficied, use all baselines selected in the
        // strategy that match the correlation selection of this context.
        for(vector<baseline_t>::const_iterator it =
            itsChunkData->grid.baselines.begin(),
            end = itsChunkData->grid.baselines.end();
            it != end;
            ++it)
        {
            const baseline_t &baseline = *it;

            if(context.correlation.selection == "ALL"
                || (baseline.first == baseline.second
                    && context.correlation.selection == "AUTO")
                || (baseline.first != baseline.second
                    && context.correlation.selection == "CROSS"))
            {
                itsContext.baselines.insert(baseline);
            }
        }
    }
    else
    {
        const Instrument &instrument = itsMeasurement->getInstrument();

        vector<string>::const_iterator baseline_it1 =
            context.baselines.station1.begin();
        vector<string>::const_iterator baseline_it2 =
            context.baselines.station2.begin();

        while(baseline_it1 != context.baselines.station1.end())
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
            // station indices. If a baseline is selected in the current
            // strategy _and_ matches the correlation selection of this context,
            // select it for processing.
            for(set<size_t>::const_iterator it1 = stationGroup1.begin();
                it1 != stationGroup1.end();
                ++it1)
            {
                for(set<size_t>::const_iterator it2 = stationGroup2.begin();
                it2 != stationGroup2.end();
                ++it2)
                {
                    if(context.correlation.selection == "ALL"
                        || ((*it1 == *it2)
                            && (context.correlation.selection == "AUTO"))
                        || ((*it1 != *it2)
                            && context.correlation.selection == "CROSS"))
                    {
                        baseline_t baseline(*it1, *it2);

                        if(itsChunkData->hasBaseline(baseline))
                        {
                            itsContext.baselines.insert(baseline);
                        }
                    }
                }
            }

            ++baseline_it1;
            ++baseline_it2;
        }
    }

    if(itsContext.baselines.empty())
    {
        LOG_ERROR("At least one baseline should be selected.");
        return false;
    }

    return true;
}


bool Prediffer::setContext(const PredictContext &context)
{
    if(!setContext(dynamic_cast<const Context &>(context)))
    {
        return false;
    }

    // Create the measurement equation for each interferometer (baseline).
    itsModel->makeEquations(Model::PREDICT, context.instrumentModel,
        itsContext.baselines, context.sources, itsParmGroup,
        itsInstrumentDBase.get(), &itsPhaseRef, itsChunkData);

    // Put funklets in parms which are not filled yet. (?)
    for(MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        it->second.fillFunklets(itsParmValues, itsWorkDomain);
    }

    itsContext.outputColumn = context.outputColumn;
    return true;
}


bool Prediffer::setContext(const SubtractContext &context)
{
    if(!setContext(dynamic_cast<const Context &>(context)))
    {
        return false;
    }

    // Create the measurement equation for each interferometer (baseline).
    itsModel->makeEquations(Model::PREDICT, context.instrumentModel,
        itsContext.baselines, context.sources, itsParmGroup,
        itsInstrumentDBase.get(), &itsPhaseRef, itsChunkData);

    // Put funklets in parms which are not filled yet. (?)
    for(MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        it->second.fillFunklets(itsParmValues, itsWorkDomain);
    }

    itsContext.outputColumn = context.outputColumn;
    return true;
}


bool Prediffer::setContext(const CorrectContext &context)
{
    if(!setContext(dynamic_cast<const Context &>(context)))
    {
        return false;
    }

    // Create the measurement equation for each interferometer (baseline).
    itsModel->makeEquations(Model::CORRECT, context.instrumentModel,
        itsContext.baselines, context.sources, itsParmGroup,
        itsInstrumentDBase.get(), &itsPhaseRef, itsChunkData);

    // Put funklets in parms which are not filled yet. (?)
    for(MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        it->second.fillFunklets(itsParmValues, itsWorkDomain);
    }

    itsContext.outputColumn = context.outputColumn;
    return true;
}


bool Prediffer::setContext(const GenerateContext &context)
{
    if(!setContext(dynamic_cast<const Context &>(context)))
    {
        return false;
    }

    // Create the measurement equation for each interferometer (baseline).
    itsModel->makeEquations(Model::PREDICT, context.instrumentModel,
        itsContext.baselines, context.sources, itsParmGroup,
        itsInstrumentDBase.get(), &itsPhaseRef, itsChunkData);

    // Put funklets in parms which are not filled yet. (?)
    for(MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        it->second.fillFunklets(itsParmValues, itsWorkDomain);
    }

    // Clear the solvable flag on all parameters (may still be set from the
    // previous step.
    clearSolvableParms();

    // Convert patterns to AIPS++ regular expression objects.
    vector<casa::Regex> included(context.unknowns.size());
    vector<casa::Regex> excluded(context.excludedUnknowns.size());

    try
    {
        transform(context.unknowns.begin(), context.unknowns.end(),
            included.begin(), ptr_fun(casa::Regex::fromPattern));

        transform(context.excludedUnknowns.begin(),
            context.excludedUnknowns.end(), excluded.begin(),
            ptr_fun(casa::Regex::fromPattern));
    }
    catch(std::exception &_ex)
    {
        LOG_ERROR_STR("Error parsing Parms/ExclParms pattern (exception: "
            << _ex.what() << ")");
        return false;
    }

    // Find all parameters matching context.unknowns, exclude those that
    // match context.excludedUnknowns.
    for(MeqParmGroup::iterator parameter_it = itsParmGroup.begin();
        parameter_it != itsParmGroup.end();
        ++parameter_it)
    {
        casa::String name(parameter_it->second.getName());

        // Loop through all regex-es until a match is found.
        for(vector<casa::Regex>::iterator included_it = included.begin();
            included_it != included.end();
            ++included_it)
        {
            if(name.matches(*included_it))
            {
                bool include = true;

                // Test if excluded.
                for(vector<casa::Regex>::const_iterator excluded_it =
                    excluded.begin();
                    excluded_it != excluded.end();
                    ++excluded_it)
                {
                    if(name.matches(*excluded_it))
                    {
                        include = false;
                        break;
                    }
                }

                if(include)
                {
                    parameter_it->second.setSolvable(true);
                }
                
                break;
            }
        }
    }

    // Initialize solve domain grid.
    pair<size_t, size_t> domainSize(context.domainSize);
    if(domainSize.first == 0
        || domainSize.first > itsChunkData->grid.freq.size())
    {
        domainSize.first = itsChunkData->grid.freq.size();
    }
    
    if(domainSize.second == 0
        || domainSize.second > itsChunkData->grid.time.size())
    {
        domainSize.second = itsChunkData->grid.time.size();
    }

    LOG_DEBUG_STR("Nominal solve domain size: " << domainSize.first
        << " channels by " << domainSize.second << " timeslots.");

    initSolveDomains(domainSize);

    return (itsContext.unknownCount > 0);
}


void Prediffer::initSolveDomains(pair<size_t, size_t> size)
{
    size_t nFreqDomains =
        ceil(itsChunkData->grid.freq.size() / static_cast<double>(size.first));
    size_t nTimeDomains =
        ceil(itsChunkData->grid.time.size() / static_cast<double>(size.second));
    size_t nSolveDomains = nFreqDomains * nTimeDomains;

//    cout << "freq.size(): " << itsChunkData->grid.freq.size() << " #domains: "
//        << nFreqDomains << endl;
//    cout << "time.size(): " << itsChunkData->grid.time.size() << " #domains: "
//        << nTimeDomains << endl;

    itsContext.domainSize = size;
    itsContext.domainCount = make_pair(nFreqDomains, nTimeDomains);
    itsContext.domains.resize(nSolveDomains);

    // Determine solve domains.
    size_t idx = 0, tstart = 0;
    for(size_t tdomain = 0; tdomain < nTimeDomains; ++tdomain)
    {
        size_t tend = tstart + size.second - 1;
        if(tend >= itsChunkData->grid.time.size())
            tend = itsChunkData->grid.time.size() - 1;

        size_t fstart = 0;
        for(size_t fdomain = 0; fdomain < nFreqDomains; ++fdomain)
        {
            size_t fend = fstart + size.first - 1;
            if(fend >= itsChunkData->grid.freq.size())
                fend = itsChunkData->grid.freq.size() - 1;

            itsContext.domains[idx] =
                MeqDomain(itsChunkData->grid.freq.lower(fstart),
                    itsChunkData->grid.freq.upper(fend),
                    itsChunkData->grid.time.lower(tstart),
                    itsChunkData->grid.time.upper(tend));

            fstart += size.first;
            ++idx;
        }
        tstart += size.second;
    }

    // Initialize meta data needed for processing.
    int nUnknowns = 0;
    // Temporary vector (required by MeqParmFunklet::initDomain() but not
    // needed here).
    vector<int> tmp(nSolveDomains, 0);
    
    for(MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        // Determine maximal number of unknowns over all solve domains for this
        // parameter.
        it->second.initDomain(itsContext.domains, nUnknowns, tmp);
    }
    itsContext.unknownCount = nUnknowns;

    // Initialize unknowns.
    itsContext.unknowns.resize(nSolveDomains);
    for(size_t i = 0; i < nSolveDomains; ++i)
    {
    	itsContext.unknowns[i].resize(nUnknowns);
    	fill(itsContext.unknowns[i].begin(), itsContext.unknowns[i].end(), 0.0);
    }
    
    // Copy unknowns (we need the unknowns of all parameters for each domain,
    // instead of the unknowns of all domains for each parameter).
    for(MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        if(it->second.isSolvable())
        {
            // TODO: Create method on MeqParmFunklet to do what follows.
            const vector<MeqFunklet*> &funklets = it->second.getFunklets();
            ASSERT(funklets.size() == nSolveDomains);

            for(size_t domain = 0; domain < nSolveDomains; ++domain)
            {
                size_t offset = funklets[domain]->scidInx();            
                const MeqMatrix &coeff = funklets[domain]->getCoeff();
                const vector<bool> &mask = funklets[domain]->getSolvMask();

                for(size_t i = 0; i < coeff.nelements(); ++i)
                {
                    if(mask[i])
                    {
                        itsContext.unknowns[domain][i + offset] =
                            coeff.doubleStorage()[i];
                        ++offset;
                    }
                }
            }
        }
    }

    LOG_DEBUG_STR("nDerivatives: " << nUnknowns);
}


void Prediffer::predict()
{
    process(false, true, false, make_pair(0, 0),
        make_pair(itsChunkData->grid.freq.size() - 1,
            itsChunkData->grid.time.size() - 1),
        &Prediffer::copyBaseline, 0);

    if(!itsContext.outputColumn.empty())
    {
        itsMeasurement->write(itsChunkSelection, itsChunkData,
            itsContext.outputColumn, false);
    }
    
    LOG_DEBUG_STR("Predict: " << itsPredTimer);
    LOG_DEBUG_STR("Copy: " << itsEqTimer);
    itsPredTimer.reset();
    itsEqTimer.reset();
}


void Prediffer::subtract()
{
    process(false, true, false, make_pair(0, 0),
        make_pair(itsChunkData->grid.freq.size() - 1,
            itsChunkData->grid.time.size() - 1),
        &Prediffer::subtractBaseline, 0);

    if(!itsContext.outputColumn.empty())
    {
        itsMeasurement->write(itsChunkSelection, itsChunkData,
            itsContext.outputColumn, false);
    }

    LOG_DEBUG_STR("Predict: " << itsPredTimer);
    LOG_DEBUG_STR("Subtract: " << itsEqTimer);
    itsPredTimer.reset();
    itsEqTimer.reset();
}


void Prediffer::correct()
{
    process(false, true, false, make_pair(0, 0),
        make_pair(itsChunkData->grid.freq.size() - 1,
            itsChunkData->grid.time.size() - 1),
        &Prediffer::copyBaseline, 0);

    if(!itsContext.outputColumn.empty())
    {
        itsMeasurement->write(itsChunkSelection, itsChunkData,
            itsContext.outputColumn, false);
    }

    LOG_DEBUG_STR("Correct: " << itsEqTimer);
    LOG_DEBUG_STR("Copy: " << itsPredTimer);
    itsPredTimer.reset();
    itsEqTimer.reset();
}


void Prediffer::generate(pair<size_t, size_t> start, pair<size_t, size_t> end,
    vector<casa::LSQFit> &solvers)
{
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
    ASSERT(vstart.first < itsChunkData->grid.freq.size());
    ASSERT(vstart.second < itsChunkData->grid.time.size());

    // Clip against data boundary.
    if(vend.first >= itsChunkData->grid.freq.size())
        vend.first = itsChunkData->grid.freq.size() - 1;
    if(vend.second >= itsChunkData->grid.time.size())
        vend.second = itsChunkData->grid.time.size() - 1;

//    LOG_DEBUG_STR("Processing visbility domain: " << vstart.first << ", "
//        << vstart.second << " - " << vend.first << ", " << vend.second <<
//endl);
    // Generate equations.
    process(true, true, true, vstart, vend, &Prediffer::generateBaseline, 0);

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
    
    LOG_DEBUG_STR("Predict: " << itsPredTimer);
    LOG_DEBUG_STR("Construct equations: " << itsEqTimer);
    itsPredTimer.reset();
    itsEqTimer.reset();
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
    itsInstrumentDBase->getValues(itsParmValues, emptyvec, pdomain);
    itsSkyDBase->getValues(itsParmValues, emptyvec, pdomain);

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
        timeAxis[i] = itsChunkData->grid.time.lower(start.second + i);
    timeAxis[nTimeslots] = itsChunkData->grid.time.upper(end.second);

    // Initialize the ComplexArr pool with the most frequently used size.
    uint64 defaultPoolSize = itsMeasurement->getChannelCount() * nTimeslots;
    MeqMatrixComplexArr::poolDeactivate();
    MeqMatrixRealArr::poolDeactivate();
    MeqMatrixComplexArr::poolActivate(defaultPoolSize);
    MeqMatrixRealArr::poolActivate(defaultPoolSize);

    // Create a request.
    MeqDomain domain(itsChunkData->grid.freq.lower(start.first),
        itsChunkData->grid.freq.upper(end.first),
        itsChunkData->grid.time.lower(start.second),
        itsChunkData->grid.time.upper(end.second));

    MeqRequest request(domain, nChannels, timeAxis, nPerturbedValues);
    request.setOffset(start);

    // Precalculate if required.
    if(precalc)
    {
        itsModel->precalculate(request);
    }
    
/************** DEBUG DEBUG DEBUG **************/
/*
    cout << "Processing correlations: ";
    copy(itsContext.correlations.begin(), itsContext.correlations.end(),
        ostream_iterator<size_t>(cout, " "));
    cout << endl;

    cout << "Processing baselines:" << endl;
    for(set<baseline_t>::const_iterator it = itsContext.baselines.begin();
        it != itsContext.baselines.end();
        ++it)
    {
        cout << it->first << " - " << it->second << endl;
    }
*/
/************** DEBUG DEBUG DEBUG **************/

#pragma omp parallel
    {
#pragma omp for schedule(dynamic)
        // Process all selected baselines.
        for(set<baseline_t>::const_iterator it = itsContext.baselines.begin();
            it != itsContext.baselines.end();
            ++it)
        {
#if defined _OPENMP
            size_t thread = omp_get_thread_num();
#else
            size_t thread = 0;
#endif
            // Process baseline.
            (this->*processor)
                (thread, arguments, itsChunkData, start, request, *it, false);
        }
    } // omp parallel
}


void Prediffer::copyBaseline(int, void*, VisData::Pointer chunk,
    pair<size_t, size_t> offset, const MeqRequest& request, baseline_t baseline,
    bool showd)
{
    // Do the actual correct.
    itsPredTimer.start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    itsPredTimer.stop();

    itsEqTimer.start();
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
    itsEqTimer.stop();
}


void Prediffer::subtractBaseline(int, void*, VisData::Pointer chunk,
    pair<size_t, size_t> offset, const MeqRequest& request, baseline_t baseline,
    bool showd)
{
    // Do the actual correct.
    itsPredTimer.start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    itsPredTimer.stop();

    itsEqTimer.start();
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
    itsEqTimer.stop();
}


void Prediffer::generateBaseline(int threadnr, void *arguments,
    VisData::Pointer chunk, pair<size_t, size_t> offset,
    const MeqRequest& request, baseline_t baseline, bool showd)
{
    // Check precondition.
    ASSERT(offset.first % itsContext.domainSize.first == 0);
    ASSERT(offset.second % itsContext.domainSize.second == 0);

    // Do the actual predict.
    itsPredTimer.start();
    MeqJonesResult jresult = itsModel->evaluate(baseline, request);
    itsPredTimer.stop();

    itsEqTimer.start();
    
    // Get thread context.
    ThreadContext &threadContext = itsThreadContexts[threadnr];
    
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

/************** DEBUG DEBUG DEBUG **************/
//                if (showd)
//                {
//                    showData(corr, sdch, inc, nrchan, cflags, cdata,
//                        realVals, imagVals);
//                }
/************** DEBUG DEBUG DEBUG **************/

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

    itsEqTimer.stop();
}


/*
void Prediffer::getData (bool useTree,
             Array<Complex>& dataArr, Array<Bool>& flagArr)
{
  int nrusedbl = itsBaselineInx.size();
  int nrchan   = itsLastChan-itsFirstChan+1;
  dataArr.resize (IPosition(4, itsNCorr, nrchan, nrusedbl, itsNrTimes));
  flagArr.resize (IPosition(4, itsNCorr, nrchan, nrusedbl, itsNrTimes));
  pair<Complex*,bool*> p;
  p.first = dataArr.data();
  p.second = flagArr.data();
  if (!useTree) {
    process (true, false, false, &Prediffer::getBaseline, &p);
    return;
  }
  vector<MeqJonesExpr> expr(nrusedbl);
  for (int i=0; i<nrusedbl; ++i) {
    expr[i] = new MeqJonesMMap (itsMSMapInfo, itsBaselinenInx[i]);
  }
  pair<pair<Complex*,bool*>*, vector<MeqJonesExpr>*> p1;
  p1.first = &p;
  p1.second = &expr;
  process (true, false, false, &Prediffer::getMapBaseline, &p1);
}
*/


/*
vector<MeqResult> Prediffer::getResults(bool calcDeriv)
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsExpr);
  // Allocate result vector.
  vector<MeqResult> results;
  int nrchan = itsLastChan-itsFirstChan+1;
  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  for (unsigned int tStep=0; tStep<itsNrTimes; tStep++)
  {
    double time = itsMSDesc.times[itsTimeIndex+tStep];
    double interv = itsMSDesc.exposures[itsTimeIndex+tStep];
    MeqDomain domain(startFreq, endFreq, time-interv/2, time+interv/2);
    MeqRequest request(domain, nrchan, 1, 0);
    if (calcDeriv) {
      request = MeqRequest(domain, nrchan, 1, itsContext.unknownCount);
    }
    // Loop through expressions to be precalculated.
    // We can parallellize them at each level.
    // Level 0 is formed by itsExpr which are not calculated here.
    for (int level = itsPrecalcNodes.size(); -- level > 0;) {
      vector<MeqExprRep*> exprs = itsPrecalcNodes[level];
      int nrExprs = exprs.size();
      if (nrExprs > 0) {
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < nrExprs; i ++) {
      exprs[i]->precalculate (request);
    }
      }
    }
    // Evaluate for all selected baselines.
    for (uint blindex=0; blindex<itsBaselineInx.size(); ++blindex) {
      // Get the result for this baseline.
      MeqJonesExpr& expr = itsExpr[blindex];
      // This is the actual predict.
      MeqJonesResult result = expr.getResult (request);
      results.push_back (result.getResult11());
      results.push_back (result.getResult12());
      results.push_back (result.getResult21());
      results.push_back (result.getResult22());
    }
  }
  return results;
}
*/


/*
void Prediffer::getBaseline(int, void* arguments, const fcomplex* data,
    fcomplex*, const bool* flags, const MeqRequest&, int blindex, bool)
{
  Complex* datap = static_cast<pair<Complex*,bool*>*>(arg)->first;
  bool*    flagp = static_cast<pair<Complex*,bool*>*>(arg)->second;
  int nrchan = itsLastChan-itsFirstChan+1;
  memcpy (datap+blindex*nrchan*itsNCorr, data,
      nrchan*itsNCorr*sizeof(Complex));
  memcpy (flagp+blindex*nrchan*itsNCorr, flags,
      nrchan*itsNCorr*sizeof(bool));
}
*/

/*
void Prediffer::getMapBaseline (int, void* arguments, const fcomplex*,
    fcomplex*, const bool* flags, const MeqRequest& request, int blindex, bool)
{
  pair<pair<Complex*,bool*>*, vector<MeqJonesExpr>*>* p1 =
    static_cast<pair<pair<Complex*,bool*>*, vector<MeqJonesExpr>*>*>(arg);
  Complex* datap = p1->first->first;
  bool*    flagp = p1->first->second;
  vector<MeqJonesExpr>& expr = *p1->second;
  int nrchan = itsLastChan-itsFirstChan+1;
  datap += blindex*nrchan*itsNCorr;
  memcpy (flagp+blindex*nrchan*itsNCorr, flags,
      nrchan*itsNCorr*sizeof(bool));
  MeqJonesResult jresult = expr[blindex].getResult (request);
  const double *r1, *i1, *r2, *i2, *r3, *i3, *r4, *i4;
  jresult.result11().getValue().dcomplexStorage (r1, i1);
  jresult.result12().getValue().dcomplexStorage (r2, i2);
  jresult.result21().getValue().dcomplexStorage (r3, i3);
  jresult.result22().getValue().dcomplexStorage (r4, i4);
  int nx = jresult.result11().getValue().nx();
  for (int iy=0; iy<jresult.result11().getValue().ny(); ++iy) {
    if (itsReverseChan) {
      for (int ix=nx; ix>0;) {
    ix--;
    int i = iy*nx + ix;
    *datap++ = Complex(r1[i], i1[i]);
    if (itsNCorr > 2) {
      *datap++ = Complex(r2[i], i2[i]);
      *datap++ = Complex(r3[i], i3[i]);
    }
    if (itsNCorr > 1) {
      *datap++ = Complex(r4[i], i4[i]);
    }
      }
    } else {
      for (int ix=0; ix<nx; ++ix) {
    int i = iy*nx + ix;
    *datap++ = Complex(r1[i], i1[i]);
    if (itsNCorr > 2) {
      *datap++ = Complex(r2[i], i2[i]);
      *datap++ = Complex(r3[i], i3[i]);
    }
    if (itsNCorr > 1) {
      *datap++ = Complex(r4[i], i4[i]);
    }
      }
    }
  }
}
*/



/*
void Prediffer::testFlagsBaseline(int, void*, VisData::Pointer chunk,
    const MeqRequest& request, baseline_t baseline, bool showd)
{
    itsEqTimer.start();
    int nrchan = request.nx();
    int nrtime = request.ny();

    map<baseline_t, size_t>::iterator it = chunk->baselines.find(baseline);
    ASSERT(it != chunk->baselines.end());
    size_t basel = it->second;

    // Loop through the times and correlations.
    for(size_t tslot = 0; tslot < nrtime; ++tslot)
    {
        for(set<size_t>::const_iterator it = itsContext.correlations.begin();
            it != itsContext.correlations.end();
            ++it)
        {
            size_t correlation = *it;

            for(size_t ch=0; ch < nrchan; ++ch)
            {
                chunk->vis_data[basel][tslot][ch][correlation] =
                    (chunk->vis_flag[basel][tslot][ch][correlation] ?
                        makefcomplex(0.0, 0.0) : makefcomplex(1.0, 1.0));
            }
       }
    }
    itsEqTimer.stop();
}
*/


/*
void Prediffer::updateSolvableParms(const vector<double>& values)
{
  // Iterate through all parms.
  for (MeqParmGroup::iterator iter = itsParmGroup.begin();
       iter != itsParmGroup.end();
       ++iter)
  {
    if (iter->second.isSolvable()) {
      iter->second.update (values);
    }
  }
}
*/

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


/*
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
*/

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

// Log the values of the solvable parameters and the quality indicators of the
// last iteration to a ParmDB. The type of the parameters is set to 'history'
// to ensure that they cannot accidentily be used as input for BBSkernel. Also,
// ParmFacade::getHistory() checks on this type.
/*
void Prediffer::logIteration(const string &stepName, size_t solveDomainIndex,
    double rank, double chiSquared, double LMFactor)
{
    if(!itsHistoryDBase)
        return;

    ostringstream os;
    os << setfill('0');

    // Default values for the coefficients (every parameters of the history
    // type has exactly one coefficient, and only solvable parameters are
    // logged).
    bool solvable = false;
    vector<int> shape(2, 1);

    const SolveDomainDescriptor &descriptor =
        itsContext.domains[solveDomainIndex];

    // For each solve domain, log the solver quality indicators and the values of the
    // coefficients of each solvable parameter.
    LOFAR::ParmDB::ParmValue value;
    LOFAR::ParmDB::ParmValueRep &valueRep = value.rep();

    valueRep.setType("history");
    valueRep.setDomain(LOFAR::ParmDB::ParmDomain(
        descriptor.domain.startX(),
        descriptor.domain.endX(),
        descriptor.domain.startY(),
        descriptor.domain.endY()));

    string stepPrefix = stepName + ":";
    string domainSuffix;
    if(itsContext.domains.size() > 1)
    {
        os.str("");
        os << ":domain"
            << setw(((int) log10(double(itsContext.domains.size())) + 1))
            << solveDomainIndex;
        domainSuffix = os.str();
    }

    value.setNewParm();
    valueRep.setCoeff(&rank, &solvable, shape);
    itsHistoryDBase->putValue(stepPrefix + "solver:rank" + domainSuffix, value);

    value.setNewParm();
    valueRep.setCoeff(&chiSquared, &solvable, shape);
    itsHistoryDBase->putValue(stepPrefix + "solver:chi_squared" + domainSuffix,
value);

    value.setNewParm();
    valueRep.setCoeff(&LMFactor, &solvable, shape);
    itsHistoryDBase->putValue(stepPrefix + "solver:lm_factor" + domainSuffix,
value);

    // Log each solvable parameter. Each combinaton of coefficient
    // and solve domain is logged separately. Otherwise, one would
    // currently not be able to view coefficients separately in the
    // plotter.
    for(size_t i = 0; i < descriptor.parameters.size(); ++i)
    {
        const MeqFunklet *funklet = descriptor.parameters[i].getFunklets()[solveDomainIndex];
        const vector<bool> &mask = funklet->getSolvMask();
        const MeqMatrix &coeff = funklet->getCoeff();
        const double *data = coeff.doubleStorage();

        int indexWidth = ((int) log10(double(mask.size()))) + 1;

        // Log each coefficient of the current parameter.
        for(size_t j = 0; j < mask.size(); ++j)
        {
            // Only log solvable coefficients.
            if(mask[j])
            {
                value.setNewParm();
                valueRep.setCoeff(&data[j], &solvable, shape);
                os.str("");
                os << stepPrefix << descriptor.parameters[i].getName() << ":c" << setw(indexWidth) << j << domainSuffix;
                itsHistoryDBase->putValue(os.str(), value);
            }
        }
    }
}
*/

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

/*
    SolveDomainDescriptor &descriptor = itsContext.domains[solveDomainIndex];
    for(size_t i = 0; i < descriptor.parameters.size(); ++i)
    {
        descriptor.parameters[i].save(solveDomainIndex);
    }
*/
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

} // namespace BBS
} // namespace LOFAR
