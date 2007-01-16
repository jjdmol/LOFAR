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
#include <BBSKernel/MMap.h>
#include <BBSKernel/FlagsMap.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/MNS/MeqLMN.h>
#include <BBSKernel/MNS/MeqDFTPS.h>
#include <BBSKernel/MNS/MeqDiag.h>
#include <BBSKernel/MNS/MeqBaseDFTPS.h>
#include <BBSKernel/MNS/MeqBaseLinPS.h>
#include <BBSKernel/MNS/MeqStatExpr.h>
#include <BBSKernel/MNS/MeqJonesSum.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <BBSKernel/MNS/MeqMatrixComplexArr.h>
#include <BBSKernel/MNS/MeqMatrixRealArr.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <BBSKernel/MNS/MeqParmSingle.h>
#include <BBSKernel/MNS/MeqPointSource.h>
#include <BBSKernel/MNS/MeqJonesCMul3.h>
#include <BBSKernel/MNS/MeqJonesInvert.h>
#include <BBSKernel/MNS/MeqJonesMMap.h>

#include <Common/Timer.h>
#include <Common/LofarLogger.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>
#include <Blob/BlobArray.h>
#include <Common/StreamUtil.h>
#include <Common/DataConvert.h>
#include <Common/LofarLogger.h>

#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
// Vector2.cc: necessary to instantiate .tovector()
#include <casa/Arrays/Vector2.cc>
#include <casa/Quanta/MVBaseline.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/OS/Timer.h>
#include <casa/OS/RegularFile.h>
//#include <casa/OS/SymLink.h>
#include <casa/Containers/Record.h>
#include <casa/IO/AipsIO.h>
#include <casa/IO/MemoryIO.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Regex.h>
#include <casa/Utilities/GenSort.h>

#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/Stokes.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TiledColumnStMan.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>

#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <malloc.h>
#include <unistd.h>
#include <algorithm>

#include <BBSKernel/BBSTestLogger.h>

#if defined _OPENMP
#include <omp.h>
#endif

using namespace casa;
using namespace std;

