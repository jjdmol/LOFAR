//#  AntennaSets.cc: one_line_description
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include <Common/LofarLocators.h>
#include <Common/lofar_fstream.h>
#include <ApplCommon/AntennaSets.h>

namespace LOFAR {

const int	IDX_CORE	= 0;
const int	IDX_REMOTE	= 1;
const int	IDX_EUROPE	= 2;

//-------------------------- creation and destroy ---------------------------
static AntennaSets* globalAntennaSetsInstance = 0;

AntennaSets* globalAntennaSets()
{
  if (globalAntennaSetsInstance == 0) {
    globalAntennaSetsInstance = new AntennaSets("AntennaSets.conf");
  }
  return (globalAntennaSetsInstance);
}

//
// AntennaSets(fileName)
//
AntennaSets::AntennaSets(const string& filename) :
	itsAntennaSetFile(filename)
{
	ConfigLocator	cl;
	itsAntennaSetFile = cl.locate(filename);
	ifstream	inputStream;
	inputStream.open(itsAntennaSetFile.c_str());

	ASSERTSTR(inputStream.good(), "File " << filename << " cannot be opened succesfully.");

	char		line      [2048];
	char		setName   [1024];
	char		fieldName [1024];
	char		stnType   [1024];
	char		selector  [1024];
	string		curName;
	int			lineNr(0);
	setTriple*	newTriple(0);
	bitset<3>	stnTypeMask;
	stnTypeMask.reset();
	// read file and skip lines that start with '#' or are empty
	while(inputStream.getline(line, 2048)) {
		lineNr++;
		if (line[0] == '#' || line[0] == '\0') {
			continue;
		}	
		
		ASSERTSTR (sscanf(line, "%s%s%s%s", setName, fieldName, stnType, selector) == 4, 
							"Line " << lineNr << " (" << line << ") has the wrong format");

		if (stnTypeMask.count() == 0) {
			newTriple = new setTriple();
			curName = setName;
			ASSERTSTR(newTriple, "Cannot get memory for storing the antennaSets");
		}
		else {
			ASSERTSTR(curName.compare(setName) == 0, 
					lineNr << ": Definition of antennaSet '" << curName << "' is not yet complete.");
		}

		if ((!strcmp(stnType,"Europe") && stnTypeMask[IDX_EUROPE]) || 
			(!strcmp(stnType,"Remote") && stnTypeMask[IDX_REMOTE]) || 
			(!strcmp(stnType,"Core")   && stnTypeMask[IDX_CORE])) {
			ASSERTSTR(false, lineNr << ": StationType '" << stnType << "' already defined for antennaSet '" << setName << "'");
		}
	
		ASSERTSTR(!strcmp(stnType,"Europe") || !strcmp(stnType,"Remote") || !strcmp(stnType,"Core"), 
				lineNr << ": Unknown stationType '" << stnType << "'");

		try {
			if (!strcmp(stnType,"Europe") && _adoptSelector(selector, fieldName, newTriple->europe, 192)) {
				stnTypeMask.set(IDX_EUROPE);
			}
			if (!strcmp(stnType,"Remote") && _adoptSelector(selector, fieldName, newTriple->remote, 96)) {
				stnTypeMask.set(IDX_REMOTE);
			}
			if (!strcmp(stnType,"Core") && _adoptSelector(selector, fieldName, newTriple->core, 96)) {
				stnTypeMask.set(IDX_CORE);
			}
		}
		catch (Exception&	ex) {
			ASSERTSTR(false, lineNr << ex.what());
		}

		if (stnTypeMask.count() == 3) {
			LOG_DEBUG_STR("Storing definition for antennaSet '" << setName << "'");
			ASSERTSTR(itsDefinitions.find(setName) == itsDefinitions.end(), 
						lineNr << ": AntennaSet '" << setName << "' already defined");
			itsDefinitions[setName] = *newTriple;
			delete newTriple;
			newTriple = 0;
			stnTypeMask.reset();
		}
	} // while not EOF

	ASSERTSTR(itsDefinitions.size() != 0, "File '" << itsAntennaSetFile << "' does not contain antennaSet definitions");
	ASSERTSTR(stnTypeMask.count() == 0 || stnTypeMask.count() == 3, "Last definition '" << setName << "' is not complete.");

	LOG_INFO_STR("Found " << itsDefinitions.size() << " antennaSet definitions in " << itsAntennaSetFile);
		
}

//
// ~AntennaSets()
//
AntennaSets::~AntennaSets()
{
}

//
// _adoptSelector(selector, singleSet, rcuCount)
//
bool AntennaSets::_adoptSelector(const string&	selector, const string& antennaField, singleSet&	antSet, uint rcuCount)
{
	ASSERTSTR(!selector.empty(), "SelectorString may not be empty");
	ASSERTSTR(rcuCount, "rcuCount must be > 0");

	antSet.posIndex.assign(MAX_RCUS, -1);	// reset index array
	uint	strLen(selector.length());
	uint	rcuNr(0);
	uint	sIdx(0);
	while (sIdx < strLen) {
		// first get the loopcount if any
		uint	loopCnt(0);
		while (sIdx < strLen && isdigit(selector[sIdx])) {
			loopCnt = loopCnt * 10 + (selector[sIdx] - '0');
			sIdx++;
		}
		if (!loopCnt) {
			loopCnt = 1;
		}

		// next get the pattern
		string	pattern;
		while (sIdx < strLen && !isdigit(selector[sIdx])) {
			pattern += selector[sIdx];
			sIdx++;
		}
		ASSERTSTR(pattern.length(), "Expected a pattern at position " << sIdx << " of selector " << selector);

		// now we have both the loopcount and the pattern, apply it to Set.
		for (uint l = 0; l < loopCnt; l++) {
			for (uint p = 0; p < pattern.length(); p++) {
				char	input = pattern[p];
				ASSERTSTR(input=='l' || input=='h' || input=='H' || input=='.', 
						"character '" << input << 
						"' not allowed for selecting an RCUinput, only lhH. are allowed.");
				ASSERTSTR(rcuNr < (uint)MAX_RCUS, 
						"selector:'"<< selector << "' specified more than " << MAX_RCUS << " RCUs");

				switch (input) {
				case 'l':
					antSet.LBAallocation.set(rcuNr);
					antSet.posIndex[rcuNr] = rcuCount + rcuNr;
					break;
				case 'h':
					antSet.LBAallocation.set(rcuNr);
					antSet.posIndex[rcuNr] = rcuNr;
					break;
				case 'H' :
					antSet.HBAallocation.set(rcuNr);
					if (antennaField != "HBA1") {
						antSet.posIndex[rcuNr] = rcuNr;
					}
					else {
						antSet.posIndex[rcuNr] = rcuNr - (rcuCount/2);
					}
				}
				antSet.RCUinputs[rcuNr] = input;
				rcuNr++;
			} // pattern len
		} // pattern count
	} // for whole string
	antSet.antennaField = antennaField;

	ASSERTSTR(rcuNr == rcuCount, "selector '" << selector << "' specifies " << rcuNr << " inputs, while " << rcuCount << " inputs are required.");

	return (true);
}

//
// RCUinputs(setName)
//
string	AntennaSets::RCUinputs    (const string&		setName, uint	stationType) const
{
	AntSetIter		iter = itsDefinitions.find(setName);
	ASSERTSTR(iter != itsDefinitions.end(), setName << " is not defined in " << itsAntennaSetFile);
	switch (stationType) {
	case IDX_CORE:   return (iter->second.core.RCUinputs);
	case IDX_REMOTE: return (iter->second.remote.RCUinputs);
	case IDX_EUROPE: return (iter->second.europe.RCUinputs);
	default: ASSERT(false);		// satisfy compiler.
	}
}

//
// RCUallocation(setName)
//
bitset<MAX_RCUS>	AntennaSets::RCUallocation(const string&		setName, uint	stationType) const
{
	bool		LBAfield = usesLBAfield(setName, stationType);
	AntSetIter	iter 	 = itsDefinitions.find(setName);
	ASSERTSTR(iter != itsDefinitions.end(), setName << " is not defined in " << itsAntennaSetFile);
	switch (stationType) {
	case IDX_CORE:   return (LBAfield ? iter->second.core.LBAallocation   : iter->second.core.HBAallocation);
	case IDX_REMOTE: return (LBAfield ? iter->second.remote.LBAallocation : iter->second.remote.HBAallocation);
	case IDX_EUROPE: return (LBAfield ? iter->second.europe.LBAallocation : iter->second.europe.HBAallocation);
	default: ASSERT(false);		// satisfy compiler.
	}
}
//
// LBAallocation(setName)
//
bitset<MAX_RCUS>	AntennaSets::LBAallocation(const string&		setName, uint	stationType) const
{
	AntSetIter		iter = itsDefinitions.find(setName);
	ASSERTSTR(iter != itsDefinitions.end(), setName << " is not defined in " << itsAntennaSetFile);
	switch (stationType) {
	case IDX_CORE:   return (iter->second.core.LBAallocation);
	case IDX_REMOTE: return (iter->second.remote.LBAallocation);
	case IDX_EUROPE: return (iter->second.europe.LBAallocation);
	default: ASSERT(false);		// satisfy compiler.
	}
}

//
// HBAallocation(setName)
//
bitset<MAX_RCUS>	AntennaSets::HBAallocation(const string&		setName, uint	stationType) const
{
	AntSetIter		iter = itsDefinitions.find(setName);
	ASSERTSTR(iter != itsDefinitions.end(), setName << " is not defined in " << itsAntennaSetFile);
	switch (stationType) {
	case IDX_CORE:   return (iter->second.core.HBAallocation);
	case IDX_REMOTE: return (iter->second.remote.HBAallocation);
	case IDX_EUROPE: return (iter->second.europe.HBAallocation);
	default: ASSERT(false);		// satisfy compiler.
	}
}

//
// positionIndex(name, [type])
//
vector<int16>	AntennaSets::positionIndex(const string&		setName, uint stationType) const
{
	AntSetIter		iter = itsDefinitions.find(setName);
	ASSERTSTR(iter != itsDefinitions.end(), setName << " is not defined in " << itsAntennaSetFile);
	switch (stationType) {
	case IDX_CORE:   return (iter->second.core.posIndex);
	case IDX_REMOTE: return (iter->second.remote.posIndex);
	case IDX_EUROPE: return (iter->second.europe.posIndex);
	default: ASSERT(false);		// satisfy compiler.
	}
}

//
// isAntennaSet(setName)
//
bool	AntennaSets::isAntennaSet (const string&		setName) const
{
	return(itsDefinitions.find(setName) != itsDefinitions.end());
}

//
// antennaSetList()
//
vector<string>	AntennaSets::antennaSetList() const
{
	vector<string>		result;

	AntSetIter		iter = itsDefinitions.begin();
	AntSetIter		end  = itsDefinitions.end();
	while (iter != end) {
		result.push_back(iter->first);
		iter++;
	}
	return (result);
}

//
// usesLBAfield(setName)
//
bool	AntennaSets::usesLBAfield(const string&		setName, uint	stationType) const
{
	AntSetIter		iter = itsDefinitions.find(setName);
	ASSERTSTR(iter != itsDefinitions.end(), setName << " is not defined in " << itsAntennaSetFile);
	switch (stationType) {
	case IDX_CORE:   return (iter->second.core.LBAallocation.count());
	case IDX_REMOTE: return (iter->second.remote.LBAallocation.count());
	case IDX_EUROPE: return (iter->second.europe.LBAallocation.count());
	default: ASSERT(false);		// satisfy compiler.
	}
}

//
// antennaField(setName)
//
const string AntennaSets::antennaField(const string&		setName, uint	stationType) const
{
	AntSetIter		iter = itsDefinitions.find(setName);
	ASSERTSTR(iter != itsDefinitions.end(), setName << " is not defined in " << itsAntennaSetFile);
	switch (stationType) {
	case IDX_CORE:   return (iter->second.core.antennaField);
	case IDX_REMOTE: return (iter->second.remote.antennaField);
	case IDX_EUROPE: return (iter->second.europe.antennaField);
	default: ASSERT(false);		// satisfy compiler.
	}
}


} // namespace LOFAR
