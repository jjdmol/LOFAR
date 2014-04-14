//# MSWriterDAL.cc: an implementation of MSWriter using the DAL to write HDF5
//# Copyright (C) 2011-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#ifdef HAVE_DAL

#include "MSWriterDAL.h"

#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#ifdef basename // some glibc have this as a macro
#undef basename
#endif
#include <Common/SystemUtil.h>
#include <Common/StreamUtil.h>
#include <Common/Thread/Mutex.h>
#include <CoInterface/StreamableData.h>
#include <OutputProc/Package__Version.h>

#include <dal/lofar/BF_File.h>
#include <dal/dal_version.h>


using namespace std;
using namespace dal;
using boost::format;

static string timeStr( double time )
{
  time_t timeSec = static_cast<time_t>(floor(time));
  unsigned long timeNSec = static_cast<unsigned long>(round( (time - floor(time)) * 1e9 ));

  char utcstr[50];
  if (strftime( utcstr, sizeof utcstr, "%Y-%m-%dT%H:%M:%S", gmtime(&timeSec) ) == 0)
    return "";

  return str(format("%s.%09lu") % utcstr % timeNSec);
}

static string toUTC( double time )
{
  return timeStr(time) + "Z";
}

static double toMJD( double time )
{
  // 40587 modify Julian day number = 00:00:00 January 1, 1970, GMT
  return 40587.0 + time / (24 * 60 * 60);
}

static string stripextension( const string pathname )
{
  size_t endPos = pathname.rfind('.');
  if (endPos + 1 < pathname.size() && pathname[endPos + 1] == '/')
    endPos = string::npos; // don't recognize an extension in ./foo
  return pathname.substr(0, endPos);
}

static string forceextension( const string pathname, const string extension )
{
  return stripextension(pathname) + extension;
}

namespace LOFAR
{

  namespace Cobalt
  {
    // Prevent concurrent access to HDF5, which may not be compiled thread-safe. The Thread-safe version
    // uses global locks too anyway.
    static Mutex HDF5Mutex;