namespace LOFAR
{
namespace BBS 
{
using LOFAR::operator<<;
using std::max;

Prediffer::Prediffer(   const string& measurementSet,
                        const string& inputColumn,
                        const string& skyParameterDB,
                        const string& instrumentParameterDB,
                        uint subbandID,
                        bool calcUVW)
    :   itsSubbandID        (subbandID),
        itsCalcUVW          (calcUVW),
        itsMEP              (0),
        itsGSMMEP           (0),
        itsSources          (0),
        itsInDataColumn     (inputColumn),
        itsNrPert           (0),
        itsNCorr            (0),
        itsNrBl             (0),
        itsTimeIndex        (0),
        itsNrTimes          (0),
        itsNrTimesDone      (0),
        itsInDataMap        (0),
        itsOutDataMap       (0),
        itsFlagsMap         (0),
        itsWeightMap        (0),
        itsIsWeightSpec     (false),
        itsPredTimer        ("P:predict", false),
        itsEqTimer          ("P:eq|save", false)
{
    // Get path to measurement set.
    itsMSName = Path(measurementSet).absoluteName();
    LOG_INFO_STR("Input: " << itsMSName << "::" << itsInDataColumn);
    
    // Read meta data (needed to know for instance the number of available
    // correlations).
    try
    {
        readMeasurementSetMetaData(itsMSName);
    }
    catch (AipsError &_ex)
    {
        THROW(BBSKernelException, "Failed to read input meta data (AipsError: " << _ex.what() << ")");
    }
    
    // Amongst other things, processMSDesc() calls fillStations(), which
    // sets itsStations. Eventually it may make sense to remove MSDesc from
    // the Prediffer.
    processMSDesc(itsSubbandID);
    
    // Open sky parmdb.
    try
    {
        LOFAR::ParmDB::ParmDBMeta skyParameterDBMeta("aips", skyParameterDB);
        itsGSMMEP = new LOFAR::ParmDB::ParmDB(skyParameterDBMeta);
        itsGSMMEPName = skyParameterDBMeta.getTableName();
        LOG_INFO_STR("Sky model ParmDB: " << itsGSMMEPName);
    }
    catch (AipsError &_ex)
    {
        THROW(BBSKernelException, "Failed to open sky parameter db: " << skyParameterDB << endl << "(AipsError: " << _ex.what() << ")");
    }

    // Open instrument parmdb.
    try
    {
        LOFAR::ParmDB::ParmDBMeta instrumentParameterDBMeta("aips", instrumentParameterDB);
        itsMEP = new LOFAR::ParmDB::ParmDB(instrumentParameterDBMeta);
        itsMEPName = instrumentParameterDBMeta.getTableName();
        LOG_INFO_STR("Instrument model ParmDB: " << itsMEPName);
    }
    catch (AipsError &_ex)
    {
        THROW(BBSKernelException, "Failed to open instrument parameter db: " << instrumentParameterDB << endl << "(AipsError: " << _ex.what() << ")");
    }
    
          
    // Allocate thread private buffers.
#if defined _OPENMP
    itsNthread = omp_get_max_threads();
#else
    itsNthread = 1;
#endif
    itsFlagVecs.resize(itsNthread);
    itsResultVecs.resize(itsNthread);
    itsDiffVecs.resize(itsNthread);
    itsIndexVecs.resize(itsNthread);
}


Prediffer::~Prediffer()
{
  LOG_TRACE_FLOW( "Prediffer destructor" );

  delete itsSources;
  for (vector<MeqStatUVW*>::iterator iter = itsStatUVW.begin();
       iter != itsStatUVW.end();
       iter++) {
    delete *iter;
  }
  for (vector<MeqStation*>::iterator iter = itsStations.begin();
       iter != itsStations.end();
       iter++) {
    delete *iter;
  }

  delete itsMEP;
  delete itsGSMMEP;
  
  delete itsInDataMap;
  delete itsOutDataMap;
  delete itsFlagsMap;
  delete itsWeightMap;

  // clear up the matrix pool
  MeqMatrixComplexArr::poolDeactivate();
  MeqMatrixRealArr::poolDeactivate();
}


void Prediffer::updateCorrelationMask(vector<bool> &mask, const vector<string> &requestedCorrelations)
{
    if(requestedCorrelations.empty()) return;
    
    const vector<string> &availableCorrelations = itsMSDesc.corrTypes;
    for(int i = 0; i < availableCorrelations.size(); ++i)
    {
        if(mask[i])
        {
            mask[i] = false;
            for(int j = 0; j < requestedCorrelations.size(); ++j)
            {
                if(requestedCorrelations[j] == availableCorrelations[i])
                {
                    mask[i] = true;
                    break;
                }
            }
        }
    }
}


bool Prediffer::setSelection(const vector<string> &stations, const Correlation &correlation)
{    
    // Select correlations.
    const vector<string> &availableCorrelations = itsMSDesc.corrTypes;
    const vector<string> &requestedCorrelations = correlation.type;
    LOG_INFO_STR("Available correlation(s): " << availableCorrelations);
    LOG_INFO_STR("Requested correlation(s): " << requestedCorrelations);
    
    // Initially select all correlations.
    fill(itsSelectedCorr.begin(), itsSelectedCorr.end(), true);
    
    // Deselect all correlations _not_ present in requestedCorrelations.
    updateCorrelationMask(itsSelectedCorr, requestedCorrelations);
    
    // Verify that at least one correlations is still selected.
    if(count(itsSelectedCorr.begin(), itsSelectedCorr.end(), true) == 0)
    {
        LOG_ERROR("At least one correlation should be selected.");
        return false;
    }
    
    // Output selected correlations (inform the user).
    ostringstream os;
    os << "Selected correlation(s):";
    for(unsigned int i = 0; i < availableCorrelations.size(); ++i)
    {
        if(itsSelectedCorr[i])
        {
            os << " " << availableCorrelations[i];
        }
    }
    LOG_INFO_STR(os.str());
        
    // Select stations.
    const vector<string> &availableStations = itsMSDesc.antNames;
    const vector<string> &requestedStations = stations;
    LOG_INFO_STR("Available station(s): " << availableStations);
    LOG_INFO_STR("Requested station(s): " << requestedStations);
    
    vector<string> debugSelectedStations;
    if(requestedStations.empty())
    {
        // Select all available stations if none are requested explicitly.
        itsSelStations = itsStations;
        debugSelectedStations = availableStations;
    }
    else
    {
        itsSelStations = vector<MeqStation*>(availableStations.size(), (MeqStation*) 0);
        
        for(vector<string>::const_iterator it = requestedStations.begin();
            it != requestedStations.end();
            ++it)
        {
            casa::Regex regex = casa::Regex::fromPattern(*it);
            
            // If a names matches, add its antennanr to the vector.
            for(uint i = 0; i < availableStations.size(); ++i)
            {
                String stationName(availableStations[i]);
                if(stationName.matches(regex))
                {
                    itsSelStations[i] = itsStations[i];
                    debugSelectedStations.push_back(availableStations[i]);
                }
            }
        }
    }
    LOG_INFO_STR("Selected station(s): " << debugSelectedStations);
    
    // Select baselines.
    LOG_INFO_STR("Requested baselines: " << correlation.selection);
    
    itsSelectedBaselines.clear();    
    for(int i = 0; i < itsMSDesc.ant1.size(); ++i)
    {
        const pair<int, int> baseline(itsMSDesc.ant1[i], itsMSDesc.ant2[i]);
        
        if(itsSelStations[baseline.first] && itsSelStations[baseline.second])
        {
            if( correlation.selection == "ALL"
                || (baseline.first == baseline.second && correlation.selection == "AUTO")
                || (baseline.first != baseline.second && correlation.selection == "CROSS"))
            {
                itsSelectedBaselines[baseline] = i;
            }
        }
    }
    LOG_INFO_STR("No. of selected baselines: " << itsSelectedBaselines.size());
        
    // Map flags into memory.
    string flagFile = getFileForColumn(MS::columnName(MS::FLAG));
    if(flagFile.empty())
    {
        return false;
    }
    LOG_INFO_STR("Input column " << MS::columnName(MS::FLAG) << " maps to: " << flagFile);
    itsFlagsMap = new FlagsMap(flagFile, MMap::Read);
    
    // Get all sources from the ParmDB.
    itsSources = new MeqSourceList(*itsGSMMEP, itsParmGroup);
    
    // Create the UVW nodes and fill them with uvw-s from MS if not calculated.
    fillUVW();
  
    return (itsSelectedBaselines.size() > 0);
}


bool Prediffer::setContext(const Context &context)
{
    // Select correlations.
    const vector<string> &availableCorrelations = itsMSDesc.corrTypes;
    const vector<string> &requestedCorrelations = context.correlation.type;
//    LOG_INFO_STR("Available correlation(s): " << availableCorrelations);
    LOG_INFO_STR("Requested correlation(s): " << requestedCorrelations);
    
    
    // Deselect all correlations _not_ present in requestedCorrelations.
    itsCorr = itsSelectedCorr;
    updateCorrelationMask(itsCorr, requestedCorrelations);
    
    // Verify that at least one correlations is still selected.
    if(count(itsCorr.begin(), itsCorr.end(), true) == 0)
    {
        LOG_ERROR("At least one correlation should be selected.");
        return false;
    }
    
    // Output selected correlations (inform the user).
    ostringstream os;
    os << "Selected correlations(s):";
    for(unsigned int i = 0; i < availableCorrelations.size(); ++i)
    {
        if(itsCorr[i])
        {
            os << " " << availableCorrelations[i];
        }
    }
    LOG_INFO_STR(os.str());
    
    // Select baselines.
    itsBLInx.clear();
    if(context.baselines.station1.empty())
    {
        // If no baselines are speficied, use all baselines selected in the strategy that
        // match the correlation selection of this context.
        map<pair<int, int>, int>::const_iterator it = itsSelectedBaselines.begin();
        while(it != itsSelectedBaselines.end())
        {
            const pair<int, int> &baseline = it->first;
            
            if( context.correlation.selection == "ALL"
                || (baseline.first == baseline.second && context.correlation.selection == "AUTO")
                || (baseline.first != baseline.second && context.correlation.selection == "CROSS"))
            {
                itsBLInx.push_back(it->second);
            }
            ++it;
        }
    }
    else
    {
        vector<string>::const_iterator baseline_it1 = context.baselines.station1.begin();
        vector<string>::const_iterator baseline_it2 = context.baselines.station2.begin();

        while(baseline_it1 != context.baselines.station1.end())
        {   
            // Find the IDs of all the stations of which the name matches the regex
            // specified in the context (Baselines.station1 and Baselines.station2).
            vector<int> stationGroup1, stationGroup2;
            casa::Regex regex1 = casa::Regex::fromPattern(*baseline_it1);
            casa::Regex regex2 = casa::Regex::fromPattern(*baseline_it2);

            for(int i = 0; i < itsMSDesc.antNames.size(); ++i)
            {
                String stationName(itsMSDesc.antNames[i]);

                if(stationName.matches(regex1))
                {
                    stationGroup1.push_back(i);
                }

                if(stationName.matches(regex2))
                {
                    stationGroup2.push_back(i);
                }
            }

            ASSERTSTR(!stationGroup1.empty() && !stationGroup2.empty(), "Baseline specification does not match any station in the measurement set.");

            // Generate all possible baselines (pairs) from the two groups of station IDs. If a baseline
            // is selected in the current strategy _and_ matches the correlation selection of this context,
            // select it for processing (by adding the ID of the _baseline_ to the itsBLInx vector).
            for(vector<int>::const_iterator it1 = stationGroup1.begin(); it1 != stationGroup1.end(); ++it1)
            {
                for(vector<int>::const_iterator it2 = stationGroup2.begin(); it2 != stationGroup2.end(); ++it2)
                {
                    if( context.correlation.selection == "ALL"
                        || (*it1 == *it2 && context.correlation.selection == "AUTO")
                        || (*it1 != *it2 && context.correlation.selection == "CROSS"))
                    {
                        const pair<int, int> baseline(*it1, *it2);
                        
                        map<pair<int, int>, int>::const_iterator index = itsSelectedBaselines.find(baseline);
                        
                        if(index != itsSelectedBaselines.end())
                        {
                            itsBLInx.push_back(index->second);
                        }
                    }
                }
            }

            ++baseline_it1;
            ++baseline_it2;
        }
    }
    ASSERTSTR(itsBLInx.size() > 0, "No baselines selected in current context.");
    
    // Clear the solvable flag on all parameters (may still be set from the
    // previous step.
    clearSolvableParms();
    
    // Create the measurement equation for each interferometer (baseline).
    if(context.sources.empty())
    {
        // If no sources are specified, use all available sources.
        makeTree(context.instrumentModel, itsSources->getSourceNames());
    }
    else
    {
        makeTree(context.instrumentModel, context.sources);
    }

    // Put funklets in parms which are not filled yet. (?)
    for (MeqParmGroup::iterator it = itsParmGroup.begin();
        it != itsParmGroup.end();
        ++it)
    {
        it->second.fillFunklets(itsParmValues, itsWorkDomain);
    }
    
    return true;
}


bool Prediffer::setWorkDomain(const MeqDomain &domain)
{
  int startChan = int(0.5 + (domain.startX() - itsStartFreq) / itsStepFreq);
  int endChan   = int(0.5 + (domain.endX() - itsStartFreq) / itsStepFreq) - 1;
  // Exit if this MS part has no overlap with the freq domain.
  if (endChan < 0  ||  startChan >= itsNrChan) {
    return false;
  }
  // Determine the first and last channel to process.
  if (startChan < 0) {
    itsFirstChan = 0;
  } else {
    itsFirstChan = startChan;
  }
  if (endChan >= itsNrChan) {
    itsLastChan = itsNrChan-1;
  } else {
    itsLastChan = endChan;
  }
  ASSERT (itsFirstChan <= itsLastChan);
  // Find the times matching the given time interval.
  double tstart = domain.startY();
  // Usually the times are processed in sequential order, so we can
  // continue searching.
  // Otherwise start the search at the start.
  itsTimeIndex += itsNrTimes;
  if (itsTimeIndex >= itsMSDesc.times.size()
  ||  tstart < itsMSDesc.times[itsTimeIndex]) {
    itsTimeIndex = 0;
  }
  // Find the time matching the start time.
  while (itsTimeIndex < itsMSDesc.times.size()
     &&  tstart > itsMSDesc.times[itsTimeIndex]) {
    ++itsTimeIndex;
  }
  // Exit if no more chunks.
  if (itsTimeIndex >= itsMSDesc.times.size()) {
    return false;
  }
  BBSTest::Logger::log("BeginOfInterval");
  // Find the end of the interval.
  uint endIndex = itsTimeIndex;
  double startTime = itsMSDesc.times[itsTimeIndex] -
                     itsMSDesc.exposures[itsTimeIndex]/2;
  double endTime = domain.endY();
  itsNrTimes     = 0;
  while (endIndex < itsMSDesc.times.size()
     && endTime >= itsMSDesc.times[endIndex]) {
    ///cout << "time find " << itsMSDesc.times[endIndex]-itsMSDesc.times[0] << ' ' << endIndex<<endl;
    ++endIndex;
    ++itsNrTimes;
  }
  ASSERT (itsNrTimes > 0);
  endTime = itsMSDesc.times[endIndex-1] + itsMSDesc.exposures[endIndex-1]/2;
  ///cout << "time-indices " << itsTimeIndex << ' ' << endIndex << ' ' << itsNrTimes << ' ' << itsMSDesc.times[endIndex]-itsMSDesc.times[0]<<' '<<endTime-tstart<<' '<<startTime-itsMSDesc.times[0]<< ' '<<endTime-itsMSDesc.times[0]<<endl;

  BBSTest::ScopedTimer parmTimer("P:readparms");
  itsWorkDomain = MeqDomain(itsStartFreq + itsFirstChan*itsStepFreq,
                itsStartFreq + (itsLastChan+1)*itsStepFreq,
                startTime,
                endTime);
  // Read all parameter values which are part of the work domain.
  readParms();
  // Show frequency domain.
  LOG_INFO_STR("Work domain: " << setprecision(10) << itsWorkDomain
        << " (channel(s) " << itsFirstChan << " - " << itsLastChan
        << " of " << itsStepFreq / 1000.0 << " kHz, " << itsNrTimes
        << " samples(s) of " << setprecision(3) << (endTime - startTime) / itsNrTimes
        << " s on average)");

  return true;
}


bool Prediffer::setWorkDomainSize(double freq, double time)
{
    double startTime = itsMSDesc.times[0] - itsMSDesc.exposures[0] / 2.0;
    return setWorkDomain(MeqDomain(itsStartFreq, itsStartFreq + freq, startTime, startTime + time));
}


bool Prediffer::setWorkDomain(int startChan, int endChan, double tstart, double tlength)
{
  // Determine the first and last channel to process.
  if (startChan < 0) {
    startChan = 0;
  }
  if (endChan < 0  ||  endChan >= itsNrChan) {
    endChan = itsNrChan-1;
  }
  return setWorkDomain (MeqDomain(itsStartFreq + startChan*itsStepFreq,
                  itsStartFreq + (endChan+1)*itsStepFreq,
                  tstart,
                  tstart+tlength));
}


bool Prediffer::setWorkDomain(double startFreq, double endFreq, double startTime, double endTime)
{
    return setWorkDomain(MeqDomain(startFreq, endFreq, startTime, endTime));
}

/*
bool Prediffer::setWorkDomain(int startChannel, int endChannel, double startTime, double endTime)
{
    if(startChannel < 0)
    {
        startChan = 0;
    }
    else if(startChannel >= itsNrChan)
    {
        startChannel = itsNrChan - 1;
    }
    
    if(endChannel < startChannel)
    {
        endChannel = startChannel;
    }
    else if(endChannel >= itsNrChan)
    {
        endChannel = itsNrChan - 1;
    }
    
    return setWorkDomain(MeqDomain( itsStartFreq + startChannel * itsStepFreq,
                                    itsStartFreq + (endChannel + 1) * itsStepFreq,
                                    startTime,
                                    endTime));
}
*/

bool Prediffer::setContext(const PredictContext &context)
{
    if(!setContext(dynamic_cast<const Context &>(context)))
    {
        return false;
    }
    
    itsOutDataColumn = context.outputColumn;
    return true;
}


bool Prediffer::setContext(const SubtractContext &context)
{
    if(!setContext(dynamic_cast<const Context &>(context)))
    {
        return false;
    }
    
    itsOutDataColumn = context.outputColumn;
    return true;
}


bool Prediffer::setContext(const CorrectContext &context)
{
    if(!setContext(dynamic_cast<const Context &>(context)))
    {
        return false;
    }
    
    itsOutDataColumn = context.outputColumn;
    return true;
}


bool Prediffer::setContext(const GenerateContext &context)
{
    if(!setContext(dynamic_cast<const Context &>(context)))
    {
        return false;
    }
    
    const vector<string>& unknowns = context.unknowns;
    const vector<string>& excludedUnknowns = context.excludedUnknowns;
    
    vector<casa::Regex> unknownsRegex;
    vector<casa::Regex> excludedUnknownsRegex;

    try
    {
        // Convert patterns to aips++ regexes.
        for(unsigned int i = 0; i < unknowns.size(); i++)
        {
            unknownsRegex.push_back(casa::Regex::fromPattern(unknowns[i]));
        }
    
        for(unsigned int i = 0; i < excludedUnknowns.size(); i++)
        {
            excludedUnknownsRegex.push_back(casa::Regex::fromPattern(excludedUnknowns[i]));
        }
    }
    catch(exception &_ex)
    {
        LOG_ERROR_STR("Error parsing Parms/ExclParms regular expression (exception: " << _ex.what() << ")");
        return false;
    }
    
    // Find all parms matching context.unknowns, exclude those that
    // match context.excludedUnknowns.
    int unknownCount = 0;
    for(MeqParmGroup::iterator parameter_it = itsParmGroup.begin();
        parameter_it != itsParmGroup.end();
        ++parameter_it)
    {
        String parameterName(parameter_it->second.getName());

        // Loop through all regex-es until a match is found.
        for(vector<casa::Regex>::iterator included_it = unknownsRegex.begin();
            included_it != unknownsRegex.end();
            included_it++)
        {
            if(parameterName.matches(*included_it))
            {
                bool include = true;
                
                // Test if excluded.
                for(vector<casa::Regex>::const_iterator excluded_it = excludedUnknownsRegex.begin();
                    excluded_it != excludedUnknownsRegex.end();
                    excluded_it++)
                {
                    if(parameterName.matches(*excluded_it))
                    {
                        include = false;
                        break;
                    }
                }
        
                if(include)
                {
                    LOG_TRACE_OBJ_STR("setSolvable: " << parameter_it->second.getName());
                    parameter_it->second.setSolvable(true);
                    unknownCount++;
                }
                
                break;
            }
        }
    }
    
    if(unknownCount == 0)
    {
	LOG_ERROR("No unknowns selected in this context.");
        return false;
    }
                        
    itsSolveDomains = context.solveDomains;
    initSolvableParms(itsSolveDomains);
    
    return itsNrPert>0;
}


void Prediffer::predictVisibilities()
{
    writePredictedData();
}


void Prediffer::subtractVisibilities()
{
    subtractData();
}


void Prediffer::correctVisibilities()
{
    correctData();
}


void Prediffer::generateEquations(vector<casa::LSQFit> &equations)
{
    fillFitters(equations);
}


//-----------------------[ Model ]-----------------------//


void Prediffer::makeTree (const vector<string>& modelType,
              const vector<string>& sourceNames)
{
  // Determine which parts of the model to use.
  bool asAP = true;
  bool useDipole = false;
  bool usePatchGain = false;
  bool useTotalGain = false;
  bool useBandpass = false;
  for (uint i=0; i<modelType.size(); ++i) {
    if (modelType[i] != "") {
      if (modelType[i] == "TOTALGAIN") {
    useTotalGain = true;
      } else if (modelType[i] == "PATCHGAIN") {
    usePatchGain = true;
      } else if (modelType[i] == "REALIMAG") {
    asAP = false;
      } else if (modelType[i] == "DIPOLE") {
    useDipole = true;
      } else if (modelType[i] == "BANDPASS") {
    useBandpass = true;
      } else {
    ASSERTSTR (false, "Modeltype part " << modelType[i] << " is invalid; "
           "valid are TOTALGAIN,PATCHGAIN,REALIMAG,DIPOLE,BANDPASS");
      }
    }
  }
  
  LOG_INFO_STR("Instrument model:"
    << (asAP ? "" : " REALIMAG") 
    << (useDipole ? " DIPOLE" : "") 
    << (useTotalGain ? " TOTALGAIN" : "") 
    << (usePatchGain ? " PATCHGAIN" : "") 
    << (useBandpass ? " BANDPASS" : ""));
  
  // Find all sources and groups to use.
  // Make an LMN node for each source used.
  int nrsrc = sourceNames.size();
  vector<MeqSource*> sources;
  map<string,vector<int> > groups;
  itsLMN.clear();
  itsLMN.reserve (nrsrc);
  for (int i=0; i<nrsrc; ++i) {
    MeqSource* src = itsSources->getSource (sourceNames[i]);
    // Add source to list.
    sources.push_back (src);
    // Add source index to group.
    groups[src->getGroupName()].push_back (i);
    MeqLMN* lmn = new MeqLMN(src);
    lmn->setPhaseRef (&itsPhaseRef);
    itsLMN.push_back (lmn);
  }
  // Set up the expression tree for all baselines.
  makeLOFARExprs (sources, groups,
          useTotalGain, usePatchGain, asAP, useDipole, useBandpass);
}

void Prediffer::makeLOFARExprs (const vector<MeqSource*>& sources,
                const map<string, vector<int> >& groups,
                bool useTotalGain, bool usePatchGain,
                bool asAP,
                bool useDipole, bool useBandpass)
{
  // Allocate the vectors holding the expressions.
  int nrstat = itsStations.size();
  int nrsrc  = sources.size();
  int nrgrp  = groups.size();
  // GJ is real/imag or ampl/phase
  string gjname1 = "real:";
  string gjname2 = "imag:";
  if (asAP) {
    gjname1 = "ampl:";
    gjname2 = "phase:";
  }
  // Vector containing DipoleExpr-s.
  vector<MeqJonesExpr> dipoleExpr(nrstat);
  // Vector containing DFTPS-s.
  vector<MeqExpr> pdfts(nrsrc*nrstat);
  // Vector containing all gains per station per patch.
  vector<MeqJonesExpr> patchGJ(nrgrp*nrstat);
  // Vector containing all Gain-s per station.
  vector<MeqJonesExpr> totalGJ(nrstat);
  // Correction per station.
  vector<MeqJonesExpr> corrStat(nrstat);
  // Bandpass per station.
  vector<MeqJonesExpr> bandpass(nrstat);

  // Fill the vectors for each station.
  for (int stat=0; stat<nrstat; ++stat) {
    // Do it only if the station is actually used.
    if (itsSelStations[stat] != 0) {
      // Do pure station parameters only if told so.
      if (useDipole) {
    MeqExpr frot (MeqParmFunklet::create ("frot:" +
                          itsStations[stat]->getName(),
                          itsParmGroup, itsMEP));
    MeqExpr drot (MeqParmFunklet::create ("drot:" +
                          itsStations[stat]->getName(),
                          itsParmGroup, itsMEP));
    MeqExpr dell (MeqParmFunklet::create ("dell:" +
                          itsStations[stat]->getName(),
                          itsParmGroup, itsMEP));
    MeqExpr gj11 (MeqParmFunklet::create ("dgain:X:" +
                          itsStations[stat]->getName(),
                          itsParmGroup, itsMEP));
    MeqExpr gj22 (MeqParmFunklet::create ("dgain:Y:" +
                          itsStations[stat]->getName(),
                          itsParmGroup, itsMEP));
    dipoleExpr[stat] = MeqJonesExpr(new MeqStatExpr (frot, drot, dell,
                             gj11, gj22));
      }
      // Make a bandpass per station
      if (useBandpass) {
        string stationName = itsStations[stat]->getName();
        MeqExpr bandpassXX(MeqParmFunklet::create("bandpass:X:" + stationName,
                          itsParmGroup, itsMEP));
        MeqExpr bandpassYY(MeqParmFunklet::create("bandpass:Y:" + stationName,
                          itsParmGroup, itsMEP));

        bandpass[stat] = new MeqDiag(MeqExpr(bandpassXX), MeqExpr(bandpassYY));
      }
      // Make a DFT per station per source.
      for (int src=0; src<nrsrc; ++src) {
    pdfts[stat*nrsrc + src] = MeqExpr(new MeqDFTPS (itsLMN[src],
                            itsStatUVW[stat]));
      }
      // Make optionally GJones expressions.
      MeqExprRep* gj11;
      MeqExprRep* gj12;
      MeqExprRep* gj21;
      MeqExprRep* gj22;
      if (useTotalGain) {
    // Make a gain/phase expression per station.
    string nm = itsStations[stat]->getName();
    MeqExpr gj11r (MeqParmFunklet::create ("gain:11:" + gjname1 + nm,
                           itsParmGroup, itsMEP));
    MeqExpr gj11i (MeqParmFunklet::create ("gain:11:" + gjname2 + nm,
                           itsParmGroup, itsMEP));
    MeqExpr gj12r (MeqParmFunklet::create ("gain:12:" + gjname1 + nm,
                           itsParmGroup, itsMEP));
    MeqExpr gj12i (MeqParmFunklet::create ("gain:12:" + gjname2 + nm,
                           itsParmGroup, itsMEP));
    MeqExpr gj21r (MeqParmFunklet::create ("gain:21:" + gjname1 + nm,
                           itsParmGroup, itsMEP));
    MeqExpr gj21i (MeqParmFunklet::create ("gain:21:" + gjname2 + nm,
                           itsParmGroup, itsMEP));
    MeqExpr gj22r (MeqParmFunklet::create ("gain:22:" + gjname1 + nm,
                           itsParmGroup, itsMEP));
    MeqExpr gj22i (MeqParmFunklet::create ("gain:22:" + gjname2 + nm,
                           itsParmGroup, itsMEP));
    if (asAP) {
      gj11 = new MeqExprAPToComplex (gj11r, gj11i);
      gj12 = new MeqExprAPToComplex (gj12r, gj12i);
      gj21 = new MeqExprAPToComplex (gj21r, gj21i);
      gj22 = new MeqExprAPToComplex (gj22r, gj22i);
    } else {
      gj11 = new MeqExprToComplex (gj11r, gj11i);
      gj12 = new MeqExprToComplex (gj12r, gj12i);
      gj21 = new MeqExprToComplex (gj21r, gj21i);
      gj22 = new MeqExprToComplex (gj22r, gj22i);
    }
    totalGJ[stat] = new MeqJonesNode (MeqExpr(gj11), MeqExpr(gj12),
                      MeqExpr(gj21), MeqExpr(gj22));
    corrStat[stat] = new MeqJonesInvert (totalGJ[stat]);
      }
      if (usePatchGain) {
    // Make a complex gain expression per station per patch.
    int grp=0;
    for (map<string,vector<int> >::const_iterator grpiter = groups.begin();
         grpiter != groups.end();
         grpiter++, grp++) {
      string nm = itsStations[stat]->getName() + ":" + grpiter->first;
      MeqExpr gj11r (MeqParmFunklet::create ("gain:11:" + gjname1 + nm,
                         itsParmGroup, itsMEP));
      MeqExpr gj11i (MeqParmFunklet::create ("gain:11:" + gjname2 + nm,
                         itsParmGroup, itsMEP));
      MeqExpr gj12r (MeqParmFunklet::create ("gain:12:" + gjname1 + nm,
                         itsParmGroup, itsMEP));
      MeqExpr gj12i (MeqParmFunklet::create ("gain:12:" + gjname2 + nm,
                         itsParmGroup, itsMEP));
      MeqExpr gj21r (MeqParmFunklet::create ("gain:21:" + gjname1 + nm,
                         itsParmGroup, itsMEP));
      MeqExpr gj21i (MeqParmFunklet::create ("gain:21:" + gjname2 + nm,
                         itsParmGroup, itsMEP));
      MeqExpr gj22r (MeqParmFunklet::create ("gain:22:" + gjname1 + nm,
                         itsParmGroup, itsMEP));
      MeqExpr gj22i (MeqParmFunklet::create ("gain:22:" + gjname2 + nm,
                         itsParmGroup, itsMEP));
      if (asAP) {
        gj11 = new MeqExprAPToComplex (gj11r, gj11i);
        gj12 = new MeqExprAPToComplex (gj12r, gj12i);
        gj21 = new MeqExprAPToComplex (gj21r, gj21i);
        gj22 = new MeqExprAPToComplex (gj22r, gj22i);
      } else {
        gj11 = new MeqExprToComplex (gj11r, gj11i);
        gj12 = new MeqExprToComplex (gj12r, gj12i);
        gj21 = new MeqExprToComplex (gj21r, gj21i);
        gj22 = new MeqExprToComplex (gj22r, gj22i);
      }
      patchGJ[stat*nrgrp + grp] = new MeqJonesNode (MeqExpr(gj11),
                            MeqExpr(gj12),
                            MeqExpr(gj21),
                            MeqExpr(gj22));
      // Only AP of first group is used for correction.
      if (grp == 0) {
        corrStat[stat] = new MeqJonesInvert (patchGJ[stat*nrgrp + grp]);
      }
    }
      }
    }
  }
  // Make an expression for each baseline.
  int nrusedbl = itsBLInx.size();
  itsExpr.resize (nrusedbl);
  itsCorrExpr.resize (nrusedbl);
  itsCorrMMap.resize (nrusedbl);
  for (int blindex=0; blindex<nrusedbl; blindex++) {
    int bl = itsBLInx[blindex];
    int ant1 = itsMSDesc.ant1[bl];
    int ant2 = itsMSDesc.ant2[bl];
    if (usePatchGain || useTotalGain) {
      // Make correction expressions.
      itsCorrMMap[blindex] = new MeqJonesMMap (itsMSMapInfo, bl);
      itsCorrExpr[blindex] = new MeqJonesCMul3 (corrStat[ant1],
                        itsCorrMMap[blindex],
                        corrStat[ant2]);
    }
    // Predict expressions.
    vector<MeqJonesExpr> vecPatch;
    // Loop through all source groups.
    int grp=0;
    for (map<string,vector<int> >::const_iterator grpiter = groups.begin();
     grpiter != groups.end();
     grpiter++, grp++) {
      const vector<int>& srcgrp = grpiter->second;;
      vector<MeqJonesExpr> vecSrc;
      vecSrc.reserve (srcgrp.size());
      for (uint j=0; j<srcgrp.size(); ++j) {
    // Create the total DFT per source.
    int src = srcgrp[j];
    MeqExpr expr1 (new MeqBaseDFTPS (pdfts[ant1*nrsrc + src],
                     pdfts[ant2*nrsrc + src],
                     itsLMN[src]));
    // For the time being only point sources are supported.
    MeqPointSource& mps = dynamic_cast<MeqPointSource&>(*sources[src]);
    vecSrc.push_back (MeqJonesExpr (new MeqBaseLinPS(expr1, &mps)));
      }
      MeqJonesExpr sum;
      // Sum all sources in the group.
      if (vecSrc.size() == 1) {
    sum = vecSrc[0];
      } else {
    sum = MeqJonesExpr (new MeqJonesSum(vecSrc));
      }
      // Multiply by ionospheric gain/phase per station per patch.
      if (usePatchGain) {
    vecPatch.push_back (new MeqJonesCMul3(patchGJ[ant1*nrgrp + grp],
                          sum,
                          patchGJ[ant2*nrgrp + grp]));
      } else {
    vecPatch.push_back (sum);
      }
    }
    // Sum all patches.
    MeqJonesExpr sumAll;
    if (vecPatch.size() == 1) {
      sumAll = vecPatch[0];
    } else {
      sumAll = MeqJonesExpr (new MeqJonesSum(vecPatch));
    }
    // Multiply by total gain/phase per station.
    if (useTotalGain) {
      sumAll = new MeqJonesCMul3(totalGJ[ant1],
                 sumAll,
                 totalGJ[ant2]);
    }
    if (useBandpass) {
      sumAll = new MeqJonesCMul3(bandpass[ant1],
                 sumAll,
                 bandpass[ant2]);
    }
    if (useDipole) {
      sumAll = new MeqJonesCMul3(dipoleExpr[ant1],
                 sumAll,
                 dipoleExpr[ant2]);
    }
    itsExpr[blindex] = sumAll;
  }
}

void Prediffer::setPrecalcNodes (vector<MeqJonesExpr>& nodes)
{
  // First clear the levels of all nodes in the tree.
  for (uint i=0; i<nodes.size(); ++i) {
    if (! nodes[i].isNull()) {
      nodes[i].clearDone();
    }
  }
  // Now set the levels of all nodes in the tree.
  // The top nodes have level 0; lower nodes have 1, 2, etc..
  int nrLev = -1;
  for (uint i=0; i<nodes.size(); ++i) {
    if (! nodes[i].isNull()) {
      nrLev = max (nrLev, nodes[i].setLevel(0));
    }
  }
  nrLev++;
  ASSERT (nrLev > 0);
  itsPrecalcNodes.resize (nrLev);
  // Find the nodes to be precalculated at each level.
  // That is not needed for the root nodes (the baselines).
  // The nodes used by the baselines are always precalculated (even if
  // having one parent).
  // It may happen that a station is used by only one baseline. Calculating
  // such a baseline is much more work if the station was not precalculated.
  for (int level=1; level<nrLev; ++level) {
    vector<MeqExprRep*>& pcnodes = itsPrecalcNodes[level];
    pcnodes.resize (0);
    for (vector<MeqJonesExpr>::iterator iter=nodes.begin();
     iter != nodes.end();
     ++iter) {
      if (! iter->isNull()) {
    iter->getCachingNodes (pcnodes, level, false);
      }
    }
  }
  LOG_TRACE_FLOW_STR("#levels=" << nrLev);
  for (int i=0; i<nrLev; ++i) {
    LOG_TRACE_FLOW_STR("#expr on level " << i << " is " << itsPrecalcNodes[i].size());
  }
}

void Prediffer::precalcNodes (const MeqRequest& request)
{
#pragma omp parallel
  {
    // Loop through expressions to be precalculated.
    // At each level the expressions can be executed in parallel.
    // Level 0 is formed by itsExpr which are not calculated here.
    for (int level = itsPrecalcNodes.size(); --level > 0;) {
      vector<MeqExprRep*>& exprs = itsPrecalcNodes[level];
      int nrExprs = exprs.size();
      if (nrExprs > 0) {
#pragma omp for schedule(dynamic)
    for (int i=0; i<nrExprs; ++i) {
      exprs[i]->precalculate (request);
    }
      }
    }
  } // end omp parallel
}

//-----------------------[ MS Access ]-----------------------//

void Prediffer::readMeasurementSetMetaData(const string &fileName)
{
    MeasurementSet ms(fileName);

    Path absolutePath = Path(fileName).absoluteName();
    itsMSDesc.msPath = absolutePath.dirName();
    itsMSDesc.msName = absolutePath.baseName();
    itsMSDesc.npart  = 1;

    /*
      Get baselines.
    */
    Block<String> sortColumns(2);
    sortColumns[0] = "ANTENNA1";
    sortColumns[1] = "ANTENNA2";
    Table uniqueBaselines = ms.sort(sortColumns, Sort::Ascending, Sort::QuickSort + Sort::NoDuplicates);
    ROScalarColumn<Int> station1Column(uniqueBaselines, "ANTENNA1");
    ROScalarColumn<Int> station2Column(uniqueBaselines, "ANTENNA2");
    station1Column.getColumn().tovector(itsMSDesc.ant1);
    station2Column.getColumn().tovector(itsMSDesc.ant2);

    /*
      Get information about frequency grid.
    */
    MSDataDescription dataDescription(ms.dataDescription());
    ROMSDataDescColumns dataDescriptionColumns(dataDescription);
    MSSpectralWindow spectralWindow(ms.spectralWindow());
    ROMSSpWindowColumns spectralWindowColumns(spectralWindow);
    int spectralWindowCount = dataDescription.nrow();

    itsMSDesc.nchan.resize(spectralWindowCount);
    itsMSDesc.startFreq.resize(spectralWindowCount);
    itsMSDesc.endFreq.resize(spectralWindowCount);

    for(int i = 0; i < spectralWindowCount; i++)
    {
        Vector<double> channelFrequency = spectralWindowColumns.chanFreq()(i);
        Vector<double> channelWidth = spectralWindowColumns.chanWidth()(i);

        // So far, only equal frequency spacings are possible.
        ASSERTSTR(allEQ(channelWidth, channelWidth(0)), "Channels must have equal spacings");

        int channelCount = channelWidth.nelements();
        itsMSDesc.nchan[i] = channelCount;

        double step = abs(channelWidth(0));
        if(channelFrequency(0) > channelFrequency(channelCount - 1))
        {
            itsMSDesc.startFreq[i] = channelFrequency(0) + step / 2;
            itsMSDesc.endFreq[i]   = itsMSDesc.startFreq[i] - channelCount * step;
        }
        else
        {
            itsMSDesc.startFreq[i] = channelFrequency(0) - step / 2;
            itsMSDesc.endFreq[i]   = itsMSDesc.startFreq[i] + channelCount * step;
        }
    }

    /*
      Get information about time grid.
    */
    ROScalarColumn<double> timeColumn(ms, "TIME");
    Vector<double> time = timeColumn.getColumn();
    Vector<uInt> timeIndex;
    uInt timeCount = GenSortIndirect<double>::sort(timeIndex, time, Sort::Ascending, Sort::InsSort + Sort::NoDuplicates);

    itsMSDesc.times.resize(timeCount);
    itsMSDesc.exposures.resize(timeCount);
    ROScalarColumn<double> exposureColumn(ms, "EXPOSURE");
    Vector<double> exposure = exposureColumn.getColumn();

    for(uInt i = 0; i < timeCount; i++)
    {
        itsMSDesc.times[i] = time[timeIndex[i]];
        itsMSDesc.exposures[i] = exposure[timeIndex[i]];
    }

    /*
      Get phase center as RA and DEC (J2000).
     
      From AIPS++ note 229 (MeasurementSet definition version 2.0):
      ---
      FIELD: Field positions for each source
      Notes:
      The FIELD table defines a field position on the sky. For interferometers,
      this is the correlated field position. For single dishes, this is the
      nominal pointing direction.
      ---

      The way this column is used by SelfCal seems to have nothing to do with sources.
      In LOFAR/CEP/BB/MS/src/makemsdesc.cc the following line can be found:
      MDirection phaseRef = mssubc.phaseDirMeasCol()(0)(IPosition(1,0));
      which should be equivalent to:
      MDirection phaseRef = mssubc.phaseDirMeas(0);
      as used in the code below.
    */
    MSField field(ms.field());
    ROMSFieldColumns fieldColumns(field);
    MDirection phaseCenter = MDirection::Convert(fieldColumns.phaseDirMeas(0), MDirection::J2000)();
    Quantum<Vector<double> > phaseCenterAngles = phaseCenter.getAngle();
    itsMSDesc.ra  = phaseCenterAngles.getBaseValue()(0);
    itsMSDesc.dec = phaseCenterAngles.getBaseValue()(1);

    /*
      Get correlation types.
    */
    MSPolarization polarization(ms.polarization());
    ROMSPolarizationColumns polarizationColumn(polarization);
    Vector<Int> correlationTypes = polarizationColumn.corrType()(0);
    int correlationTypeCount = correlationTypes.nelements();
    itsMSDesc.corrTypes.resize(correlationTypeCount);
    for(int i = 0; i < correlationTypeCount; i++)
    {
        itsMSDesc.corrTypes[i] = Stokes::name(Stokes::type(correlationTypes(i)));
    }

    /*
      Get station names and positions in ITRF coordinates.
    */
    MSAntenna antenna(ms.antenna());
    ROMSAntennaColumns antennaColumns(antenna);
    int antennaCount = antenna.nrow();

    MVPosition sumVector;
    itsMSDesc.antNames.resize(antennaCount);
    itsMSDesc.antPos.resize(IPosition(2, 3, antennaCount));
    for(int i = 0; i < antennaCount; i++)
    {
        itsMSDesc.antNames[i] = antennaColumns.name()(i);
        MPosition position = antennaColumns.positionMeas()(i);
        position = MPosition::Convert(position, MPosition::ITRF)();
        const MVPosition& positionVector = position.getValue();
        sumVector += positionVector;
      
        for(int j = 0; j < 3; j++)
        {
            itsMSDesc.antPos(IPosition(2, j, i)) = positionVector(j);
        }
    }

    /*
      Get the array position in ITRF coordinates, or use the centroid
      of the station positions if the array position is unknown.
    */
    MSObservation observation(ms.observation());
    ROMSObservationColumns observationColumns(observation);

    MPosition position;
    MVPosition positionVector;
    if(observation.nrow() > 0 && MeasTable::Observatory(position, observationColumns.telescopeName()(0)))
    {
        position = MPosition::Convert(position, MPosition::ITRF)();
        positionVector = position.getValue();
        //LOG_TRACE_FLOW("Array position: " << position);
    }
    else
    {
        LOG_WARN("Array position unknown; will use centroid of stations.");
        positionVector = sumVector * (1.0 / (double) antennaCount);
    }

    itsMSDesc.arrayPos.resize(3);
    for(int i = 0; i < 3; i++)
    {
        itsMSDesc.arrayPos[i] = positionVector(i);
    }

    /*
      Determine the startTime and endTime of the observation.
    */
    if(observation.nrow() > 0)
    {
        Vector<double> times = observationColumns.timeRange()(0);
        itsMSDesc.startTime  = times(0);
        itsMSDesc.endTime    = times(1);
    }
    else
    {
        itsMSDesc.startTime = 0.0;
        itsMSDesc.endTime   = 0.0;
    }

    if(itsMSDesc.startTime >= itsMSDesc.endTime)
    {
        /*
          Invalid start / end times; derive from times and interval.
          Difference between interval and exposure is startup time which
          is taken into account.
        */
        if(timeCount > 0)
        {
            ROScalarColumn<double> interval(ms, "INTERVAL");
            itsMSDesc.startTime = itsMSDesc.times[0] - itsMSDesc.exposures[0] / 2 + (interval(0) - itsMSDesc.exposures[0]);
            itsMSDesc.endTime   = itsMSDesc.times[timeCount - 1] + interval(timeCount - 1) / 2;
        }
    }

    /*
        Create a map from column name to file on disk.
    */
    Record info = ms.dataManagerInfo();
    for(unsigned int i = 0; i < info.nfields(); ++i)
    {
        const Record &dm = info.subRecord(i);
        const Array<String> columns = dm.asArrayString("COLUMNS");
        
        /*
            BBS requires TiledColumnStMan/TiledShapeStMan (or another StorageManager that
            stores data linearly on disk) and one column per DataManager. This is because
            mmap is used to access the data.
        */
        if((dm.asString("TYPE") == "TiledColumnStMan" ||
           dm.asString("TYPE") == "TiledShapeStMan") &&
           columns.size() == 1)
        {
            /*
                The data bound to a TiledStMan is stored in a file called table.f<seqnr>_TSM<stman> where
                <seqnr> is SPEC.SEQNR stored in the DataManager info record and <stman> indicates the
                specific tiled storage manager. If several TiledStMan are used in one MS, they are numbered
                sequentially. Because there is no easy way to determine which number belongs to which
                TiledStMan, the mapping created here will not always be correct. However, it is guaranteed
                to be correct for MSs created by Storage.
            */
            unsigned int idx = i;
            
            try
            {
                const Record &spec = dm.asRecord("SPEC");
                idx = spec.asuInt("SEQNR");
            }
            catch(AipsError &_ex)
            {
                LOG_WARN_STR("DataManager " << i + 1 << " has no SPEC.SEQNR. Please verify (e.g. with glish) that the columns bound to this DataManager are stored in " << fileName << "/table.f" << i << "_TSM" << (dm.asString("TYPE") == "TiledColumnStMan" ? "0" : "1") << ".");
            }
        
            /*
                Update the column map.
            */
            ostringstream os;
            if(dm.asString("TYPE") == "TiledColumnStMan")
                os << itsMSName << "/table.f" << idx << "_TSM0";
            else
                os << itsMSName << "/table.f" << idx << "_TSM1";
            
            for(Array<String>::const_iterator it = columns.begin(); it != columns.end(); ++it)
            {
                itsColumns[static_cast<string>(*it)] = os.str();
            }
        }            
    }
}

string Prediffer::getFileForColumn(const string &column)
{
    map<string, string>::const_iterator it = itsColumns.find(column);
    
    if(it == itsColumns.end())
    {
        return string();    
    }
    else
    {
        return it->second;
    }
}

void Prediffer::processMSDesc (uint ddid)
{
  ASSERT (ddid < itsMSDesc.nchan.size());
  // Set the observation info.
  itsNCorr     = itsMSDesc.corrTypes.size();
  itsSelectedCorr.resize(itsNCorr);
  itsCorr.resize(itsNCorr);
  itsNrChan    = itsMSDesc.nchan[ddid];
  itsStartFreq = itsMSDesc.startFreq[ddid];
  itsEndFreq   = itsMSDesc.endFreq[ddid];
  itsStepFreq  = (itsEndFreq - itsStartFreq)/itsNrChan;
  itsNrBl      = itsMSDesc.ant1.size();
  itsReverseChan = itsStartFreq > itsEndFreq;
  if (itsReverseChan) {
    double  tmp  = itsEndFreq;
    itsEndFreq   = itsStartFreq;
    itsStartFreq = tmp;
    itsStepFreq  = abs(itsStepFreq);
  }
  // Set the MS info.
  itsMSMapInfo = MMapMSInfo (itsMSDesc, ddid, itsReverseChan);
  // Set the phase center info.
  getPhaseRef (itsMSDesc.ra, itsMSDesc.dec, itsMSDesc.startTime);
  // Set station info.
  fillStations();
}

void Prediffer::getPhaseRef(double ra, double dec, double startTime)
{
  // Use the phase reference of the given J2000 ra/dec.
  MVDirection mvdir(ra, dec);
  MDirection phaseRef(mvdir, MDirection::J2000);
  itsPhaseRef = MeqPhaseRef (phaseRef, startTime);
}


void Prediffer::fillStations()
{
  // Use the array as a 2D one.
  Matrix<double> antPos (itsMSDesc.antPos);
  uint nrant = antPos.ncolumn();
  itsStations = vector<MeqStation*>(nrant, (MeqStation*)0);
  // Get all stations actually used.
  for (uint ant=0; ant<nrant; ant++) {
    // Store each position as a constant parameter.
    // Use the antenna name as the parameter name.
    Vector<double> antpos = antPos.column(ant);
    const string& name = itsMSDesc.antNames[ant];
    MeqParmSingle* px = new MeqParmSingle("AntPosX." + name,
                      antpos(0));
    MeqParmSingle* py = new MeqParmSingle("AntPosY." + name,
                      antpos(1));
    MeqParmSingle* pz = new MeqParmSingle("AntPosZ." + name,
                      antpos(2));
    itsStations[ant] = new MeqStation(px, py, pz, name);
  }
}

void Prediffer::mapDataFiles (const string& inColumnName,
                  const string& outColumnName)
{
  Table tab;
  
  if (! inColumnName.empty()) {
    string inFile = getFileForColumn(inColumnName);
    ASSERTSTR(!inFile.empty(), "Column " << inColumnName << " does not exist or has non-standard storage manager.");
    
    if (itsInDataMap == 0  ||  itsInDataMap->getFileName() != inFile) {
      tab = Table(itsMSName);
      ASSERTSTR (tab.tableDesc().isColumn(inColumnName),
         "Column " << inColumnName << " does not exist");
      delete itsInDataMap;
      itsInDataMap = 0;
      // See if the input file is the previous output file.
      if (itsOutDataMap  &&  itsOutDataMap->getFileName() == inFile) {
    itsInDataMap = itsOutDataMap;
    itsOutDataMap = 0;
      } else {
    itsInDataMap = new MMap (inFile, MMap::Read);
      }
      LOG_INFO_STR("Input column " << inColumnName << " maps to: " << inFile);
    }
  }
  
  
  if (! outColumnName.empty()) {
    if (tab.isNull()) {
      tab = Table(itsMSName);
    }
    
    string outFile = getFileForColumn(outColumnName);
    if (itsOutDataMap == 0 || outFile.empty() || itsOutDataMap->getFileName() != outFile) {
      if (!tab.tableDesc().isColumn(outColumnName)) {
//    addDataColumn (tab, outColumnName, outFile);
        addDataColumn (tab, outColumnName);
        outFile = getFileForColumn(outColumnName);
      }
      ASSERTSTR(!outFile.empty(), "Column " << outColumnName << " does not exist or has non-standard storage manager.");
      
      delete itsOutDataMap;
      itsOutDataMap = 0;
      itsOutDataMap = new MMap (outFile, MMap::ReWr);
      
      LOG_INFO_STR("Output column " << outColumnName << " maps to: " << outFile);
    }
  }
}


void Prediffer::addDataColumn(Table& tab, const string& columnName)
{
  ASSERT(itsColumns.find(columnName) == itsColumns.end());
  
  ArrayColumnDesc<Complex> resCol(columnName, IPosition(2, itsNCorr, itsNrChan), ColumnDesc::FixedShape);
  String stManName = "Tiled_"+columnName;
  TiledColumnStMan tiledRes(stManName, IPosition(3, itsNCorr, itsNrChan, 1));
  
  tab.reopenRW();
  tab.addColumn (resCol, tiledRes);
  tab.flush();
  
  /*
    Find out which datamanager the new column is in.
  */
  Record dminfo = tab.dataManagerInfo();
  for (uint i=0; i<dminfo.nfields(); ++i)
  {
    const Record& dm = dminfo.subRecord(i);
    if (dm.asString("NAME") == stManName)
    {
      ostringstream os;
      os << itsMSName << "/table.f" << i << "_TSM0";
      itsColumns[columnName] = os.str();
      break;
    }
  }
}

//-----------------------[ Parameter Access ]-----------------------//

void Prediffer::readParms()
{
  // Read all parms for this domain into a single map.
  itsParmValues.clear();
  vector<string> emptyvec;
  LOFAR::ParmDB::ParmDomain pdomain(itsWorkDomain.startX(), itsWorkDomain.endX(),
                 itsWorkDomain.startY(), itsWorkDomain.endY());
  itsMEP->getValues (itsParmValues, emptyvec, pdomain);
  itsGSMMEP->getValues (itsParmValues, emptyvec, pdomain);
  // Remove the funklets from all parms.
  for (MeqParmGroup::iterator iter = itsParmGroup.begin();
       iter != itsParmGroup.end();
       ++iter)
  {
    iter->second.removeFunklets();
  }
}

void Prediffer::initSolvableParms (const vector<MeqDomain>& solveDomains)
{
  itsParmData.set (solveDomains, itsWorkDomain);
  const vector<MeqDomain>& localSolveDomains = itsParmData.getDomains();

  itsNrPert = 0;
  itsNrScids.resize (localSolveDomains.size());
  fill (itsNrScids.begin(), itsNrScids.end(), 0);
  for (MeqParmGroup::iterator iter = itsParmGroup.begin();
       iter != itsParmGroup.end();
       ++iter)
  {
    int nr = iter->second.initDomain (localSolveDomains,
                      itsNrPert, itsNrScids);
    if (nr > 0) {
      itsParmData.parms().push_back (ParmData(iter->second.getName(),
                          iter->second.getParmDBSeqNr(),
                          iter->second.getFunklets()));
    }
  }
  // Determine the fitter indices for the frequency and time axis.
  // First check if the (local) solve domains are ordered and regular.
  // Start with finding the number of frequency intervals.
  for (itsFreqNrFit=1; itsFreqNrFit<localSolveDomains.size(); itsFreqNrFit++) {
    if (localSolveDomains[itsFreqNrFit].startX() <
    localSolveDomains[itsFreqNrFit-1].endX()) {
      break;
    }
  }
  // Now check if regular in frequency and time.
  uint timeNrFit = localSolveDomains.size() / itsFreqNrFit;
  ASSERT (timeNrFit*itsFreqNrFit == localSolveDomains.size());
  for (uint inxt=0; inxt<timeNrFit; inxt++) {
    const MeqDomain& first = localSolveDomains[inxt*itsFreqNrFit];
    for (uint inxf=1; inxf<itsFreqNrFit; inxf++) {
      const MeqDomain& cur = localSolveDomains[inxt*itsFreqNrFit + inxf];
      ASSERT (cur.startY() == first.startY()  &&  cur.endY() == first.endY());
    }
  }
  for (uint inxf=0; inxf<itsFreqNrFit; inxf++) {
    const MeqDomain& first = localSolveDomains[inxf];
    for (uint inxt=1; inxt<timeNrFit; inxt++) {
      const MeqDomain& cur = localSolveDomains[inxt*itsFreqNrFit + inxf];
      ASSERT (cur.startX() == first.startX()  &&  cur.endX() == first.endX());
    }
  }
  // Determine for each frequency point to which fitter it belongs.
  // For the time axis it is determined by nextDataChunk.
  int nrchan = itsLastChan-itsFirstChan+1;
  itsFreqFitInx.resize (nrchan);
  double step = itsWorkDomain.sizeX() / nrchan;
  double freq = itsWorkDomain.startX() + step / 2;
  int interv = 0;
  for (int i=0; i<nrchan; i++) {
    if (freq > localSolveDomains[interv].endX()) {
      interv++;
    }
    itsFreqFitInx[i] = interv;
    freq += step;
  }
}

void Prediffer::clearSolvableParms()
{
  LOG_TRACE_FLOW( "clearSolvableParms" );
  for (MeqParmGroup::iterator iter = itsParmGroup.begin();
       iter != itsParmGroup.end();
       ++iter)
  {
    iter->second.setSolvable(false);
  }
}


//-----------------------[ Computation ]-----------------------//


// Currently this function always returns all data in the work domain.
// In the future it can be made smarter and process less if the work domain
// is very large.
bool Prediffer::nextDataChunk (bool useFitters)
{
  if (itsNrTimesDone >= itsNrTimes) {
    return false;
  }
  /// For the time being all times of the work domain are mapped in.
  int nt = itsNrTimes;
  int st = itsNrTimesDone;
  // Map the part of the file matching the given times.
  BBSTest::ScopedTimer mapTimer("P:file-mapping");

  int64 nrValues = nt * itsMSMapInfo.timeSize();
  int64 startOffset = itsTimeIndex * itsMSMapInfo.timeSize();
  if (itsInDataMap) {
    itsInDataMap->mapFile(startOffset*sizeof(fcomplex),
              nrValues*sizeof(fcomplex));
    itsMSMapInfo.setInData(static_cast<fcomplex*>(itsInDataMap->getStart()));
  }
  if (itsOutDataMap) {
    itsOutDataMap->mapFile(startOffset*sizeof(fcomplex),
               nrValues*sizeof(fcomplex));
    itsMSMapInfo.setOutData(static_cast<fcomplex*>(itsOutDataMap->getStart()));
  }
  // Map the correct flags subset (this time interval).
  itsFlagsMap->mapFile(startOffset, nrValues);
  // Map the weights.
  ///  itsWeightMap->mapFile(..,..);
  // Fill the time vector.
  itsChunkTimes.resize (nt+1);
  for (int i=0; i<nt; ++i) {
    itsChunkTimes[i] = itsMSDesc.times[itsTimeIndex + st + i] -
                       itsMSDesc.exposures[itsTimeIndex + st + i] / 2;
  }
  itsChunkTimes[nt] = itsMSDesc.times[itsTimeIndex + st + nt-1] +
                      itsMSDesc.exposures[itsTimeIndex + st + nt-1] / 2;
  itsMSMapInfo.setTimes (st, nt);
  itsNrTimesDone += nt;
  if (useFitters) {
    // Determine for each time point to which fitter it belongs.
    const vector<MeqDomain>& localSolveDomains = itsParmData.getDomains();
    itsTimeFitInx.resize (nt);
    uint interv = 0;
    for (int i=0; i<nt; i++) {
      double time = itsMSDesc.times[itsTimeIndex + st + i];
      while (time > localSolveDomains[interv*itsFreqNrFit].endY()) {
    interv++;
    DBGASSERT (interv < localSolveDomains.size()/itsFreqNrFit);
      }
      itsTimeFitInx[i] = interv;
    }
  }
  return true;
}

void Prediffer::processData (bool useFlags, bool preCalc, bool calcDeriv,
                 ProcessFuncBL pfunc,
                 void* arg)
{
  // Map the correct input and output file (if needed).
  mapDataFiles (itsInDataColumn, itsOutDataColumn);
  // Calculate frequency axis info.
  int nrchan = itsLastChan-itsFirstChan+1;
  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  // Add offset for dd (band) and channel.
  unsigned int freqOffset = itsFirstChan*itsNCorr + itsMSMapInfo.ddOffset();
  // Determine if perturbed values have to be calculated.
  int nrpert = calcDeriv ? itsNrPert:0;
  // Loop through the domain of the data to be processed.
  // Process as much data as possible (hopefully everything).
  itsNrTimesDone = 0;
  while (nextDataChunk(nrpert>0)) {
    int nrtimes = itsMSMapInfo.nrTimes();
    // Initialize the ComplexArr pool with the most frequently used size
    // itsNrChan is the number of frequency channels
    MeqMatrixComplexArr::poolDeactivate();
    MeqMatrixRealArr::poolDeactivate();
    MeqMatrixComplexArr::poolActivate(itsNrChan * nrtimes);
    MeqMatrixRealArr::poolActivate(itsNrChan * nrtimes);

    // Size the thread private flag buffers.
    if (useFlags) {
      for (int i=0; i<itsNthread; ++i) {
    itsFlagVecs[i].resize (nrtimes*nrchan*itsNCorr);
      }
    }
    fcomplex* inDataStart  = itsMSMapInfo.inData();
    fcomplex* outDataStart = itsMSMapInfo.outData();
    void* flagStart = itsFlagsMap->getStart();
    int flagStartBit = itsFlagsMap->getStartBit();

    // Create a request.
    MeqDomain domain(startFreq, endFreq, itsChunkTimes[0],
             itsChunkTimes[itsChunkTimes.size()-1]);
    MeqRequest request(domain, nrchan, itsChunkTimes, nrpert);
    request.setFirstX (itsFirstChan);
    // Loop through all baselines.
    //static NSTimer timer("Prediffer::fillFitters", true);
    //timer.start();
    if (preCalc) {
        precalcNodes (request);
    }

    // Loop through all baselines and fill its equations if selected.
#pragma omp parallel
    {
#pragma omp for schedule(dynamic)
      for (uint blindex=0; blindex<itsBLInx.size(); ++blindex) {
    int bl = itsBLInx[blindex];
    // Get pointer to correct data part.
    unsigned int offset = freqOffset + bl*itsNrChan*itsNCorr;
    fcomplex* idata = inDataStart + offset;
    fcomplex* odata = outDataStart + offset;
    // Convert the flag bits to bools.
    ///   if (ant1==8&&ant2==11&&tStep<5) {
      ///cout << "flagmap: start="<<flagStart<<" sbit="<<flagStartBit
      /// << " offs="<<offset<<endl;
      ///     }
#if defined _OPENMP
    int threadNr = omp_get_thread_num();
#else
    int threadNr = 0;
#endif
    // If needed, convert flag bits to bools.
    bool* flags = 0;
    if (useFlags) {
      flags = &itsFlagVecs[threadNr][0];
      for (int i=0; i<itsMSMapInfo.nrTimes(); ++i) {
        bitToBool (flags+i*nrchan*itsNCorr, flagStart, nrchan*itsNCorr,
               offset + flagStartBit);
        offset += itsMSMapInfo.timeSize();
      }
    }
    // Call the given member function.
    (this->*pfunc) (threadNr, arg, idata, odata, flags,
            request, blindex, false);
            ///request, blindex, itsAnt1[bl]==4&&itsAnt2[bl]==8);
      }
    } // end omp parallel
      //timer.stop();
  }
}

void Prediffer::fillFitters (vector<casa::LSQFit>& fitters)
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsExpr);
  // Initialize the fitters.
  int nfitter = itsNrScids.size();
  fitters.resize (nfitter);
  // Create thread private fitters for parallel execution.
  vector<vector<LSQFit*> > threadPrivateFitters(itsNthread);
  for (int i=0; i<itsNthread; ++i) {
    threadPrivateFitters[i].resize (nfitter);
  }
  for (int i=0; i<nfitter; ++i) {
    fitters[i].set (itsNrScids[i]);
    threadPrivateFitters[0][i] = &fitters[i];
    for (int j=1; j<itsNthread; j++) {
      threadPrivateFitters[j][i] = new LSQFit(itsNrScids[i]);
    }
  }
  // Size the thread private buffers.
  ///int nrchan = itsLastChan-itsFirstChan+1;
  for (int i=0; i<itsNthread; ++i) {
    itsResultVecs[i].resize (2*itsNrPert);
    itsDiffVecs[i].resize (3*itsNrPert);
    itsIndexVecs[i].resize (itsNrPert);
    ///    itsOrdFlagVecs[i].resize (2*nrchan);
  }
  // Process the data and use the flags.
  processData (true, true, true,
                            &Prediffer::fillEquation, &threadPrivateFitters);
  // Merge the thread-specific fitters into the main ones.
  for (int j=1; j<itsNthread; ++j) {
    for (int i=0; i<nfitter; ++i) {
      fitters[i].merge (*threadPrivateFitters[j][i]);
      delete threadPrivateFitters[j][i];
    }
  }
  BBSTest::Logger::log(itsPredTimer);
  BBSTest::Logger::log(itsEqTimer);
  itsPredTimer.reset();
  itsEqTimer.reset();
}

