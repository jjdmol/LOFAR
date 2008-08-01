//#  CS1_Parset.cc: one line description
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

#if defined HAVE_APS

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
//#include <APL/APLCommon/APLUtilities.h>
#include <CS1_Interface/CS1_Parset.h>

#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>

namespace LOFAR {
namespace CS1 {

using namespace ACC::APS;

CS1_Parset::CS1_Parset() :
	name()
{
}

CS1_Parset::~CS1_Parset()
{
}

void CS1_Parset::IONodeRSPDestPorts(uint32 pset, vector<pair<string, string> > &RSPDestPort) const
{
  char buf[50];
  
  sprintf(buf,"PIC.Core.IONode[%d].RSP",pset);
  vector<string> snames = getStringVector(buf);
  string rspdest_ports;
    
  for (uint i = 0; i < snames.size(); i++) {
    rspdest_ports = getStringVector("PIC.Core." + snames[i].substr(0, snames[i].length()-1) + ".dest.ports")[atoi(snames[i].substr(snames[i].length()-1,snames[i].length()).c_str())];
    if (rspdest_ports.substr(0,5) == "file:" || rspdest_ports.substr(0,5) == "FILE:") {
      RSPDestPort.push_back(pair<string, string>(rspdest_ports.substr(rspdest_ports.find(":")+1),""));
    }
    else if (rspdest_ports.substr(0,4) == "udp:" || rspdest_ports.substr(0,4) == "UDP:" ||
             rspdest_ports.substr(0,4) == "tcp:" || rspdest_ports.substr(0,4) == "TCP:") {
      RSPDestPort.push_back(pair<string, string>(StringUtil::split(rspdest_ports, ':')[1], rspdest_ports.substr(rspdest_ports.rfind(":")+1)));
    }
    else if (rspdest_ports.find(":") != string::npos){
      // udp
      RSPDestPort.push_back(pair<string, string>(StringUtil::split(rspdest_ports, ':')[0], StringUtil::split(rspdest_ports, ':')[1]));
    }
    else {
      // file
      RSPDestPort.push_back(pair<string, string>(rspdest_ports,""));
    }
  }
}


vector<pair<string, unsigned> > CS1_Parset::getStationNamesAndRSPboardNumbers(unsigned psetNumber) const
{
  vector<string> inputs = getStringVector(string("PIC.Core.IONProc.") + partitionName() + '[' + boost::lexical_cast<string>(psetNumber) + "].inputs");
  vector<pair<string, unsigned> > stationsAndRSPs(inputs.size());

  for (unsigned i = 0; i < inputs.size(); i ++) {
    vector<string> split = StringUtil::split(inputs[i], '/');

    if (split.size() != 2 || split[1].substr(0, 3) != "RSP")
      throw std::runtime_error(string("expected stationname/RSPn pair in \"") + inputs[i] + '"');

    string   &stationName   = split[0];
    unsigned rspBoardNumber = boost::lexical_cast<unsigned>(split[1].substr(3));

    stationsAndRSPs[i] = pair<string, unsigned>(stationName, rspBoardNumber);
  }

  return stationsAndRSPs;
}


string CS1_Parset::getInputDescription(const string &stationName, unsigned rspBoardNumber) const
{
  return getStringVector(string("PIC.Core.Station.") + stationName + ".RSP.ports")[rspBoardNumber];
}


Stream *CS1_Parset::createStream(const string &description, bool asReader)
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
    throw std::runtime_error(string("unrecognized connector format: \"" + description + '"'));
}


uint32 CS1_Parset::nrSubbands() const
{
  uint32 nrSubbands(0);
  vector<int32> b2s = beamlet2subbands();
  
  for (uint i = 0; i < 54; i++) {
    if (b2s[i] != -1) {
      nrSubbands++;
    }  
  }
	
  return nrSubbands;
}

uint32 CS1_Parset::rspId(const string& stationName) const
{
  return(getUint32("PIC.Core." + stationName + ".RSP"));
}

string CS1_Parset::inputPortnr(const string& s) const
{
  uint32 index = s.find("_");
  string destPorts = getStringVector("PIC.Core." + s.substr(0, index) + "_RSP.dest.ports")[rspId(s)];
  uint32 i = destPorts.find(":");
  
  return destPorts.substr(i+1, 4);  
}

vector<int32>  CS1_Parset::beamlet2beams(uint32 rspid) const
{
  vector<int32> b2b;
  
  switch (rspid) {
    case 0 :	{
                  vector<int32>::const_iterator begin(&itsObservation.beamlet2beams[0]);
                  vector<int32>::const_iterator end(&itsObservation.beamlet2beams[54]);
		  
		  std::copy(begin,end,std::back_inserter(b2b));
		}
		
		return b2b;
		
    case 1 :	{
                  vector<int32>::const_iterator begin(&itsObservation.beamlet2beams[54]);
                  vector<int32>::const_iterator end(&itsObservation.beamlet2beams[108]);
                  
		  std::copy(begin,end,std::back_inserter(b2b));
		}
		
		return b2b;
		
    case 2 :	{
                  vector<int32>::const_iterator begin(&itsObservation.beamlet2beams[108]);
                  vector<int32>::const_iterator end(&itsObservation.beamlet2beams[162]);
                  
		  std::copy(begin,end,std::back_inserter(b2b));
 		}
		  
		return b2b;
		
    case 3 :	{
                  vector<int32>::const_iterator begin(&itsObservation.beamlet2beams[162]);
                  vector<int32>::const_iterator end(&itsObservation.beamlet2beams[216]);
                  
		  std::copy(begin,end,std::back_inserter(b2b));
 		}
		
		return b2b;
		 
    default :	return b2b;
  }
}

vector<int32>  CS1_Parset::beamlet2subbands(uint32 rspid) const
{
  vector<int32> b2s;
  
  switch (rspid) {
    case 0 :	{
                  vector<int32>::const_iterator begin(&itsObservation.beamlet2subbands[0]);
                  vector<int32>::const_iterator end(&itsObservation.beamlet2subbands[54]);
                  
		  std::copy(begin,end,std::back_inserter(b2s));
 		}  
		
		return b2s;
		
    case 1 :	{
                  vector<int32>::const_iterator begin(&itsObservation.beamlet2subbands[54]);
                  vector<int32>::const_iterator end(&itsObservation.beamlet2subbands[108]);
                  
		  std::copy(begin,end,std::back_inserter(b2s));
   		}  
		
		return b2s;
    
    case 2 :	{
                  vector<int32>::const_iterator begin(&itsObservation.beamlet2subbands[108]);
                  vector<int32>::const_iterator end(&itsObservation.beamlet2subbands[162]);
                  
		  std::copy(begin,end,std::back_inserter(b2s));
		}  
		
		return b2s;
   
    case 3 :	{
                  vector<int32>::const_iterator begin(&itsObservation.beamlet2subbands[162]);
                  vector<int32>::const_iterator end(&itsObservation.beamlet2subbands[216]);
		  
		  std::copy(begin,end,std::back_inserter(b2s));
		}
		
		return b2s;
		 
    default :	return b2s;
  }
}

vector<uint32>  CS1_Parset::subband2Index(uint32 rspid) const
{
  vector<int32> s2b = beamlet2subbands(rspid);
  vector<uint32> subband2Index;
  
  for (uint i = 0; i < s2b.size(); i++) {
    if (s2b[i] != -1) {
      subband2Index.push_back(i);
    }  
  }

  return subband2Index;
}

vector<double> CS1_Parset::positions() const
{
  vector<string> stNames = getStringVector("OLAP.storageStationNames");
  vector<double> pos, list;
  
  for (uint i = 0; i < nrStations(); i++)
  {
    pos = getDoubleVector("PIC.Core." + stNames[i] + ".position");
    list.insert(list.end(), pos.begin(), pos.end());
  }

  return list;
}

vector<double> CS1_Parset::getRefPhaseCentres() const
{
  vector<double> list;
  vector<string> stNames = getStringVector("OLAP.storageStationNames");
  int index;
  
  index = stNames[0].find("_");
  list = getDoubleVector(locateModule(stNames[0].substr(0,index)) + stNames[0].substr(0,index)  + ".phaseCenter");
 
  return list; 
}

vector<double> CS1_Parset::getPhaseCentresOf(const string& name) const
{
  vector<double> list;
  int index;
  
  index = name.find("_");
  list = getDoubleVector(locateModule(name.substr(0,index)) + name.substr(0,index)  + ".phaseCenter");
 
  return list; 
}

string CS1_Parset::getMSname(unsigned sb) const
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
  replace_all(name, "${SUBBAND}", str(format("%d") % sb));

