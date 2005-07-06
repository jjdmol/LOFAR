//#  ParameterSet.cc: Implements a map of Key-Value pairs.
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

#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>


namespace LOFAR {
  namespace ACC {
    namespace APS {

//-------------------------- creation and destroy ---------------------------
//
// Default constructor
//
ParameterSet::ParameterSet()
{}

//
// Construction by reading a parameter file.
//
ParameterSet::ParameterSet(const string&	theFilename)
{
	readFile(theFilename, false);
}

//
// Copying is allowed.
//
ParameterSet::ParameterSet(const ParameterSet& that)
  : map<string, string> (that)
{}

//
// operator= copying
//
ParameterSet& 
ParameterSet::operator=(const ParameterSet& that)
{
	if (this != &that) {
		map<string, string>::operator= (that);
	}
	return (*this);
}

//
//	Destructor
//
ParameterSet::~ParameterSet()
{}

//
// operator<<
//
std::ostream&	operator<< (std::ostream& os, const ParameterSet &thePS)
{
	map<string,string>::const_iterator		iter = thePS.begin();

	while (iter != thePS.end()) {
		os << "[" << iter->first << "],[" << iter->second << "]" << endl;
		iter++;
	}

	return os;
}

//-------------------------- merge and split --------------------------------
//
// makeSubset(baseKey [, prefix])
//
// Creates a Subset from the current ParameterSet containing all the 
// parameters that start with the given baseKey. 
// The baseKey is cut off from the Keynames in the created subset, the 
// optional prefix is put before the keynames.
//
ParameterSet ParameterSet::makeSubset(const string& baseKey,
												    const string& prefix) const
{
	const_iterator			scanner    = begin();
	int						baseKeyLen = strlen(baseKey.c_str());
	ParameterSet		subSet;

	LOG_TRACE_CALC_STR("makeSubSet(" << baseKey << "," << prefix << ")");

	// Scan through whole ParameterSet
	while (scanner != end()) {
		// starts with basekey?
		if (!scanner->first.compare(0, baseKeyLen, baseKey)) {
			// cut off baseString and copy to subset
			subSet.insert(make_pair(prefix+scanner->first.substr(baseKeyLen),
									scanner->second));
		}
		scanner++;
	}

	return (subSet);
}

//
// adoptFile
//
// Adds the parameters from the file to the current ParameterSet.
//
void ParameterSet::adoptFile(const string&	theFilename)
{
	readFile(theFilename, true);
}

//
// adoptBuffer
//
// Adds the parameters from the string to the current ParameterSet.
//
void ParameterSet::adoptBuffer(const string&	theBuffer)
{
	readBuffer(theBuffer, true);
}

//
// adoptCollection
//
// Adds the parameters from the ParColl. to the current ParameterSet.
//
void ParameterSet::adoptCollection(const ParameterSet& theCollection)
{
	const_iterator		newItem = theCollection.begin();

	while (newItem != theCollection.end()) {
		replace(newItem->first, newItem->second);
		++newItem;
	}
}

//
// readFile
// (private)
//
// Disentangles the file and adds the Key-Values pair to the current ParameterSet.
//
void ParameterSet::readFile(const	string&	theFilename,
								   const	bool	merge)
{
	ifstream		paramFile;

	// Try to pen the file
	paramFile.open(theFilename.c_str(), ifstream::in);
	if (!paramFile) {
		THROW (Exception, 
			   formatString("Unable to open file %s", theFilename.c_str()));
	}

	addStream(paramFile, merge);

	paramFile.close();
}

//
// readBuffer
// (private)
//
// Disentangles the file and adds the Key-Values pair to the current ParameterSet.
//
void ParameterSet::readBuffer(const	string&	theBuffer,
									 const	bool	merge)
{
	istringstream		iss(theBuffer, istringstream::in);

	addStream(iss, merge);

}

//
// addStream
// (private)
//
// Disentangles the stream and adds the Key-Values pair to the current 
// ParameterSet.
//
void ParameterSet::addStream(istream&	inputStream, bool	merge)
{
	char	paramLine[1024];
	char*	keyStr;
	char*	valueStr;
	bool	multiLine = false;			// current line is multiline
	bool	addToPrev = false;			// previous line was multiline
	string	lineColl;					// Collects multiline values
	string	keyCopy;					// Saved key in multiline

	// Read the file line by line and convert it to Key Value pairs.
	while (inputStream.getline (paramLine, 1024)) {
		LOG_TRACE_LOOP(formatString("data:>%s<", paramLine));
	
		if (!paramLine[0] || paramLine[0] == '#') {		// skip empty lines
			continue;									// and comment lines
			// Note: this way empty and commentline do not interfere
			// with multiline definitions.
		}
		
		if (addToPrev) {
			valueStr = paramLine;		// whole line is value
		}
		else {
			// line must have an equal-character ofcourse.
			char* separator = strstr(paramLine, "=");
			if (!separator) {
				THROW (Exception,formatString("No '=' found in %s", paramLine));
			}
			*separator= '\0';					// terminate key string
			valueStr = separator + 1;			// ValueStr starts after = sign

			// skip trailing spaces from key (allowing spaces before = sign)
			keyStr = paramLine;
			rtrim(keyStr);
		}

		// skip leading spaces from value (allowing space afer = sign)
		// but don't strip multiline lines
		if (!addToPrev) {
			valueStr = ltrim(valueStr);
		}
		// skip trailing spaces from value
		int32 valLen = rtrim(valueStr) - 1;

		// cut of any comment from value part
		char* hashPos = strchr(valueStr, '#');		// any # in valueStr?
		if (hashPos) { 
			// if valueStr is quoted don't bother about the #
			bool quoted = ((valLen > 0) &&
						   (*valueStr == '"' || *valueStr == '\'') &&
						   (*valueStr == valueStr[valLen]));
			if (!quoted) {
				*hashPos = '\0';		// cut off at hash
				// and skip trailing spaces again
				// Note: valLen already points to '\0' char
				valLen = rtrim(valueStr, valLen) - 1;
			} // quoted
		} // has hash

		// If last char is a \ we enter multiline mode.
		if (valueStr[valLen] == '\\') {
			multiLine = true;
			// cut of backslash and trailing spaces
			valueStr[valLen] = '\0';
			valLen = rtrim(valueStr, valLen) - 1;
			if (!addToPrev) {			// first multiline?
				keyCopy = keyStr;		// save copy of key
			}
		}
		else {
			multiLine = false;
		}

		// finally check (again) for quoted strings, cutoff quotes
		valLen = strlen(valueStr)-1;
		if ((valLen > 0) && (*valueStr == '"' || *valueStr == '\'') &&
									(*valueStr == valueStr[valLen])) {
			valueStr[valLen] = '\0';
			++valueStr;
		}
	
		// if this or previous line is multiline, collect result
		if (multiLine || addToPrev) {	
			lineColl.append(valueStr);
			LOG_TRACE_LOOP_STR("Multiline collecting:" << lineColl);
		}

		// Store result if this line is not a multiline
		if (!multiLine) {
			if (addToPrev) {	// result in tmp strings?
				valueStr = const_cast<char*>(lineColl.c_str());
				keyStr   = const_cast<char*>(keyCopy.c_str());
			}
			LOG_TRACE_VAR(formatString("pair:[%s][%s]", keyStr, valueStr));

			// remove any existed value and insert this value
			if ((erase(keyStr) > 0) && !merge) {
				LOG_WARN (
				formatString("Key %s is defined twice. Ignoring first value.", 
																  keyStr));
			}
			// Finally add to map
			pair< map<string, string>::iterator, bool>		result;
			result = insert(std::make_pair(keyStr, valueStr)); 
			if (!result.second) {
				THROW (Exception, 
					   formatString("Key %s double defined?", keyStr));
			}

			// Clear tmp strings
			lineColl.erase();
			keyCopy.erase();
		}

		addToPrev = multiLine;
	}
}

//------------------------- single pair functions ----------------------------
//
// findKV(key) [private]
//
ParameterSet::const_iterator
ParameterSet::findKV(const string& aKey) const
{
	const_iterator	iter = find(aKey);
	if (iter == end()) {
		THROW (Exception, formatString("Key %s unknown", aKey.c_str()));
	}

	return (iter);
}

//
// add (key, value)
//
void ParameterSet::add(const string& aKey, const string& aValue)
{
	pair< map<string, string>::iterator, bool>		result;

	result = insert(std::make_pair(aKey, aValue)); 

	if (!result.second) {
		THROW (Exception, formatString("add:Key %s double defined?", aKey.c_str()));
	}
}

//
// add (pair)
//
void ParameterSet::add(const KVpair&	aPair)
{
	add(aPair.first, aPair.second);
}

//
// replace (key, value)
//
void ParameterSet::replace(const string& aKey, const string& aValue)
{
	// remove any existed value
	erase(aKey);

	add (aKey, aValue);
}

//
// replace (pair)
//
void ParameterSet::replace(const KVpair&	aPair)
{
	replace(aPair.first, aPair.second);
}

//
// remove (key)
//
void ParameterSet::remove(const string& aKey)
{
	// remove any existed value
	erase(aKey);
}

//-------------------------- parse functions -----------------------------
//
// splitVector [private]
//
// Splits the given C string into a array of char* in the original string!
// Original string is destroyed because null-characters are placed in it.
//
// Syntax: [ <element> , <element> , <element> ]
// An element may be placed between single or double qoutes.
//
vector<char*> splitVector(char*	target)
{
	vector<char*>	result;

	// trim target and check array markers
	uint32			lastPos = rtrim(target = ltrim(target)) - 1;
	if (target[0] != '[' || target[lastPos] != ']') {
		THROW (Exception, 
				formatString("Array %s should be limited with '[' and ']'", 
				target));
	}

	// strip off array marker and trim again.
	target[lastPos] = '\0';
	lastPos = rtrim(target = ltrim(target+1)) - 1;

	// the comma seperated elements are left now.
	// scan string and set pointers.
	uint32	idx = 0;
	while (idx <= lastPos) {
		// skip leading space.
		while(idx <= lastPos && (target[idx]==' ' || target[idx]=='\t')) {
			++idx;
		}

		uint32		start = idx;		// remember first char.
		if (target[idx] == '\'' || target[idx] == '"') {
			++idx;
			// find matching qoute
			while (idx <= lastPos && target[idx] != target[start]) {
				++idx;
			}
			if (target[idx] != target[start]) {
				THROW(Exception,
					formatString("%s: unmatched quote", target+start));
			}
			// start and idx now at qoutes of same type, remove both
			target[start++] = '\0';
			target[idx++]   = '\0';
		}

		// find comma.
		while (idx <= lastPos && target[idx] != ',') {
			++idx;
		}

		if (idx != start) {
			result.push_back(target+start);
			target[idx++] = '\0'; 	// hop over , while removing it
		}
	}

	return (result);
}

//
//-------------------------- retrieve functions -----------------------------
// getName
//
//string	ParameterSet::getName() const
//{
//	string fullKeyName = begin()->first;
//	char*	firstPoint = strchr(fullKeyName.c_str(), '.');
//
//	return(fullKeyName.substr(0, firstPoint - fullKeyName.c_str()));
//}

//
// getVersionNr
//
//string	ParameterSet::getVersionNr() const
//{
//	const_iterator	iter = find(getName()+"."+PC_KEY_VERSIONNR);
//
//	if (iter != end()) {
//		return (iter->second);
//	}
//	
//	return("");
//}

//
//	getBoolVector(key)
//
vector<bool> ParameterSet::getBoolVector(const string& theKey) const
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<bool>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToBool(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getInt16Vector(key)
//
vector<int16> ParameterSet::getInt16Vector(const string& theKey) const
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<int16>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToInt16(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getUint16Vector(key)
//
vector<uint16> ParameterSet::getUint16Vector(const string& theKey) const
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<uint16>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToUint16(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getInt32Vector(key)
//
vector<int32> ParameterSet::getInt32Vector(const string& theKey) const
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<int32>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToInt32(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getUint32Vector(key)
//
vector<uint32> ParameterSet::getUint32Vector(const string& theKey) const
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<uint32>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToUint32(string(elemPtrs[i])));
	}

	return (result);
}

#if HAVE_LONG_LONG
//
//	getInt64Vector(key)
//
vector<int64> ParameterSet::getInt64Vector(const string& theKey) const
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<int64>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToInt64(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getUint64Vector(key)
//
vector<uint64> ParameterSet::getUint64Vector(const string& theKey) const
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<uint64>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToUint64(string(elemPtrs[i])));
	}