void Prediffer::correctData()
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsCorrExpr);
  processData (false, true, false,
           &Prediffer::correctBL, 0);
  ///  if (flush) {
    ///    itsOutDataMap->flush();
    ///  }
}

void Prediffer::subtractData()
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsExpr);
  processData (false, true, false,
           &Prediffer::subtractBL, 0);
  ///  if (flush) {
    ///    itsOutDataMap->flush();
    ///  }
}

void Prediffer::writePredictedData()
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsExpr);
  processData (false, true, false,
           &Prediffer::predictBL, 0);
}

void Prediffer::getData (bool useTree,
             Array<Complex>& dataArr, Array<Bool>& flagArr)
{
  int nrusedbl = itsBLInx.size();
  int nrchan   = itsLastChan-itsFirstChan+1;
  dataArr.resize (IPosition(4, itsNCorr, nrchan, nrusedbl, itsNrTimes));
  flagArr.resize (IPosition(4, itsNCorr, nrchan, nrusedbl, itsNrTimes));
  pair<Complex*,bool*> p;
  p.first = dataArr.data();
  p.second = flagArr.data();
  if (!useTree) {
    processData (true, false, false, &Prediffer::getBL, &p);
    return;
  }
  vector<MeqJonesExpr> expr(nrusedbl);
  for (int i=0; i<nrusedbl; ++i) {
    expr[i] = new MeqJonesMMap (itsMSMapInfo, itsBLInx[i]);
  }
  pair<pair<Complex*,bool*>*, vector<MeqJonesExpr>*> p1;
  p1.first = &p;
  p1.second = &expr;
  processData (true, false, false, &Prediffer::getMapBL, &p1);
}