  return name;
}

vector<string> CS1_Parset::getPortsOf(const string& aKey) const
{
  ParameterSet    pParset;
  
  string portsString("ports=" + expandedArrayString(getString(aKey + "_Ports")));
  pParset.adoptBuffer(portsString);
  
  return pParset.getStringVector("ports");
}

vector<double> CS1_Parset::getBeamDirection(const unsigned currentBeam) const
{
  char buf[50];
  std::vector<double> beamDirs(2);
 
  sprintf(buf,"Observation.Beam[%d].angle1",currentBeam);
  beamDirs[0] = getDouble(buf);
  sprintf(buf,"Observation.Beam[%d].angle2",currentBeam);
  beamDirs[1] = getDouble(buf);

  return beamDirs;
}

string CS1_Parset::getBeamDirectionType(const unsigned currentBeam) const
{
  char buf[50];
  string beamDirType;
 
  sprintf(buf,"Observation.Beam[%d].directionTypes",currentBeam);
  beamDirType = getString(buf);

  return beamDirType;
}

int CS1_Parset::findIndex(uint32 pset, const vector<uint32> &psets)
{
  unsigned index = std::find(psets.begin(), psets.end(), pset) - psets.begin();

  return index != psets.size() ? (int) index : -1;
}

//
// expandedArrayString(string)
//
// Given een array string ( '[ xx..xx, xx ]' ) this utility expands the string
// by replacing ranges with the fill series.
// Eg. [ lii001..lii003, lii005 ] --> [ lii001, lii002, lii003, lii005 ]
//
// ----------------------- ATTENTION !!!----------------------------------
// This routine has been copied to the Navigator software
// (MAC/Navigator/scripts/libs/nav_usr/CS1/CS1_Common.ctl)
// if you change anything struktural change the Navigator part also please
// -----------------------------------------------------------------------

