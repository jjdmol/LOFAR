//# Parset.cc
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CoInterface/Parset.h>

#include <cstring>
#include <cmath>
#include <set>
#include <algorithm>
#include <memory>   // auto_ptr

#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <Common/LofarLogger.h>
#include <Common/DataConvert.h>
#include <Common/LofarBitModeInfo.h>
#include <ApplCommon/PosixTime.h>
#include <CoInterface/OutputTypes.h>
#include <CoInterface/Align.h>
#include <CoInterface/Config.h>
#include <CoInterface/Exceptions.h>
#include <CoInterface/PrintVector.h>
#include <CoInterface/SetOperations.h>
#include <CoInterface/RingCoordinates.h>

using namespace std;
using boost::format;


namespace LOFAR
{
  namespace Cobalt
  {


    StokesType stokesType( const std::string &name )
    {
      if (name == "I")
        return STOKES_I;

      if (name == "IQUV")
        return STOKES_IQUV;

      if (name == "XXYY")
        return STOKES_XXYY;

      return INVALID_STOKES;
    }


    size_t nrStokes( StokesType type )
    {
      switch(type) {
        case STOKES_I:
          return 1;

        case STOKES_IQUV:
        case STOKES_XXYY:
          return 4;

        case INVALID_STOKES:
        default:
          return 0;
      }
    }


    string stokesType( StokesType type )
    {
      switch(type) {
      case STOKES_I: 
        return "I";

      case STOKES_IQUV: 
        return "IQUV";

      case STOKES_XXYY:
        return "XXYY";

      case INVALID_STOKES:
      default:
        return "";
      }
    }


    unsigned ObservationSettings::nyquistZone() const
    {
      if (bandFilter == "LBA_10_70" ||
          bandFilter == "LBA_30_70" ||
          bandFilter == "LBA_10_90" ||
          bandFilter == "LBA_30_90" )
        return 1;

      if (bandFilter == "HBA_110_190")
        return 2;

      if (bandFilter == "HBA_170_230" ||
          bandFilter == "HBA_210_250")
        return 3;

      THROW(CoInterfaceException, std::string("unknown band filter \"" + bandFilter + '"'));
    }


    unsigned ObservationSettings::clockHz() const
    {
      return clockMHz * 1000000;
    }


    Parset::Parset()
    {
    }


    Parset::Parset(const string &name)
      :
      ParameterSet(name.c_str()),
      itsName(name)
    {
      // we check the parset once we can communicate any errors
      //check();

      updateSettings();
    }


    void readParameterSet(Stream &stream, ParameterSet &parameterSet)
    {
      // Read size
      uint64 size;
      stream.read(&size, sizeof size);

      // Read data
      std::vector<char> tmp(size + 1);
      stream.read(&tmp[0], size);
      tmp[size] = '\0';

      // Add data to parset
      std::string buffer(&tmp[0], size);
      parameterSet.adoptBuffer(buffer);
    }


    Parset::Parset(Stream *stream)
    {
      readParameterSet(*stream, *this);

      // Update the settings
      updateSettings();
    }


    void Parset::write(Stream *stream) const
    {
      // stream == NULL fills the settings,
      // causing subsequent write()s to use it
      bool readCache = !itsWriteCache.empty();
      bool writeCache = !stream;

      std::string newbuffer;
      std::string &buffer = readCache || writeCache ? itsWriteCache : newbuffer;

      if (buffer.empty())
        writeBuffer(buffer);

      if (!stream) {
        // we only filled the settings
        return;
      }

      uint64 size = buffer.size();
      stream->write(&size, sizeof size);
      stream->write(buffer.data(), size);
    }


    vector<struct ObservationSettings::AntennaFieldName> ObservationSettings::antennaFieldNames(const vector<string> &stations, const string &antennaSet) {
      vector<struct AntennaFieldName> result;

      for (vector<string>::const_iterator i = stations.begin(); i != stations.end(); ++i) {
        const string &station = *i;

        bool coreStation = station.substr(0,2) == "CS";

        if (station.length() != 5) {
          // Backward compatibility: the key
          // Observation.VirtualInstrument.stationList could contain full
          // antennafield names in the past, such as "CS001LBA".
          LOG_WARN_STR("Warning: old (preparsed) station name: " << station);

          // Do not assume the standard station name format (silly "S9" test name).
          string stName;
          string antFieldName;
          if (station.length() <= 1)
            stName = station; // if stName or antFieldName is empty, writing an MS table will fail
          else if (station.length() <= 5) {
            stName = station.substr(0, station.length()-1);
            antFieldName = station.substr(station.length()-1);
          } else {
            stName = station.substr(0, 5);
            antFieldName = station.substr(5);
          }
          result.push_back(AntennaFieldName(stName, antFieldName));
          continue;
        }

        if (antennaSet == "LBA" /* used for debugging */
         || antennaSet == "LBA_INNER"
         || antennaSet == "LBA_OUTER"
         || antennaSet == "LBA_X"
         || antennaSet == "LBA_Y"
         || antennaSet == "LBA_SPARSE_EVEN"
         || antennaSet == "LBA_SPARSE_ODD") {

          result.push_back(AntennaFieldName(station, "LBA"));

        } else if (
            antennaSet == "HBA" /* used for debugging */
         || antennaSet == "HBA_JOINED"
         || antennaSet == "HBA_JOINED_INNER") {

          result.push_back(AntennaFieldName(station, "HBA"));

        } else if (
            antennaSet == "HBA_ZERO"
         || antennaSet == "HBA_ZERO_INNER") {

          result.push_back(AntennaFieldName(station, coreStation ? "HBA0" : "HBA"));

        } else if (
            antennaSet == "HBA_ONE"
         || antennaSet == "HBA_ONE_INNER") {

          result.push_back(AntennaFieldName(station, coreStation ? "HBA1" : "HBA"));

        } else if (
            antennaSet == "HBA_DUAL"
         || antennaSet == "HBA_DUAL_INNER") {

          if (coreStation) {
            result.push_back(AntennaFieldName(station, "HBA0"));
            result.push_back(AntennaFieldName(station, "HBA1"));
          } else {
            result.push_back(AntennaFieldName(station, "HBA"));
          }
        } else {
          THROW(CoInterfaceException, "Unknown antennaSet: " << antennaSet);
        }
      }

      return result;
    }