void Prediffer::fillEquation (int threadnr, void* arg,
                  const fcomplex* data, fcomplex*,
                  const bool* flags,
                  const MeqRequest& request, int blindex,
                  bool showd)
{
  ///int bl = itsBLInx[blindex];
  ///int ant1 = itsAnt1[bl];
  ///int ant2 = itsAnt2[bl];
  //static NSTimer fillEquationTimer("fillEquation", true);
  //fillEquationTimer.start();
  // Get the fitter objects.
  vector<LSQFit*>& fitters =
    (*static_cast<vector<vector<LSQFit*> >*>(arg))[threadnr];
  // Allocate temporary vectors.
  // If needed, they can be pre-allocated per thread, so there is no
  // malloc needed for each invocation. Currently it is believed that
  // the work domains are so large, that the malloc overhead is negligible.
  // ON the other hand, there are many baselines and the malloc is done
  // for each of them.
  uint* indices = &(itsIndexVecs[threadnr][0]);
  const double** pertReal = &(itsResultVecs[threadnr][0]);
  const double** pertImag = pertReal + itsNrPert;
  double* invPert = &(itsDiffVecs[threadnr][0]);
  double* resultr = invPert + itsNrPert;
  double* resulti = resultr + itsNrPert;
  // Get all equations.
  itsPredTimer.start();
  MeqJonesExpr& expr = itsExpr[blindex];
  // Do the actual predict for the entire work domain.
  MeqJonesResult jresult = expr.getResult (request);
  itsPredTimer.stop();

  itsEqTimer.start();
  int nrchan = request.nx();
  int nrtime = request.ny();
  int timeSize = itsMSMapInfo.timeSize();
  // Put the results in a single array for easier handling.
  const MeqResult* predResults[4];
  predResults[0] = &(jresult.getResult11());
  if (itsNCorr == 2) {
    predResults[1] = &(jresult.getResult22());
  } else if (itsNCorr == 4) {
    predResults[1] = &(jresult.getResult12());
    predResults[2] = &(jresult.getResult21());
    predResults[3] = &(jresult.getResult22());
  }

  // Determine start and incr reflecting the order of the observed data.
  int sdch = itsReverseChan ? itsNCorr * (nrchan - 1) : 0;
  int inc  = itsReverseChan ? -itsNCorr : itsNCorr;
  // To avoid having to use large temporary arrays, step through the
  // data by timestep and correlation.
  uint nreq=0;
  for (int corr=0; corr<itsNCorr; corr++, data++, flags++) {
    if (itsCorr[corr]) {
      // Get the results for this correlation.
      const MeqResult& tcres = *predResults[corr];
      
      // Get pointers to the main data.
      const MeqMatrix& val = tcres.getValue();
      const double* realVals;
      const double* imagVals;
      val.dcomplexStorage(realVals, imagVals);

      // Determine which parameters have derivatives and keep that info.
      // E.g. when solving for station parameters, only a few parameters
      // per baseline have derivatives.
      // Note that this is the same for the entire work domain.
      // Also get pointers to the perturbed values.
      int nrParamsFound = 0;
      for (int scinx=0; scinx<itsNrPert; ++scinx) {
    if (tcres.isDefined(scinx)) {
      indices[nrParamsFound] = scinx;
      const MeqMatrix& valp = tcres.getPerturbedValue(scinx);
      valp.dcomplexStorage (pertReal[nrParamsFound],
                pertImag[nrParamsFound]);
      invPert[nrParamsFound] = 1. / tcres.getPerturbation(scinx, 0);
      nrParamsFound++;
    }
      }

      if (nrParamsFound > 0) {
    const fcomplex* cdata  = data;
    const bool*     cflags = flags;
    if (fitters.size() == 1) {
      // No fitter indexing needed if only one fitter.
      for (int tim=0; tim<nrtime; tim++) {
        // Loop through all channels.
        if (showd) {
          showData (corr, sdch, inc, nrchan, cflags, cdata,
            realVals, imagVals);
        }
        // Form two equations for each unflagged data point.
        int dch = sdch;
        for (int ch=0; ch<nrchan; ++ch, dch+=inc) {
          if (! cflags[dch]) {
        double diffr = real(cdata[dch]) - *realVals;
        double diffi = imag(cdata[dch]) - *imagVals;
        for (int scinx=0; scinx<nrParamsFound; ++scinx) {
          // Calculate the derivative for real and imag part.
          double invp = invPert[scinx];
          resultr[scinx] = (*pertReal[scinx]++ - *realVals) * invp;
          resulti[scinx] = (*pertImag[scinx]++ - *imagVals) * invp;
        }
        // Now add the equations to the correct fitter object.
        if (nrParamsFound != itsNrPert) {
          fitters[0]->makeNorm (nrParamsFound, indices,
                    resultr, 1., diffr);
          fitters[0]->makeNorm (nrParamsFound, indices,
                    resulti, 1., diffi);
        } else {
          fitters[0]->makeNorm (resultr, 1., diffr);
          fitters[0]->makeNorm (resulti, 1., diffi);
        }
        if (showd) {
          cout << "eq"<<corr<<" ch " << 2*ch << " diff " << diffr;
          for (int ii=0; ii<nrParamsFound; ii++) {
            cout << ' '<<resultr[ii];
          }
          cout << endl;
          cout << "eq"<<corr<<" ch " << 2*ch+1 << " diff " << diffi;
          for (int ii=0; ii<nrParamsFound; ii++) {
            cout << ' '<<resulti[ii];
          }
          cout << endl;
        }
        nreq += 2;
          } else {
        for (int scinx=0; scinx<nrParamsFound; ++scinx) {
          pertReal[scinx]++;
          pertImag[scinx]++;
        }
          }
          realVals++;
          imagVals++;
        }
        cdata  += timeSize;       // next observed data time step
        cflags += nrchan*itsNCorr;
      }
    } else {
      // Multiple fitters, so for each point the correct fitter
      // has to be determined.
      for (int tim=0; tim<nrtime; tim++) {
        // Loop through all channels.
        if (showd) {
          showData (corr, sdch, inc, nrchan, cflags, cdata,
            realVals, imagVals);
        }
        // Get first fitter index for this time line.
        int fitInxT = itsTimeFitInx[tim] * itsFreqNrFit;
        // Form two equations for each unflagged data point.
        int dch = sdch;
        for (int ch=0; ch<nrchan; ++ch, dch+=inc) {
          if (! cflags[dch]) {
        int fitInx = fitInxT + itsFreqFitInx[ch];
        double diffr = real(cdata[dch]) - *realVals;
        double diffi = imag(cdata[dch]) - *imagVals;
        for (int scinx=0; scinx<nrParamsFound; ++scinx) {
          // Calculate the derivative for real and imag part.
          double invp = 1. / tcres.getPerturbation(indices[scinx],
                               fitInx);
          resultr[scinx] = (*pertReal[scinx]++ - *realVals) * invp;
          resulti[scinx] = (*pertImag[scinx]++ - *imagVals) * invp;
        }
        // Now add the equations to the correct fitter object.
        if (nrParamsFound != itsNrPert) {
          fitters[fitInx]->makeNorm (nrParamsFound, indices,
                         resultr, 1., diffr);
          fitters[fitInx]->makeNorm (nrParamsFound, indices,
                         resulti, 1., diffi);
        } else {
          fitters[fitInx]->makeNorm (resultr, 1., diffr);
          fitters[fitInx]->makeNorm (resulti, 1., diffi);
        }
        if (showd) {
          cout << "eq"<<corr<<" ch " << 2*ch << " diff " << diffr;
          for (int ii=0; ii<nrParamsFound; ii++) {
            cout << ' '<<resultr[ii];
          }
          cout << endl;
          cout << "eq"<<corr<<" ch " << 2*ch+1 << " diff " << diffi;
          for (int ii=0; ii<nrParamsFound; ii++) {
            cout << ' '<<resulti[ii];
          }
          cout << endl;
        }
        nreq += 2;
          } else {
        for (int scinx=0; scinx<nrParamsFound; ++scinx) {
          pertReal[scinx]++;
          pertImag[scinx]++;
        }
          }
          realVals++;
          imagVals++;
        }
        cdata  += timeSize;       // next observed data time step
        cflags += nrchan*itsNCorr;
      }
    }
      }
    }
  }
  //fillEquationTimer.stop();
  itsEqTimer.stop();
  ///  cout << "nreq="<<nreq<<endl;
}