string CS1_Parset::expandedArrayString(const string& orgStr)
{
	// any ranges in the string?
	if (orgStr.find("..",0) == string::npos) {
		return (orgStr);						// no, just return original
	}

	string	baseString(orgStr);					// destroyable copy
	ltrim(baseString, " 	[");				// strip of brackets
	rtrim(baseString, " 	]");

	// and split into a vector
	vector<string>	strVector = StringUtil::split(baseString, ',');

	// note: we assume that the format of each element is [xxxx]9999
	string::size_type	firstDigit(strVector[0].find_first_of("0123456789"));
	if (firstDigit == string::npos) {	// no digits? then return org string
		return (orgStr);
	}

	// construct scanmask and outputmask.
	string	elemName(strVector[0].substr(0,firstDigit));
	string	scanMask(elemName+"%ld");
	int		nrDigits;
	if (strVector[0].find("..",0) != string::npos) {	// range element?
		nrDigits = ((strVector[0].length() - 2)/2) - elemName.length();
	}
	else {
		nrDigits = strVector[0].length() - elemName.length();
	}
	string	outMask (formatString("%s%%0%dld", elemName.c_str(), nrDigits));

	// handle all elements
	string 	result("[");
	bool	firstElem(true);
	int		nrElems(strVector.size());
	for (int idx = 0; idx < nrElems; idx++) {
		long	firstVal;
		long	lastVal;
		// should match scanmask.
		if (sscanf(strVector[idx].c_str(), scanMask.c_str(), &firstVal) != 1) {
			LOG_DEBUG_STR("Element " << strVector[idx] << " does not match mask " 
					    	<< scanMask << ". Returning orignal string");
			return (orgStr);
		}

		// range element?
		string::size_type	rangePos(strVector[idx].find("..",0));
		if (rangePos == string::npos) {
			lastVal = firstVal;
		}
		else {	// yes, try to get second element.
			if (sscanf(strVector[idx].data()+rangePos+2, scanMask.c_str(), &lastVal) != 1) {
				LOG_DEBUG_STR("Second part of element " << strVector[idx]
							<< " does not match mask " << scanMask 
							<< ". Returning orignal string");
				return (orgStr);
			}
			// check range
			if (lastVal < firstVal) {
				LOG_DEBUG_STR("Illegal range specified in " << strVector[idx] <<
								". Returning orignal string");
				return (orgStr);
			}
		}

		// finally construct one or more elements
		for	(long val = firstVal ; val <= lastVal; val++) {
			if (firstElem) {
				result += formatString(outMask.c_str(), val);
				firstElem = false;
			}
			else {
				result += "," + formatString(outMask.c_str(), val);
			}
		}
	}

	return (result+"]");
}



} // namespace CS1
} // namespace LOFAR

#endif // defined HAVE_APS
