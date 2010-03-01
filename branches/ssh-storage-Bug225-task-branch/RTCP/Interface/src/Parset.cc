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
//#include <APL/APLCommon/APLUtilities.h>
#include <Interface/Parset.h>
#include <Interface/Exceptions.h>
#include <Interface/PrintVector.h>

#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <cstdio>


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
    THROW(InterfaceException, string(key) << " contains wrong number (" << boost::lexical_cast<string>(getUint32Vector(key,true).size()) << ") of subbands (expected " << boost::lexical_cast<string>(nrSubbands()) << ')');
}

void Parset::check() const
{
  checkSubbandCount("Observation.beamList");
  checkSubbandCount("Observation.rspBoardList");
  checkSubbandCount("Observation.rspSlotList");

  unsigned		slotsPerFrame = nrSlotsInFrame();
  std::vector<unsigned> boards	      = subbandToRSPboardMapping();
  std::vector<unsigned> slots	      = subbandToRSPslotMapping();
  
  for (unsigned subband = 0; subband < slots.size(); subband ++)
    if (slots[subband] >= slotsPerFrame)
      THROW(InterfaceException, "Observation.rspSlotList contains slot numbers >= Observation.nrSlotsInFrame");
  
  // check not needed when using Storage
  if (isDefined("OLAP.CNProc.phaseOnePsets")) {
    std::vector<unsigned> inputs = phaseOnePsets();
    
    for (std::vector<unsigned>::const_iterator pset = inputs.begin(); pset != inputs.end(); pset ++) {
      unsigned nrRSPboards = getStationNamesAndRSPboardNumbers(*pset).size();

      for (unsigned subband = 0; subband < boards.size(); subband ++)
        if (boards[subband] >= nrRSPboards)
	  THROW(InterfaceException, "Observation.rspBoardList contains rsp board numbers that do not exist");
    }
  }
}