vector<MeqResult> Prediffer::getResults (bool calcDeriv)
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
      request = MeqRequest(domain, nrchan, 1, itsNrPert);
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
    for (uint blindex=0; blindex<itsBLInx.size(); ++blindex) {
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

void Prediffer::getBL (int, void* arg,
               const fcomplex* data, fcomplex*,
               const bool* flags,
               const MeqRequest&, int blindex,
               bool)
{
  Complex* datap = static_cast<pair<Complex*,bool*>*>(arg)->first;
  bool*    flagp = static_cast<pair<Complex*,bool*>*>(arg)->second;
  int nrchan = itsLastChan-itsFirstChan+1;
  memcpy (datap+blindex*nrchan*itsNCorr, data,
      nrchan*itsNCorr*sizeof(Complex));
  memcpy (flagp+blindex*nrchan*itsNCorr, flags,
      nrchan*itsNCorr*sizeof(bool));
}

void Prediffer::getMapBL (int, void* arg,
              const fcomplex*, fcomplex*,
              const bool* flags,
              const MeqRequest& request, int blindex,
              bool)
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

void Prediffer::subtractBL (int, void*,
                const fcomplex* dataIn, fcomplex* dataOut,
                const bool*,
                const MeqRequest& request, int blindex,
                bool)
{
  itsPredTimer.start();
  MeqJonesExpr& expr = itsExpr[blindex];
  // Do the actual predict.
  MeqJonesResult jresult = expr.getResult (request);
  itsPredTimer.stop();

  itsEqTimer.start();
  int nrchan = request.nx();
  int nrtime = request.ny();
  int timeSize = itsMSMapInfo.timeSize();
  const double* predReal[4];
  const double* predImag[4];
  jresult.getResult11().getValue().dcomplexStorage(predReal[0], predImag[0]);
  if (itsNCorr == 2) {
    jresult.getResult22().getValue().dcomplexStorage(predReal[1], predImag[1]);
  } else if (itsNCorr == 4) {
    jresult.getResult12().getValue().dcomplexStorage(predReal[1], predImag[1]);
    jresult.getResult21().getValue().dcomplexStorage(predReal[2], predImag[2]);
    jresult.getResult22().getValue().dcomplexStorage(predReal[3], predImag[3]);
  }
  // Loop through the times and correlations.
  for (int tim=0; tim<nrtime; tim++) {
    const fcomplex* idata = dataIn;
    fcomplex* odata = dataOut;
    for (int corr=0; corr<itsNCorr; corr++, idata++, odata++) {
      if (itsCorr[corr]) {
    const double* realVals = predReal[corr];
    const double* imagVals = predImag[corr];
    // Subtract predicted from the data.
    int dch = itsReverseChan ? itsNCorr * (nrchan - 1) : 0;
    int inc = itsReverseChan ? -itsNCorr : itsNCorr;
    if (idata == odata) {
      for (int ch=0; ch<nrchan; ch++, dch+=inc) {
        odata[dch] -= makefcomplex(realVals[ch], imagVals[ch]);
      }
    } else {
      for (int ch=0; ch<nrchan; ch++, dch+=inc) {
        odata[dch] = idata[dch] - makefcomplex(realVals[ch],
                           imagVals[ch]);
      }
    }
    predReal[corr] += nrchan;
    predImag[corr] += nrchan;
      }
    }
    dataIn  += timeSize;
    dataOut += timeSize;
  }
  itsEqTimer.stop();
}

void Prediffer::correctBL (int, void*,
               const fcomplex*, fcomplex*,
               const bool*,
               const MeqRequest& request, int blindex,
               bool)
{
  itsPredTimer.start();
  MeqJonesResult res = itsCorrExpr[blindex].getResult (request);
  itsPredTimer.stop();
  itsEqTimer.start();
  itsCorrMMap[blindex]->putJResult (res, request);
  itsEqTimer.stop();
}

void Prediffer::predictBL (int, void*,
               const fcomplex*, fcomplex* dataOut,
               const bool*,
               const MeqRequest& request, int blindex,
               bool)
{
  itsPredTimer.start();
  MeqJonesExpr& expr = itsExpr[blindex];
  // Do the actual predict.
  MeqJonesResult jresult = expr.getResult (request);
  itsPredTimer.stop();

  itsEqTimer.start();
  int nrchan = request.nx();
  int nrtime = request.ny();
  int timeSize = itsMSMapInfo.timeSize();
  // Put the results in a single array for easier handling.
  const double* predReal[4];
  const double* predImag[4];
  jresult.getResult11().getValue().dcomplexStorage(predReal[0], predImag[0]);
  if (itsNCorr == 2) {
    jresult.getResult22().getValue().dcomplexStorage(predReal[1], predImag[1]);
  } else if (itsNCorr == 4) {
    jresult.getResult12().getValue().dcomplexStorage(predReal[1], predImag[1]);
    jresult.getResult21().getValue().dcomplexStorage(predReal[2], predImag[2]);
    jresult.getResult22().getValue().dcomplexStorage(predReal[3], predImag[3]);
  }
  
  // Loop through the times and correlations.
  for (int tim=0; tim<nrtime; tim++) {
    fcomplex* data = dataOut;
    for (int corr=0; corr<itsNCorr; corr++, data++) {
      if (itsCorr[corr]) {
    const double* realVals = predReal[corr];
    const double* imagVals = predImag[corr];
    // Store the predict in data.
    int dch = itsReverseChan ? itsNCorr * (nrchan - 1) : 0;
    int inc = itsReverseChan ? -itsNCorr : itsNCorr;
    for (int ch=0; ch<nrchan; ch++, dch+=inc) {
      data[dch] = makefcomplex(realVals[ch], imagVals[ch]);
    }
    predReal[corr] += nrchan;
    predImag[corr] += nrchan;
      }
    }
    dataOut += timeSize;
  }
  itsEqTimer.stop();
}


void Prediffer::fillUVW()
{
  // Create the UVW nodes.
  int nrstat = itsStations.size();
  itsStatUVW.reserve (nrstat);
  for (int i=0; i<nrstat; ++i) {
    MeqStatUVW* uvw = 0;
    // Do it only if the station is actually used.
    if (itsStations[i] != 0) {
      // Expression to calculate UVW per station
      uvw = new MeqStatUVW(itsStations[i], pair<double, double>(itsMSDesc.ra, itsMSDesc.dec), itsMSDesc.arrayPos);
    }
    itsStatUVW.push_back (uvw);
  }
  if (itsCalcUVW) {
    return;
  }
  // Fill the UVW objects with the uvw-s from the MS.
  LOG_TRACE_RTTI( "get UVW coordinates from MS" );
  int nant = itsStatUVW.size();
  vector<bool> statFnd (nant);
  vector<bool> statDone (nant);
  vector<double> statuvw(3*nant);

  // Determine the number of stations (found)
  statFnd.assign (statFnd.size(), false);
  int nStatFnd = 0;
  for (unsigned int bl=0; bl<itsNrBl; bl++) {
    int a1 = itsMSDesc.ant1[bl];
    int a2 = itsMSDesc.ant2[bl];
    if (!statFnd[a1]) {
      nStatFnd++;
      statFnd[a1] = true;
    }
    if (!statFnd[a2]) {
      nStatFnd++;
      statFnd[a2] = true;
    }
  }
  // Map uvw data into memory
  size_t nrBytes = itsMSDesc.times.size() * itsNrBl * 3 * sizeof(double);
  double* uvwDataPtr = 0;
  string UVWFile = getFileForColumn(MS::columnName(MS::UVW));
  LOG_INFO_STR("Input column " << MS::columnName(MS::UVW) << " maps to: " << UVWFile);
  MMap* mapPtr = new MMap(UVWFile, MMap::Read);
  mapPtr->mapFile(0, nrBytes);
  uvwDataPtr = (double*)mapPtr->getStart();

  // Step time by time through the MS.
  for (unsigned int tStep=0; tStep < itsMSDesc.times.size(); tStep++) {
    // Set uvw pointer to beginning of this time
    unsigned int tOffset = tStep * itsNrBl * 3;
    double* uvw = uvwDataPtr + tOffset;
    double time = itsMSDesc.times[tStep];

    // Set UVW of first station used to 0 (UVW coordinates are relative!).
    statDone.assign (statDone.size(), false);
    int ant0 = itsMSDesc.ant1[0];
    statuvw[3*ant0]   = 0;
    statuvw[3*ant0+1] = 0;
    statuvw[3*ant0+2] = 0;
    statDone[ant0] = true;
    itsStatUVW[ant0]->set (time, 0, 0, 0);

//     cout << "itsStatUVW[" << itsAnt1Data[0] << "] time: " << time << " 0, 0, 0" << endl;

    int ndone = 1;
    // Loop until all found stations are handled. This is necessary when not all
    // stations can be calculated in one loop (depends on the order)
    while (ndone < nStatFnd) {
      int nd = 0;
      // Loop over baselines
      for (unsigned int bl=0; bl<itsNrBl; bl++) {
    int a1 = itsMSDesc.ant1[bl];
    int a2 = itsMSDesc.ant2[bl];
    if (!statDone[a2]) {
      if (statDone[a1]) {
        statuvw[3*a2]   = uvw[3*bl]   - statuvw[3*a1];
        statuvw[3*a2+1] = uvw[3*bl+1] - statuvw[3*a1+1];
        statuvw[3*a2+2] = uvw[3*bl+2] - statuvw[3*a1+2];
        statDone[a2] = true;
        itsStatUVW[a2]->set (time, statuvw[3*a2], statuvw[3*a2+1],
                 statuvw[3*a2+2]);

//        cout << "itsStatUVW[" << a2 << "] time: " << time << statuvw[3*a2] << " ," << statuvw[3*a2+1] << " ," << statuvw[3*a2+2] << endl;

        ndone++;
        nd++;
      }
    } else if (!statDone[a1]) {
      if (statDone[a2]) {
        statuvw[3*a1]   = statuvw[3*a2]   - uvw[3*bl];
        statuvw[3*a1+1] = statuvw[3*a2+1] - uvw[3*bl+1];
        statuvw[3*a1+2] = statuvw[3*a2+2] - uvw[3*bl+2];
        statDone[a1] = true;
        itsStatUVW[a1]->set (time, statuvw[3*a1], statuvw[3*a1+1],
                   statuvw[3*a1+2]);
//        cout << "itsStatUVW[" << a1 << "] time: " << time << statuvw[3*a1] << " ," << statuvw[3*a1+1] << " ," << statuvw[3*a1+2] << endl;

        ndone++;
        nd++;
      }
    }
    if (ndone == nStatFnd) {
      break;
    }
      } // End loop baselines
      //      ASSERT (nd > 0);
    } // End loop stations found
  } // End loop time

  // Finished with map
  delete mapPtr;
}

void Prediffer::updateSolvableParms (const vector<double>& values)
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
  resetEqLoop();
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
  resetEqLoop();
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
  resetEqLoop();
}