    template <typename T,unsigned DIM>
    MSWriterDAL<T,DIM>::MSWriterDAL (const string &filename,
     const Parset &parset,
     unsigned fileno)
      :
      MSWriterFile(forceextension(string(filename),".raw")),
      itsParset(parset),
      itsNextSeqNr(0),
      itsFileNr(fileno)
    {
      itsNrExpectedBlocks = itsParset.nrBeamFormedBlocks();

      string h5filename = forceextension(string(filename),".h5");
      string rawfilename = forceextension(string(filename),".raw");

      ScopedLock sl(HDF5Mutex);

#if 0
      // install our own error handler
      H5Eset_auto_stack(H5E_DEFAULT, my_hdf5_error_handler, NULL);
#endif

      const struct ObservationSettings::BeamFormer::File &f = parset.settings.beamFormer.files[fileno];

      const unsigned sapNr = f.sapNr;
      const unsigned beamNr = f.tabNr;
      const unsigned stokesNr = f.stokesNr;

      const struct ObservationSettings::BeamFormer::StokesSettings &stokesSet =
        f.coherent ? parset.settings.beamFormer.coherentSettings
                   : parset.settings.beamFormer.incoherentSettings;

      //*******************************

      // All subbands in the SAP that we (partly or fully) store in this file.
      // We could have multiple SAPs and/or have split up the subbands over multiple files (parts).
      vector<unsigned> subbandIndices = parset.settings.SAPs[sapNr].subbandIndices();

      unsigned firstSubbandIdx = f.firstSubbandIdx;
      unsigned nrSubbands = f.lastSubbandIdx - f.firstSubbandIdx;

      itsNrChannels = stokesSet.nrChannels * nrSubbands; 
      itsNrSamples = parset.settings.nrSamplesPerSubband() /
                     stokesSet.nrChannels / stokesSet.timeIntegrationFactor;

      itsBlockSize = itsNrSamples * itsNrChannels;

      unsigned nrBlocks = parset.nrBeamFormedBlocks();
      
      //*******************************

      vector<string> stokesVars;
      vector<string> stokesVars_LTA;


      switch (stokesSet.type) {
      case STOKES_I:
        stokesVars.push_back("I");
        stokesVars_LTA = stokesVars;
        break;

      case STOKES_IQUV:
        stokesVars.push_back("I");
        stokesVars.push_back("Q");
        stokesVars.push_back("U");
        stokesVars.push_back("V");
        stokesVars_LTA = stokesVars;
        break;

      case STOKES_XXYY:
        stokesVars.push_back("Xr");
        stokesVars.push_back("Xi");
        stokesVars.push_back("Yr");
        stokesVars.push_back("Yi");
        stokesVars_LTA.push_back("Xre");
        stokesVars_LTA.push_back("Xim");
        stokesVars_LTA.push_back("Yre");
        stokesVars_LTA.push_back("Yim");
        break;

      case INVALID_STOKES:
        LOG_ERROR("MSWriterDAL asked to write INVALID_STOKES");
        return;
      }

      LOG_DEBUG_STR("MSWriterDAL: opening " << filename);

      // create the top structure
      BF_File file(h5filename, BF_File::CREATE);

      // Common Attributes
      file.groupType().value = "Root";
      //file.fileName() is set by DAL
      //file.fileDate() is set by DAL

      //file.fileType() is set by DAL
      //file.telescope() is set by DAL

      file.projectID().value = parset.getString("Observation.Campaign.name", "");
      file.projectTitle().value = parset.getString("Observation.Scheduler.taskName", "");
      file.projectPI().value = parset.getString("Observation.Campaign.PI", "");
      ostringstream oss;
      // Use ';' instead of ',' to pretty print, because ',' already occurs in names (e.g. Smith, J.).
      writeVector(oss, parset.getStringVector("Observation.Campaign.CO_I", vector<string>(0), true), "; ", "", "");
      file.projectCOI().value = oss.str();
      file.projectContact().value = parset.getString("Observation.Campaign.contact", "");

      file.observationID().value = str(format("%u") % parset.observationID());

      file.observationStartUTC().value = toUTC(parset.settings.startTime);
      file.observationStartMJD().value = toMJD(parset.settings.startTime);

      // The stop time can be a bit further than the one actually specified, because we process in blocks.
      double stopTime = parset.settings.startTime + nrBlocks * parset.settings.blockDuration();

      file.observationEndUTC().value = toUTC(stopTime);
      file.observationEndMJD().value = toMJD(stopTime);

      file.observationNofStations().value = parset.nrStations(); // TODO: SS beamformer?
      file.observationStationsList().value = parset.allStationNames(); // TODO: SS beamformer?

      double subbandBandwidth = parset.subbandBandwidth();
      double channelBandwidth = subbandBandwidth / stokesSet.nrChannels;

      // if PPF is used, the frequencies are shifted down by half a channel
      // We'll annotate channel 0 to be below channel 1, but in reality it will
      // contain frequencies from both the top and the bottom half-channel.
      double frequencyOffsetPPF = stokesSet.nrChannels > 1 ? 0.5 * channelBandwidth : 0.0; // TODO: cover both CS and IS!

      // For the whole obs, regardless which SAP and subbands (parts) this file contains.
      vector<double> subbandCenterFrequencies(parset.nrSubbands());
      for(size_t sb = 0; sb < parset.nrSubbands(); ++sb)
        subbandCenterFrequencies[sb] = parset.settings.subbands[sb].centralFrequency;

      double min_centerfrequency = *min_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
      double max_centerfrequency = *max_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
      double sum_centerfrequencies = accumulate( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end(), 0.0 );

      file.observationFrequencyMax().value = (max_centerfrequency + subbandBandwidth / 2 - frequencyOffsetPPF) / 1e6;
      file.observationFrequencyMin().value = (min_centerfrequency - subbandBandwidth / 2 - frequencyOffsetPPF) / 1e6;
      file.observationFrequencyCenter().value = (sum_centerfrequencies / subbandCenterFrequencies.size() - frequencyOffsetPPF) / 1e6;
      file.observationFrequencyUnit().value = "MHz";

      file.observationNofBitsPerSample().value = parset.settings.nrBitsPerSample;
      file.clockFrequency().value = parset.settings.clockMHz;
      file.clockFrequencyUnit().value = "MHz";

      file.antennaSet().value = parset.settings.antennaSet;
      file.filterSelection().value = parset.settings.bandFilter;

      size_t nrSAPs = parset.settings.SAPs.size();
      vector<string> targets(nrSAPs);

      for (size_t sap = 0; sap < nrSAPs; sap++)
        targets[sap] = parset.settings.SAPs[sap].target;

      file.targets().value = targets;

      file.systemVersion().value = OutputProcVersion::getVersion();   // LOFAR version

      //file.docName() is set by DAL
      //file.docVersion() is set by DAL

      file.notes().value = "";

      // BF_File specific root group parameters

      file.createOfflineOnline().value = parset.settings.realTime ? "Online" : "Offline";
      file.BFFormat().value = "TAB";
      file.BFVersion().value = str(format("Cobalt/OutputProc %s r%s using DAL %s and HDF5 %s") % OutputProcVersion::getVersion() % OutputProcVersion::getRevision() % dal::version().to_string() % dal::version_hdf5().to_string());

      file.totalIntegrationTime().value = nrBlocks * parset.settings.blockDuration();
      file.totalIntegrationTimeUnit().value = "s";

      //file.subArrayPointingDiameter().value = 0.0;
      //file.subArrayPointingDiameterUnit().value = "arcmin";
      file.bandwidth().value = parset.settings.subbands.size() * subbandBandwidth / 1e6;
      file.bandwidthUnit().value = "MHz";
      //file.beamDiameter()            .value = 0.0;
      //file.beamDiameterUnit()          .value = "arcmin";

      file.observationNofSubArrayPointings().value = parset.settings.SAPs.size();
      file.nofSubArrayPointings().value = 1;

      // SysLog group -- empty for now
      file.sysLog().create();

      // information about the station beam (SAP)
      BF_SubArrayPointing sap = file.subArrayPointing(sapNr);
      sap.create();
      sap.groupType().value = "SubArrayPointing";

      sap.expTimeStartUTC().value = toUTC(parset.settings.startTime);
      sap.expTimeStartMJD().value = toMJD(parset.settings.startTime);

      sap.expTimeEndUTC().value = toUTC(stopTime);
      sap.expTimeEndMJD().value = toMJD(stopTime);

      // TODO: fix the system to use the parset.beamDuration(sapNr), but OLAP
      // does not work that way yet (beamDuration is currently unsupported).
      sap.totalIntegrationTime().value = nrBlocks * parset.settings.blockDuration();
      sap.totalIntegrationTimeUnit().value = "s";

      // TODO: non-J2000 pointings.
      // Idem for TABs: now we subtract absolute angles to store TAB offsets. Also see TODO below.
      if( parset.settings.SAPs[sapNr].direction.type != "J2000" )
        LOG_WARN("HDF5 writer does not record positions of non-J2000 observations yet.");

      const struct ObservationSettings::Direction &beamDir = parset.settings.SAPs[sapNr].direction;
      sap.pointRA().value = beamDir.angle1 * 180.0 / M_PI;
      sap.pointRAUnit().value = "deg";
      sap.pointDEC().value = beamDir.angle2 * 180.0 / M_PI;
      sap.pointDECUnit().value = "deg";

      sap.observationNofBeams().value = parset.settings.beamFormer.SAPs[sapNr].TABs.size();
      sap.nofBeams().value = 1;

      BF_ProcessingHistory sapHistory = sap.processHistory();
      sapHistory.create();
      sapHistory.groupType().value = "ProcessingHistory";

      Attribute<string> sapObservationParset(sapHistory, "OBSERVATION_PARSET");
      string parsetAsString;
      parset.writeBuffer(parsetAsString);

      sapObservationParset.value = parsetAsString;

      // information about the pencil beam

      BF_BeamGroup beam = sap.beam(beamNr);
      beam.create();
      beam.groupType().value = "Beam";

      if (parset.settings.beamFormer.doFlysEye) {
        beam.nofStations().value = 1;
        beam.stationsList().value = vector<string>(1, parset.settings.antennaFields[beamNr].name);
      } else {
        beam.nofStations().value = parset.settings.antennaFields.size();
        beam.stationsList().value = parset.allStationNames();
      }

      const vector<string> beamtargets(1, targets[sapNr]);

      beam.targets().value = beamtargets;
      beam.tracking().value = parset.settings.SAPs[sapNr].direction.type;

      const struct ObservationSettings::Direction &tabDir = parset.settings.beamFormer.SAPs[sapNr].TABs[beamNr].direction;
      beam.pointRA().value = tabDir.angle1 * 180.0 / M_PI;
      beam.pointRAUnit().value = "deg";
      beam.pointDEC().value = tabDir.angle2 * 180.0 / M_PI;
      beam.pointDECUnit().value = "deg";
      beam.pointOffsetRA().value = (tabDir.angle1 - beamDir.angle1) * 180.0 / M_PI;
      beam.pointOffsetRAUnit().value = "deg";
      beam.pointOffsetDEC().value = (tabDir.angle2 - beamDir.angle2) * 180.0 / M_PI;
      beam.pointOffsetDECUnit().value = "deg";


      beam.subbandWidth().value = subbandBandwidth;
      beam.subbandWidthUnit().value = "Hz";

      beam.beamDiameterRA().value = 0;
      beam.beamDiameterRAUnit().value = "arcmin";
      beam.beamDiameterDEC().value = 0;
      beam.beamDiameterDECUnit().value = "arcmin";

      beam.nofSamples().value = itsNrSamples * nrBlocks;
      beam.samplingRate().value = parset.subbandBandwidth() / stokesSet.nrChannels / stokesSet.timeIntegrationFactor;
      beam.samplingRateUnit().value = "Hz";
      beam.samplingTime().value = parset.sampleDuration() * stokesSet.nrChannels * stokesSet.timeIntegrationFactor;
      beam.samplingTimeUnit().value = "s";

      beam.channelsPerSubband().value = stokesSet.nrChannels;
      beam.channelWidth().value = channelBandwidth;
      beam.channelWidthUnit().value = "Hz";

      vector<double> beamCenterFrequencies(nrSubbands, 0.0);

      for (unsigned sb = firstSubbandIdx; sb < nrSubbands; sb++)
        beamCenterFrequencies[sb] = subbandCenterFrequencies[subbandIndices[sb]];

      double beamCenterFrequencySum = accumulate(beamCenterFrequencies.begin(), beamCenterFrequencies.end(), 0.0);

      beam.beamFrequencyCenter().value = (beamCenterFrequencySum / nrSubbands - frequencyOffsetPPF) / 1e6;
      beam.beamFrequencyCenterUnit().value = "MHz";

      double DM = parset.settings.corrections.dedisperse ? parset.settings.beamFormer.SAPs[sapNr].TABs[beamNr].dispersionMeasure : 0.0;

      beam.foldedData().value = false;
      beam.foldPeriod().value = 0.0;
      beam.foldPeriodUnit().value = "s";

      beam.dedispersion().value = DM == 0.0 ? "NONE" : "COHERENT";
      beam.dispersionMeasure().value = DM;
      beam.dispersionMeasureUnit().value = "pc/cm^3";

      beam.barycentered().value = false;

      beam.observationNofStokes().value = stokesSet.nrStokes;
      beam.nofStokes().value = 1;

      vector<string> stokesComponents(1, stokesVars[stokesNr]);

      beam.stokesComponents().value = stokesComponents;
      beam.complexVoltage().value = stokesSet.type == STOKES_XXYY;
      beam.signalSum().value = stokesSet.coherent ? "COHERENT" : "INCOHERENT";

      beam.stokesComponents().value = stokesComponents;
      beam.complexVoltage().value = stokesSet.type == STOKES_XXYY;
      beam.signalSum().value = stokesSet.coherent ? "COHERENT" : "INCOHERENT";

      BF_ProcessingHistory beamHistory = beam.processHistory();
      beamHistory.create();

      Attribute<string> beamObservationParset(beamHistory, "OBSERVATION_PARSET");

      beamObservationParset.value = parsetAsString;

      CoordinatesGroup coordinates = beam.coordinates();
      coordinates.create();
      coordinates.groupType().value = "Coordinates";

      coordinates.refLocationValue().value = parset.settings.delayCompensation.referencePhaseCenter;
      coordinates.refLocationUnit().value = vector<string>(3, "m");
      coordinates.refLocationFrame().value = "ITRF";

      coordinates.refTimeValue().value = toMJD(parset.settings.startTime);
      coordinates.refTimeUnit().value = "d";
      coordinates.refTimeFrame().value = "MJD";

      coordinates.nofCoordinates().value = 2;
      coordinates.nofAxes().value = 2;

      vector<string> coordinateTypes(2);
      coordinateTypes[0] = "Time"; // or TimeCoord ?
      coordinateTypes[1] = "Spectral"; // or SpectralCoord ?
      coordinates.coordinateTypes().value = coordinateTypes;

      vector<double> unitvector(1,1);

      SmartPtr<TimeCoordinate> timeCoordinate = dynamic_cast<TimeCoordinate*>(coordinates.coordinate(0));
      timeCoordinate.get()->create();
      timeCoordinate.get()->groupType().value = "TimeCoord";

      timeCoordinate.get()->coordinateType().value = "Time";
      timeCoordinate.get()->storageType().value = vector<string>(1,"Linear");
      timeCoordinate.get()->nofAxes().value = 1;
      timeCoordinate.get()->axisNames().value = vector<string>(1,"Time");
      timeCoordinate.get()->axisUnits().value = vector<string>(1,"us");

      // linear coordinates:
      //   referenceValue = offset from starting time, in axisUnits
      //   referencePixel = offset from first sample
      //   increment      = time increment for each sample
      //   pc             = scaling factor (?)

      timeCoordinate.get()->referenceValue().value = 0;
      timeCoordinate.get()->referencePixel().value = 0;
      timeCoordinate.get()->increment().value = parset.sampleDuration() * stokesSet.nrChannels * stokesSet.timeIntegrationFactor;
      timeCoordinate.get()->pc().value = unitvector;

      timeCoordinate.get()->axisValuesPixel().value = vector<unsigned>(1, 0); // not used
      timeCoordinate.get()->axisValuesWorld().value = vector<double>(1, 0.0); // not used

      SmartPtr<SpectralCoordinate> spectralCoordinate = dynamic_cast<SpectralCoordinate*>(coordinates.coordinate(1));
      spectralCoordinate.get()->create();
      spectralCoordinate.get()->groupType().value = "SpectralCoord";

      spectralCoordinate.get()->coordinateType().value = "Spectral";
      spectralCoordinate.get()->storageType().value = vector<string>(1,"Tabular");
      spectralCoordinate.get()->nofAxes().value = 1;
      spectralCoordinate.get()->axisNames().value = vector<string>(1,"Frequency");
      spectralCoordinate.get()->axisUnits().value = vector<string>(1,"MHz");

      spectralCoordinate.get()->referenceValue().value = 0; // not used
      spectralCoordinate.get()->referencePixel().value = 0; // not used
      spectralCoordinate.get()->increment().value = 0;      // not used
      spectralCoordinate.get()->pc().value = unitvector;             // not used

      // tabular coordinates:
      //   axisValuePixel = data indices
      //   axisValueWorld = corresponding (central) frequencies

      vector<unsigned> spectralPixels;
      vector<double> spectralWorld;

      for(unsigned sb = firstSubbandIdx; sb < nrSubbands; sb++) {
        const double subbandBeginFreq = parset.channel0Frequency( subbandIndices[sb], stokesSet.nrChannels );

        // NOTE: channel 0 will be wrongly annotated if nrChannels > 1, because it is a combination of the
        // highest and the lowest frequencies (half a channel each).

        for(unsigned ch = 0; ch < stokesSet.nrChannels; ch++) {
          spectralPixels.push_back(spectralPixels.size());
          spectralWorld.push_back(subbandBeginFreq + ch * channelBandwidth);
        }
      }

      spectralCoordinate.get()->axisValuesPixel().value = spectralPixels;
      spectralCoordinate.get()->axisValuesWorld().value = spectralWorld;

      BF_StokesDataset stokesDS = beam.stokes(stokesNr);

      vector<ssize_t> dims(2), maxdims(2);

      dims[0] = itsNrSamples * nrBlocks;
      dims[1] = itsNrChannels;

      maxdims[0] = -1;
      maxdims[1] = itsNrChannels;

      stokesDS.create(dims, maxdims, LOFAR::basename(rawfilename), BF_StokesDataset::LITTLE);
      stokesDS.groupType().value = "bfData";
      stokesDS.dataType().value = "float";

      stokesDS.stokesComponent().value = stokesVars[stokesNr];
      stokesDS.nofChannels().value = vector<unsigned>(nrSubbands, stokesSet.nrChannels);
      stokesDS.nofSubbands().value = nrSubbands;
      stokesDS.nofSamples().value = dims[0];

      // construct feedback for LTA -- Implements Output_Beamformed_.comp

      const string type = 
        parset.settings.beamFormer.doFlysEye ? "FlysEyeBeam" : 
        (stokesSet.coherent ? "CoherentStokesBeam" : "IncoherentStokesBeam");

      itsConfiguration.add("fileFormat",                "HDF5");
      itsConfiguration.add("filename",                  LOFAR::basename(h5filename));
      itsConfiguration.add("size",                      "0");
      itsConfiguration.add("location",                  f.location.host + ":" + LOFAR::dirname(h5filename));
      itsConfiguration.add("percentageWritten",         "0");

      itsConfiguration.add("nrOfCoherentStokesBeams",   "0");
      itsConfiguration.add("nrOfIncoherentStokesBeams", "0");
      itsConfiguration.add("nrOfFlysEyeBeams",          "0");
      itsConfiguration.replace(str(format("nrOf%ss") % type), "1");

      itsConfiguration.add("beamTypes",                 "[]");

      string prefix = str(format("%s[0].") % type);

      itsConfiguration.add(prefix + "SAP",               str(format("%u") % sapNr));
      itsConfiguration.add(prefix + "TAB",               str(format("%u") % beamNr));
      itsConfiguration.add(prefix + "samplingTime",      str(format("%f") % (parset.sampleDuration() * stokesSet.nrChannels * stokesSet.timeIntegrationFactor)));
      itsConfiguration.add(prefix + "dispersionMeasure", str(format("%f") % DM));
      itsConfiguration.add(prefix + "nrSubbands",        str(format("%u") % nrSubbands));

      ostringstream centralFreqsStr;
      centralFreqsStr << "[";
      for (size_t i = 0; i < beamCenterFrequencies.size(); ++i) {
        if( i > 0 )
          centralFreqsStr << ", ";
        centralFreqsStr << str(format("%.4lf") % beamCenterFrequencies[i]);
      }
      centralFreqsStr << "]";

      itsConfiguration.add(prefix + "centralFrequencies", centralFreqsStr.str());

      ostringstream stationSubbandsStr;
      stationSubbandsStr << "[";
      for (size_t i = 0; i < nrSubbands; ++i) {
        if( i > 0 )
          stationSubbandsStr << ", ";
        stationSubbandsStr << str(format("%u") % parset.settings.SAPs[sapNr].subbands[i].stationIdx);
      }
      stationSubbandsStr << "]";

      itsConfiguration.add(prefix + "stationSubbands",  stationSubbandsStr.str());

      itsConfiguration.add(prefix + "channelWidth",      str(format("%f") % channelBandwidth));
      itsConfiguration.add(prefix + "channelsPerSubband",str(format("%u") % stokesSet.nrChannels));
      itsConfiguration.add(prefix + "stokes",            str(format("[%s]") % stokesVars_LTA[stokesNr]));

      if (type == "CoherentStokesBeam") {
        itsConfiguration.add(prefix + "Pointing.equinox",   "J2000");
        itsConfiguration.add(prefix + "Pointing.coordType", "RA-DEC");
        itsConfiguration.add(prefix + "Pointing.angle1",    str(format("%f") % tabDir.angle1));
        itsConfiguration.add(prefix + "Pointing.angle2",    str(format("%f") % tabDir.angle2));

        itsConfiguration.add(prefix + "Offset.equinox",     "J2000");
        itsConfiguration.add(prefix + "Offset.coordType",   "RA-DEC");
        itsConfiguration.add(prefix + "Offset.angle1",      str(format("%f") % (tabDir.angle1 - beamDir.angle1)));
        itsConfiguration.add(prefix + "Offset.angle2",      str(format("%f") % (tabDir.angle2 - beamDir.angle2)));
      } else if (type == "FlysEyeBeam") {
        string fullName = parset.settings.antennaFields.at(beamNr).name;
        string stationName = fullName.substr(0,5);
        string antennaFieldName = fullName.substr(5);
        itsConfiguration.add(prefix + "stationName", stationName);
        itsConfiguration.add(prefix + "antennaFieldName", antennaFieldName);
      }
    }

