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

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
//#include <APL/APLCommon/APLUtilities.h>
#include <CS1_Interface/CS1_Parset.h>

namespace LOFAR {
	using namespace ACC::APS;
	namespace CS1 {

CS1_Parset::CS1_Parset() :
	name()
{
}

CS1_Parset::~CS1_Parset()
{
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

vector<double> CS1_Parset::phaseCenters() const
{
  vector<double> pos, list;
  vector<string> stNames = getStringVector("OLAP.storageStationNames");
  int index;
  
  for (uint i = 0; i < nrStations(); i++)
  {
    index = stNames[i].find("_");
    pos = getDoubleVector(locateModule(stNames[i].substr(0,index)) + stNames[i].substr(0,index)  + ".phaseCenter");
    list.insert(list.end(), pos.begin(), pos.end());
  }
 
  return list; 
}

vector<double> CS1_Parset::refFreqs() const
{
  vector<uint32> subbandIDs = getUint32Vector("Observation.subbandList");
  vector<double> refFreqs;
  
  for (uint i = 0; i < nrSubbands(); i++)
  {
   
    refFreqs.push_back((getUint32("Observation.nyquistZone")-1)*(getUint32("Observation.sampleClock")*1000000/2) + 
                        sampleRate()*subbandIDs[i]);
  }
  
  return refFreqs;
}

vector<string> CS1_Parset::msNames() const
{
  uint32 first, last;
  string msName;
  vector<string> msNames;
  char obsid[32];
  char subbandID[32];
  
  if (!isDefined("Observation.MSNameMask"))
  {
    sprintf(obsid, "%05u", getUint32("Observation.treeID"));
    string msNumber = "/data/L" + getString("Observation.year") + "_" + string(obsid);
  
    if (getUint32("OLAP.subbandsPerPset") == 1 )
    {
      for (uint i = 0; i < nrSubbands(); i++)
      {
        sprintf(subbandID, "_SB%u", i);
        msName = msNumber + string(subbandID) + ".MS";
        msNames.push_back(msName);
      }  
    }
    else
    {
      for (uint i = 0; i < nrSubbands()/getUint32("OLAP.subbandsPerPset") ; i++)
      {
        first = i * getUint32("OLAP.subbandsPerPset");
        last =  first + getUint32("OLAP.subbandsPerPset") -1;
        sprintf(subbandID, "_SB%u-%u", first, last);
        msName = msNumber + string(subbandID) + ".MS";
       msNames.push_back(msName);
      }
    }
  }
  else
    return getStringVector("Observation.MSNameMask");
 
  return (msNames);
}

vector<string> CS1_Parset::delay_Ports() const
{
  ParameterSet    dlParset;
  
  string delayString("dlPorts=" + expandedArrayString(getString("OLAP.DelayComp.ports")));
  dlParset.adoptBuffer(delayString);
  
  return dlParset.getStringVector("dlPorts");
}

vector<string> CS1_Parset::getPortsOf(const string& aKey) const
{
  ParameterSet    pParset;
  
  string portsString("ports=" + expandedArrayString(getString(aKey + "_Ports")));
  pParset.adoptBuffer(portsString);
  
  return pParset.getStringVector("ports");
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

string CS1_Parset::expandedArrayString(const string& orgStr) const
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