void Prediffer::resetEqLoop()
{
}

void Prediffer::writeParms()
{
  BBSTest::ScopedTimer saveTimer("P:write-parm");
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
  cout << "wrote timeIndex=" << itsTimeIndex
       << " nrTimes=" << itsNrTimes << endl;
}

#ifdef EXPR_GRAPH
void Prediffer::writeExpressionGraph(const string &fileName, int baselineIndex)
{
    ASSERT(baselineIndex >= 0 && baselineIndex < itsExpr.size());

    ofstream os(fileName.c_str());
    os << "digraph MeasurementEquation" << endl;
    os << "{" << endl;
    os << "node [shape=box];" << endl;
    itsExpr[baselineIndex].writeExpressionGraph(os);
    os << "}" << endl;
}
#endif

void Prediffer::showSettings() const
{
  cout << "Prediffer settings:" << endl;
  cout << "  msname:    " << itsMSName << endl;
  cout << "  mepname:   " << itsMEPName << endl;
  cout << "  gsmname:   " << itsGSMMEPName << endl;
  cout << "  solvparms: " << itsParmData << endl;
  if (itsReverseChan) {
    cout << "  stchan:    " << itsNrChan - 1 - itsFirstChan << endl;
    cout << "  endchan:   " << itsNrChan - 1 - itsLastChan << endl;
    cout << "    Note: channels are in reversed order" << endl;
  } else {
    cout << "  stchan:    " << itsFirstChan << endl;
    cout << "  endchan:   " << itsLastChan << endl;
  }
  cout << "  corr     : " << itsNCorr << "  " << itsCorr[0];
  for (int i=1; i<itsNCorr; ++i) {
    cout << ',' << itsCorr[i];
  }
  cout << endl;
  cout << "  calcuvw  : " << itsCalcUVW << endl;
  cout << endl;
}