    template <typename T,unsigned DIM>
    MSWriterDAL<T,DIM>::~MSWriterDAL()
    {
    }

    template <typename T,unsigned DIM>
    void MSWriterDAL<T,DIM>::write(StreamableData *data)
    {
      SampleData<T,DIM> *sdata = dynamic_cast<SampleData<T,DIM> *>(data);

      ASSERT( data );
      ASSERT( sdata );
      
      ASSERTSTR( sdata->samples.num_elements() >= itsBlockSize,
             "A block is at least " << itsBlockSize <<
             " elements, but provided sdata only has " << 
             sdata->samples.num_elements() << " elements" );

      unsigned seqNr = data->sequenceNumber();
      unsigned bytesPerBlock = itsBlockSize * sizeof(T);  

      // fill in zeroes for lost blocks
      if (itsNextSeqNr < seqNr)
        itsFile.skip((seqNr - itsNextSeqNr) * bytesPerBlock);

      // make sure we skip |2 in the highest dimension
      itsFile.write(sdata->samples.origin(), bytesPerBlock);

      itsNextSeqNr = seqNr + 1;
      itsNrBlocksWritten++;

      itsConfiguration.replace("size",              str(format("%u") % getDataSize()));
      itsConfiguration.replace("percentageWritten", str(format("%u") % percentageWritten()));
    }

    // specialisation for FinalBeamFormedData
    template class MSWriterDAL<float,3>;

  } // namespace Cobalt
} // namespace LOFAR

#endif // HAVE_DAL

