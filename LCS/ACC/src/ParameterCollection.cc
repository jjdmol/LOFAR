//#  ParameterCollection.cc: Implements a map of Key-Value pairs.
//#
//#  Copyright (C) 2002-2003
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

#include <ACC/ParameterCollection.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>

namespace LOFAR {
  namespace ACC {

//#-------------------------- creation and destroy ---------------------------
//#
//# Default constructor
//#
ParameterCollection::ParameterCollection()
{}

//#
//# Construction by reading a parameter file.
//#
ParameterCollection::ParameterCollection(const string&	theFilename)
{
	readFile(theFilename, false);
}

//#
//# Copying is allowed.
//#
ParameterCollection::ParameterCollection(const ParameterCollection& that)
  : map<string, string> (that)
{}

//#
//# operator= copying
//#
ParameterCollection& ParameterCollection::operator=(const ParameterCollection& that)
{
	if (this != &that) {
		map<string, string>::operator= (that);
	}
	return (*this);
}

//#
//#	Destructor
//#
ParameterCollection::~ParameterCollection()
{}

//#
//# operator<<
//#
std::ostream&	operator<< (std::ostream& os, const ParameterCollection &thePS)
{
	map<string,string>::const_iterator		iter = thePS.begin();

	while (iter != thePS.end()) {
		os << "[" << iter->first << "],[" << iter->second << "]" << endl;
		iter++;
	}

	return os;
}

//#-------------------------- merge and split --------------------------------
//#
//# makeSubset(baseKey)
//#
// Creates a Subset from the current ParameterCollection containing all the parameters
// that start with the given baseKey. The baseKey is cut off from the Keynames
// in the created subset.
//
ParameterCollection ParameterCollection::makeSubset(const string& baseKey) const
{
	const_iterator			scanner    = begin();
	int						baseKeyLen = strlen(baseKey.c_str());
	ParameterCollection		subSet;

	//# Scan through whole ParameterCollection
	while (scanner != end()) {
		//# starts with basekey?
		if (!strncmp(baseKey.c_str(), scanner->first.c_str(), baseKeyLen)) {
			//# cut off and copy to subset
			subSet.insert(make_pair(scanner->first.c_str()+baseKeyLen,
									scanner->second));
		}
		scanner++;
	}

	return (subSet);
}

//#
//# adoptFile
//#
// Adds the parameters from the file to the current ParameterCollection.
//
void ParameterCollection::adoptFile(const string&	theFilename)
{
	readFile(theFilename, true);
}

//#
//# adoptBuffer
//#
// Adds the parameters from the string to the current ParameterCollection.
//
void ParameterCollection::adoptBuffer(const string&	theBuffer)
{
	readBuffer(theBuffer, true);
}

//#
//# readFile
//# (private)
//#
// Disentangles the file and adds the Key-Values pair to the current ParameterCollection.
//
void ParameterCollection::readFile(const	string&	theFilename,
								   const	bool	merge)
{
	ifstream		paramFile;

	//# Try to pen the file
	paramFile.open(theFilename.c_str(), ifstream::in);
	if (!paramFile) {
		THROW (Exception, formatString("Unable to open file %s", theFilename.c_str()));
	}

	addStream(paramFile, merge);

	paramFile.close();
}

//#
//# readBuffer
//# (private)
//#
// Disentangles the file and adds the Key-Values pair to the current ParameterCollection.
//
void ParameterCollection::readBuffer(const	string&	theBuffer,
									 const	bool	merge)
{
	istringstream		iss(theBuffer, istringstream::in);

	addStream(iss, merge);

}

//#
//# addStream
//# (private)
//#
// Disentangles the stream and adds the Key-Values pair to the current ParameterCollection.
//
void ParameterCollection::addStream(istream&	inputStream, bool	merge)
{
	char	paramLine[1024];
	char*	separator;
	//# Read the file line by line and convert it to Key Value pairs.
	while (inputStream.getline (paramLine, 1024)) {
		LOG_TRACE_VAR(formatString("data:>%s<", paramLine));
	
		if (!paramLine[0] || paramLine[0] == '#') {		//# skip empty lines
			continue;									//# and comment lines
		}
		
		//# TODO: allow multiline parametervalues
		//# line must have an equal-character ofcourse.
		separator = strstr(paramLine, "=");
		if (!separator) {
			THROW (Exception, formatString("No '=' found in %s", paramLine));
		}
		*separator= '\0';					//# terminate key string
		++separator;						//# place on 1st char of value-part

		//# check for quoted strings, cutoff quotes
		int valueLen = strlen(separator)-1;
		if ((valueLen > 0) && (*separator == '"' || *separator == '\'') &&
			(*separator == separator[valueLen])) {
			separator[valueLen] = '\0';
			++separator;
		}

		LOG_TRACE_VAR(formatString("pair:[%s][%s]", paramLine, separator));

		//# remove any existed value and insert this value
		if ((erase(paramLine) > 0) && !merge) {
			LOG_WARN (
				formatString("Key %s is defined twice. Ignoring first value.", 
																  paramLine));
		}
		pair< map<string, string>::iterator, bool>		result;
		result = insert(std::make_pair(paramLine, separator));  //# add to map
		if (!result.second) {
			THROW (Exception, formatString("Key %s double defined?", paramLine));
		}
	}
}

//#-------------------------- retrieve functions -----------------------------
//#
//# getName
//#
string	ParameterCollection::getName() const
{
	string fullKeyName = begin()->first;
	char*	firstPoint = strchr(fullKeyName.c_str(), '.');

	return(fullKeyName.substr(0, firstPoint - fullKeyName.c_str()));
}

//#
//# getVersionNr
//#
string	ParameterCollection::getVersionNr() const
{
	const_iterator	iter = find(getName()+"."+PC_KEY_VERSIONNR);

	if (iter != end()) {
		return (iter->second);
	}
	
	return("");
}

//#
//#	getInt(key)
//#
int ParameterCollection::getInt(const string& theKey) const
{
	int				theInt;
	const_iterator	iter = find (theKey);

	if (iter == end()) {
		THROW (Exception, formatString("Key %s unknown", theKey.c_str()));
	}
	if (sscanf (iter->second.c_str(), "%d", &theInt) != 1) {
		THROW (Exception, formatString("%s is not an integer value", iter->second.c_str()));
	}
	return (theInt);
}

//#
//# getDouble(key)
//#
double ParameterCollection::getDouble(const string& theKey) const

{
	double			theDouble;
	const_iterator	iter = find (theKey);

	if (iter == end()) {
		THROW (Exception, formatString("Key %s unknown", theKey.c_str()));
	}
	if (sscanf (iter->second.c_str(), "%lg", &theDouble) != 1) {
		THROW (Exception, formatString("%s is not a double value", iter->second.c_str()));
	}
	return (theDouble);
}

//#
//# getFloat(key)
//#
float ParameterCollection::getFloat(const string& theKey) const

{
  float theFloat;
  const_iterator iter = find (theKey);

  if (iter == end()) {
    THROW (Exception, formatString("Key %s unknown", theKey.c_str()));
  }
  if (sscanf (iter->second.c_str(), "%e", &theFloat) != 1) {
    THROW (Exception, formatString("%s is not a float value", iter->second.c_str()));
  }
  return (theFloat);
}

//#
//# getString(key)
//#
string ParameterCollection::getString(const string& theKey) const
{
	const_iterator	iter = find (theKey);

	if (iter == end()) {
		THROW (Exception, formatString("Key %s unknown", theKey.c_str()));
	}
	return (iter->second);
}

//#---------------------------- save functions -------------------------------
//#
//# writeFile
//#
// Writes the Key-Values pair from the current ParameterCollection to the given file
// thereby overwritting any file contents.
//
void ParameterCollection::writeFile(const string&	theFilename) const
{
	ofstream		paramFile;

	//# Try to pen the file
	paramFile.open(theFilename.c_str(), ofstream::out | ofstream::trunc);
	if (!paramFile) {
		THROW (Exception, formatString("Unable to open file %s", theFilename.c_str()));
	}

	//# Write all the pairs to the file
	const_iterator		curPair = begin();
	while (curPair != end()) {
		//# Key can always be written.
		paramFile << curPair->first << "=";

		//* value may begin or end in a space: use quotes
		if (*(curPair->second.begin()) == ' ' || *(curPair->second.rbegin()) == ' ') {
			paramFile << "\"" << curPair->second << "\"" << endl;
		}
		else {
 			paramFile << curPair->second << endl;
		}

		curPair++;
	}

	paramFile.close();
}

//#
//# writeBuffer
//#
// Writes the Key-Values pair from the current ParameterCollection to the given 
// string.
//
void ParameterCollection::writeBuffer(string&	aBuffer) const
{
	//# Write all the pairs to the file
	const_iterator		curPair = begin();
	while (curPair != end()) {
		//# Key can always be written.
		aBuffer += (curPair->first + "=");

		//* value may begin or end in a space: use quotes
		if (*(curPair->second.begin()) == ' ' || *(curPair->second.rbegin()) == ' ') {
			aBuffer += ("\"" + curPair->second + "\"\n");
		}
		else {
 			aBuffer += (curPair->second + "\n");
		}

		curPair++;
	}
}
//#-------------------------- global functions -----------------------------
//#
//# isValidVersionNr(versionNr)
//#
//# Check format of versionnumber.
//#
bool isValidVersionNr   (const string& versionNr)
{
	int		release, update, patch;
	char	toomuch[20];

	return (sscanf(versionNr.c_str(), "%d.%d.%d%10s", 
					&release, &update, &patch, toomuch) == 3);
}

//#
//# isValidVersionNrRef(versionNr)
//#
//# Check format of versionnumber, used as a reference.
//#
bool isValidVersionNrRef(const string& versionNr)
{
	return (isValidVersionNr(versionNr) || (versionNr == PC_QUAL_STABLE) || 
		    (versionNr == PC_QUAL_DEVELOP));

}

//#
//# isValidSeqNr(aString)
//#
//# Check is given string is a valid sequencenumber
//#
uint32	seqNr(const string& aString)
{
	int32	theNumber;

	sscanf(aString.c_str(), "%d", &theNumber);

	if (theNumber <= 0) {
		return (0);
	}

	return (theNumber);
}

//#
//# keyName(fullKeyName)
//#
//# Returns the real name of the key (without the module hierachy)
//#
string keyName(const string& fullKeyName)
{
	char*	lastPoint = strrchr(fullKeyName.c_str(), '.');

	if (!lastPoint) {
		return ("");
	}

	return (fullKeyName.substr(lastPoint + 1 - fullKeyName.c_str()));

}


//#
//# moduleName(fullKeyName)
//#
//# Returns the module hierarchy of key
//#
string moduleName(const string& fullKeyName)
{
	char*	lastPoint = strrchr(fullKeyName.c_str(), '.');

	if (!lastPoint) {
		return ("");
	}

	return (fullKeyName.substr(0, lastPoint - fullKeyName.c_str()));
}

//#
//# keyPart(parameterline)
//#
//# Returns the key part of a parameter line.
//#
string	keyPart	  (const string& parameterLine)
{
	char*	firstEqual = strchr(parameterLine.c_str(), '=');

	if (!firstEqual) {
		return (parameterLine);
	}

	return (parameterLine.substr(0, firstEqual - parameterLine.c_str()));
}

// Returns the value of a parameterline
string	valuePart   (const string& parameterLine)
{
	char*	firstEqual = strchr(parameterLine.c_str(), '=');

	if (!firstEqual) {
		return ("");
	}

	return (parameterLine.substr(firstEqual + 1 - parameterLine.c_str()));
}

} // namespace ACC
} // namespace LOFAR
