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

#include <cstdio>
#include <set>
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
          result.push_back(AntennaFieldName(station.substr(0,5), station.substr(5)));
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


    struct ObservationSettings Parset::observationSettings() const
    {
      struct ObservationSettings settings;

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
      settings.corrections.dedisperse = getBool(renamedKey("Cobalt.Beamformer.coherentDedisperseChannels", "OLAP.coherentDedisperseChannels"), true);

      settings.delayCompensation.enabled              = getBool(renamedKey("Cobalt.delayCompensation", "OLAP.delayCompensation"), true);
      settings.delayCompensation.referencePhaseCenter = getDoubleVector("Observation.referencePhaseCenter", emptyVectorDouble, true);

      settings.nrPPFTaps = 16;

      // Station information
      settings.antennaSet     = getString("Observation.antennaSet", "LBA");
      settings.bandFilter     = getString("Observation.bandFilter", "LBA_30_70");

      vector<string> stations = getStringVector("Observation.VirtualInstrument.stationList", emptyVectorString, true);

      vector<ObservationSettings::AntennaFieldName> fieldNames = ObservationSettings::antennaFields(stations, settings.antennaSet);

      size_t nrStations = fieldNames.size();

      settings.stations.resize(nrStations);
      for (unsigned i = 0; i < nrStations; ++i) {
        struct ObservationSettings::Station &station = settings.stations[i];

        station.name              = fieldNames[i].fullName();
        station.clockCorrection   = getDouble(str(format("PIC.Core.%s.clockCorrectionTime") % station.name), 0.0);
        station.phaseCenter       = getDoubleVector(str(format("PIC.Core.%s.phaseCenter") % station.name), emptyVectorDouble, true);
        station.phaseCorrection.x = getDouble(str(format("PIC.Core.%s.%s.%s.phaseCorrection.X") % fieldNames[i].station % settings.antennaSet % settings.bandFilter), 0.0);
        station.phaseCorrection.y = getDouble(str(format("PIC.Core.%s.%s.%s.phaseCorrection.Y") % fieldNames[i].station % settings.antennaSet % settings.bandFilter), 0.0);
        station.delayCorrection.x = getDouble(str(format("PIC.Core.%s.%s.%s.delayCorrection.X") % fieldNames[i].station % settings.antennaSet % settings.bandFilter), 0.0);
        station.delayCorrection.y = getDouble(str(format("PIC.Core.%s.%s.%s.delayCorrection.Y") % fieldNames[i].station % settings.antennaSet % settings.bandFilter), 0.0);

        string key = std::string(str(format("Observation.Dataslots.%s.RSPBoardList") % station.name));
        if (!isDefined(key)) key = "Observation.rspBoardList";
        station.rspBoardMap = getUint32Vector(key, emptyVectorUnsigned, true);

        key = std::string(str(format("Observation.Dataslots.%s.DataslotList") % station.name));
        if (!isDefined(key)) key = "Observation.rspSlotList";
        station.rspSlotMap = getUint32Vector(key, emptyVectorUnsigned, true);
      }

      // Pointing information
      size_t nrSAPs = getUint32("Observation.nrBeams", 1);
      unsigned subbandOffset = 512 * (settings.nyquistZone() - 1);
      
      settings.SAPs.resize(nrSAPs);
      settings.subbands.clear();
      for (unsigned sapNr = 0; sapNr < nrSAPs; ++sapNr) {
        struct ObservationSettings::SAP &sap = settings.SAPs[sapNr];

        sap.direction.type   = getString(str(format("Observation.Beam[%u].directionType") % sapNr), "J2000");
        sap.direction.angle1 = getDouble(str(format("Observation.Beam[%u].angle1") % sapNr), 0.0);
        sap.direction.angle2 = getDouble(str(format("Observation.Beam[%u].angle2") % sapNr), 0.0);
        sap.target           = getString(str(format("Observation.Beam[%u].target") % sapNr), "");

        // Process the subbands of this SAP
        vector<unsigned> subbandList = getUint32Vector(str(format("Observation.Beam[%u].subbandList") % sapNr), emptyVectorUnsigned, true);
        vector<double> frequencyList = getDoubleVector(str(format("Observation.Beam[%u].frequencyList") % sapNr), emptyVectorDouble, true);

        for (unsigned sb = 0; sb < subbandList.size(); ++sb) {
          struct ObservationSettings::Subband subband;

          subband.idx              = settings.subbands.size();
          subband.stationIdx       = subbandList[sb];
          subband.SAP              = sapNr;
          subband.centralFrequency = frequencyList.empty()
                                     ? settings.subbandWidth() * (subband.stationIdx + subbandOffset)
                                     : frequencyList[sb];

          settings.subbands.push_back(subband);
        }
      }

      settings.anaBeam.enabled = settings.antennaSet.substr(0,3) == "HBA";
      if (settings.anaBeam.enabled) {
        settings.anaBeam.direction.type   = getString("Observation.AnaBeam[0].directionType", "J2000");
        settings.anaBeam.direction.angle1 = getDouble("Observation.AnaBeam[0].angle1", 0.0);
        settings.anaBeam.direction.angle2 = getDouble("Observation.AnaBeam[0].angle2", 0.0);
      }

      /* ===============================
       * Correlator pipeline information
       * ===============================
       */

      settings.correlator.enabled = getBool("Observation.DataProducts.Output_Correlated.enabled", false);
      if (settings.correlator.enabled || true) { // for now, always fill in correlator values, since they're still used outside the correlator to determine the block size, etc (TODO: move generic ones outside, but see TODO below)
        settings.correlator.nrChannels = getUint32(renamedKey("Cobalt.Correlator.nrChannelsPerSubband", "Observation.channelsPerSubband"), 64);
        settings.correlator.channelWidth = settings.subbandWidth() / settings.correlator.nrChannels;
        settings.correlator.nrSamplesPerChannel = getUint32(renamedKey("Cobalt.Correlator.nrSamplesPerChannelPerBlock", "OLAP.CNProc.integrationSteps"), 3052);
        settings.correlator.nrBlocksPerIntegration = getUint32(renamedKey("Cobalt.Correlator.nrBlocksPerIntegration", "OLAP.IONProc.integrationSteps"), 1);
        settings.correlator.nrBlocksPerObservation = static_cast<size_t>(floor((settings.stopTime - settings.startTime) / settings.correlator.integrationTime()));

        // super-station beam former

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

        if (settings.correlator.enabled) { // TODO: redundant check, but as long as '|| true' is there (just above), this is needed as some test parsets (e.g. tKernel.parset.in) has no locations and filenames (and enabled) keys. See tCorrelatorPipelineProcessObs.parset what is needed or refactor this function.
          // Files to output
          settings.correlator.files.resize(settings.subbands.size());
          for (size_t i = 0; i < settings.correlator.files.size(); ++i) {
            settings.correlator.files[i].location = getFileLocation("Correlated", i);
          }
        }
      }

      /* ===============================
       * Beamformer pipeline information
       * ===============================
       */

      // SAP/TAB-crossing counter for the files we generate
      size_t bfStreamNr = 0;

      settings.beamFormer.enabled = getBool("Observation.DataProducts.Output_Beamformed.enabled", false);
      if (settings.beamFormer.enabled || true) { // for now, the values below are also used even if no beam forming is performed

        // Parse global settings
        for (unsigned i = 0; i < 2; ++i) {
          // Set coherent and incoherent Stokes settings by
          // iterating twice.

          string prefix = "";
          struct ObservationSettings::BeamFormer::StokesSettings *set = 0;
          
          // Select coherent or incoherent for this iteration
          switch(i) {
            case 0:
              prefix = "OLAP.CNProc_CoherentStokes";
              set = &settings.beamFormer.coherentSettings;
              break;

            case 1:
              prefix = "OLAP.CNProc_IncoherentStokes";
              set = &settings.beamFormer.incoherentSettings;
              break;

            default:
              ASSERT(false);
              break;
          }

          // Obtain settings of selected stokes
          set->type = stokesType(getString(prefix + ".which", "I"));
          set->nrStokes = nrStokes(set->type);
          set->nrChannels = getUint32(prefix + ".channelsPerSubband", 0);
          if (set->nrChannels == 0) {
            // apply default
            set->nrChannels = settings.correlator.nrChannels;
          }
          set->timeIntegrationFactor = getUint32(prefix + ".timeIntegrationFactor", 1);
          set->nrSubbandsPerFile = getUint32(prefix + ".subbandsPerFile", 0);
          if (set->nrSubbandsPerFile == 0) {
            // apply default
            set->nrSubbandsPerFile = settings.subbands.size();
          }

          ASSERTSTR(set->nrSubbandsPerFile >= settings.subbands.size(), "Multiple parts/file are not yet supported!");
        }

        // Parse all TABs
        settings.beamFormer.SAPs.resize(nrSAPs);

        for (unsigned i = 0; i < nrSAPs; ++i) {
          struct ObservationSettings::BeamFormer::SAP &sap = settings.beamFormer.SAPs[i];

          size_t nrTABs    = getUint32(str(format("Observation.Beam[%u].nrTiedArrayBeams") % i), 0);
          size_t nrRings   = getUint32(str(format("Observation.Beam[%u].nrTabRings") % i), 0);
          double ringWidth = getDouble(str(format("Observation.Beam[%u].ringWidth") % i), 0.0);

          ASSERTSTR(nrRings == 0, "TAB rings are not supported yet!");

          sap.TABs.resize(nrTABs);
          for (unsigned j = 0; j < nrTABs; ++j) {
            struct ObservationSettings::BeamFormer::TAB &tab = sap.TABs[j];

            const string prefix = str(format("Observation.Beam[%u].TiedArrayBeam[%u]") % i % j);

            tab.directionDelta.type    = getString(prefix + ".directionType", "J2000");
            tab.directionDelta.angle1  = getDouble(prefix + ".angle1", 0.0);
            tab.directionDelta.angle2  = getDouble(prefix + ".angle2", 0.0);

            tab.coherent          = getBool(prefix + ".coherent", true);
            tab.dispersionMeasure = getDouble(prefix + ".dispersionMeasure", 0.0);

            struct ObservationSettings::BeamFormer::StokesSettings &set =
               tab.coherent ? settings.beamFormer.coherentSettings
                            : settings.beamFormer.incoherentSettings;

            // Generate file list
            tab.files.resize(set.nrStokes);
            for (size_t s = 0; s < set.nrStokes; ++s) {
              tab.files[s].stokesNr = s;
              tab.files[s].streamNr = bfStreamNr++;
              tab.files[s].location = getFileLocation("Beamformed", tab.files[s].streamNr);
            }
          }
        }

        settings.beamFormer.dedispersionFFTsize = getUint32(renamedKey("Cobalt.Beamformer.dedispersionFFTsize", "OLAP.CNProc.dedispersionFFTsize"), settings.correlator.nrSamplesPerChannel);
      }

      return settings;
    }

    double ObservationSettings::subbandWidth() const {
      return 1.0 * clockMHz * 1000000 / 1024;
    }

    unsigned ObservationSettings::nrCrossPolarisations() const {
      return nrPolarisations * nrPolarisations;
    }

    size_t ObservationSettings::nrSamplesPerSubband() const {
      return correlator.nrChannels * correlator.nrSamplesPerChannel;
    }

    double ObservationSettings::Correlator::integrationTime() const {
      return 1.0 * nrSamplesPerChannel * nrBlocksPerIntegration / channelWidth;
    }

    struct ObservationSettings::FileLocation Parset::getFileLocation(const std::string outputType, unsigned idx) const {
      //
      const string prefix = "Observation.DataProducts.Output_" + outputType;

      vector<string> empty;
      vector<string> filenames = getStringVector(prefix + ".filenames", empty, true);
      vector<string> locations = getStringVector(prefix + ".locations", empty, true);

      if (idx >= filenames.size()) {
        THROW(CoInterfaceException, "Invalid index for " << prefix << ".filenames: " << idx);
      }

      if (idx >= locations.size()) {
        THROW(CoInterfaceException, "Invalid index for " << prefix << ".locations: " << idx);
      }

      vector<string> host_dir = StringUtil::split(locations[idx], ':');

      if (host_dir.size() != 2) {
        THROW(CoInterfaceException, "Location must adhere to 'host:directory' in " << prefix << ".locations: " << locations[idx]);
      }

      ObservationSettings::FileLocation location;
      location.filename  = filenames[idx];
      location.host      = host_dir[0];
      location.directory = host_dir[1];

      return location;
    }


    size_t ObservationSettings::BeamFormer::maxNrTABsPerSAP() const
    {
      size_t max = 0;

      for (size_t sapNr = 0; sapNr < SAPs.size(); ++sapNr)
        max = std::max(max, SAPs[sapNr].TABs.size());

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
      checkInputConsistency();
      checkVectorLength("Observation.beamList", nrSubbands());

      for (OutputType outputType = FIRST_OUTPUT_TYPE; outputType < LAST_OUTPUT_TYPE; outputType++)
        if (outputThisType(outputType)) {
          std::string prefix = keyPrefix(outputType);
          unsigned expected = nrStreams(outputType);

          checkVectorLength(prefix + ".locations", expected);
          checkVectorLength(prefix + ".filenames", expected);
        }

      if (CNintegrationSteps() % dedispersionFFTsize() != 0)
        THROW(CoInterfaceException, "OLAP.CNProc.integrationSteps (" << CNintegrationSteps() << ") must be divisible by OLAP.CNProc.dedispersionFFTsize (" << dedispersionFFTsize() << ')');

      if (outputThisType(BEAM_FORMED_DATA) || outputThisType(TRIGGER_DATA)) {
        // second transpose is performed
      }

      // check whether the beam forming parameters are valid
      const Transpose2 &logic = transposeLogic();

      for (unsigned i = 0; i < logic.nrStreams(); i++) {
        const StreamInfo &info = logic.streamInfo[i];

        if ( info.timeIntFactor == 0 )
          THROW(CoInterfaceException, "Temporal integration factor needs to be > 0 (it is set to 0 for " << (info.coherent ? "coherent" : "incoherent") << " beams).");

        if ( info.coherent
             && info.stokesType == STOKES_XXYY
             && info.timeIntFactor != 1 )
          THROW(CoInterfaceException, "Cannot perform temporal integration if calculating Coherent Stokes XXYY. Integration factor needs to be 1, but is set to " << info.timeIntFactor);
      }
    }


    bool Parset::correctClocks() const
    {
      return settings.corrections.clock;
    }


    string Parset::getInputStreamName(const string &stationName, unsigned rspBoardNumber) const
    {
      return getStringVector(string("PIC.Core.Station.") + stationName + ".RSP.ports", true)[rspBoardNumber];
    }


    std::string Parset::keyPrefix(OutputType outputType)
    {
      switch (outputType) {
      case CORRELATED_DATA:   return "Observation.DataProducts.Output_Correlated";
      case BEAM_FORMED_DATA:  return "Observation.DataProducts.Output_Beamformed";
      case TRIGGER_DATA:      return "Observation.DataProducts.Output_Trigger";
      default:                THROW(CoInterfaceException, "Unknown output type");
      }
    }


    std::string Parset::getHostName(OutputType outputType, unsigned streamNr) const
    {
      if (outputType == CORRELATED_DATA)
        return settings.correlator.files[streamNr].location.host; // TODO: add to check() to reject parset or obsconfig early to avoid segfault here if streamNr >= settings.correlator.files.size()

      return StringUtil::split(getStringVector(keyPrefix(outputType) + ".locations", true)[streamNr], ':')[0];
    }


    std::string Parset::getFileName(OutputType outputType, unsigned streamNr) const
    {
      if (outputType == CORRELATED_DATA)
        return settings.correlator.files[streamNr].location.filename;

      const std::string keyname = keyPrefix(outputType) + ".filenames";
      if (!isDefined(keyname))
        THROW(CoInterfaceException, "Could not find filename key: " << keyname);

      const std::vector<std::string> filenames = getStringVector(keyname, true);

      if (streamNr >= filenames.size())
        THROW(CoInterfaceException, "Filename index out of bounds for key " << keyname << ": " << streamNr << " >= " << filenames.size());

      return filenames[streamNr];
    }


    std::string Parset::getDirectoryName(OutputType outputType, unsigned streamNr) const
    {
      if (outputType == CORRELATED_DATA)
        return settings.correlator.files[streamNr].location.directory;

      return StringUtil::split(getStringVector(keyPrefix(outputType) + ".locations", true)[streamNr], ':')[1];
    }


    unsigned Parset::nrStreams(OutputType outputType, bool force) const
    {
      if (!outputThisType(outputType) && !force)
        return 0;

      switch (outputType) {
      case CORRELATED_DATA:   return nrSubbands();
      case BEAM_FORMED_DATA:        // FALL THROUGH
      case TRIGGER_DATA:      return transposeLogic().nrStreams();
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
        pos = getDoubleVector("PIC.Core." + stationList[i] + ".position");
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
          pos = getDoubleVector("PIC.Core." + name + ".position", true);

        ASSERT(pos.size() == 3);

        list[i][0] = pos[0];
        list[i][1] = pos[1];
        list[i][2] = pos[2];
      }

      return list;
    }
    /*
       std::vector<double> Parset::getPhaseCorrection(const string &name, char pol) const
       {
       return getDoubleVector(str(format("PIC.Core.%s.%s.phaseCorrection.%c") % name % antennaSet() % pol));
       }
     */

    string Parset::beamTarget(unsigned beam) const
    {
      return settings.SAPs[beam].target;
    }


    std::vector<double> Parset::getTAB(unsigned beam, unsigned pencil) const
    {
      std::vector<double> TAB(2);

      TAB[0] = settings.beamFormer.SAPs[beam].TABs[pencil].directionDelta.angle1;
      TAB[1] = settings.beamFormer.SAPs[beam].TABs[pencil].directionDelta.angle2;

      return TAB;
    }


    bool Parset::isCoherent(unsigned beam, unsigned pencil) const
    {
      return settings.beamFormer.SAPs[beam].TABs[pencil].coherent;
    }


    double Parset::dispersionMeasure(unsigned beam, unsigned pencil) const
    {
      if (!settings.corrections.dedisperse)
        return 0.0;

      return settings.beamFormer.SAPs[beam].TABs[pencil].dispersionMeasure;
    }


    std::vector<string> Parset::TABStationList(unsigned beam, unsigned pencil, bool raw) const
    {
      // can't use settings until 'raw' is supported, which is needed to
      // distinguish between fly's eye mode with one station, and coherent
      // addition with one station
      string key = str(format("Observation.Beam[%u].TiedArrayBeam[%u].stationList") % beam % pencil);
      std::vector<string> stations;

      if (isDefined(key))
        stations = getStringVector(key,true);

      if (raw)
        return stations;

      // default to all stations
      if (stations.empty())
        stations = mergedStationNames();

      return stations;
    }


    std::vector<double> Parset::getBeamDirection(unsigned beam) const
    {
      std::vector<double> beamDirs(2);

      beamDirs[0] = settings.SAPs[beam].direction.angle1;
      beamDirs[1] = settings.SAPs[beam].direction.angle2;

      return beamDirs;
    }


    std::string Parset::getBeamDirectionType(unsigned beam) const
    {
      return settings.SAPs[beam].direction.type;
    }


    bool Parset::haveAnaBeam() const
    {
      return settings.anaBeam.enabled;
    }


    std::vector<double> Parset::getAnaBeamDirection() const
    {
      std::vector<double> anaBeamDirections(2);

      anaBeamDirections[0] = settings.anaBeam.direction.angle1;
      anaBeamDirections[1] = settings.anaBeam.direction.angle2;

      return anaBeamDirections;
    }


    std::string Parset::getAnaBeamDirectionType() const
    {
      return settings.anaBeam.direction.type;   }

    double Parset::getTime(const std::string &name, const std::string &defaultValue) const
    {
      return to_time_t(boost::posix_time::time_from_string(getString(name, defaultValue)));
    }

    unsigned Parset::nrTABs(unsigned beam) const
    {
      return settings.beamFormer.SAPs[beam].TABs.size();
    }

    std::string Parset::name() const
    {
      return itsName;
    }

    const Transpose2 &Parset::transposeLogic() const
    {
      if (!itsTransposeLogic)
        itsTransposeLogic = new Transpose2(*this);

      return *itsTransposeLogic;
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

    string Parset::stationName(int index) const
    {
      return settings.stations[index].name;
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
      return settings.clockMHz * 1000000;
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

    unsigned Parset::coherentStokesTimeIntegrationFactor() const
    {
      return settings.beamFormer.coherentSettings.timeIntegrationFactor;
    }

    unsigned Parset::incoherentStokesTimeIntegrationFactor() const
    {
      return settings.beamFormer.incoherentSettings.timeIntegrationFactor;
    }

    bool Parset::outputCorrelatedData() const
    {
      return settings.correlator.enabled;
    }

    bool Parset::outputBeamFormedData() const
    {
      return settings.beamFormer.enabled;
    }

    bool Parset::outputTrigger() const
    {
      return getBool("Observation.DataProducts.Output_Trigger.enabled", false);
    }

    bool Parset::outputThisType(OutputType outputType) const
    {
      return getBool(keyPrefix(outputType) + ".enabled", false);
    }

#if 0
    bool Parset::onlineFlagging() const
    {
      return getBool("OLAP.CNProc.onlineFlagging", false);
    }

    bool Parset::onlinePreCorrelationFlagging() const
    {
      return getBool("OLAP.CNProc.onlinePreCorrelationFlagging", false);
    }

    bool Parset::onlinePreCorrelationNoChannelsFlagging() const
    {
      return getBool("OLAP.CNProc.onlinePreCorrelationNoChannelsFlagging", false);
    }

    bool Parset::onlinePostCorrelationFlagging() const
    {
      return getBool("OLAP.CNProc.onlinePostCorrelationFlagging", false);
    }

    unsigned Parset::onlinePreCorrelationFlaggingIntegration() const
    {
      return getUint32("OLAP.CNProc.onlinePostCorrelationFlaggingIntegration", 0);
    }


    string Parset::onlinePreCorrelationFlaggingType(std::string defaultVal) const
    {
      return getString("OLAP.CNProc.onlinePreCorrelationFlaggingType", defaultVal);
    }

    string Parset::onlinePreCorrelationFlaggingStatisticsType(std::string defaultVal) const
    {
      return getString("OLAP.CNProc.onlinePreCorrelationFlaggingStatisticsType", defaultVal);
    }

    string Parset::onlinePostCorrelationFlaggingType(std::string defaultVal) const
    {
      return getString("OLAP.CNProc.onlinePostCorrelationFlaggingType", defaultVal);
    }

    string Parset::onlinePostCorrelationFlaggingStatisticsType(std::string defaultVal) const
    {
      return getString("OLAP.CNProc.onlinePostCorrelationFlaggingStatisticsType", defaultVal);
    }

    bool Parset::onlinePostCorrelationFlaggingDetectBrokenStations() const
    {
      return getBool("OLAP.CNProc.onlinePostCorrelationFlaggingDetectBrokenStations", false);
    }
#endif

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
      return settings.correlator.nrSamplesPerChannel;
    }

    unsigned Parset::nrHistorySamples() const
    {
      return nrChannelsPerSubband() > 1 ? (nrPPFTaps() - 1) * nrChannelsPerSubband() : 0;
    }

    unsigned Parset::nrPPFTaps() const
    {
      return settings.nrPPFTaps;
    }

    unsigned Parset::nrChannelsPerSubband() const
    {
      return settings.correlator.nrChannels;
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

    unsigned Parset::nrCalcDelays() const
    {
      return 16;
    }

    string Parset::positionType() const
    {
      return "ITRF";
    }

    bool Parset::correctBandPass() const
    {
      return settings.corrections.bandPass;
    }

    double Parset::channel0Frequency(size_t subband) const
    {
      const double sbFreq = settings.subbands[subband].centralFrequency;

      if (nrChannelsPerSubband() == 1)
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

    std::vector<unsigned> Parset::nrTABs() const
    {
      std::vector<unsigned> counts(nrBeams());

      for (unsigned beam = 0; beam < nrBeams(); beam++)
        counts[beam] = nrTABs(beam);

      return counts;
    }

    unsigned Parset::maxNrTABs() const
    {
      std::vector<unsigned> beams = nrTABs();

      if (beams.empty())
        return 0;

      return *std::max_element(beams.begin(), beams.end());
    }

    BeamCoordinates Parset::TABs(unsigned beam) const
    {
      BeamCoordinates coordinates;

      for (unsigned pencil = 0; pencil < nrTABs(beam); pencil++) {
        const std::vector<double> coords = getTAB(beam, pencil);

        // assume ra,dec
        coordinates += BeamCoord3D(coords[0],coords[1]);
      }

      return coordinates;
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

    void StreamInfo::log() const
    {
      LOG_DEBUG_STR( "Stream " << stream << " is sap " << sap << " beam " << beam << " stokes " << stokes << " part " << part << " consisting of subbands " << subbands );
    }

    Transpose2::Transpose2( const Parset &parset )
      :

      streamInfo( generateStreamInfo(parset) )
    {
    }

    unsigned Transpose2::nrStreams() const
    {
      return streamInfo.size();
    }

    // compose and decompose a stream number
    unsigned Transpose2::stream( unsigned sap, unsigned beam, unsigned stokes, unsigned part, unsigned startAt ) const
    {
      for (unsigned i = startAt; i < streamInfo.size(); i++) {
        const struct StreamInfo &info = streamInfo[i];

        if (sap == info.sap && beam == info.beam && stokes == info.stokes && part == info.part)
          return i;
      }

      // shouldn't reach this point
      ASSERTSTR(false, "Requested non-existing sap " << sap << " beam " << beam << " stokes " << stokes << " part " << part);

      return 0;
    }

    void Transpose2::decompose( unsigned stream, unsigned &sap, unsigned &beam, unsigned &stokes, unsigned &part ) const
    {
      const struct StreamInfo &info = streamInfo[stream];

      sap = info.sap;
      beam = info.beam;
      stokes = info.stokes;
      part = info.part;
    }

    std::vector<unsigned> Transpose2::subbands( unsigned stream ) const
    {
      ASSERT(stream < streamInfo.size());

      return streamInfo[stream].subbands;
    }

    unsigned Transpose2::nrSubbands( unsigned stream ) const
    {
      return stream >= streamInfo.size() ? 0 : subbands(stream).size();
    }

    static bool streamInfoSubbandsComp( const struct StreamInfo &a, const struct StreamInfo &b )
    {
      return a.subbands.size() < b.subbands.size();
    }

    unsigned Transpose2::maxNrSubbands() const
    {
      return streamInfo.size() == 0 ? 0 : std::max_element( streamInfo.begin(), streamInfo.end(), &streamInfoSubbandsComp )->subbands.size();
    }

    static bool streamInfoChannelsComp( const struct StreamInfo &a, const struct StreamInfo &b )
    {
      return a.nrChannels < b.nrChannels;
    }

    unsigned Transpose2::maxNrChannels() const
    {
      return streamInfo.size() == 0 ? 0 : std::max_element( streamInfo.begin(), streamInfo.end(), &streamInfoChannelsComp )->nrChannels;
    }

    static bool streamInfoSamplesComp( const struct StreamInfo &a, const struct StreamInfo &b )
    {
      return a.nrSamples < b.nrSamples;
    }

    unsigned Transpose2::maxNrSamples() const
    {
      return streamInfo.size() == 0 ? 0 : std::max_element( streamInfo.begin(), streamInfo.end(), &streamInfoSamplesComp )->nrSamples;
    }

    size_t Transpose2::subbandSize( unsigned stream ) const
    {
      const StreamInfo &info = streamInfo[stream];

      return (size_t)info.nrChannels * (info.nrSamples | 2) * sizeof(float);
    }


    std::vector<struct StreamInfo> Transpose2::generateStreamInfo( const Parset &parset ) const
    {
      // get all info from parset, since we will be called while constructing our members

      // ParameterSets are SLOW, so settings any info we need repeatedly

      std::vector<struct StreamInfo> infoset;
      const unsigned nrSAPs = parset.settings.SAPs.size();
      const unsigned nrSubbands = parset.settings.subbands.size();

      const unsigned nrSamples = parset.CNintegrationSteps();

      struct StreamInfo info;
      info.stream = 0;

      for (unsigned sap = 0; sap < nrSAPs; sap++) {
        const unsigned nrBeams = parset.nrTABs(sap);

        info.sap = sap;

        std::vector<unsigned> sapSubbands;

        for (unsigned sb = 0; sb < nrSubbands; sb++)
          if (parset.settings.subbands[sb].SAP == sap)
            sapSubbands.push_back(sb);

        for (unsigned beam = 0; beam < nrBeams; beam++) {
          info.beam = beam;

          const bool coherent = parset.settings.beamFormer.SAPs[sap].TABs[beam].coherent;
          const ObservationSettings::BeamFormer::StokesSettings &set =
              coherent ? parset.settings.beamFormer.coherentSettings
                       : parset.settings.beamFormer.incoherentSettings;
          const unsigned nrStokes = set.nrStokes;

          info.coherent = coherent;
          info.nrChannels = set.nrChannels;
          info.timeIntFactor = set.timeIntegrationFactor;
          info.nrStokes = set.nrStokes;
          info.stokesType = set.type;
          info.nrSamples = nrSamples / info.timeIntFactor;

          const unsigned nrSubbandsPerFile = set.nrSubbandsPerFile;

          for (unsigned stokes = 0; stokes < nrStokes; stokes++) {
            info.stokes = stokes;
            info.part = 0;

            // split into parts of at most nrSubbandsPerFile
            for (unsigned sb = 0; sb < sapSubbands.size(); sb += nrSubbandsPerFile ) {
              for (unsigned i = 0; sb + i < sapSubbands.size() && i < nrSubbandsPerFile; i++)
                info.subbands.push_back(sapSubbands[sb + i]);

              infoset.push_back(info);

              info.subbands.clear();
              info.part++;
              info.stream++;
            }
          }
        }
      }

      return infoset;
    }
  } // namespace Cobalt
} // namespace LOFAR

