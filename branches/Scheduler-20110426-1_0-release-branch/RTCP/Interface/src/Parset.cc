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

#if ! defined HAVE_BGP_CN

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
#include <Interface/Parset.h>
#include <Interface/Exceptions.h>
#include <Interface/PrintVector.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

using boost::format;

#include <algorithm>
#include <cstdio>
#include <set>


namespace LOFAR {
namespace RTCP {


Parset::Parset(const char *name)
:
  ParameterSet(name),
  itsName(name)
{
  maintainBackwardCompatibility();
  check();
}


Parset::~Parset()
{
}


void Parset::checkSubbandCount(const char *key) const
{
  if (getUint32Vector(key,true).size() != nrSubbands())
    THROW(InterfaceException, string(key) << " contains wrong number (" << getUint32Vector(key,true).size() << ") of subbands (expected " << nrSubbands() << ')');
}


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
  checkSubbandCount("Observation.beamList");
  checkInputConsistency();

  if (CNintegrationSteps() % dedispersionFFTsize() != 0)
    THROW(InterfaceException, "OLAP.CNProc.integrationSteps (" << CNintegrationSteps() << ") must be divisible by OLAP.CNProc.dedispersionFFTsize (" << dedispersionFFTsize() << ')');
}


void Parset::maintainBackwardCompatibility()
{
  // maintain compatibility with MAC, which does not provide the latest greatest keys

  if (!isDefined("OLAP.CNProc.usedCoresInPset")) {
    //LOG_WARN("Specifying \"OLAP.CNProc.coresPerPset\" instead of \"OLAP.CNProc.usedCoresInPset\" is deprecated");

    unsigned		  coresPerPset = getUint32("OLAP.CNProc.coresPerPset");
    std::vector<unsigned> usedCoresInPset(coresPerPset);

    for (unsigned core = 0; core < coresPerPset; core ++)
      usedCoresInPset[core] = core;

    std::stringstream str;
    str << usedCoresInPset;
    add("OLAP.CNProc.usedCoresInPset", str.str());
  }
}


vector<Parset::StationRSPpair> Parset::getStationNamesAndRSPboardNumbers(unsigned psetNumber) const
{
  vector<string> inputs = getStringVector(str(format("PIC.Core.IONProc.%s[%u].inputs") % partitionName() % psetNumber),true);
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
  return getStringVector(string("PIC.Core.Station.") + stationName + ".RSP.ports",true)[rspBoardNumber];
}


string Parset::constructBeamFormedFilename( const string &mask, unsigned beam, unsigned stokes, unsigned file ) const
{
  using namespace boost;

  string         name = mask;
  /*
  string	 startTime = getString("Observation.startTime");
  vector<string> splitStartTime;
  split(splitStartTime, startTime, is_any_of("- :"));

  replace_all(name, "${YEAR}", splitStartTime[0]);
  replace_all(name, "${MONTH}", splitStartTime[1]);
  replace_all(name, "${DAY}", splitStartTime[2]);
  replace_all(name, "${HOURS}", splitStartTime[3]);
  replace_all(name, "${MINUTES}", splitStartTime[4]);
  replace_all(name, "${SECONDS}", splitStartTime[5]);
  */

  replace_all(name, "${OBSID}", str(format("%05u") % observationID()));
  replace_all(name, "${MSNUMBER}", str(format("%05u") % observationID()));
  replace_all(name, "${SAP}", "000"); // station beams not supported yet
  replace_all(name, "${PART}", str(format("%03u") % file));
  replace_all(name, "${BEAM}", str(format("%03u") % beam));
  replace_all(name, "${STOKES}", str(format("%u") % stokes));

  return name;
}


string Parset::constructSubbandFilename( const string &mask, unsigned subband ) const
{
  using namespace boost;

  string         name = mask;
  /*
  string	 startTime = getString("Observation.startTime");
  vector<string> splitStartTime;
  split(splitStartTime, startTime, is_any_of("- :"));

  replace_all(name, "${YEAR}", splitStartTime[0]);
  replace_all(name, "${MONTH}", splitStartTime[1]);
  replace_all(name, "${DAY}", splitStartTime[2]);
  replace_all(name, "${HOURS}", splitStartTime[3]);
  replace_all(name, "${MINUTES}", splitStartTime[4]);
  replace_all(name, "${SECONDS}", splitStartTime[5]);
  */

  replace_all(name, "${OBSID}", str(format("%05u") % observationID()));
  replace_all(name, "${MSNUMBER}", str(format("%05u") % observationID()));
  replace_all(name, "${SAP}", str(format("%03u") % subbandToSAPmapping()[subband]));
  replace_all(name, "${SUBBAND}", str(format("%03u") % subband));

  return name;
}


unsigned Parset::nyquistZone() const
{
  string bandFilter = getString("Observation.bandFilter");

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

  THROW(InterfaceException, string("unknown band filter \"" + bandFilter + '"'));
}


unsigned Parset::nrBeams() const
{
  vector<unsigned> sapMapping = subbandToSAPmapping();

  return *std::max_element(sapMapping.begin(), sapMapping.end()) + 1;
}


vector<double> Parset::subbandToFrequencyMapping() const
{
  unsigned	   subbandOffset = 512 * (nyquistZone() - 1);
  
  vector<unsigned> subbandIds = getUint32Vector("Observation.subbandList",true);
  vector<double>   subbandFreqs(subbandIds.size());

  for (unsigned subband = 0; subband < subbandIds.size(); subband ++)
    subbandFreqs[subband] = sampleRate() * (subbandIds[subband] + subbandOffset);

  return subbandFreqs;
}

vector<double> Parset::centroidPos(const string &stations) const
{
  vector<double> Centroid, posList, pos;
  Centroid.resize(3);
  
  vector<string> stationList = StringUtil::split(stations, '+');
  for (uint i = 0; i < stationList.size(); i++)
  {   
    pos = getDoubleVector("PIC.Core." + stationList[i] + ".position");
    posList.insert(posList.end(), pos.begin(), pos.end()); 
  }
  
  for (uint i = 0; i < posList.size()/3; i++)
  {
    Centroid[0] += posList[3*i];   // x in m
    Centroid[1] += posList[3*i+1]; // y in m
    Centroid[2] += posList[3*i+2]; // z in m
  }  
  
  Centroid[0] /= posList.size()/3;
  Centroid[1] /= posList.size()/3;
  Centroid[2] /= posList.size()/3;
   
  return Centroid;
}

vector<double> Parset::positions() const
{
  vector<string> stNames;
  vector<double> pos, list;
  unsigned nStations;
  
  if (nrTabStations() > 0) {
    stNames = getStringVector("OLAP.tiedArrayStationNames",true);
    nStations = nrTabStations();
  }
  else {
    stNames = getStringVector("OLAP.storageStationNames",true);
    nStations = nrStations();
  }
  
  for (uint i = 0; i < nStations; i++)
  {
    if (stNames[i].find("+") != string::npos)
      pos = centroidPos(stNames[i]);
    else
      pos = getDoubleVector("PIC.Core." + stNames[i] + ".position");
    
    list.insert(list.end(), pos.begin(), pos.end());
  }

  return list;
}

vector<double> Parset::getRefPhaseCentre() const
{
  vector<double> list;
  list = getDoubleVector("Observation.referencePhaseCenter");
 
  return list; 
}

vector<double> Parset::getPhaseCentreOf(const string& name) const
{
  return getDoubleVector(str(format("PIC.Core.%s.phaseCenter") % name));
}


vector<double> Parset::getManualPencilBeam(const unsigned pencil) const
{
  char buf[50];
  std::vector<double> pencilBeam(2);
 
  sprintf(buf, "OLAP.Pencil[%d].angle1", pencil);
  pencilBeam[0] = getDouble(buf);
  sprintf(buf, "OLAP.Pencil[%d].angle2", pencil);
  pencilBeam[1] = getDouble(buf);

  return pencilBeam;
}


vector<double> Parset::getBeamDirection(const unsigned beam) const
{
  char buf[50];
  std::vector<double> beamDirs(2);
 
  sprintf(buf, "Observation.Beam[%d].angle1", beam);
  beamDirs[0] = getDouble(buf);
  sprintf(buf, "Observation.Beam[%d].angle2", beam);
  beamDirs[1] = getDouble(buf);

  return beamDirs;
}


string Parset::getBeamDirectionType(const unsigned beam) const
{
  char buf[50];
  string beamDirType;
 
  sprintf(buf,"Observation.Beam[%d].directionType", beam);
  beamDirType = getString(buf);

  return beamDirType;
}


vector<uint32> Parset::usedPsets() const
{
  std::vector<uint32> phaseone   = phaseOnePsets();
  std::vector<uint32> phasetwo   = phaseTwoPsets();
  std::vector<uint32> phasethree = phaseThreePsets();

  std::vector<uint32> one_two(phaseone.size() + phasetwo.size());
  std::vector<uint32> one_two_three(phaseone.size() + phasetwo.size() + phasethree.size());

  sort(phaseone.begin(), phaseone.end());
  sort(phasetwo.begin(), phasetwo.end());
  sort(phasethree.begin(), phasethree.end());

  one_two.resize(set_union(phaseone.begin(), phaseone.end(), phasetwo.begin(), phasetwo.end(), one_two.begin()) - one_two.begin());
  one_two_three.resize(set_union(one_two.begin(), one_two.end(), phasethree.begin(), phasethree.end(), one_two_three.begin()) - one_two_three.begin());

  return one_two_three;
}


bool Parset::disjointCores(const Parset &otherParset, std::stringstream &error) const
{
  // return false if jobs (partially) use same cores within psets

  std::vector<uint32> myPsets    = usedPsets();
  std::vector<uint32> otherPsets = otherParset.usedPsets();
  std::vector<uint32> overlappingPsets(myPsets.size() + otherPsets.size());

  overlappingPsets.resize(set_intersection(myPsets.begin(), myPsets.end(), otherPsets.begin(), otherPsets.end(), overlappingPsets.begin()) - overlappingPsets.begin());

  if (overlappingPsets.size() == 0)
    return true;

  std::vector<uint32> myCores    = usedCoresInPset();
  std::vector<uint32> otherCores = otherParset.usedCoresInPset();
  std::vector<uint32> overlappingCores(myCores.size() + otherCores.size());

  sort(myCores.begin(),    myCores.end());
  sort(otherCores.begin(), otherCores.end());

  overlappingCores.resize(set_intersection(myCores.begin(), myCores.end(), otherCores.begin(), otherCores.end(), overlappingCores.begin()) - overlappingCores.begin());

  if (overlappingCores.size() == 0)
    return true;

  error << "cores " << overlappingCores << " within psets " << overlappingPsets << " overlap;";
  return false;
}


bool Parset::compatibleInputSection(const Parset &otherParset, std::stringstream &error) const
{
  std::vector<uint32> myStations    = phaseOnePsets();
  std::vector<uint32> otherStations = otherParset.phaseOnePsets();
  std::vector<uint32> psets(myStations.size() + otherStations.size());

  bool overlappingStations = set_intersection(myStations.begin(), myStations.end(), otherStations.begin(), otherStations.end(), psets.begin()) != psets.begin();
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


int Parset::findIndex(uint32 pset, const vector<uint32> &psets)
{
  unsigned index = std::find(psets.begin(), psets.end(), pset) - psets.begin();

  return index != psets.size() ? static_cast<int>(index) : -1;
}

} // namespace RTCP
} // namespace LOFAR

#endif