    /*
     * operator<() for station names.
     *
     * Sorts in the following order:
     *   1. Core stations (CSxxx)
     *   2. Remote stations (RSxxx)
     *   3. International stations (others)
     *
     * Within each group, the stations are
     * sorted lexicographically. For group 3
     * we skip the first 2 chars when sorting.
     */
    bool compareStationNames( const string &a, const string &b ) {
      if (a.size() >= 5 && b.size() >= 5) { // common case
        if ( (a[0] == 'C' || a[0] == 'R') && a[1] == 'S' &&
             (b[0] == 'C' || b[0] == 'R') && b[1] == 'S' ) {
          return a < b; // both CS/RS stations; 'C'<'R'
        } else { // at least 1 non-CS/RS name; cmp (presumed) nrs
          return std::strcmp(&a.c_str()[2], &b.c_str()[2]) < 0;
        }
      }
      return a < b; // at least 1 short name
    }


    struct ObservationSettings Parset::observationSettings() const
    {
      struct ObservationSettings settings;

      // the set of hosts on which outputProc has to run, which will
      // be constructed during the parsing of the parset
      set<string> outputProcHosts;

      // NOTE: Make sure that all keys have defaults, to make test parsets
      // a lot shorter.

      vector<string>   emptyVectorString;
      vector<unsigned> emptyVectorUnsigned;
      vector<double>   emptyVectorDouble;

      // Generic information
      settings.realTime = getBool("Cobalt.realTime", false);
      settings.observationID = getUint32("Observation.ObsID", 0);
      settings.momID         = getUint32("Observation.momID", 0);
      settings.commandStream = getString("Cobalt.commandStream", "null:");
      settings.startTime = getTime("Observation.startTime", "2013-01-01 00:00:00");
      settings.stopTime  = getTime("Observation.stopTime",  "2013-01-01 00:01:00");
      settings.clockMHz = getUint32("Observation.sampleClock", 200);

      settings.nrBitsPerSample = getUint32("Observation.nrBitsPerSample", 16);

      settings.nrPolarisations = 2;

      settings.corrections.bandPass   = getBool("Cobalt.correctBandPass", true);
      settings.corrections.clock      = getBool("Cobalt.correctClocks", true);
      settings.corrections.dedisperse = getBool("Cobalt.BeamFormer.coherentDedisperseChannels", true);

      settings.delayCompensation.enabled              = getBool("Cobalt.delayCompensation", true);
      settings.delayCompensation.referencePhaseCenter = getDoubleVector("Observation.referencePhaseCenter", vector<double>(3,0), true);
      if (settings.delayCompensation.referencePhaseCenter == emptyVectorDouble)
        LOG_WARN("Parset: Observation.referencePhaseCenter is missing (or (0.0, 0.0, 0.0)).");

      // Station information (required by pointing information)
      settings.antennaSet     = getString("Observation.antennaSet", "LBA_INNER");
      settings.bandFilter     = getString("Observation.bandFilter", "LBA_30_70");

      // Pointing information
      size_t nrSAPs = getUint32("Observation.nrBeams", 1);
      unsigned subbandOffset = 512 * (settings.nyquistZone() - 1);

      settings.SAPs.resize(nrSAPs);
      settings.subbands.clear();
      for (unsigned sapNr = 0; sapNr < nrSAPs; ++sapNr) 
      {
        struct ObservationSettings::SAP &sap = settings.SAPs[sapNr];

        sap.direction.type   = getString(str(format("Observation.Beam[%u].directionType") % sapNr), "J2000");
        sap.direction.angle1 = getDouble(str(format("Observation.Beam[%u].angle1") % sapNr), 0.0);
        sap.direction.angle2 = getDouble(str(format("Observation.Beam[%u].angle2") % sapNr), 0.0);
        sap.target           = getString(str(format("Observation.Beam[%u].target") % sapNr), "");

        // Process the subbands of this SAP
        vector<unsigned> subbandList = getUint32Vector(str(format("Observation.Beam[%u].subbandList") % sapNr), emptyVectorUnsigned, true);
        ASSERTSTR(!subbandList.empty(), "subband list for SAP " << sapNr << " must be non-empty (Observation.Beam[" << sapNr << "].subbandList)");
        vector<double> frequencyList = getDoubleVector(str(format("Observation.Beam[%u].frequencyList") % sapNr), emptyVectorDouble, true);

        for (unsigned sb = 0; sb < subbandList.size(); ++sb)
        {
          struct ObservationSettings::Subband subband;

          subband.idx              = settings.subbands.size();
          subband.stationIdx       = subbandList[sb];
          subband.SAP              = sapNr;
          subband.idxInSAP         = sb;
          subband.centralFrequency = frequencyList.empty()
                                     ? settings.subbandWidth() * (subband.stationIdx + subbandOffset)
                                     : frequencyList[sb];

          // Register the subband both globally and in the SAP structure
          settings.subbands.push_back(subband);
          settings.SAPs[sapNr].subbands.push_back(subband);
        }
      }

      settings.anaBeam.enabled = settings.antennaSet.substr(0,3) == "HBA";
      if (settings.anaBeam.enabled) {
        settings.anaBeam.direction.type   = getString("Observation.AnaBeam[0].directionType", "J2000");
        settings.anaBeam.direction.angle1 = getDouble("Observation.AnaBeam[0].angle1", 0.0);
        settings.anaBeam.direction.angle2 = getDouble("Observation.AnaBeam[0].angle2", 0.0);
      }

      settings.blockSize = getUint32("Cobalt.blockSize", 196608);