void Prediffer::showData (int corr, int sdch, int inc, int nrchan,
              const bool* flags, const fcomplex* data,
              const double* realVals, const double* imagVals)
{
  cout << "flag=" << corr << "x ";
  int dch = sdch;
  for (int ch=0; ch<nrchan; ch++, dch+=inc) {
    cout << flags[dch]<< ' ';
  }
  cout << endl;
  cout << "cor=" << corr << "x ";
  dch = sdch;
  for (int ch=0; ch<nrchan; ch++, dch+=inc) {
    cout << '(' << setprecision(12)
     << real(data[dch])
     << ',' << setprecision(12)
     << imag(data[dch])<< ')';
  }
  cout << endl;
  cout << "corr=" << corr << "x ";
  for (int ch=0; ch<nrchan; ch++) {
    cout << '(' << setprecision(12)
     << realVals[ch]
     << ',' << setprecision(12)
     << imagVals[ch]<< ')';
  }
  cout << endl;
}


// DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED
// DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED

Prediffer::Prediffer(const string& msName,
             const LOFAR::ParmDB::ParmDBMeta& meqPdm,
             const LOFAR::ParmDB::ParmDBMeta& skyPdm,
             uint ddid,
             bool calcUVW)
: itsCalcUVW      (calcUVW),
  itsMEPName      (meqPdm.getTableName()),
  itsGSMMEPName   (skyPdm.getTableName()),
  itsSources      (0),
  itsNrPert       (0),
  itsNCorr        (0),
  itsNrBl         (0),
  itsTimeIndex    (0),
  itsNrTimes      (0),
  itsNrTimesDone  (0),
  itsInDataMap    (0),
  itsOutDataMap   (0),
  itsFlagsMap     (0),
  itsWeightMap    (0),
  itsIsWeightSpec (false),
  itsPredTimer    ("P:predict", false),
  itsEqTimer      ("P:eq|save", false)
{
  // Get absolute path name for MS.
  itsMSName = Path(msName).absoluteName();
  LOG_INFO_STR( "Prediffer constructor ("
        << "'" << itsMSName   << "', "
        << "'" << meqPdm.getTableName() << "', "
        << "'" << skyPdm.getTableName() << "', "
        << itsCalcUVW << ")" );
  // Read the meta data and map the flags file.
  //readDescriptiveData(msName);
  readMeasurementSetMetaData(itsMSName);
  processMSDesc(ddid);
    
  itsGSMMEP = new LOFAR::ParmDB::ParmDB(skyPdm);
  itsMEP = new LOFAR::ParmDB::ParmDB(meqPdm);

  itsFlagsMap = new FlagsMap(getFileForColumn(MS::columnName(MS::FLAG)), MMap::Read);
  // Get all sources from the ParmDB.
  itsSources = new MeqSourceList(*itsGSMMEP, itsParmGroup);
  // Create the UVW nodes and fill them with uvw-s from MS if not calculated.
  fillUVW();
  // Allocate thread private buffers.
#if defined _OPENMP
  itsNthread = omp_get_max_threads();
#else
  itsNthread = 1;
#endif
  itsFlagVecs.resize (itsNthread);
  itsResultVecs.resize (itsNthread);
  itsDiffVecs.resize (itsNthread);
  itsIndexVecs.resize (itsNthread);
}

