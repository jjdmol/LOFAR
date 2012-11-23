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
#include <ApplCommon/PosixTime.h>
#include <Interface/Parset.h>
#include <Interface/SmartPtr.h>
#include <Interface/Exceptions.h>
#include <Interface/PrintVector.h>
#include <Interface/SetOperations.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <cstdio>
#include <set>


namespace LOFAR {
namespace RTCP {


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
}


Parset::Parset(Stream *stream)
{
  uint64 size;
  stream->read(&size, sizeof size);

#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, &size, 1);
#endif

  std::vector<char> tmp(size + 1);
  stream->read(&tmp[0], size);
  tmp[size] = '\0';

  std::string buffer(&tmp[0], size);
  adoptBuffer(buffer);
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


void Parset::checkVectorLength(const std::string &key, unsigned expectedSize) const
{
  unsigned actualSize = getStringVector(key, true).size();

  if (actualSize != expectedSize)
    THROW(InterfaceException, "Key \"" << string(key) << "\" contains wrong number of entries (expected: " << expectedSize << ", actual: " << actualSize << ')');
}


#if 0
void Parset::checkPsetAndCoreConfiguration() const
{
  std::vector<unsigned> phaseOnePsets = phaseOnePsets();
  std::vector<unsigned> phaseTwoPsets = phaseTwoPsets();
  std::vector<unsigned> phaseThreePsets = phaseThreePsets();
  std::vector<unsigned> phaseOneTwoCores = phaseOneTwoCores();
  std::vector<unsigned> phaseThreeCores = phaseThreeCores();

  if (phaseOnePsets.size() == 0 || 
}
#endif


void Parset::checkInputConsistency() const
{
  using std::set;

  map<string, set<unsigned> > allRSPboards;
  vector<unsigned> inputs = phaseOnePsets();
  
  for (vector<unsigned>::const_iterator pset = inputs.begin(); pset != inputs.end(); pset ++) {
    vector<StationRSPpair> stationRSPpairs = getStationNamesAndRSPboardNumbers(*pset);

    for (vector<StationRSPpair>::const_iterator pair = stationRSPpairs.begin(); pair != stationRSPpairs.end(); pair ++) {
      const string &station = pair->station;
      unsigned     rsp      = pair->rsp;

      map<string, set<unsigned> >::const_iterator stationRSPs = allRSPboards.find(station);

      if (stationRSPs != allRSPboards.end() && stationRSPs->second.find(rsp) != stationRSPs->second.end())
	THROW(InterfaceException, station << "/RSP" << rsp << " multiple times defined in \"PIC.Core.IONProc.*.inputs\"");

      allRSPboards[station].insert(rsp);
    }
  }

  for (map<string, set<unsigned> >::const_iterator stationRSPs = allRSPboards.begin(); stationRSPs != allRSPboards.end(); stationRSPs ++) {
    const string	  &station = stationRSPs->first;
    const set<unsigned> &rsps    = stationRSPs->second;

    vector<unsigned> rspsOfStation  = subbandToRSPboardMapping(station);
    vector<unsigned> slotsOfStation = subbandToRSPslotMapping(station);

    if (rspsOfStation.size() != nrSubbands())
      THROW(InterfaceException, string("the size of \"Observation.Dataslots.") + station + ".RSPBoardList\" does not equal the number of subbands");

    if (slotsOfStation.size() != nrSubbands())
      THROW(InterfaceException, string("the size of \"Observation.Dataslots.") + station + ".DataslotList\" does not equal the number of subbands");

    for (int subband = nrSubbands(); -- subband >= 0;) {
      if (rsps.find(rspsOfStation[subband]) == rsps.end())
	THROW(InterfaceException, "\"Observation.Dataslots." << station << ".RSPBoardList\" mentions RSP board " << rspsOfStation[subband] << ", which does not exist");

      if (slotsOfStation[subband] >= nrSlotsInFrame())
	THROW(InterfaceException, "\"Observation.Dataslots." << station << ".DataslotList\" mentions RSP slot " << slotsOfStation[subband] << ", which is more than the number of slots in a frame");
    }
  }
}

void Parset::check() const
{
  //checkPsetAndCoreConfiguration();
  checkInputConsistency();
  checkVectorLength("Observation.beamList", nrSubbands());
  
  for (OutputType outputType = FIRST_OUTPUT_TYPE; outputType < LAST_OUTPUT_TYPE; outputType ++)
    if (outputThisType(outputType)) {
      std::string prefix   = keyPrefix(outputType);
      unsigned    expected = nrStreams(outputType);

      checkVectorLength(prefix + ".locations", expected);
      checkVectorLength(prefix + ".filenames", expected);
    }

  if (CNintegrationSteps() % dedispersionFFTsize() != 0)
    THROW(InterfaceException, "OLAP.CNProc.integrationSteps (" << CNintegrationSteps() << ") must be divisible by OLAP.CNProc.dedispersionFFTsize (" << dedispersionFFTsize() << ')');

  if (outputThisType(BEAM_FORMED_DATA) || outputThisType(TRIGGER_DATA)) {
    // second transpose is performed

    if (nrSubbands() > phaseTwoPsets().size() * phaseOneTwoCores().size() )
      THROW(InterfaceException, "For the second transpose to function, there need to be at least nrSubbands cores in phase 2 (requested: " << nrSubbands() << " subbands on " << (phaseTwoPsets().size() * phaseOneTwoCores().size()) << " cores)");
  }

  // check whether the beam forming parameters are valid
  const Transpose2 &logic = transposeLogic();

  for (unsigned i = 0; i < logic.nrStreams(); i++) {
    const StreamInfo &info = logic.streamInfo[i];

    if ( info.timeIntFactor == 0 )
      THROW(InterfaceException, "Temporal integration factor needs to be > 0 (it is set to 0 for " << (info.coherent ? "coherent" : "incoherent") << " beams).");

    if ( info.coherent
      && info.stokesType == STOKES_XXYY
      && info.timeIntFactor != 1 )
      THROW(InterfaceException, "Cannot perform temporal integration if calculating Coherent Stokes XXYY. Integration factor needs to be 1, but is set to " << info.timeIntFactor);
  }
}


vector<Parset::StationRSPpair> Parset::getStationNamesAndRSPboardNumbers(unsigned psetNumber) const
{
  vector<string> inputs = getStringVector(str(boost::format("PIC.Core.IONProc.%s[%u].inputs") % partitionName() % psetNumber), true);
  vector<StationRSPpair> stationsAndRSPs(inputs.size());

  for (unsigned i = 0; i < inputs.size(); i ++) {
    vector<string> split = StringUtil::split(inputs[i], '/');

    if (split.size() != 2 || split[1].substr(0, 3) != "RSP")
      THROW(InterfaceException, string("expected stationname/RSPn pair in \"") << inputs[i] << '"');

    stationsAndRSPs[i].station = split[0];
    stationsAndRSPs[i].rsp     = boost::lexical_cast<unsigned>(split[1].substr(3));
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
    case TRIGGER_DATA:	    return "Observation.DataProducts.Output_Trigger";
    default:		    THROW(InterfaceException, "Unknown output type");
  }
}


std::string Parset::getHostName(OutputType outputType, unsigned streamNr) const
{
  return StringUtil::split(getStringVector(keyPrefix(outputType) + ".locations", true)[streamNr], ':')[0];
}


std::string Parset::getFileName(OutputType outputType, unsigned streamNr) const
{
  return getStringVector(keyPrefix(outputType) + ".filenames", true)[streamNr];
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
    case CORRELATED_DATA :   return nrSubbands();
    case BEAM_FORMED_DATA :         // FALL THROUGH
    case TRIGGER_DATA :      return transposeLogic().nrStreams();
    default:		     THROW(InterfaceException, "Unknown output type");
  }
}


unsigned Parset::maxNrStreamsPerPset(OutputType outputType, bool force) const
{
  unsigned nrOutputStreams = nrStreams(outputType, force);
  unsigned nrPsets;

  switch (outputType) {
    case CORRELATED_DATA :   nrPsets = phaseTwoPsets().size();
			     break;

    case BEAM_FORMED_DATA :         // FALL THROUGH
    case TRIGGER_DATA :	     nrPsets = phaseThreePsets().size();
			     break;

    default:		     THROW(InterfaceException, "Unknown output type");
  }

  return nrPsets == 0 ? 0 : (nrOutputStreams + nrPsets - 1) / nrPsets;
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

  THROW(InterfaceException, std::string("unknown band filter \"" + bandFilter + '"'));
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
  unsigned		subbandOffset = 512 * (nyquistZone() - 1);
  
  std::vector<unsigned> subbandIds = getUint32Vector("Observation.subbandList", true);
  std::vector<double>   subbandFreqs(subbandIds.size());

  for (unsigned subband = 0; subband < subbandIds.size(); subband ++)
    subbandFreqs[subband] = sampleRate() * (subbandIds[subband] + subbandOffset);

  return subbandFreqs;
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
  
  for (unsigned i = 0; i < posList.size() / 3; i ++)
  {
    Centroid[0] += posList[3*i];   // x in m
    Centroid[1] += posList[3*i+1]; // y in m
    Centroid[2] += posList[3*i+2]; // z in m
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


std::vector<double> Parset::getPencilBeam(unsigned beam, unsigned pencil) const
{
  std::vector<double> pencilBeam(2);
 
  pencilBeam[0] = getDouble(str(boost::format("Observation.Beam[%u].TiedArrayBeam[%u].angle1") % beam % pencil));
  pencilBeam[1] = getDouble(str(boost::format("Observation.Beam[%u].TiedArrayBeam[%u].angle2") % beam % pencil));

  return pencilBeam;
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


std::vector<string> Parset::pencilBeamStationList(unsigned beam, unsigned pencil) const
{
  string key = str(boost::format("Observation.Beam[%u].TiedArrayBeam[%u].stationList") % beam % pencil);
  std::vector<string> stations;
  
  if (isDefined(key))
    stations = getStringVector(key,true);

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


std::vector<unsigned> Parset::usedCoresInPset() const
{
  return phaseOneTwoCores() | phaseThreeCores();
}


std::vector<unsigned> Parset::usedPsets() const
{
  return phaseOnePsets() | phaseTwoPsets() | phaseThreePsets();
}


bool Parset::phaseThreeDisjunct() const
{
  return (phaseOneTwoCores() & phaseThreeCores()).empty() && ((phaseOnePsets() | phaseTwoPsets()) & phaseThreePsets()).empty();
}


bool Parset::disjointCores(const Parset &otherParset, std::stringstream &error) const
{
  // return false if jobs (partially) use same cores within psets

  std::vector<unsigned> overlappingPsets = usedPsets() & otherParset.usedPsets();
  std::vector<unsigned> overlappingCores = usedCoresInPset() & otherParset.usedCoresInPset();

  if (overlappingPsets.empty() || overlappingCores.empty())
    return true;

  error << "cores " << overlappingCores << " within psets " << overlappingPsets << " overlap;";
  return false;
}


bool Parset::compatibleInputSection(const Parset &otherParset, std::stringstream &error) const
{
  bool overlappingStations = !(phaseOnePsets() & otherParset.phaseOnePsets()).empty();
  bool good = true;

  if (overlappingStations) {
    if (nrBitsPerSample() != otherParset.nrBitsPerSample()) {
      error << " uses " << nrBitsPerSample() << " instead of " << otherParset.nrBitsPerSample() << " bits;";
      good = false;
    }

    if (clockSpeed() != otherParset.clockSpeed()) {
      error << " uses " << clockSpeed() / 1e6 << " instead of " << otherParset.clockSpeed() / 1e6 << " MHz clock;";
      good = false;
    }

    if (nrSlotsInFrame() != otherParset.nrSlotsInFrame()) {
      error << " uses " << nrSlotsInFrame() << " instead of " << otherParset.nrSlotsInFrame() << " slots per frame;";
      good = false;
    }
  }

  return good;
}


bool Parset::conflictingResources(const Parset &otherParset, std::stringstream &error) const
{
  return !disjointCores(otherParset, error) | !compatibleInputSection(otherParset, error); // no McCarthy evaluation
}


vector<unsigned> Parset::subbandToRSPboardMapping(const string &stationName) const
{
  std::string key = std::string("Observation.Dataslots.") + stationName + ".RSPBoardList";

  if (!isDefined(key)) {
    //LOG_WARN_STR('"' << key << "\" not defined, trying \"Observation.rspBoardList\"");
    key = "Observation.rspBoardList";
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

  return getUint32Vector(key, true);
}


int Parset::findIndex(unsigned pset, const vector<unsigned> &psets)
{
  unsigned index = std::find(psets.begin(), psets.end(), pset) - psets.begin();

  return index != psets.size() ? static_cast<int>(index) : -1;
}

} // namespace RTCP
} // namespace LOFAR
