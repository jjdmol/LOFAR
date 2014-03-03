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


    static StokesType stokesType( const std::string &name )
    {
      if (name == "I")
        return STOKES_I;

      if (name == "IQUV")
        return STOKES_IQUV;

      if (name == "XXYY")
        return STOKES_XXYY;

      return INVALID_STOKES;
    }


    static size_t nrStokes( StokesType type)
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


    static string stokesType( StokesType type )
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


    Parset::Parset(Stream *stream)
    {
      // Read size
      uint64 size;
      stream->read(&size, sizeof size);

#if !defined WORDS_BIGENDIAN
      dataConvert(LittleEndian, &size, 1);
#endif

      // Read data
      std::vector<char> tmp(size + 1);
      stream->read(&tmp[0], size);
      tmp[size] = '\0';

      // Add data to parset
      std::string buffer(&tmp[0], size);
      adoptBuffer(buffer);

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

#if !defined WORDS_BIGENDIAN
      uint64 size_be = size;
      dataConvert(BigEndian, &size_be, 1);
      stream->write(&size_be, sizeof size_be);
#else
      stream->write(&size, sizeof size);
#endif

      stream->write(buffer.data(), size);
    }


    vector<struct ObservationSettings::AntennaFieldName> ObservationSettings::antennaFields(const vector<string> &stations, const string &antennaSet) {
      vector<struct AntennaFieldName> result;

      for (vector<string>::const_iterator i = stations.begin(); i != stations.end(); ++i) {
        const string &station = *i;

        bool coreStation = station.substr(0,2) == "CS";

        if (station.length() != 5) {
          // Backward compatibility: the key
          // Observation.VirtualInstrument.stationList can contain full
          // antennafield names such as CS001LBA.
          LOG_WARN_STR("Warning: old (preparsed) station name: " << station);

          // Do not assume the standard station name format (sily "S9").
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

    std::string Parset::renamedKey(const std::string &newname, const std::string &oldname) const {
      if (isDefined(newname))
        return newname;

      if (isDefined(oldname)) {
        LOG_WARN_STR("Parset: key " << oldname << " is deprecated. Please use " << newname << " instead.");
        return oldname;
      }

      return newname;
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
      settings.realTime = getBool(renamedKey("Cobalt.realTime", "OLAP.realTime"), false);
      settings.observationID = getUint32("Observation.ObsID", 0);
      settings.startTime = getTime("Observation.startTime", "2013-01-01 00:00:00");
      settings.stopTime  = getTime("Observation.stopTime",  "2013-01-01 00:01:00");
      settings.clockMHz = getUint32("Observation.sampleClock", 200);

      settings.nrBitsPerSample = getUint32(renamedKey("Observation.nrBitsPerSample","OLAP.nrBitsPerSample"), 16);

      settings.nrPolarisations = 2;

      settings.corrections.bandPass   = getBool(renamedKey("Cobalt.correctBandPass", "OLAP.correctBandPass"), true);
      settings.corrections.clock      = getBool(renamedKey("Cobalt.correctClocks", "OLAP.correctClocks"), true);
      settings.corrections.dedisperse = getBool(renamedKey("Cobalt.BeamFormer.coherentDedisperseChannels", "OLAP.coherentDedisperseChannels"), true);

      settings.delayCompensation.enabled              = getBool(renamedKey("Cobalt.delayCompensation", "OLAP.delayCompensation"), true);
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

      if (isDefined("Cobalt.blockSize")) {
        settings.blockSize = getUint32("Cobalt.blockSize", static_cast<size_t>(1.0 * settings.subbandWidth()));
      } else {
        // Old, fall-back configuration
        settings.blockSize = getUint32("OLAP.CNProc.integrationSteps", 3052) * getUint32("Observation.channelsPerSubband", 64);
      }

      // Station information (used pointing information to verify settings)
      vector<string> stations = getStringVector("Observation.VirtualInstrument.stationList", emptyVectorString, true);

      // Sort stations (CS, RS, intl), to get a consistent and predictable
      // order in the MeasurementSets.
      std::sort(stations.begin(), stations.end(), compareStationNames);

      vector<ObservationSettings::AntennaFieldName> fieldNames = ObservationSettings::antennaFields(stations, settings.antennaSet);

      size_t nrStations = fieldNames.size();

      settings.stations.resize(nrStations);
      for (unsigned i = 0; i < nrStations; ++i) {
        struct ObservationSettings::Station &station = settings.stations[i];

        station.name              = fieldNames[i].fullName();
        station.inputStreams      = getStringVector(
            renamedKey(str(format("PIC.Core.%s.RSP.ports") % station.name),
                       str(format("PIC.Core.Station.%s.RSP.ports") % station.name)),
            emptyVectorString, true);
        station.receiver          = getString(str(format("PIC.Core.%s.RSP.receiver") % station.name), "");

        // NOTE: Support for clockCorrectionTime can be phased out when the
        // BG/P is gone. delay.X and delay.Y are superior to it, being
        // polarisation specific.
        station.clockCorrection   = getDouble(str(format("PIC.Core.%s.clockCorrectionTime") % station.name), 0.0);
        station.phaseCenter = getDoubleVector(str(format("PIC.Core.%s.phaseCenter") % station.name), vector<double>(3, 0), true);
        if (station.phaseCenter == emptyVectorDouble)
          LOG_WARN_STR("Parset: PIC.Core." << station.name << ".phaseCenter is missing (or (0.0, 0.0, 0.0)).");
        station.phase0.x = getDouble(str(format("PIC.Core.%s.%s.%s.phase0.X") % fieldNames[i].fullName() % settings.antennaSet % settings.bandFilter), 0.0);
        station.phase0.y = getDouble(str(format("PIC.Core.%s.%s.%s.phase0.Y") % fieldNames[i].fullName() % settings.antennaSet % settings.bandFilter), 0.0);
        station.delay.x = getDouble(str(format("PIC.Core.%s.%s.%s.delay.X") % fieldNames[i].fullName() % settings.antennaSet % settings.bandFilter), 0.0);
        station.delay.y = getDouble(str(format("PIC.Core.%s.%s.%s.delay.Y") % fieldNames[i].fullName() % settings.antennaSet % settings.bandFilter), 0.0);

        if (station.delay.x > 0.0 || station.delay.y > 0.0) {
          if (station.clockCorrection != 0.0) {
            // Ignore clockCorrectionTime if delay.X or delay.Y are specified.

            station.clockCorrection = 0.0;
            LOG_WARN_STR("Ignoring PIC.Core." << station.name << ".clockCorrectionTime in favor of PIC.Core." << fieldNames[i].fullName() << "." << settings.antennaSet << "." << settings.bandFilter << ".delay.{X,Y}");
          }
        }

        string key = std::string(str(format("Observation.Dataslots.%s.RSPBoardList") % station.name));
        if (!isDefined(key)) key = "Observation.rspBoardList";
        station.rspBoardMap = getUint32Vector(key, emptyVectorUnsigned, true);

        ASSERTSTR(station.rspBoardMap.size() >= settings.subbands.size(), "Observation has " << settings.subbands.size() << " subbands, but station " << station.name << " has only board numbers defined for " << station.rspBoardMap.size() << " subbands. Please correct either Observation.rspBoardList or Observation.Dataslots." << station.name << ".RSPBoardList" );

        key = std::string(str(format("Observation.Dataslots.%s.DataslotList") % station.name));
        if (!isDefined(key)) key = "Observation.rspSlotList";
        station.rspSlotMap = getUint32Vector(key, emptyVectorUnsigned, true);

        ASSERTSTR(station.rspSlotMap.size() >= settings.subbands.size(), "Observation has " << settings.subbands.size() << " subbands, but station " << station.name << " has only board numbers defined for " << station.rspSlotMap.size() << " subbands. Please correct either Observation.rspSlotList or Observation.Dataslots." << station.name << ".DataslotList" );
      }

      // Resource information
      vector<string> nodes = getStringVector("Cobalt.Nodes", emptyVectorString, true);
      settings.nodes.resize(nodes.size());

      for (size_t i = 0; i < nodes.size(); ++i) {
        struct ObservationSettings::Node &node = settings.nodes[i];

        node.rank     = i;
        node.name     = nodes[i];

        string prefix = str(format("PIC.Core.Cobalt.%s.") % node.name);

        node.hostName = getString(prefix + "host", "localhost");
        node.cpu      = getUint32(prefix + "cpu",  0);
        node.nic      = getString(prefix + "nic",  "");
        node.gpus     = getUint32Vector(prefix + "gpus", vector<unsigned>(1,0)); // default to [0]
      }

      /* ===============================
       * Correlator pipeline information
       * ===============================
       */

      settings.correlator.enabled = getBool("Observation.DataProducts.Output_Correlated.enabled", false);
      if (settings.correlator.enabled) {
        settings.correlator.nrChannels = getUint32(renamedKey("Cobalt.Correlator.nrChannelsPerSubband", "Observation.channelsPerSubband"), 64);
        //settings.correlator.nrChannels = getUint32("Observation.channelsPerSubband", 64);
        settings.correlator.channelWidth = settings.subbandWidth() / settings.correlator.nrChannels;
        settings.correlator.nrSamplesPerChannel = settings.blockSize / settings.correlator.nrChannels;
        settings.correlator.nrBlocksPerIntegration = getUint32(renamedKey("Cobalt.Correlator.nrBlocksPerIntegration", "OLAP.IONProc.integrationSteps"), 1);
        settings.correlator.nrBlocksPerObservation = static_cast<size_t>(floor((settings.stopTime - settings.startTime) / settings.correlator.integrationTime()));

        // super-station beam former
        //
        // TODO: Super-station beam former is unused, so will likely be
        // implemented differently. The code below is only there to show how
        // the OLAP.* keys used to be interpreted.

        // OLAP.CNProc.tabList[i] = j <=> superstation j contains (input) station i
        vector<unsigned> tabList = getUint32Vector("OLAP.CNProc.tabList", emptyVectorUnsigned, true);

        // Names for all superstations, including those that are simple copies
        // of (input) stations.
        vector<string> tabNames = getStringVector("OLAP.tiedArrayStationNames", emptyVectorString, true);

        if (tabList.empty()) {
          // default: input station list = output station list
          settings.correlator.stations.resize(settings.stations.size());
          for (size_t i = 0; i < settings.correlator.stations.size(); ++i) {
            struct ObservationSettings::Correlator::Station &station = settings.correlator.stations[i];

            station.name = settings.stations[i].name;
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

      settings.beamFormer.enabled =
           getBool("Observation.DataProducts.Output_CoherentStokes.enabled", false)
        || getBool("Observation.DataProducts.Output_IncoherentStokes.enabled", false);
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

        settings.beamFormer.doFlysEye = 
          getBool("OLAP.PencilInfo.flysEye", false);

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

        for (unsigned i = 0; i < 2; ++i) {
          // Set coherent and incoherent Stokes settings by
          // iterating twice.

          string oldprefix = "";
          string newprefix = "";
          struct ObservationSettings::BeamFormer::StokesSettings *set = 0;
          
          // Select coherent or incoherent for this iteration
          switch(i) {
            case 0:
              oldprefix = "OLAP.CNProc_CoherentStokes";
              newprefix = "Cobalt.BeamFormer.CoherentStokes";
              set = &settings.beamFormer.coherentSettings;
              set->coherent = true;
              break;

            case 1:
              oldprefix = "OLAP.CNProc_IncoherentStokes";
              newprefix = "Cobalt.BeamFormer.IncoherentStokes";
              set = &settings.beamFormer.incoherentSettings;
              set->coherent = false;
              break;

            default:
              ASSERT(false);
              break;
          }

          // Obtain settings of selected stokes
          set->type = stokesType(getString(
                renamedKey(newprefix + ".which", oldprefix + ".which"),
                "I"));
          set->nrStokes = nrStokes(set->type);
          set->nrChannels = getUint32(
                renamedKey(newprefix + ".nrChannelsPerSubband", oldprefix + ".channelsPerSubband"),
                1);
          ASSERT(set->nrChannels > 0);
          set->timeIntegrationFactor = getUint32(
                renamedKey(newprefix + ".timeIntegrationFactor", oldprefix + ".timeIntegrationFactor"),
                1);
          ASSERT(set->timeIntegrationFactor > 0);
          set->nrSubbandsPerFile = getUint32(
                renamedKey(newprefix + ".subbandsPerFile", oldprefix + ".subbandsPerFile"),
                0);
          set->nrSamples = settings.blockSize / set->timeIntegrationFactor / set->nrChannels;

          if (set->nrSubbandsPerFile == 0) {
            // apply default
            set->nrSubbandsPerFile = settings.subbands.size();
          }

          ASSERTSTR(set->nrSubbandsPerFile >= settings.subbands.size(), "Multiple parts/file are not yet supported!");
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

          size_t nrTABs    = getUint32(str(format("Observation.Beam[%u].nrTiedArrayBeams") % i), 0);
          size_t nrTABSParset = nrTABs;
          size_t nrRings   = getUint32(str(format("Observation.Beam[%u].nrTabRings") % i), 0);
          double ringWidth = getDouble(str(format("Observation.Beam[%u].ringWidth") % i), 0.0);

          // Create ptr to RingCoordinates object
          // If there are tab rings the object will be created
          // The actual tabs will be extracted after we added all manual tabs
          // But we need the number of tabs from rings at this location
          std::auto_ptr<RingCoordinates> ptrRingCoords;
          if (nrRings != 0)
          {
            const string prefix = str(format("Observation.Beam[%u]") % i);
            string directionType = getString(prefix + ".directionType", "J2000");
            
            double angle1 = getDouble(prefix + ".angle1", 0.0);
            double angle2 = getDouble(prefix + ".angle2", 0.0);

            // Convert to COORDTYPE default == OTHER
            RingCoordinates::COORDTYPES type = RingCoordinates::OTHER;
            if (directionType == "J2000")
              type = RingCoordinates::J2000;
            else if (directionType == "B1950")
              type = RingCoordinates::B1950;
              

            ptrRingCoords = std::auto_ptr<RingCoordinates>(
              new RingCoordinates(nrRings, ringWidth,
              RingCoordinates::Coordinate(angle1, angle2), type));

            nrTABs = nrTABSParset + (*ptrRingCoords).nCoordinates();
          }         
          else if (settings.beamFormer.doFlysEye) 
          // For Fly's Eye mode we have exactly one TAB per station.
          {
           nrTABs = nrStations;
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
              if (j < nrTABSParset)
              {
                const string prefix = str(format("Observation.Beam[%u].TiedArrayBeam[%u]") % i % j);
                tab.direction.type    = getString(prefix + ".directionType", "J2000");
              
                tab.direction.angle1  = getDouble(renamedKey(prefix + ".absoluteAngle1",
                                                             prefix + ".angle1"), 0.0);
                tab.direction.angle2  = getDouble(renamedKey(prefix + ".absoluteAngle2",
                                                             prefix + ".angle2"), 0.0);

                // Always store absolute angles. So this is for backwards compat.
                if (!isDefined(prefix + ".absoluteAngle1"))
                  tab.direction.angle1 += settings.SAPs[i].direction.angle1;
                if (!isDefined(prefix + ".absoluteAngle2"))
                  tab.direction.angle2 += settings.SAPs[i].direction.angle2;

                tab.dispersionMeasure     = getDouble(prefix + ".dispersionMeasure", 0.0);
                tab.coherent              = getBool(prefix + ".coherent", true);
              }
              else
              {
                // Get the pointing, start counting at zero
                // TODO What happens if the number does not match?
                RingCoordinates::Coordinate pointing = 
                    ptrRingCoords->coordinates()[j - nrTABSParset];


                tab.direction.type = ptrRingCoords->coordTypeAsString();
                tab.direction.angle1 = pointing.first;
                tab.direction.angle2 = pointing.second;
                // Cannot search for the absolute angle for an entry that does not exist
                // TODO is this still the correct key?
                tab.dispersionMeasure = getInt("OLAP.dispersionMeasure", 0);
                tab.coherent =  true;  // always coherent


              }
            }

            if (tab.coherent)
              sap.nrCoherent++;
            else
              sap.nrIncoherent++;

            struct ObservationSettings::BeamFormer::StokesSettings &set =
               tab.coherent ? settings.beamFormer.coherentSettings
                            : settings.beamFormer.incoherentSettings;

            // Generate file list
            tab.files.resize(set.nrStokes);
            for (size_t s = 0; s < set.nrStokes; ++s) 
            {
              struct ObservationSettings::BeamFormer::File file;

              file.sapNr    = i;
              file.tabNr    = j;
              file.coherent = tab.coherent;
              file.stokesNr = s;
              file.streamNr = bfStreamNr++;

              if (file.coherent) 
              {
                file.coherentIdxInSAP = sap.nrCoherent - 1;

                if (coherent_idx >= coherent_locations.size())
                  THROW(CoInterfaceException, "No CoherentStokes filename or location specified for file " << file.streamNr);
                file.location = coherent_locations[coherent_idx++];
              } 
              else 
              {
                file.incoherentIdxInSAP = sap.nrIncoherent - 1;

                if (incoherent_idx >= incoherent_locations.size())
                  THROW(CoInterfaceException, "No IncoherentStokes filename or location specified for file " << file.streamNr);
                file.location = incoherent_locations[incoherent_idx++];
              }

              tab.files[s] = file;
              settings.beamFormer.files.push_back(file);

              outputProcHosts.insert(file.location.host);
            }
          }         
        }

        settings.beamFormer.dedispersionFFTsize = getUint32(renamedKey("Cobalt.BeamFormer.dedispersionFFTsize", "OLAP.CNProc.dedispersionFFTsize"), settings.correlator.nrSamplesPerChannel);
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

      for (unsigned st = 0; st < settings.stations.size(); st++) {
        vector<double> phaseCenter = settings.stations[st].phaseCenter;
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
        LOG_ERROR_STR("Parset: calcNrDelayCompensationChannels(): upper bound " <<
                      max_n_FFT << " ends up below lower bound " << min_n_ch <<
                      ". Returning " << min_n_ch_pow2 << ". Stations far from"
                      " the core may not be delay compensated optimally.");
        max_n_FFT_pow2 = min_n_ch_pow2;
      }

      return max_n_FFT_pow2;
    }


    double ObservationSettings::subbandWidth() const {
      return 1.0 * clockHz() / 1024;
    }

    unsigned ObservationSettings::nrCrossPolarisations() const {
      return nrPolarisations * nrPolarisations;
    }

    size_t ObservationSettings::nrSamplesPerSubband() const {
      return blockSize;
    }

    double ObservationSettings::blockDuration() const {
      return nrSamplesPerSubband() / subbandWidth();
    }

    vector<unsigned> ObservationSettings::SAP::subbandIndices() const {
      vector<unsigned> indices;

      for(size_t i = 0; i < subbands.size(); ++i) {
        indices.push_back(subbands[i].idx);
      }

      return indices;
    }

    double ObservationSettings::Correlator::integrationTime() const {
      return 1.0 * nrSamplesPerChannel * nrBlocksPerIntegration / channelWidth;
    }

    std::vector<struct ObservationSettings::FileLocation> Parset::getFileLocations(const std::string outputType) const {
      //
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


    bool Parset::correctClocks() const
    {
      return settings.corrections.clock;
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


    unsigned Parset::nrBeams() const
    {
      return settings.SAPs.size();
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
      return to_time_t(boost::posix_time::time_from_string(getString(name, defaultValue)));
    }

    std::string Parset::name() const
    {
      return itsName;
    }

    unsigned Parset::observationID() const
    {
      return settings.observationID;
    }

    double Parset::startTime() const
    {
      return settings.startTime;
    }

    double Parset::stopTime() const
    {
      return settings.stopTime;
    }

    unsigned Parset::nrCorrelatedBlocks() const
    {
      return settings.correlator.nrBlocksPerObservation;
    }

    unsigned Parset::nrBeamFormedBlocks() const
    {
      return static_cast<unsigned>(floor( (stopTime() - startTime()) / CNintegrationTime()));
    }

    ssize_t ObservationSettings::stationIndex(const std::string &name) const
    {
      for (size_t station = 0; station < stations.size(); ++station) {
        if (stations[station].name == name)
          return station;
      }

      return -1;
    }

    std::vector<std::string> Parset::allStationNames() const
    {
      vector<string> names(nrStations());

      for (unsigned station = 0; station < names.size(); ++station)
        names[station] = settings.stations[station].name;

      return names;
    }

    unsigned Parset::nrStations() const
    {
      return settings.stations.size();
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

    unsigned Parset::nrCrossPolarisations() const
    {
      return settings.nrCrossPolarisations();
    }

    unsigned Parset::clockSpeed() const
    {
      return settings.clockHz();
    }

    double Parset::subbandBandwidth() const
    {
      return settings.subbandWidth();
    }

    double Parset::sampleDuration() const
    {
      return 1.0 / subbandBandwidth();
    }

    unsigned Parset::dedispersionFFTsize() const
    {
      return settings.beamFormer.dedispersionFFTsize;
    }

    unsigned Parset::nrBitsPerSample() const
    {
      return settings.nrBitsPerSample;
    }

    unsigned Parset::CNintegrationSteps() const
    {
      return settings.correlator.nrSamplesPerChannel;
    }

    unsigned Parset::IONintegrationSteps() const
    {
      return settings.correlator.nrBlocksPerIntegration;
    }

    unsigned Parset::integrationSteps() const
    {
      return CNintegrationSteps() * IONintegrationSteps();
    }

    bool Parset::outputThisType(OutputType outputType) const
    {
      switch (outputType) {
      case CORRELATED_DATA:   return settings.correlator.enabled;
      case BEAM_FORMED_DATA:  return settings.beamFormer.enabled;
      default:                THROW(CoInterfaceException, "Unknown output type");
      }
    }

    double Parset::CNintegrationTime() const
    {
      return nrSamplesPerSubband() / subbandBandwidth();
    }

    double Parset::IONintegrationTime() const
    {
      return settings.correlator.integrationTime();
    }

    unsigned Parset::nrSamplesPerSubband() const
    {
      return settings.nrSamplesPerSubband();
    }

    unsigned Parset::nrSamplesPerChannel() const
    {
      return settings.correlator.enabled ? settings.correlator.nrSamplesPerChannel : 0;
    }

    unsigned Parset::nrChannelsPerSubband() const
    {
      return settings.correlator.enabled ? settings.correlator.nrChannels : 0;
    }

    size_t Parset::nrSubbands() const
    {
      return settings.subbands.size();
    }

    double Parset::channelWidth() const
    {
      return settings.correlator.channelWidth;
    }

    bool Parset::delayCompensation() const
    {
      return settings.delayCompensation.enabled;
    }

    string Parset::positionType() const
    {
      return "ITRF";
    }

    bool Parset::correctBandPass() const
    {
      return settings.corrections.bandPass;
    }

    double Parset::channel0Frequency(size_t subband, size_t nrChannels) const
    {
      const double sbFreq = settings.subbands[subband].centralFrequency;

      if (nrChannels == 1)
        return sbFreq;

      // if the 2nd PPF is used, the subband is shifted half a channel
      // downwards, so subtracting half a subband results in the
      // center of channel 0 (instead of the bottom).
      return sbFreq - 0.5 * subbandBandwidth();
    }

    bool Parset::realTime() const
    {
      return settings.realTime;
    }

    string Parset::bandFilter() const
    {
      return settings.bandFilter;
    }

    string Parset::antennaSet() const
    {
      return settings.antennaSet;
    }

    string Parset::PVSS_TempObsName() const
    {
      return getString("_DPname","");
    }


    Parset Parset::getGlobalLTAFeedbackParameters() const
    {
      Parset ps;

      // for MoM, to discriminate between Cobalt and BG/P observations
      ps.add("_isCobalt", "T");

      ps.add("Observation.DataProducts.nrOfOutput_Beamformed_", 
             str(format("%u") % nrStreams(BEAM_FORMED_DATA)));
      ps.add("Observation.DataProducts.nrOfOutput_Correlated_", 
             str(format("%u") % nrStreams(CORRELATED_DATA)));

      if (settings.correlator.enabled) {
        ps.add("Observation.Correlator.integrationInterval",
               str(format("%.16g") % settings.correlator.integrationTime()));
      }

      if (settings.beamFormer.enabled) {
        // For Cobalt, we do not collapse channels. There's no need to do so,
        // because we can do the final PPF/FFT on the desired number of output
        // channels. The BlueGene, on the other hand, uses a fixed PPF/FFT size,
        // so that the number of channels has to be reduced in the final step.
        // As a result, `rawSamplingTime` is identical to `samplingTime`,
        // `nrOfCollapsedChannels` is equal to the number of output channels per
        // subband, and `frequencyDownsamplingFactor` is always 1.
        const ObservationSettings::BeamFormer::StokesSettings&
          coherentStokes = settings.beamFormer.coherentSettings;
        const ObservationSettings::BeamFormer::StokesSettings&
          incoherentStokes = settings.beamFormer.incoherentSettings;
        ps.add("Observation.CoherentStokes.rawSamplingTime",
               str(format("%.16g") % 
                   (sampleDuration() * coherentStokes.nrChannels)));
        ps.add("Observation.IncoherentStokes.rawSamplingTime",
               str(format("%.16g") % 
                   (sampleDuration() * incoherentStokes.nrChannels)));
        ps.add("Observation.CoherentStokes.samplingTime",
               str(format("%.16g") % 
                   (sampleDuration() * coherentStokes.nrChannels)));
        ps.add("Observation.IncoherentStokes.samplingTime",
               str(format("%.16g") % 
                   (sampleDuration() * incoherentStokes.nrChannels)));
        ps.add("Observation.CoherentStokes.timeDownsamplingFactor",
               str(format("%.16g") % coherentStokes.timeIntegrationFactor));
        ps.add("Observation.IncoherentStokes.timeDownsamplingFactor",
               str(format("%.16g") % incoherentStokes.timeIntegrationFactor));
        ps.add("Observation.CoherentStokes.nrOfCollapsedChannels",
               str(format("%u") % coherentStokes.nrChannels));
        ps.add("Observation.IncoherentStokes.nrOfCollapsedChannels",
               str(format("%u") % incoherentStokes.nrChannels));
        ps.add("Observation.CoherentStokes.frequencyDownsamplingFactor", "1");
        ps.add("Observation.IncoherentStokes.frequencyDownsamplingFactor", "1");
        ps.add("Observation.CoherentStokes.stokes",
               stokesType(coherentStokes.type));
        ps.add("Observation.IncoherentStokes.stokes",
               stokesType(incoherentStokes.type));
        ps.add("Observation.CoherentStokes.antennaSet",
               settings.antennaSet);
        ps.add("Observation.IncoherentStokes.antennaSet",
               settings.antennaSet);
        ps.add("Observation.CoherentStokes.stationList",
               get("Observation.VirtualInstrument.stationList"));
        ps.add("Observation.IncoherentStokes.stationList",
               get("Observation.VirtualInstrument.stationList"));
      }
      return ps;
    }


    size_t ObservationSettings::BeamFormer::SAP::nrCoherentTAB() const
    {
      return nrCoherent;
    }

    size_t ObservationSettings::BeamFormer::SAP::nrIncoherentTAB() const
    {
      return nrIncoherent;
    }
  } // namespace Cobalt
} // namespace LOFAR