bool Prediffer::setStrategyProp (const StrategyProp& strat)
{
  itsInDataColumn = strat.getInColumn();
  // Initially use all correlations.
  for (int i=0; i<itsNCorr; ++i) {
    itsCorr[i] = true;
  }
  strat.expandPatterns (itsMSDesc.antNames);
  return selectStations (strat.getAntennas(),         // Available antennas
             strat.getAutoCorr());
}

bool Prediffer::setStepProp (const StepProp& stepProp)
{
  itsOutDataColumn = stepProp.getOutColumn();
  stepProp.expandPatterns (itsMSDesc.antNames);
  if (!selectStep (stepProp.getAnt1(), stepProp.getAnt2(),
           stepProp.getAutoCorr(),
           stepProp.getCorr())) {
    return false;
  }
  // If no sources given, use all sources.
  if (stepProp.getSources().empty()) {
    makeTree (stepProp.getModel(), itsSources->getSourceNames());
  } else {
    makeTree (stepProp.getModel(), stepProp.getSources());
  }
  // Put funklets in parms which are not filled yet.
  for (MeqParmGroup::iterator iter = itsParmGroup.begin();
       iter != itsParmGroup.end();
       ++iter)
  {
    iter->second.fillFunklets (itsParmValues, itsWorkDomain);
  }
  return true;
}

bool Prediffer::setSolveProp (const SolveProp& solveProp)
{
  LOG_INFO_STR( "setSolveProp");
  const vector<string>& parms = solveProp.getParmPatterns();
  const vector<string>& excludePatterns = solveProp.getExclPatterns();
  // Convert patterns to regexes.
  vector<Regex> parmRegex;
  for (unsigned int i=0; i<parms.size(); i++) {
    parmRegex.push_back (Regex::fromPattern(parms[i]));
  }
  vector<Regex> excludeRegex;
  for (unsigned int i=0; i<excludePatterns.size(); i++) {
    excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
  }
  // Find all parms matching the parms.
  // Exclude them if matching an excludePattern
  int nrsolv = 0;
  for (MeqParmGroup::iterator iter = itsParmGroup.begin();
       iter != itsParmGroup.end();
       ++iter)
  {
    String parmName (iter->second.getName());
    // Loop through all regex-es until a match is found.
    for (vector<Regex>::iterator incIter = parmRegex.begin();
     incIter != parmRegex.end();
     incIter++) {
      if (parmName.matches(*incIter)) {
    bool parmExc = false;
    // Test if not excluded.
    for (vector<Regex>::const_iterator excIter = excludeRegex.begin();
         excIter != excludeRegex.end();
         excIter++) {
      if (parmName.matches(*excIter)) {
        parmExc = true;
        break;
      }
    }
    if (!parmExc) {
      LOG_TRACE_OBJ_STR( "setSolvable: " << iter->second.getName());
      iter->second.setSolvable (true);
      nrsolv++;
    }
    break;
      }
    }
  }
  if (nrsolv == 0) {
    return false;
  }
  initSolvableParms (solveProp.getDomains());
  return itsNrPert>0;
}

void Prediffer::readDescriptiveData (const string& fileName)
{
  ASSERTSTR(false, "DEPRECATED -- will be removed in next release. Use Prediffer::readMeasurementSetMetaData instead.");

  // Get meta data from description file.
  string name(fileName+"/vis.des");
  ifstream istr(name.c_str());
  ASSERTSTR (istr, "File " << fileName << "/vis.des could not be opened");
  BlobIBufStream bbs(istr);
  BlobIStream bis(bbs);
  bis >> itsMSDesc;
}

bool Prediffer::selectStations (const vector<int>& antnrs, bool useAutoCorr)
{
  int nrant = itsStations.size();
  // Get all stations actually used.
  // Default is all stations.
  if (antnrs.empty()) {
    itsSelStations = itsStations;
  } else {
    // Set given stations.
    itsSelStations = vector<MeqStation*>(nrant, (MeqStation*)0);
    for (uint i=0; i<antnrs.size(); i++) {
      int ant = antnrs[i];
      if (ant >= 0  &&  ant < nrant) {
    itsSelStations[ant] = itsStations[ant];
      }
    }
  }
  // Fill a a matrix telling for each antenna pair where contained in the MS.
  // First initialize to not contained.
  itsBLSel.resize (nrant, nrant);
  itsBLSel = false;
  // Set if selected for each baseline in the MS.
  int nr = 0;
  for (uint i=0; i<itsMSDesc.ant1.size(); ++i) {
    int a1 = itsMSDesc.ant1[i];
    int a2 = itsMSDesc.ant2[i];
    if (itsSelStations[a1] && itsSelStations[a2]) {
      if (useAutoCorr  ||  a1 != a2) {
    itsBLSel(a1,a2) = true;
    nr++;
      }
    }
  }
  return nr>0;
}

bool Prediffer::selectStep (const vector<int>& ant1,
                const vector<int>& ant2,
                bool useAutoCorrelations,
                const vector<bool>& corr)
{
  int nrant = itsBLSel.nrow();
  ASSERT (ant1.size() == ant2.size());
  Matrix<bool> blSel;
  if (ant1.size() == 0) {
    // No baselines specified, select all baselines in strategy.
    blSel = itsBLSel;
  } else {
    blSel.resize (nrant, nrant);
    blSel = false;
    for (uint i=0; i<ant1.size(); i++) {
      int a1 = ant1[i];
      int a2 = ant2[i];
      if (a1 < nrant  &&  a2 < nrant) {
    blSel(a1, a2) = itsBLSel(a1, a2);
      }
    }
  }
  // Unset auto-correlations if needed.
  if (!useAutoCorrelations) {
    for (int i=0; i<nrant; i++) {
      blSel(i,i) = false;
    }
  }
  // Fill the correlations to use. Use all if vector is empty.
  for (int i=0; i<itsNCorr; ++i) {
    itsCorr[i] = corr.empty();
  }
  if (corr.size() > 0) {
    // Some values given; first one is always XX (or LL, etc.)
    itsCorr[0] = corr[0];
    if (corr.size() > 1  &&  itsNCorr > 1) {
      if (corr.size() == 2) {
    // Two values given; last one is always YY (or RR, etc.)
    itsCorr[itsNCorr-1] = corr[1];
      } else {
    // More than 2 must be 4 values.
    ASSERTSTR (corr.size()==4,
           "Correlation selection vector must have 1, 2 or 4 flags");
    if (itsNCorr == 2) {
      itsCorr[1] = corr[3];
    } else {
      itsCorr[1] = corr[1];
      itsCorr[2] = corr[2];
      itsCorr[3] = corr[3];
    }
      }
    }
  }
  
  cout << blSel << endl;
  
  return fillBaseCorr (blSel);
}

bool Prediffer::fillBaseCorr (const Matrix<bool>& blSel)
{
  // Count the nr of baselines actually used for this step.
  // Store the seqnr of all selected baselines.
  itsBLInx.clear();
  for (uint i=0; i<itsMSDesc.ant1.size(); ++i) {
    int a1 = itsMSDesc.ant1[i];
    int a2 = itsMSDesc.ant2[i];
    if (blSel(a1,a2)) {
      itsBLInx.push_back (i);
    }
  }
  // Count the nr of selecteed correlations.
  int nSelCorr = 0;
  for (int i=0; i<itsNCorr; ++i) {
    if (itsCorr[i]) {
      nSelCorr++;
    }
  }
  return (itsBLInx.size() > 0  &&  nSelCorr > 0);
}

// DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED
// DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED



} // namespace BBS
} // namespace LOFAR