      // Station information (used pointing information to verify settings)
      vector<string> stations = getStringVector("Observation.VirtualInstrument.stationList", emptyVectorString, true);
      ASSERTSTR(!stations.empty(), "station list (Observation.VirtualInstrument.stationList) must be non-empty");
      settings.rawStationList = getString("Observation.VirtualInstrument.stationList", "[]");

      // Sort stations (CS, RS, int'l), to get a consistent and predictable
      // order in the MeasurementSets.
      std::sort(stations.begin(), stations.end(), compareStationNames);

      // Conversion from station names to antenna field names.
      vector<ObservationSettings::AntennaFieldName> fieldNames =
        ObservationSettings::antennaFieldNames(stations, settings.antennaSet);

      settings.antennaFields.resize(fieldNames.size());
      for (unsigned i = 0; i < settings.antennaFields.size(); ++i) {
        struct ObservationSettings::AntennaField &antennaField = settings.antennaFields[i];

        antennaField.name            = fieldNames[i].fullName();
        antennaField.inputStreams    = getStringVector(str(format("PIC.Core.%s.RSP.ports") % antennaField.name), emptyVectorString, true);
        antennaField.receiver        = getString(str(format("PIC.Core.%s.RSP.receiver") % antennaField.name), "");

        // NOTE: Support for clockCorrectionTime can be phased out when the
        // BG/P is gone. delay.X and delay.Y are superior to it, being
        // polarisation specific.
        antennaField.clockCorrection = getDouble(str(format("PIC.Core.%s.clockCorrectionTime") % antennaField.name), 0.0);
        antennaField.phaseCenter     = getDoubleVector(str(format("PIC.Core.%s.phaseCenter") % antennaField.name), vector<double>(3, 0), true);
        if (antennaField.phaseCenter == emptyVectorDouble)
          LOG_WARN_STR("Parset: PIC.Core." << antennaField.name << ".phaseCenter is missing (or (0.0, 0.0, 0.0)).");
        antennaField.phase0.x        = getDouble(str(format("PIC.Core.%s.%s.%s.phase0.X") % fieldNames[i].fullName() % settings.antennaSet % settings.bandFilter), 0.0);
        antennaField.phase0.y        = getDouble(str(format("PIC.Core.%s.%s.%s.phase0.Y") % fieldNames[i].fullName() % settings.antennaSet % settings.bandFilter), 0.0);
        antennaField.delay.x         = getDouble(str(format("PIC.Core.%s.%s.%s.delay.X")  % fieldNames[i].fullName() % settings.antennaSet % settings.bandFilter), 0.0);
        antennaField.delay.y         = getDouble(str(format("PIC.Core.%s.%s.%s.delay.Y")  % fieldNames[i].fullName() % settings.antennaSet % settings.bandFilter), 0.0);

        if (antennaField.delay.x > 0.0 || antennaField.delay.y > 0.0) {
          if (antennaField.clockCorrection != 0.0) {
            // Ignore clockCorrectionTime if delay.X or delay.Y are specified.
            antennaField.clockCorrection = 0.0;
            LOG_WARN_STR("Ignoring PIC.Core." << antennaField.name <<
                         ".clockCorrectionTime in favor of PIC.Core." <<
                         fieldNames[i].fullName() << "." << settings.antennaSet <<
                         "." << settings.bandFilter << ".delay.{X,Y}");
          }
        }

        string key = std::string(str(format("Observation.Dataslots.%s.RSPBoardList") % antennaField.name));
        if (!isDefined(key))
          key = "Observation.rspBoardList";
        antennaField.rspBoardMap     = getUint32Vector(key, emptyVectorUnsigned, true);

        ASSERTSTR(antennaField.rspBoardMap.size() >= settings.subbands.size(),
                  "Observation has " << settings.subbands.size() <<
                  " subbands, but antenna field " << antennaField.name <<
                  " has only board numbers defined for " << antennaField.rspBoardMap.size() <<
                  " subbands. Please correct either Observation.rspBoardList or Observation.Dataslots." <<
                  antennaField.name << ".RSPBoardList" );

        key = std::string(str(format("Observation.Dataslots.%s.DataslotList") % antennaField.name));
        if (!isDefined(key))
          key = "Observation.rspSlotList";
        antennaField.rspSlotMap = getUint32Vector(key, emptyVectorUnsigned, true);

        ASSERTSTR(antennaField.rspSlotMap.size() >= settings.subbands.size(),
                  "Observation has " << settings.subbands.size() <<
                  " subbands, but antenna field " << antennaField.name <<
                  " has only board numbers defined for " << antennaField.rspSlotMap.size() <<
                  " subbands. Please correct either Observation.rspSlotList or Observation.Dataslots." <<
                  antennaField.name << ".DataslotList" );
      }


      // Resource information
      vector<string> nodes = getStringVector("Cobalt.Nodes", emptyVectorString, true);

      // We can restrict cobalt nodes to the set that receives from antenna fields,
      // but that will break if that set cannot handle the computations.
      bool receivingNodesOnly = getBool("Cobalt.restrictNodesToStationStreams", false);
      for (size_t i = 0; i < nodes.size(); ++i) {
        if (receivingNodesOnly && !nodeReadsAntennaFieldData(settings, nodes[i]))
          continue;

        struct ObservationSettings::Node node;
        node.rank     = i;
        node.name     = nodes[i];

        string prefix = str(format("PIC.Core.Cobalt.%s.") % node.name);

        node.hostName = getString(prefix + "host", "localhost");
        node.cpu      = getUint32(prefix + "cpu",  0);
        node.nic      = getString(prefix + "nic",  "");
        node.gpus     = getUint32Vector(prefix + "gpus", vector<unsigned>(1,0)); // default to [0]

        settings.nodes.push_back(node);
      }

      /* ===============================
       * Correlator pipeline information
       * ===============================
       */