	return (result);
}
#endif

//
// getFloatVector(key)
//
vector<float> ParameterSet::getFloatVector(const string& theKey) const 
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<float>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToFloat(string(elemPtrs[i])));
	}

	return (result);
}

//
// getDoubleVector(key)
//
vector<double> ParameterSet::getDoubleVector(const string& theKey) const 
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<double>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToDouble(string(elemPtrs[i])));
	}

	return (result);
}

//
// getStringVector(key)
//
vector<string> ParameterSet::getStringVector(const string& theKey) const 
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<string>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(string(elemPtrs[i]));
	}

	return (result);
}

//
// getTimeVector(key)
//
vector<time_t> ParameterSet::getTimeVector(const string& theKey) const 
{
	// get destoyable copy of value part
	string		value(findKV(theKey)->second);	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<time_t>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToTime_t(string(elemPtrs[i])));
	}

	return (result);
}

//---------------------------- save functions -------------------------------
//
// writeFile
//
// Writes the Key-Values pair from the current ParameterSet to the given file
// thereby overwritting any file contents.
//
void ParameterSet::writeFile(const string&	theFilename,
								    bool			append) const
{
	ofstream		paramFile;

	// Try to open the file
	paramFile.open(theFilename.c_str(), 
				   ofstream::out | (append ? ofstream::app : ofstream::trunc));
	if (!paramFile) {
		THROW (Exception, formatString("Unable to open file %s", theFilename.c_str()));
	}

	// Write all the pairs to the file
	const_iterator		curPair = begin();
	while (curPair != end()) {
		// Key can always be written.
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

//
// writeBuffer
//
// Writes the Key-Values pair from the current ParameterSet to the given 
// string.
//
void ParameterSet::writeBuffer(string&	aBuffer) const
{
	// Write all the pairs to the file
	const_iterator		curPair = begin();
	while (curPair != end()) {
		// Key can always be written.
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
//-------------------------- global functions -----------------------------
//
// StringToTime_t (aString) 
//
time_t StringToTime_t (const string& aString) 
{
	time_t				theTime;
	char				unit[1024];
	unit[0] = '\0';
	if (sscanf (aString.c_str(), "%ld%s", &theTime, unit) < 1) {
		THROW (Exception, aString + " is not an time value");
	}

	switch (unit[0]) {
	case 's':
	case 'S':
	case '\0':
		break;
	case 'm':
	case 'M':
		theTime *= 60;
		break;
	case 'h':
	case 'H':
		theTime *= 3600;
		break;
	}
	return (theTime);
}

//
// isValidVersionNr(versionNr)
//
// Check format of versionnumber.
//
bool isValidVersionNr   (const string& versionNr)
{
	int		release, update, patch;
	char	toomuch[20];

	return (sscanf(versionNr.c_str(), "%d.%d.%d%10s", 
					&release, &update, &patch, toomuch) == 3);
}

//
// isValidVersionNrRef(versionNr)
//
// Check format of versionnumber, used as a reference.
//
bool isValidVersionNrRef(const string& versionNr)
{
	return (isValidVersionNr(versionNr) || (versionNr == PC_QUAL_STABLE) || 
		    (versionNr == PC_QUAL_TEST) || (versionNr == PC_QUAL_DEVELOP));

}

#if 0
//
// seqNr(aString)
//
// Check is given string is a valid sequencenumber
//
uint32	seqNr(const string& aString)
{
	int32	theNumber;

	sscanf(aString.c_str(), "%d", &theNumber);

	if (theNumber <= 0) {
		return (0);
	}

	return (theNumber);
}
#endif

//
// keyName(fullKeyName)
//
// Returns the real name of the key (without the module hierachy)
//
string keyName(const string& fullKeyName)
{
	char*	lastPoint = strrchr(fullKeyName.c_str(), '.');

	if (!lastPoint) {
		return (fullKeyName);
	}

	return (fullKeyName.substr(lastPoint + 1 - fullKeyName.c_str()));

}


//
// moduleName(fullKeyName)
//
// Returns the module hierarchy of key
//
string moduleName(const string& fullKeyName)
{
	char*	lastPoint = strrchr(fullKeyName.c_str(), '.');

	if (!lastPoint) {
		return ("");
	}

	return (fullKeyName.substr(0, lastPoint - fullKeyName.c_str()));
}

//
// keyPart(parameterline)
//
// Returns the key part of a parameter line.
//
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

// Returns the value of the index if the string contains an index otherwise
// 0 is returned. The second string contains the opening and closing chars
// that are used to indicate the index. The index must be a literal value
// not an expression.
int32 	indexValue (const string&	label, char	indexMarker[2])
{
	uint32	start = label.find_last_of(indexMarker[0]);
	if (start == string::npos) {
		return (0);
	}

	uint32	end = label.find(indexMarker[1], start);
	if (end == string::npos) {
		return(0);
	}

	return (strtol(label.data()+start+1, 0 ,0));

}

    } // namespace APS
  } // namespace ACC
} // namespace LOFAR
