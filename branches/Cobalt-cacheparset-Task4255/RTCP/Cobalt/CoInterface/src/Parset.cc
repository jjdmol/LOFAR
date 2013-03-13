//#  Parset.cc: one line description
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/DataConvert.h>
#include <Common/LofarBitModeInfo.h>
#include <ApplCommon/PosixTime.h>
#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/Exceptions.h>
#include <CoInterface/PrintVector.h>
#include <CoInterface/SetOperations.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <cstdio>
#include <set>


namespace LOFAR
{
  namespace RTCP
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
    };


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

      updateCache();
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

      // Update the cache
      updateCache();
    }


    void Parset::write(Stream *stream) const
    {
      // stream == NULL fills the cache,
      // causing subsequent write()s to use it
      bool readCache = !itsWriteCache.empty();
      bool writeCache = !stream;

      std::string newbuffer;
      std::string &buffer = readCache || writeCache ? itsWriteCache : newbuffer;

      if (buffer.empty())
        writeBuffer(buffer);

      if (!stream) {
        // we only filled the cache
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

    
    void Parset::updateCache()
    {
      // NOTE: Make sure that all keys have defaults, to make test parsets
      // a lot shorter.

      vector<string>   emptyVectorString;
      vector<unsigned> emptyVectorUnsigned;

      // Generic information
      cache.observationID = getUint32("Observation.ObsID", 0);
      cache.startTime = getTime("Observation.startTime", "2013-01-01 00:00:00");
      cache.stopTime  = getTime("Observation.stopTime",  "2013-01-01 00:01:00");
      cache.clockMHz = getUint32("Observation.sampleClock", 200);

      if (isDefined("Observation.nrBitsPerSample")) {
        cache.nrBitsPerSample = getUint32("Observation.nrBitsPerSample", 16);
      } else {
        LOG_WARN("Using depricatdd OLAP.nrBitsPerSample. Please replace by Observation.nrBitsPerSample");
        cache.nrBitsPerSample = getUint32("OLAP.nrBitsPerSample", 16);
      }

      // Station information
      vector<string> stationNames = getStringVector("OLAP.storageStationNames", emptyVectorString, true);
      size_t nrStations = stationNames.size();

      cache.stations.resize(nrStations);
      for (unsigned station = 0; station < nrStations; ++station) {
        cache.stations[station].name = stationNames[station];
      }

      // Spectral resolution information
      vector<unsigned> subbandList = getUint32Vector("Observation.subbandList", emptyVectorUnsigned, true);
      vector<unsigned> sapList     = getUint32Vector("Observation.beamList",    emptyVectorUnsigned, true);
      size_t nrSubbands = subbandList.size();

      cache.subbands.resize(nrSubbands);
      unsigned subbandOffset = 512 * (nyquistZone() - 1);
      for (unsigned subband = 0; subband < nrSubbands; ++subband) {
        cache.subbands[subband].idx              = subband;
        cache.subbands[subband].stationIdx       = subbandList[subband];
        cache.subbands[subband].SAP              = sapList[subband];
        cache.subbands[subband].centralFrequency = subbandBandwidth() * (subbandList[subband] + subbandOffset);
      }
    }


    void Parset::checkVectorLength(const std::string &key, unsigned expectedSize) const
    {
      unsigned actualSize = getStringVector(key, true).size();

      if (actualSize != expectedSize)
        THROW(CoInterfaceException, "Key \"" << string(key) << "\" contains wrong number of entries (expected: " << expectedSize << ", actual: " << actualSize << ')');
    }


    void Parset::checkInputConsistency() const
    {
      using std::set;

      map<string, set<unsigned> > allRSPboards;
      vector<unsigned> inputs = phaseOnePsets();

      for (vector<unsigned>::const_iterator pset = inputs.begin(); pset != inputs.end(); pset++) {
        vector<StationRSPpair> stationRSPpairs = getStationNamesAndRSPboardNumbers(*pset);

        for (vector<StationRSPpair>::const_iterator pair = stationRSPpairs.begin(); pair != stationRSPpairs.end(); pair++) {
          const string &station = pair->station;
          unsigned rsp = pair->rsp;

          map<string, set<unsigned> >::const_iterator stationRSPs = allRSPboards.find(station);

          if (stationRSPs != allRSPboards.end() && stationRSPs->second.find(rsp) != stationRSPs->second.end())
            THROW(CoInterfaceException, station << "/RSP" << rsp << " multiple times defined in \"PIC.Core.IONProc.*.inputs\"");

          allRSPboards[station].insert(rsp);
        }
      }

      for (map<string, set<unsigned> >::const_iterator stationRSPs = allRSPboards.begin(); stationRSPs != allRSPboards.end(); stationRSPs++) {
        const string          &station = stationRSPs->first;
        const set<unsigned> &rsps = stationRSPs->second;

        vector<unsigned> rspsOfStation = subbandToRSPboardMapping(station);
        vector<unsigned> slotsOfStation = subbandToRSPslotMapping(station);

        if (rspsOfStation.size() != nrSubbands())
          THROW(CoInterfaceException, string("the size of \"Observation.Dataslots.") + station + ".RSPBoardList\" does not equal the number of subbands");

        if (slotsOfStation.size() != nrSubbands())
          THROW(CoInterfaceException, string("the size of \"Observation.Dataslots.") + station + ".DataslotList\" does not equal the number of subbands");

        for (int subband = nrSubbands(); --subband >= 0; ) {
          if (rsps.find(rspsOfStation[subband]) == rsps.end())
            THROW(CoInterfaceException, "\"Observation.Dataslots." << station << ".RSPBoardList\" mentions RSP board " << rspsOfStation[subband] << ", which does not exist");

          if (slotsOfStation[subband] >= nrSlotsInFrame())
            THROW(CoInterfaceException, "\"Observation.Dataslots." << station << ".DataslotList\" mentions RSP slot " << slotsOfStation[subband] << ", which is more than the number of slots in a frame");
        }
      }
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


    vector<Parset::StationRSPpair> Parset::getStationNamesAndRSPboardNumbers(unsigned psetNumber) const
    {
      vector<string> inputs = getStringVector(str(boost::format("PIC.Core.IONProc.%s[%u].inputs") % partitionName() % psetNumber), true);
      vector<StationRSPpair> stationsAndRSPs(inputs.size());

      for (unsigned i = 0; i < inputs.size(); i++) {
        vector<string> split = StringUtil::split(inputs[i], '/');

        if (split.size() != 2 || split[1].substr(0, 3) != "RSP")
          THROW(CoInterfaceException, string("expected stationname/RSPn pair in \"") << inputs[i] << '"');

        stationsAndRSPs[i].station = split[0];
        stationsAndRSPs[i].rsp = boost::lexical_cast<unsigned>(split[1].substr(3));
      }

      return stationsAndRSPs;
    }


    bool Parset::correctClocks() const
    {
      if (!isDefined("OLAP.correctClocks")) {
        LOG_WARN("\"OLAP.correctClocks\" should really be defined in the parset --- assuming FALSE");
        return false;
      } else {
        return getBool("OLAP.correctClocks");
      }
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
      return StringUtil::split(getStringVector(keyPrefix(outputType) + ".locations", true)[streamNr], ':')[0];
    }


    std::string Parset::getFileName(OutputType outputType, unsigned streamNr) const
    {
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

    unsigned Parset::nyquistZone() const
    {
      std::string bandFilter = getString("Observation.bandFilter");

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


    unsigned Parset::nrBeams() const
    {
      std::vector<unsigned> sapMapping = subbandToSAPmapping();

      if (sapMapping.empty())
        return 0;

      return *std::max_element(sapMapping.begin(), sapMapping.end()) + 1;
    }


    std::vector<double> Parset::subbandToFrequencyMapping() const
    {
      vector<double> freqs(nrSubbands());

      for (unsigned subband = 0; subband < freqs.size(); ++subband)
        freqs[subband] = cache.subbands[subband].centralFrequency;

      return freqs;
    }


    std::vector<unsigned> Parset::subbandToSAPmapping() const
    {
      vector<unsigned> saps(nrSubbands());

      for (unsigned subband = 0; subband < saps.size(); ++subband)
        saps[subband] = cache.subbands[subband].SAP;

      return saps;
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


    vector<double> Parset::positions() const
    {
      vector<string> stNames;
      vector<double> pos, list;
      unsigned nStations;

      if (nrTabStations() > 0) {
        stNames = getStringVector("OLAP.tiedArrayStationNames", true);
        nStations = nrTabStations();
      } else {
        stNames = getStringVector("OLAP.storageStationNames", true);
        nStations = nrStations();
      }

      for (uint i = 0; i < nStations; i++) {
        if (stNames[i].find("+") != string::npos)
          pos = centroidPos(stNames[i]);
        else
          pos = getDoubleVector("PIC.Core." + stNames[i] + ".position");

        list.insert(list.end(), pos.begin(), pos.end());
      }

      return list;
    }


    std::vector<double> Parset::getRefPhaseCentre() const
    {
      return getDoubleVector("Observation.referencePhaseCenter");
    }


    std::vector<double> Parset::getPhaseCentreOf(const string &name) const
    {
      return getDoubleVector(str(boost::format("PIC.Core.%s.phaseCenter") % name));
    }
    /*
       std::vector<double> Parset::getPhaseCorrection(const string &name, char pol) const
       {
       return getDoubleVector(str(boost::format("PIC.Core.%s.%s.phaseCorrection.%c") % name % antennaSet() % pol));
       }
     */

    string Parset::beamTarget(unsigned beam) const
    {
      string key = str(boost::format("Observation.Beam[%u].target") % beam);

      return getString(key, "");
    }

    double Parset::beamDuration(unsigned beam) const
    {
      string key = str(boost::format("Observation.Beam[%u].duration") % beam);
      double val = getDouble(key, 0.0);

      // a sane default
      if (val == 0.0)
        val = stopTime() - startTime();

      return val;
    }


    std::vector<double> Parset::getTAB(unsigned beam, unsigned pencil) const
    {
      std::vector<double> TAB(2);

      TAB[0] = getDouble(str(boost::format("Observation.Beam[%u].TiedArrayBeam[%u].angle1") % beam % pencil));
      TAB[1] = getDouble(str(boost::format("Observation.Beam[%u].TiedArrayBeam[%u].angle2") % beam % pencil));

      return TAB;
    }


    bool Parset::isCoherent(unsigned beam, unsigned pencil) const
    {
      string key = str(boost::format("Observation.Beam[%u].TiedArrayBeam[%u].coherent") % beam % pencil);

      return getBool(key, true);
    }


    double Parset::dispersionMeasure(unsigned beam, unsigned pencil) const
    {
      if (!getBool("OLAP.coherentDedisperseChannels",true))
        return 0.0;

      string key = str(boost::format("Observation.Beam[%u].TiedArrayBeam[%u].dispersionMeasure") % beam % pencil);

      return getDouble(key, 0.0);
    }


    std::vector<string> Parset::TABStationList(unsigned beam, unsigned pencil, bool raw) const
    {
      string key = str(boost::format("Observation.Beam[%u].TiedArrayBeam[%u].stationList") % beam % pencil);
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

      beamDirs[0] = getDouble(str(boost::format("Observation.Beam[%u].angle1") % beam));
      beamDirs[1] = getDouble(str(boost::format("Observation.Beam[%u].angle2") % beam));

      return beamDirs;
    }


    std::string Parset::getBeamDirectionType(unsigned beam) const
    {
      char buf[50];
      string beamDirType;

      snprintf(buf, sizeof buf, "Observation.Beam[%d].directionType", beam);
      beamDirType = getString(buf);

      return beamDirType;
    }


    bool Parset::haveAnaBeam() const
    {
      return antennaSet().substr(0,3) == "HBA";
    }


    std::vector<double> Parset::getAnaBeamDirection() const
    {
      std::vector<double> anaBeamDirections(2);

      anaBeamDirections[0] = getDouble("Observation.AnaBeam[0].angle1");
      anaBeamDirections[1] = getDouble("Observation.AnaBeam[0].angle2");

      return anaBeamDirections;
    }


    std::string Parset::getAnaBeamDirectionType() const
    {
      return getString("Observation.AnaBeam[0].directionType");
    }


    vector<unsigned> Parset::subbandToRSPboardMapping(const string &stationName) const
    {
      std::string key = std::string("Observation.Dataslots.") + stationName + ".RSPBoardList";

      if (!isDefined(key)) {
        //LOG_WARN_STR('"' << key << "\" not defined, trying \"Observation.rspBoardList\"");
        key = "Observation.rspBoardList";
      }

      if (!isDefined(key)) {
        LOG_WARN_STR('"' << key << "\" not defined, falling back to default");

        /* Map the subbands linearly onto the RSP boards */
        size_t n = nrSubbands();
        size_t beamletsPerRSP = maxBeamletsPerRSP(nrBitsPerSample());

        vector<unsigned> rspBoards(n);

        for (size_t i = 0; i < n; i++)
          rspBoards[i] = i / beamletsPerRSP;

        return rspBoards;
      }

      return getUint32Vector(key, true);
    }


    vector<unsigned> Parset::subbandToRSPslotMapping(const string &stationName) const
    {
      std::string key = std::string("Observation.Dataslots.") + stationName + ".DataslotList";

      if (!isDefined(key)) {
        //LOG_WARN_STR('"' << key << "\" not defined, trying \"Observation.rspSlotList\"");
        key = "Observation.rspSlotList";
      }

      if (!isDefined(key)) {
        LOG_WARN_STR('"' << key << "\" not defined, falling back to default");

        /* Map the subbands linearly onto the RSP boards */
        size_t n = nrSubbands();
        size_t beamletsPerRSP = maxBeamletsPerRSP(nrBitsPerSample());

        vector<unsigned> rspSlots(n);

        for (size_t i = 0; i < n; i++)
          rspSlots[i] = i % beamletsPerRSP;

        return rspSlots;
      }

      return getUint32Vector(key, true);
    }


    int Parset::findIndex(unsigned pset, const vector<unsigned> &psets)
    {
      unsigned index = std::find(psets.begin(), psets.end(), pset) - psets.begin();

      return index != psets.size() ? static_cast<int>(index) : -1;
    }

    double Parset::getTime(const std::string &name, const std::string &defaultValue) const
    {
      return to_time_t(boost::posix_time::time_from_string(getString(name, defaultValue)));
    }

    unsigned Parset::nrTABs(unsigned beam) const
    {
      using boost::format;
      return getUint32(str(format("Observation.Beam[%u].nrTiedArrayBeams") % beam));
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
      return cache.observationID;
    }

    double Parset::startTime() const
    {
      return cache.startTime;
    }

    double Parset::stopTime() const
    {
      return cache.stopTime;
    }

    unsigned Parset::nrCorrelatedBlocks() const
    {
      return static_cast<unsigned>(floor( (stopTime() - startTime()) / IONintegrationTime()));
    }

    unsigned Parset::nrBeamFormedBlocks() const
    {
      return static_cast<unsigned>(floor( (stopTime() - startTime()) / CNintegrationTime()));
    }

    string Parset::stationName(int index) const
    {
      return cache.stations[index].name;
    }

    int Parset::stationIndex(const std::string &name) const
    {
      std::vector<std::string> names = allStationNames();
      for (unsigned i = 0; i < names.size(); i++)
        if (names[i] == name)
          return i;

      return -1;
    }

    std::vector<std::string> Parset::allStationNames() const
    {
      vector<string> names(nrStations());

      for (unsigned station = 0; station < names.size(); ++station)
        names[station] = cache.stations[station].name;

      return names;
    }

    bool Parset::hasStorage() const
    {
      return getString("OLAP.OLAP_Conn.IONProc_Storage_Transport") != "NULL";
    }

    string Parset::getTransportType(const string& prefix) const
    {
      return getString(prefix + "_Transport");
    }

    unsigned Parset::nrStations() const
    {
      return cache.stations.size();
    }

    unsigned Parset::nrTabStations() const
    {
      return getStringVector("OLAP.tiedArrayStationNames",true).size();
    }

    std::vector<std::string> Parset::mergedStationNames() const
    {
      std::vector<string> tabStations = getStringVector("OLAP.tiedArrayStationNames",true);

      if (tabStations.empty())
        return allStationNames();
      else
        return tabStations;
    }

    unsigned Parset::nrMergedStations() const
    {
      const std::vector<unsigned> list = tabList();

      if (list.empty())
        return nrStations();

      return *std::max_element( list.begin(), list.end() ) + 1;
    }

    unsigned Parset::nrBaselines() const
    {
      unsigned stations;

      if (nrTabStations() > 0)
        stations = nrTabStations();
      else
        stations = nrStations();

      return stations * (stations + 1) / 2;
    }

    unsigned Parset::nrCrossPolarisations() const
    {
      return (getUint32("Observation.nrPolarisations") * getUint32("Observation.nrPolarisations"));
    }

    unsigned Parset::clockSpeed() const
    {
      return cache.clockMHz * 1000000;
    }

    double Parset::subbandBandwidth() const
    {
      return 1.0 * clockSpeed() / 1024;
    }

    double Parset::sampleDuration() const
    {
      return 1.0 / subbandBandwidth();
    }

    unsigned Parset::dedispersionFFTsize() const
    {
      return isDefined("OLAP.CNProc.dedispersionFFTsize") ? getUint32("OLAP.CNProc.dedispersionFFTsize") : CNintegrationSteps();
    }

    unsigned Parset::nrBitsPerSample() const
    {
      return cache.nrBitsPerSample;
    }

    unsigned Parset::CNintegrationSteps() const
    {
      return getUint32("OLAP.CNProc.integrationSteps");
    }

    unsigned Parset::IONintegrationSteps() const
    {
      return getUint32("OLAP.IONProc.integrationSteps");
    }

    unsigned Parset::integrationSteps() const
    {
      return CNintegrationSteps() * IONintegrationSteps();
    }

    unsigned Parset::coherentStokesTimeIntegrationFactor() const
    {
      return getUint32("OLAP.CNProc_CoherentStokes.timeIntegrationFactor");
    }

    unsigned Parset::incoherentStokesTimeIntegrationFactor() const
    {
      return getUint32("OLAP.CNProc_IncoherentStokes.timeIntegrationFactor");
    }

    unsigned Parset::coherentStokesChannelsPerSubband() const
    {
      unsigned numch = getUint32("OLAP.CNProc_CoherentStokes.channelsPerSubband");

      if (numch == 0)
        return nrChannelsPerSubband();
      else
        return numch;
    }

    unsigned Parset::incoherentStokesChannelsPerSubband() const
    {
      unsigned numch = getUint32("OLAP.CNProc_IncoherentStokes.channelsPerSubband");

      if (numch == 0)
        return nrChannelsPerSubband();
      else
        return numch;
    }

    std::string Parset::coherentStokes() const
    {
      return getString("OLAP.CNProc_CoherentStokes.which");
    }

    std::string Parset::incoherentStokes() const
    {
      return getString("OLAP.CNProc_IncoherentStokes.which");
    }

    bool Parset::outputCorrelatedData() const
    {
      return getBool("Observation.DataProducts.Output_Correlated.enabled", false);
    }

    bool Parset::outputBeamFormedData() const
    {
      return getBool("Observation.DataProducts.Output_Beamformed.enabled", false);
    }

    bool Parset::outputTrigger() const
    {
      return getBool("Observation.DataProducts.Output_Trigger.enabled", false);
    }

    bool Parset::outputThisType(OutputType outputType) const
    {
      return getBool(keyPrefix(outputType) + ".enabled", false);
    }

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

    bool Parset::fakeInputData() const
    {
      return getBool("OLAP.CNProc.fakeInputData", false);
    }

    bool Parset::checkFakeInputData() const
    {
      return getBool("OLAP.CNProc.checkFakeInputData", false);
    }

    double Parset::CNintegrationTime() const
    {
      return nrSamplesPerSubband() / subbandBandwidth();
    }

    double Parset::IONintegrationTime() const
    {
      return CNintegrationTime() * IONintegrationSteps();
    }

    unsigned Parset::nrSamplesPerSubband() const
    {
      return CNintegrationSteps() * nrChannelsPerSubband();
    }

    unsigned Parset::nrSamplesPerChannel() const
    {
      return CNintegrationSteps();
    }

    unsigned Parset::nrHistorySamples() const
    {
      return nrChannelsPerSubband() > 1 ? (nrPPFTaps() - 1) * nrChannelsPerSubband() : 0;
    }

    unsigned Parset::nrSamplesToCNProc() const
    {
      return nrSamplesPerSubband() + nrHistorySamples() + 32 / (NR_POLARIZATIONS * 2 * nrBitsPerSample() / 8);
    }

    unsigned Parset::inputBufferSize() const
    {
      return (unsigned) (getDouble("OLAP.nrSecondsOfBuffer", 1.0) * subbandBandwidth());
    }

    unsigned Parset::maxNetworkDelay() const
    {
      return (unsigned) (getDouble("OLAP.maxNetworkDelay", 0.25) * subbandBandwidth());
    }

    unsigned Parset::coherentStokesNrSubbandsPerFile() const
    {
      return std::min( getUint32("OLAP.CNProc_CoherentStokes.subbandsPerFile"), nrSubbands() );
    }

    unsigned Parset::incoherentStokesNrSubbandsPerFile() const
    {
      return std::min( getUint32("OLAP.CNProc_IncoherentStokes.subbandsPerFile"), nrSubbands() );
    }

    unsigned Parset::nrPPFTaps() const
    {
      return getUint32("OLAP.CNProc.nrPPFTaps");
    }

    unsigned Parset::nrChannelsPerSubband() const
    {
      return getUint32("Observation.channelsPerSubband");
    }

    vector<unsigned> Parset::subbandList() const
    {
      vector<unsigned> nrs(nrSubbands());

      for (unsigned subband = 0; subband < nrs.size(); ++subband)
        nrs[subband] = cache.subbands[subband].stationIdx;

      return nrs;
    }

    unsigned Parset::nrSubbands() const
    {
      return cache.subbands.size();
    }

    unsigned Parset::nrSubbandsPerSAP(unsigned sap) const
    {
      std::vector<unsigned> mapping = subbandToSAPmapping();

      return std::count(mapping.begin(), mapping.end(), sap);
    }

    double Parset::channelWidth() const
    {
      return subbandBandwidth() / nrChannelsPerSubband();
    }

    bool Parset::delayCompensation() const
    {
      return getBool("OLAP.delayCompensation", true);
    }

    unsigned Parset::nrCalcDelays() const
    {
      return getUint32("OLAP.DelayComp.nrCalcDelays", 16);
    }

    string Parset::positionType() const
    {
      return getString("OLAP.DelayComp.positionType", "ITRF");
    }

    double Parset::clockCorrectionTime(const std::string &station) const
    {
      return getDouble(std::string("PIC.Core.") + station + ".clockCorrectionTime",0.0);
    }

    bool Parset::correctBandPass() const
    {
      return getBool("OLAP.correctBandPass");
    }

    unsigned Parset::getLofarStManVersion() const
    {
      return getUint32("OLAP.LofarStManVersion", 2);
    }

    vector<unsigned> Parset::phaseOnePsets() const
    {
      return getUint32Vector("OLAP.CNProc.phaseOnePsets",true);
    }

    vector<unsigned> Parset::tabList() const
    {
      return getUint32Vector("OLAP.CNProc.tabList",true);
    }

    double Parset::channel0Frequency(size_t subband) const
    {
      double sbFreq = subbandToFrequencyMapping()[subband];

      if (nrChannelsPerSubband() == 1)
        return sbFreq;

      // if the 2nd PPF is used, the subband is shifted half a channel
      // downwards, so subtracting half a subband results in the
      // center of channel 0 (instead of the bottom).
      return sbFreq - 0.5 * subbandBandwidth();
    }

    unsigned Parset::nrSlotsInFrame() const
    {
      unsigned nrSlots = 0;

      nrSlots = getUint32("Observation.nrSlotsInFrame", 0);

      if (nrSlots == 0) {
        // return default
        return maxBeamletsPerRSP(nrBitsPerSample());
      }

      return nrSlots;
    }

    string Parset::partitionName() const
    {
      return getString("OLAP.CNProc.partition");
    }

    bool Parset::realTime() const
    {
      return getBool("OLAP.realTime");
    }

    std::vector<unsigned> Parset::nrTABs() const
    {
      std::vector<unsigned> counts(nrBeams());

      for (unsigned beam = 0; beam < nrBeams(); beam++)
        counts[beam] = nrTABs(beam);

      return counts;
    }

    unsigned Parset::totalNrTABs() const
    {
      std::vector<unsigned> beams = nrTABs();

      return std::accumulate(beams.begin(), beams.end(), 0);
    }

    unsigned Parset::maxNrTABs() const
    {
      std::vector<unsigned> beams = nrTABs();

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
      return getString("Observation.bandFilter");
    }

    string Parset::antennaSet() const
    {
      return getString("Observation.antennaSet");
    }

    string Parset::PVSS_TempObsName() const
    {
      return getString("_DPname","");
    }

    string Parset::AntennaSetsConf() const
    {
      return getString("OLAP.Storage.AntennaSetsConf","");
    }

    string Parset::AntennaFieldsDir() const
    {
      return getString("OLAP.Storage.AntennaFieldsDir","");
    }

    string Parset::HBADeltasDir() const
    {
      return getString("OLAP.Storage.HBADeltasDir","");
    }

    void StreamInfo::log() const
    {
      LOG_DEBUG_STR( "Stream " << stream << " is sap " << sap << " beam " << beam << " stokes " << stokes << " part " << part << " consisting of subbands " << subbands );
    }

    Transpose2::Transpose2( const Parset &parset )
      :
      nrChannels( parset.nrChannelsPerSubband() ),
      nrCoherentChannels( parset.coherentStokesChannelsPerSubband() ),
      nrIncoherentChannels( parset.incoherentStokesChannelsPerSubband() ),
      nrSamples( parset.CNintegrationSteps() ),
      coherentTimeIntFactor( parset.coherentStokesTimeIntegrationFactor() ),
      incoherentTimeIntFactor( parset.incoherentStokesTimeIntegrationFactor() ),

      nrBeams( parset.totalNrTABs() ),
      coherentNrSubbandsPerFile( parset.coherentStokesNrSubbandsPerFile() ),
      incoherentNrSubbandsPerFile( parset.incoherentStokesNrSubbandsPerFile() ),
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

      // ParameterSets are SLOW, so cache any info we need repeatedly

      std::vector<struct StreamInfo> infoset;
      const std::vector<unsigned> sapMapping = parset.subbandToSAPmapping();
      const unsigned nrSAPs = parset.nrBeams();
      const unsigned nrSubbands = parset.nrSubbands();
      const unsigned nrCoherentSubbandsPerFile = parset.coherentStokesNrSubbandsPerFile();
      const unsigned nrIncoherentSubbandsPerFile = parset.incoherentStokesNrSubbandsPerFile();

      const unsigned nrCoherentStokes = parset.coherentStokes().size();
      const StokesType coherentType = stokesType( parset.coherentStokes() );
      const unsigned nrCoherentChannels = parset.coherentStokesChannelsPerSubband();
      const unsigned nrCoherentTimeIntFactor = parset.coherentStokesTimeIntegrationFactor();

      const unsigned nrIncoherentStokes = parset.incoherentStokes().size();
      const StokesType incoherentType = stokesType( parset.incoherentStokes() );
      const unsigned nrIncoherentChannels = parset.incoherentStokesChannelsPerSubband();
      const unsigned nrIncoherentTimeIntFactor = parset.incoherentStokesTimeIntegrationFactor();

      const unsigned nrSamples = parset.CNintegrationSteps();

      struct StreamInfo info;
      info.stream = 0;

      for (unsigned sap = 0; sap < nrSAPs; sap++) {
        const unsigned nrBeams = parset.nrTABs(sap);

        info.sap = sap;

        std::vector<unsigned> sapSubbands;

        for (unsigned sb = 0; sb < nrSubbands; sb++)
          if (sapMapping[sb] == sap)
            sapSubbands.push_back(sb);

        for (unsigned beam = 0; beam < nrBeams; beam++) {
          info.beam = beam;

          const bool coherent = parset.isCoherent(sap, beam);
          const unsigned nrStokes = coherent ? nrCoherentStokes : nrIncoherentStokes;

          info.coherent = coherent;
          info.nrChannels = coherent ? nrCoherentChannels : nrIncoherentChannels;
          info.timeIntFactor = coherent ? nrCoherentTimeIntFactor : nrIncoherentTimeIntFactor;
          info.nrStokes = nrStokes;
          info.stokesType = coherent ? coherentType : incoherentType;
          info.nrSamples = nrSamples / info.timeIntFactor;

          const unsigned nrSubbandsPerFile = coherent ? nrCoherentSubbandsPerFile : nrIncoherentSubbandsPerFile;

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

  } // namespace RTCP
} // namespace LOFAR