      settings.correlator.enabled = getBool("Observation.DataProducts.Output_Correlated.enabled", false);
      if (settings.correlator.enabled) {
        settings.correlator.nrChannels = getUint32("Cobalt.Correlator.nrChannelsPerSubband", 64);
        //settings.correlator.nrChannels = getUint32("Observation.channelsPerSubband", 64);
        settings.correlator.channelWidth = settings.subbandWidth() / settings.correlator.nrChannels;
        settings.correlator.nrSamplesPerBlock       = settings.blockSize / settings.correlator.nrChannels;
        settings.correlator.nrBlocksPerIntegration = getUint32("Cobalt.Correlator.nrBlocksPerIntegration", 1);
        settings.correlator.nrIntegrationsPerBlock = getUint32("Cobalt.Correlator.nrIntegrationsPerBlock", 1);

        // We either have the integration time spanning multiple blocks, or the integration time being a part
        // of a block, but never both.
        ASSERT(settings.correlator.nrBlocksPerIntegration == 1 || settings.correlator.nrIntegrationsPerBlock == 1);

        settings.correlator.nrIntegrations = settings.nrBlocks()
                                           * settings.correlator.nrIntegrationsPerBlock
                                           / settings.correlator.nrBlocksPerIntegration;

        // super-station beam former
        //
        // TODO: Super-station beam former is unused, so will likely be
        // implemented differently. The code below is only there to show how
        // the OLAP.* keys used to be interpreted.

        // OLAP.CNProc.tabList[i] = j <=> superstation j contains (input) station i
        vector<unsigned> tabList = getUint32Vector("OLAP.CNProc.tabList", emptyVectorUnsigned, true);

        // Names for all superstations, including those that are simple copies
        // of (input) antenna fields.
        vector<string> tabNames = getStringVector("OLAP.tiedArrayStationNames", emptyVectorString, true);

        if (tabList.empty()) {
          // default: input station list = output station list
          settings.correlator.stations.resize(settings.antennaFields.size());
          for (size_t i = 0; i < settings.correlator.stations.size(); ++i) {
            struct ObservationSettings::Correlator::Station &station = settings.correlator.stations[i];

            station.name = settings.antennaFields[i].name;
            station.inputStations = vector<size_t>(1, i);
          }
        } else {
          // process super-station beam former list
          settings.correlator.stations.resize(tabList.size());
          for (size_t i = 0; i < settings.correlator.stations.size(); ++i) {
            struct ObservationSettings::Correlator::Station &station = settings.correlator.stations[i];

            station.name = tabNames[i];
          }
          for (size_t i = 0; i < tabList.size(); ++i) {
            settings.correlator.stations[tabList[i]].inputStations.push_back(i);
          }
        }

        // Files to output
        const vector<ObservationSettings::FileLocation> locations = getFileLocations("Correlated");

        settings.correlator.files.resize(settings.subbands.size());
        for (size_t i = 0; i < settings.correlator.files.size(); ++i) {
          if (i >= locations.size())
            THROW(CoInterfaceException, "No correlator filename or location specified for subband " << i);

          settings.correlator.files[i].streamNr = i;
          settings.correlator.files[i].location = locations[i];

          outputProcHosts.insert(settings.correlator.files[i].location.host);
        }
      }

      /* ===============================
       * Beamformer pipeline information
       * ===============================
       */

      // SAP/TAB-crossing counter for the files we generate
      size_t bfStreamNr = 0;

      bool doCoherentStokes = getBool(
        "Observation.DataProducts.Output_CoherentStokes.enabled", false);
      bool doIncoherentStokes = getBool(
        "Observation.DataProducts.Output_IncoherentStokes.enabled", false);

      settings.beamFormer.enabled = doCoherentStokes || doIncoherentStokes;