void Parset::maintainBackwardCompatibility()
{
  // maintain compatibility with MAC, which does not provide the latest greatest keys

  if (!isDefined("OLAP.CNProc.usedCoresInPset")) {
    LOG_WARN("Specifying \"OLAP.CNProc.coresPerPset\" instead of \"OLAP.CNProc.usedCoresInPset\" is deprecated");

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
  vector<string> inputs = getStringVector(string("PIC.Core.IONProc.") + partitionName() + '[' + boost::lexical_cast<string>(psetNumber) + "].inputs",true);
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


string Parset::getInputStreamName(const string &stationName, unsigned rspBoardNumber) const
{
  return getStringVector(string("PIC.Core.Station.") + stationName + ".RSP.ports",true)[rspBoardNumber];
}


Stream *Parset::createStream(const string &description, bool asReader)
{
  vector<string> split = StringUtil::split(description, ':');

  if (description == "null:")
    return new NullStream;
  else if (split.size() == 3 && split[0] == "udp")
    return new SocketStream(split[1].c_str(), boost::lexical_cast<short>(split[2]), SocketStream::UDP, asReader ? SocketStream::Server : SocketStream::Client);
  else if (split.size() == 3 && split[0] == "tcp")
    return new SocketStream(split[1].c_str(), boost::lexical_cast<short>(split[2]), SocketStream::TCP, asReader ? SocketStream::Server : SocketStream::Client);
  else if (split.size() == 2 && split[0] == "file")
    return asReader ? new FileStream(split[1].c_str()) : new FileStream(split[1].c_str(), 0666);
  else if (split.size() == 2)
    return new SocketStream(split[0].c_str(), boost::lexical_cast<short>(split[1]), SocketStream::UDP, asReader ? SocketStream::Server : SocketStream::Client);
  else if (split.size() == 1)
    return asReader ? new FileStream(split[0].c_str()) : new FileStream(split[0].c_str(), 0666);
  else
    THROW(InterfaceException, string("unrecognized connector format: \"" + description + '"'));
}


unsigned Parset::nyquistZone() const
{
  string bandFilter = getString("Observation.bandFilter");

  if (bandFilter == "LBA_30_80" ||
      bandFilter == "LBA_10_90" )
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
  vector<unsigned> beamMapping = subbandToBeamMapping();

  return *std::max_element(beamMapping.begin(), beamMapping.end()) + 1;
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

vector<double> Parset::getRefPhaseCentres() const
{
  vector<double> list;
  vector<string> stNames = getStringVector("OLAP.storageStationNames",true);
  int index;
  
  index = stNames[0].find("_");
  list = getDoubleVector(locateModule(stNames[0].substr(0,index)) + stNames[0].substr(0,index)  + ".phaseCenter");
 
  return list; 
}

vector<double> Parset::getPhaseCentresOf(const string& name) const
{
  vector<double> list;
  int index;
  
  index = name.find("_");
  list = getDoubleVector(locateModule(name.substr(0,index)) + name.substr(0,index)  + ".phaseCenter");
 
  return list; 
}

string Parset::getMSname(unsigned sb) const
{
  using namespace boost;

  string	 name	   = getString("Observation.MSNameMask");
  string	 startTime = getString("Observation.startTime");
  vector<string> splitStartTime;
  split(splitStartTime, startTime, is_any_of("- :"));

  replace_all(name, "${YEAR}", splitStartTime[0]);
  replace_all(name, "${MONTH}", splitStartTime[1]);
  replace_all(name, "${DAY}", splitStartTime[2]);
  replace_all(name, "${HOURS}", splitStartTime[3]);
  replace_all(name, "${MINUTES}", splitStartTime[4]);
  replace_all(name, "${SECONDS}", splitStartTime[5]);

  replace_all(name, "${MSNUMBER}", str(format("%05u") % getUint32("Observation.ObsID")));
  replace_all(name, "${SUBBAND}", str(format("%u") % sb));
  replace_all(name, "${BEAM}", str(format("%u") % subbandToBeamMapping()[sb]));

  return name;
}

string Parset::getMSBaseDir() const
{
  using namespace boost;
  
  string         name = this->getMSname(0);
  string         basedir;
  vector<string> splitName;
  
  split(splitName, name, is_any_of("/"));
  
  for (unsigned i = 0; i < splitName.size()-1 ; i++) {
    basedir += splitName[i] + '/';
  }
  return basedir;
}


unsigned short Parset::getStoragePort(const string &aKey, unsigned subband, unsigned output) const
{
  return static_cast<unsigned short>(getUint32Vector(aKey + "_Ports",true)[subband + output * nrSubbands()]);
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
 
  sprintf(buf,"Observation.Beam[%d].directionTypes", beam);
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


bool Parset::overlappingResources(const Parset *otherParset) const
{
  // return true if jobs (partially) use same cores within psets

  std::vector<uint32> myPsets    = usedPsets();
  std::vector<uint32> otherPsets = otherParset->usedPsets();
  std::vector<uint32> psets(myPsets.size() + otherPsets.size());

  bool overlappingPsets = set_intersection(myPsets.begin(), myPsets.end(), otherPsets.begin(), otherPsets.end(), psets.begin()) != psets.begin();

  if (!overlappingPsets)
    return false;

  std::vector<uint32> myCores    = usedCoresInPset();
  std::vector<uint32> otherCores = otherParset->usedCoresInPset();
  std::vector<uint32> cores(myCores.size() + otherCores.size());

  sort(myCores.begin(),    myCores.end());
  sort(otherCores.begin(), otherCores.end());

  bool overlappingCores = set_intersection(myCores.begin(), myCores.end(), otherCores.begin(), otherCores.end(), cores.begin()) != cores.begin();

  return overlappingCores;
}


int Parset::findIndex(uint32 pset, const vector<uint32> &psets)
{
  unsigned index = std::find(psets.begin(), psets.end(), pset) - psets.begin();

  return index != psets.size() ? (int) index : -1;
}

} // namespace RTCP
} // namespace LOFAR

#endif