      if (settings.beamFormer.enabled) {
        // Parse global settings

        // 4096 channels is enough, but allow parset override.
        if (!isDefined("Cobalt.BeamFormer.nrHighResolutionChannels")) {
          settings.beamFormer.nrHighResolutionChannels = 4096;
        } else {
          settings.beamFormer.nrHighResolutionChannels =
              getUint32("Cobalt.BeamFormer.nrHighResolutionChannels");
          ASSERTSTR(powerOfTwo(settings.beamFormer.nrHighResolutionChannels) &&
              settings.beamFormer.nrHighResolutionChannels < 65536,
              "Parset: Cobalt.BeamFormer.nrHighResolutionChannels must be a power of 2 and < 64k");
        }

        settings.beamFormer.doFlysEye = getBool("Cobalt.BeamFormer.flysEye", false);

        unsigned nrDelayCompCh;
        if (!isDefined("Cobalt.BeamFormer.nrDelayCompensationChannels")) {
          nrDelayCompCh = calcNrDelayCompensationChannels(settings);
        } else {
          nrDelayCompCh = getUint32("Cobalt.BeamFormer.nrDelayCompensationChannels");
        }
        if (nrDelayCompCh > settings.beamFormer.nrHighResolutionChannels) {
          nrDelayCompCh = settings.beamFormer.nrHighResolutionChannels;
        }
        settings.beamFormer.nrDelayCompensationChannels = nrDelayCompCh;

        ObservationSettings::BeamFormer::StokesSettings
          defaultSettings = 
          {
            true,     // coherent stokes?
            STOKES_I, // StokesType
            1,        // nrStokes
            1,        // nrChannels
            1,        // timeIntegrationFactor
            0,        // nrSamples
            0         // nrSubbandsPerFile
          };

        settings.beamFormer.coherentSettings = defaultSettings;
        settings.beamFormer.incoherentSettings = defaultSettings;
          
        for (unsigned i = 0; i < 2; ++i) {
          // Set coherent and incoherent Stokes settings by
          // iterating twice.
          // TODO: This is an ugly way to do this.

          string prefix = "";
          struct ObservationSettings::BeamFormer::StokesSettings *stSettings = 0;
          
          // Select coherent or incoherent for this iteration
          switch(i) {
            case 0:
              prefix = "Cobalt.BeamFormer.CoherentStokes";
              stSettings = &settings.beamFormer.coherentSettings;
              stSettings->coherent = true;
              break;

            case 1:
              prefix = "Cobalt.BeamFormer.IncoherentStokes";
              stSettings = &settings.beamFormer.incoherentSettings;
              stSettings->coherent = false;
              break;

            default:
              ASSERT(false);
              break;
          }

          // Coherent Stokes
          if (i == 0 && !doCoherentStokes)
            continue;

          // Incoherent Stokes
          if (i == 1 && !doIncoherentStokes)
            continue;

          // Obtain settings of selected stokes
          stSettings->type = stokesType(getString(prefix + ".which", "I"));
          stSettings->nrStokes = nrStokes(stSettings->type);
          stSettings->nrChannels = getUint32(prefix + ".nrChannelsPerSubband", 1);
          ASSERT(stSettings->nrChannels > 0);

          stSettings->timeIntegrationFactor = getUint32(prefix + ".timeIntegrationFactor", 1);
          ASSERT(stSettings->timeIntegrationFactor > 0);
          stSettings->nrSubbandsPerFile = getUint32(prefix + ".subbandsPerFile", 0); // 0 or a large nr is interpreted below
          stSettings->nrSamples = settings.blockSize / stSettings->timeIntegrationFactor / stSettings->nrChannels;
        }

        const vector<ObservationSettings::FileLocation> coherent_locations =
          getFileLocations("CoherentStokes");
        const vector<ObservationSettings::FileLocation> incoherent_locations =
          getFileLocations("IncoherentStokes");

        size_t coherent_idx = 0;
        size_t incoherent_idx = 0;

        // Parse all TABs
        settings.beamFormer.SAPs.resize(nrSAPs);

        for (unsigned i = 0; i < nrSAPs; ++i) 
        {
          struct ObservationSettings::BeamFormer::SAP &sap = settings.beamFormer.SAPs[i];
          struct ObservationSettings::SAP &obsSap = settings.SAPs[i];

          size_t nrTABs    = getUint32(str(format("Observation.Beam[%u].nrTiedArrayBeams") % i), 0);
          size_t nrTABSParset = nrTABs;
          size_t nrRings   = getUint32(str(format("Observation.Beam[%u].nrTabRings") % i), 0);
          double ringWidth = getDouble(str(format("Observation.Beam[%u].tabRingSize") % i), 0.0);

          // Create a ptr to RingCoordinates object
          // If there are tab rings the object will be actuall constructed
          // The actual tabs will be extracted after we added all manual tabs
          // But we need the number of tabs from rings at this location
          std::auto_ptr<RingCoordinates> ptrRingCoords;
          if (nrRings > 0) {
            const string prefix = str(format("Observation.Beam[%u]") % i);
            string directionType = getString(prefix + ".directionType", "J2000");
            
            // Convert to COORDTYPES
            RingCoordinates::COORDTYPES type;
            if (directionType == "J2000")
              type = RingCoordinates::J2000;
            else if (directionType == "B1950")
              type = RingCoordinates::B1950;
            else
              type = RingCoordinates::OTHER;
              
            // Create coords object
            ptrRingCoords = std::auto_ptr<RingCoordinates>(
              new RingCoordinates(nrRings, ringWidth,
              RingCoordinates::Coordinate(obsSap.direction.angle1, obsSap.direction.angle2), type));

            // Increase the amount of tabs with the number from the coords object
            // this might be zero
            nrTABs = nrTABSParset + ptrRingCoords->nCoordinates();
          } else if (settings.beamFormer.doFlysEye) {
            // For Fly's Eye mode we have exactly one TAB per antenna field.
            nrTABs = settings.antennaFields.size();
          }

          sap.TABs.resize(nrTABs);
          for (unsigned j = 0; j < nrTABs; ++j) 
          {
            struct ObservationSettings::BeamFormer::TAB &tab = sap.TABs[j];
            // Add flys eye tabs
            if (settings.beamFormer.doFlysEye) 
            {
              const string prefix = str(format("Observation.Beam[%u]") % i);

              tab.direction.type    = getString(prefix + ".directionType", "J2000");
              tab.direction.angle1  = getDouble(prefix + ".angle1", 0.0);
              tab.direction.angle2  = getDouble(prefix + ".angle2", 0.0);

              tab.dispersionMeasure     = 0.0;
              tab.coherent              = true;
            } 
            // Add manual tabs and then the tab rings.
            else 
            {
              if (j < nrTABSParset) // If we are working on manual tabs
              {
                const string prefix = str(format("Observation.Beam[%u].TiedArrayBeam[%u]") % i % j);
                tab.direction.type    = getString(prefix + ".directionType", "J2000");

                tab.dispersionMeasure     = getDouble(prefix + ".dispersionMeasure", 0.0);
                tab.coherent              = getBool(prefix + ".coherent", true);
             
                // Incoherent TABs point in the same direction as the SAP by definition. The processing
                // pipelines do not use the angles, but the data writer does as part of its annotation.
                tab.direction.angle1  = tab.coherent ? getDouble(prefix + ".angle1", 0.0) : obsSap.direction.angle1;
                tab.direction.angle2  = tab.coherent ? getDouble(prefix + ".angle2", 0.0) : obsSap.direction.angle2;
              }
              else
              {
                // Get the pointing for the tabrings.
                // Subtract the number of manual to get index in the ringCoords
                RingCoordinates::Coordinate pointing = 
                    ptrRingCoords->coordinates().at(j - nrTABSParset);

                // Note that RingCoordinates provide *relative* coordinates, and
                // we need absolute ones.
                tab.direction.type = ptrRingCoords->coordTypeAsString();
                tab.direction.angle1 = obsSap.direction.angle1 + pointing.first;
                tab.direction.angle2 = obsSap.direction.angle2 + pointing.second;
                // One dispersion measure for all TABs in rings is inconvenient,
                // but not used anyway. Unclear if setting to 0.0 is better/worse.
                const string prefix = str(format("Cobalt.Observation.Beam[%u]") % i);
                tab.dispersionMeasure = getDouble(prefix + ".tabRingDispersionMeasure", 0.0);
                tab.coherent = true; // rings cannot be incoherent, since we use non-(0,0) pointings
              }
            }

            if (tab.coherent)
              sap.nrCoherent++;
            else
              sap.nrIncoherent++;

            struct ObservationSettings::BeamFormer::StokesSettings &stSettings =
               tab.coherent ? settings.beamFormer.coherentSettings
                            : settings.beamFormer.incoherentSettings;

            // If needed, limit to / apply default: the #subbands in this SAP.
            size_t nrSubbandsPerFile = stSettings.nrSubbandsPerFile;
            if (nrSubbandsPerFile == 0 ||
                nrSubbandsPerFile > settings.SAPs[i].subbands.size()) {
              nrSubbandsPerFile = settings.SAPs[i].subbands.size();
            }

            // Generate file list
            unsigned nrParts = max(1UL, ceilDiv(settings.SAPs[i].subbands.size(), nrSubbandsPerFile));
            tab.files.resize(stSettings.nrStokes * nrParts);
            for (size_t s = 0; s < stSettings.nrStokes; ++s) 
            {
              for (unsigned part = 0; part < nrParts; ++part)
              {
                struct ObservationSettings::BeamFormer::File file;

                file.streamNr = bfStreamNr++;
                file.sapNr    = i;
                file.tabNr    = j;
                file.stokesNr = s;
                file.partNr   = part;
                file.coherent = tab.coherent;

                if (file.coherent) {
                  file.coherentIdxInSAP = sap.nrCoherent - 1;
                  if (coherent_idx >= coherent_locations.size())
                    THROW(CoInterfaceException, "No CoherentStokes filename or location specified for file idx " << file.streamNr);
                  file.location = coherent_locations[coherent_idx++];
                } else {
                  file.incoherentIdxInSAP = sap.nrIncoherent - 1;
                  if (incoherent_idx >= incoherent_locations.size())
                    THROW(CoInterfaceException, "No IncoherentStokes filename or location specified for file idx " << file.streamNr);
                  file.location = incoherent_locations[incoherent_idx++];
                }

                file.firstSubbandIdx = settings.SAPs[i].subbands[0].idx +
                                       part * nrSubbandsPerFile;
                file.lastSubbandIdx  = min(file.firstSubbandIdx + nrSubbandsPerFile,
                                           // last file(s) in part series can have fewer subbands
                                           settings.SAPs[i].subbands[0].idx +
                                           settings.SAPs[i].subbands.size());
                ASSERTSTR(file.firstSubbandIdx < file.lastSubbandIdx,
                    "strmNr=" << file.streamNr << " 1stIdx=" << file.firstSubbandIdx << " lstIdx=" << file.lastSubbandIdx);
                ASSERTSTR(file.lastSubbandIdx <= settings.subbands.size(),
                    "strmNr=" << file.streamNr << " lstIdx=" << file.lastSubbandIdx << " nSb=" << settings.subbands.size());

                tab.files[s * nrParts + part] = file;
                settings.beamFormer.files.push_back(file);
                outputProcHosts.insert(file.location.host);
              }
            }
          }
        }

        settings.beamFormer.dedispersionFFTsize = getUint32("Cobalt.BeamFormer.dedispersionFFTsize", settings.blockSize);
      }

      // set output hosts
      settings.outputProcHosts.clear();
      for (set<string>::const_iterator i = outputProcHosts.begin(); i != outputProcHosts.end(); ++i) {
        // skip empty host names
        if (*i == "")
          continue;

        settings.outputProcHosts.push_back(*i);
      }

      return settings;
    }

    bool Parset::nodeReadsAntennaFieldData(const struct ObservationSettings& settings,
                                           const string& nodeName) const {
      for (size_t i = 0; i < settings.antennaFields.size(); ++i) {
        if (settings.antennaFields[i].receiver == nodeName)
          return true;
      }

      return false;
    }

    // pos and ref must each have at least size 3.
    double Parset::distanceVec3(const vector<double>& pos,
                                const vector<double>& ref) const {
      double dx = pos.at(0) - ref.at(0);
      double dy = pos.at(1) - ref.at(1);
      double dz = pos.at(2) - ref.at(2);
      return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // max delay distance in meters; static per obs, i.e. unprojected (some upper bound)
    double Parset::maxDelayDistance(const struct ObservationSettings& settings) const {
      // Available in each parset through included StationCalibration.parset.
      const vector<double> refPhaseCenter =
          settings.delayCompensation.referencePhaseCenter;

      double maxDelayDistance = 0.0;

      for (unsigned af = 0; af < settings.antennaFields.size(); af++) {
        vector<double> phaseCenter = settings.antennaFields[af].phaseCenter;
        double delayDist = distanceVec3(phaseCenter, refPhaseCenter);
        if (delayDist > maxDelayDistance)
          maxDelayDistance = delayDist;
      }

      return maxDelayDistance;
    }

    // Top frequency of highest subband observed in Hz.
    double Parset::maxObservationFrequency(const struct ObservationSettings& settings,
                                           double subbandWidth) const {
      double maxCentralFrequency = 0.0;

      for (unsigned sb = 0; sb < settings.subbands.size(); sb++) {
        if (settings.subbands[sb].centralFrequency > maxCentralFrequency)
          maxCentralFrequency = settings.subbands[sb].centralFrequency;
      }

      return maxCentralFrequency + 0.5 * subbandWidth;
    }

    // Determine the nr of channels per subband for delay compensation.
    // We aim for the visibility samples to be good to about 1 part in 1000.
    // See the Cobalt beamformer design doc for more info on how and why.
    unsigned Parset::calcNrDelayCompensationChannels(const struct ObservationSettings& settings) const {
      double d = maxDelayDistance(settings); // in meters
      if (d < 400.0)
        d = 400.0; // for e.g. CS002LBA only; CS001LBA-CS002LBA is ~441 m
      double nu_clk = settings.clockMHz * 1e6; // in Hz
      double subbandWidth = nu_clk / 1024.0;
      double nu = maxObservationFrequency(settings, subbandWidth); // in Hz
      if (nu < 10e6)
        nu = 10e6;

      // deltaPhi is the phase change over t_u in rad: ~= sqrt(24.0*1e-3) (Taylor approx)
      // Design doc states deltaPhi must be <= 0.155
      double deltaPhi = 0.15491933384829667540;
      const double omegaE = 7.29211585e-5; // sidereal angular velocity of Earth in rad/s
      const double speedOfLight = 299792458.0; // in vacuum in m/s
      double phi = 2.0 * M_PI * nu * omegaE / speedOfLight * d /* * cos(delta) (=1) */;

      // Fringe stopping of the residual delay is done at an interval t_u.
      double t_u = deltaPhi / phi;
      double max_n_FFT = t_u * subbandWidth;
      unsigned max_n_FFT_pow2 = roundUpToPowerOfTwo(((unsigned)max_n_FFT + 1) / 2); // round down to pow2

      // Little benefit beyond 256; more work and lower GPU FFT efficiency.
      if (max_n_FFT_pow2 > 256)
        max_n_FFT_pow2 = 256;

      // This lower bound comes from the derivation in the design doc.
      // It is pi*cbrt(2.0/(9.0*1e-3)) (also after Taylor approx).
      const double min_n_ch = 19.02884235042726617904; // design doc states n_ch >= 19
      const unsigned min_n_ch_pow2 = 32; // rounded up to pow2 for efficient FFT

      if (max_n_FFT_pow2 < min_n_ch_pow2) {
        LOG_WARN_STR("Parset: calcNrDelayCompensationChannels(): upper bound " <<
                     max_n_FFT << " ends up below lower bound " << min_n_ch <<
                     ". Returning " << min_n_ch_pow2 << ". Stations far from"
                     " the core may not be delay compensated optimally.");
        max_n_FFT_pow2 = min_n_ch_pow2;
      }

      return max_n_FFT_pow2;
    }

    size_t ObservationSettings::nrBlocks() const {
      return static_cast<size_t>(floor((stopTime - startTime) * subbandWidth() / blockSize));
    }


    double ObservationSettings::subbandWidth() const {
      return 1.0 * clockHz() / 1024;
    }


    double ObservationSettings::sampleDuration() const {
      return 1.0 / subbandWidth();
    }

    unsigned ObservationSettings::nrCrossPolarisations() const {
      return nrPolarisations * nrPolarisations;
    }

    double ObservationSettings::blockDuration() const {
      return blockSize * sampleDuration();
    }

    vector<unsigned> ObservationSettings::SAP::subbandIndices() const {
      vector<unsigned> indices;

      for (size_t i = 0; i < subbands.size(); ++i) {
        indices.push_back(subbands[i].idx);
      }

      return indices;
    }

    double ObservationSettings::Correlator::integrationTime() const {
      return 1.0 * nrSamplesPerIntegration() / channelWidth;
    }

    size_t ObservationSettings::Correlator::nrSamplesPerIntegration() const {
      return nrSamplesPerBlock / nrIntegrationsPerBlock * nrBlocksPerIntegration;
    }

    std::vector<struct ObservationSettings::FileLocation> Parset::getFileLocations(const std::string outputType) const {
      const string prefix = "Observation.DataProducts.Output_" + outputType;

      vector<string> empty;
      vector<string> filenames = getStringVector(prefix + ".filenames", empty, true);
      vector<string> locations = getStringVector(prefix + ".locations", empty, true);

      size_t numValidEntries = std::min(filenames.size(), locations.size());

      vector<struct ObservationSettings::FileLocation> result(numValidEntries);

      for (size_t i = 0; i < numValidEntries; ++i) {
        ObservationSettings::FileLocation &location = result[i];
        const vector<string> host_dir = StringUtil::split(locations[i], ':');

        if (host_dir.size() != 2) {
          THROW(CoInterfaceException, "Location must adhere to 'host:directory' in " << prefix << ".locations: " << locations[i]);
        }

        location.filename  = filenames[i];
        location.host      = host_dir[0];
        location.directory = host_dir[1];
      }

      return result;
    }


    bool ObservationSettings::BeamFormer::anyCoherentTABs() const
    {
      return enabled && maxNrCoherentTABsPerSAP() > 0;
    }


    bool ObservationSettings::BeamFormer::anyIncoherentTABs() const
    {
      return enabled && maxNrIncoherentTABsPerSAP() > 0;
    }


    size_t ObservationSettings::BeamFormer::maxNrTABsPerSAP() const
    {
      size_t max = 0;

      for (size_t sapNr = 0; sapNr < SAPs.size(); ++sapNr)
        max = std::max(max, SAPs[sapNr].TABs.size());

      return max;
    }


    size_t ObservationSettings::BeamFormer::maxNrCoherentTABsPerSAP() const
    {
      size_t max = 0;

      for (size_t sapNr = 0; sapNr < SAPs.size(); ++sapNr)
        max = std::max(max, SAPs[sapNr].nrCoherent);

      return max;
    }


    size_t ObservationSettings::BeamFormer::maxNrIncoherentTABsPerSAP() const
    {
      size_t max = 0;

      for (size_t sapNr = 0; sapNr < SAPs.size(); ++sapNr)
        max = std::max(max, SAPs[sapNr].nrIncoherent);

      return max;
    }


    void Parset::updateSettings()
    {
      settings = observationSettings();
    }


    void Parset::checkVectorLength(const std::string &key, unsigned expectedSize) const
    {
      unsigned actualSize = getStringVector(key, true).size();

      if (actualSize != expectedSize)
        THROW(CoInterfaceException, "Key \"" << string(key) << "\" contains wrong number of entries (expected: " << expectedSize << ", actual: " << actualSize << ')');
    }


    void Parset::checkInputConsistency() const
    {
    }

    void Parset::check() const
    {
    }


    std::string Parset::getHostName(OutputType outputType, unsigned streamNr) const
    {
      if (outputType == CORRELATED_DATA)
        return settings.correlator.files[streamNr].location.host;

      if (outputType == BEAM_FORMED_DATA)
        return settings.beamFormer.files[streamNr].location.host;

      return "unknown";
    }


    std::string Parset::getFileName(OutputType outputType, unsigned streamNr) const
    {
      if (outputType == CORRELATED_DATA)
        return settings.correlator.files[streamNr].location.filename;

      if (outputType == BEAM_FORMED_DATA)
        return settings.beamFormer.files[streamNr].location.filename;

      return "unknown";
    }


    std::string Parset::getDirectoryName(OutputType outputType, unsigned streamNr) const
    {
      if (outputType == CORRELATED_DATA)
        return settings.correlator.files[streamNr].location.directory;

      if (outputType == BEAM_FORMED_DATA)
        return settings.beamFormer.files[streamNr].location.directory;

      return "unknown";
    }


    unsigned Parset::nrStreams(OutputType outputType, bool force) const
    {
      if (!outputThisType(outputType) && !force)
        return 0;

      switch (outputType) {
      case CORRELATED_DATA:    return settings.correlator.files.size();
      case BEAM_FORMED_DATA:   return settings.beamFormer.files.size();
      default:                 THROW(CoInterfaceException, "Unknown output type");
      }
    }

    size_t Parset::nrBytesPerComplexSample() const
    {
      return 2 * nrBitsPerSample() / 8;
    }


    std::vector<double> Parset::centroidPos(const std::string &stations) const
    {
      std::vector<double> Centroid, posList, pos;
      Centroid.resize(3);

      vector<string> stationList = StringUtil::split(stations, '+');
      for (unsigned i = 0; i < stationList.size(); i++)
      {
        pos = position(stationList[i]);
        posList.insert(posList.end(), pos.begin(), pos.end());
      }

      for (unsigned i = 0; i < posList.size() / 3; i++)
      {
        Centroid[0] += posList[3 * i]; // x in m
        Centroid[1] += posList[3 * i + 1]; // y in m
        Centroid[2] += posList[3 * i + 2]; // z in m
      }

      Centroid[0] /= posList.size() / 3;
      Centroid[1] /= posList.size() / 3;
      Centroid[2] /= posList.size() / 3;

      return Centroid;
    }


    vector<double> Parset::position( const std::string &name ) const
    {
      const string positionKey    = "PIC.Core." + name + ".position";
      const string phaseCenterKey = "PIC.Core." + name + ".phaseCenter";

      if (isDefined(positionKey))
        return getDoubleVector(positionKey, true);
      else
        return getDoubleVector(phaseCenterKey, true);
    }


    MultiDimArray<double,2> Parset::positions() const
    {
      const vector<ObservationSettings::Correlator::Station> &stations = settings.correlator.stations;

      MultiDimArray<double,2> list(boost::extents[stations.size()][3]);

      for (size_t i = 0; i < stations.size(); i++) {
        const string &name = stations[i].name;
        vector<double> pos;

        if (name.find("+") != string::npos)
          pos = centroidPos(name); // super station
        else
          pos = position(name);

        ASSERT(pos.size() == 3);

        list[i][0] = pos[0];
        list[i][1] = pos[1];
        list[i][2] = pos[2];
      }

      return list;
    }

    double Parset::getTime(const std::string &name, const std::string &defaultValue) const
    {
      return LOFAR::to_time_t(boost::posix_time::time_from_string(getString(name, defaultValue)));
    }

    std::string Parset::name() const
    {
      return itsName;
    }

    ssize_t ObservationSettings::antennaFieldIndex(const std::string &name) const
    {
      for (size_t a = 0; a < antennaFields.size(); ++a) {
        if (antennaFields[a].name == name)
          return a;
      }

      return -1;
    }

    // TODO: rename allStationNames to allAntennaFieldNames
    std::vector<std::string> Parset::allStationNames() const
    {
      vector<string> names(settings.antennaFields.size());

      for (unsigned af = 0; af < names.size(); ++af)
        names[af] = settings.antennaFields[af].name;

      return names;
    }

    unsigned Parset::nrTabStations() const
    {
      return settings.correlator.stations.size();
    }

    std::vector<std::string> Parset::mergedStationNames() const
    {
      std::vector<string> tabStations;

      for (size_t i = 0; i < settings.correlator.stations.size(); ++i)
        tabStations.push_back(settings.correlator.stations[i].name);

      return tabStations;
    }

    unsigned Parset::nrMergedStations() const
    {
      return settings.correlator.stations.size();
    }

    unsigned Parset::nrBaselines() const
    {
      size_t stations = settings.correlator.stations.size();

      return stations * (stations + 1) / 2;
    }

    double Parset::sampleDuration() const
    {
      return 1.0 / settings.subbandWidth();
    }

    unsigned Parset::dedispersionFFTsize() const
    {
      return settings.beamFormer.dedispersionFFTsize;
    }

    unsigned Parset::nrBitsPerSample() const
    {
      return settings.nrBitsPerSample;
    }

    bool Parset::outputThisType(OutputType outputType) const
    {
      switch (outputType) {
      case CORRELATED_DATA:   return settings.correlator.enabled;
      case BEAM_FORMED_DATA:  return settings.beamFormer.enabled;
      default:                THROW(CoInterfaceException, "Unknown output type");
      }
    }

    string Parset::positionType() const
    {
      return "ITRF";
    }

    double Parset::channel0Frequency(size_t subband, size_t nrChannels) const
    {
      const double sbFreq = settings.subbands[subband].centralFrequency;

      if (nrChannels == 1)
        return sbFreq;

      // if the 2nd PPF is used, the subband is shifted half a channel
      // downwards, so subtracting half a subband results in the
      // center of channel 0 (instead of the bottom).
      return sbFreq - 0.5 * settings.subbandWidth();
    }

    string Parset::PVSS_TempObsName() const
    {
      return getString("_DPname", "LOFAR_ObsSW_TempObs_0001");
    }
  } // namespace Cobalt
} // namespace LOFAR
