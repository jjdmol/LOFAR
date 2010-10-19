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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <APS/ParameterSetImpl.h>
#include <APS/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <Common/StreamUtil.h>


namespace LOFAR {
  namespace ACC {
    namespace APS {

      using LOFAR::operator<<;

//-------------------------- creation and destroy ---------------------------
//
// Default constructor
//
ParameterSetImpl::ParameterSetImpl(KeyCompare::Mode	mode)
	: KeyValueMap(mode),
	  itsCount (1),
	  itsMode(mode)
{}

//
// Construction by reading a parameter file.
//
ParameterSetImpl::ParameterSetImpl(const string&	theFilename,
				   KeyCompare::Mode	mode)
	: KeyValueMap(mode), 
	  itsCount (1),
	  itsMode(mode)
{
	readFile(theFilename, "", false);
}

//
//	Destructor
//
ParameterSetImpl::~ParameterSetImpl()
{}

//
// operator<<
//
std::ostream&	operator<< (std::ostream& os, const ParameterSetImpl &thePS)
{
	KeyValueMap::const_iterator		iter = thePS.begin();

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
// Creates a Subset from the current ParameterSetImpl containing all the 
// parameters that start with the given baseKey. 
// The baseKey is cut off from the Keynames in the created subset, the 
// optional prefix is put before the keynames.
//
ParameterSetImpl* 
ParameterSetImpl::makeSubset(const string& baseKey, 
			     			 const string& prefix) const
{
  // Convert \a baseKey to lowercase, if we need to do case insensitve compare.
  string            base   = (itsMode == KeyCompare::NOCASE) ? toLower(baseKey) : baseKey;
  ParameterSetImpl* subSet = new ParameterSetImpl(itsMode);

  LOG_TRACE_CALC_STR("makeSubSet(" << baseKey << "," << prefix << ")");

  // Start scanning at the point where \a base might first occur.
  for (const_iterator it = lower_bound(base); it != end(); ++it) {

    bool match = (itsMode == KeyCompare::NOCASE) ?
      toLower(it->first).compare(0, base.size(), base) == 0 :
      it->first.compare(0, base.size(), base) == 0;

    // We can stop scanning once \a match becomes false, since keys are sorted.
    if (!match) break;

    LOG_TRACE_VAR_STR(base << " matches with " << it->first);
    // cut off baseString and copy to subset
    subSet->insert(make_pair(prefix + it->first.substr(base.size()),
			     it->second));
  }
  
  return (subSet);
}

//
// substractSubset(fullPrefix)
//
// Removes all the keys the start with the given prefix from the parset.
//
void ParameterSetImpl::substractSubset(const string& fullPrefix) 
{
	LOG_TRACE_CALC_STR("substractSubSet(" << fullPrefix << ")");

	// Convert \a baseKey to lowercase, if we need to do case insensitve compare.
	string	prefix = (itsMode == KeyCompare::NOCASE) ? toLower(fullPrefix) : fullPrefix;
	int		length = prefix.size();

	// Loop over parset and delete the matching keys.
	iterator	endIter  = end();
	iterator	iter     = lower_bound(prefix);
	while (iter != endIter) {
		bool match = (itsMode == KeyCompare::NOCASE) ?
					toLower(iter->first).compare(0, length, prefix) == 0 :
					iter->first.compare(0, length, prefix) == 0;

		// We can stop scanning once a match becomes false, since keys are sorted.
		if (!match) {
			return;
		}

		// erase the matching element
		iterator	old_iter = iter;
		iter++;
		erase (old_iter);
	}
}

//
// adoptFile
//
// Adds the parameters from the file to the current ParameterSetImpl.
//
void ParameterSetImpl::adoptFile(const string&	theFilename,
				 const string&	thePrefix)
{
	readFile(theFilename, thePrefix, true);
}

//
// adoptBuffer
//
// Adds the parameters from the string to the current ParameterSetImpl.
//
void ParameterSetImpl::adoptBuffer(const string&	theBuffer,
				   const string&	thePrefix)
{
	readBuffer(theBuffer, thePrefix, true);
}

//
// adoptCollection
//
// Adds the parameters from the ParColl. to the current ParameterSetImpl.
//
void ParameterSetImpl::adoptCollection(const ParameterSetImpl& theCollection,
				       const string&	thePrefix)
{
	const_iterator		newItem = theCollection.begin();

	while (newItem != theCollection.end()) {
		replace(thePrefix+newItem->first, newItem->second);
		++newItem;
	}
}

//
// readFile
// (private)
//
// Disentangles the file and adds the Key-Values pair to the current ParameterSetImpl.
//
void ParameterSetImpl::readFile(const	string&	theFilename, 
				const	string&	prefix,
				const	bool	merge)
{
	ifstream		paramFile;

	// Try to pen the file
	paramFile.open(theFilename.c_str(), ifstream::in);
	if (!paramFile) {
		THROW (APSException, 
		       formatString("Unable to open file %s", theFilename.c_str()));
	}

	if (paramFile.eof()) {
		THROW (APSException, 
		       formatString("file %s is empty", theFilename.c_str()));
	}

	addStream(paramFile, prefix, merge);

	paramFile.close();
}

//
// readBuffer
// (private)
//
// Disentangles the file and adds the Key-Values pair to the current ParameterSetImpl.
//
void ParameterSetImpl::readBuffer(const	string&	theBuffer, 
				  const	string&	prefix,
				  const	bool	merge)
{
	istringstream		iss(theBuffer, istringstream::in);

	addStream(iss, prefix, merge);

}

//
// addStream
// (private)
//
// Disentangles the stream and adds the Key-Values pair to the current 
// ParameterSetImpl.
//
void ParameterSetImpl::addStream(istream&	inputStream, 
				 const string&	prefix,
				 bool		merge)
{
	char	paramLine[4096];
	char*	keyStr;
	char*	valueStr;
	bool	multiLine = false;			// current line is multiline
	bool	addToPrev = false;			// previous line was multiline
	string	lineColl;					// Collects multiline values
	string	keyCopy;					// Saved key in multiline
	string	prefixedKey;					// Key with added prefix (which may be an empty string)

	// Read the file line by line and convert it to Key Value pairs.
	while (inputStream.getline (paramLine, 4096)) {
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
				THROW (APSException,formatString("No '=' found in %s", paramLine));
			}
			*separator= '\0';					// terminate key string
			valueStr = separator + 1;			// ValueStr starts after = sign

			// skip leading and trailing spaces from key (allowing spaces before = sign)
			rtrim(keyStr = ltrim(paramLine));

			// add prefix
			prefixedKey = prefix + string(keyStr);
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
				keyCopy = prefixedKey;
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
				keyStr   = const_cast<char*>(prefixedKey.c_str());
			}
			LOG_TRACE_VAR(formatString("pair:[%s][%s]", prefixedKey.c_str(), valueStr));

			// remove any existed value and insert this value
			if ((erase(prefixedKey) > 0) && !merge) {
				LOG_WARN (
				formatString("Key %s is defined twice. Ignoring first value.", 
																  prefixedKey.c_str()));
			}
			// Finally add to map
			pair< KeyValueMap::iterator, bool>		result;
			result = insert(std::make_pair(prefixedKey, valueStr)); 
			if (!result.second) {
				THROW (APSException, 
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
ParameterSetImpl::const_iterator
ParameterSetImpl::findKV(const string& aKey, bool doThrow) const
{
	LOG_TRACE_CALC_STR("find(" << aKey << ")");

	const_iterator	iter = find(aKey);
	if (iter == end() && doThrow) {
		THROW (APSException, formatString("Key %s unknown", aKey.c_str()));
	}

	return (iter);
}

//
// add (key, value)
//
void ParameterSetImpl::add(const string& aKey, const string& aValue)
{
	pair< KeyValueMap::iterator, bool>		result;

	result = insert(std::make_pair(aKey, aValue)); 

	if (!result.second) {
		THROW (APSException, formatString("add:Key %s double defined?", aKey.c_str()));
	}
}

//
// add (pair)
//
void ParameterSetImpl::add(const KVpair&	aPair)
{
	add(aPair.first, aPair.second);
}

//
// replace (key, value)
//
void ParameterSetImpl::replace(const string& aKey, const string& aValue)
{
	// remove any existed value
	erase(aKey);

	add (aKey, aValue);
}

//
// replace (pair)
//
void ParameterSetImpl::replace(const KVpair&	aPair)
{
	replace(aPair.first, aPair.second);
}

//
// remove (key)
//
void ParameterSetImpl::remove(const string& aKey)
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
// or 	   <element>
// An element may be placed between single or double qoutes.
//
vector<char*> splitVector(char*	target)
{
	vector<char*>	result;

	// trim target and check array markers
	uint32			lastPos = rtrim(target = ltrim(target)) - 1;

	// if no markers are used, read the one and only argument.
	if (target[0] != '[' && target[lastPos] != ']') {
		result.push_back(target);
		return (result);
	}	

	// When only one marker is used, throw exception.
	if (target[0] != '[' || target[lastPos] != ']') {
		THROW (APSException, 
				formatString("Array %s should be limited with '[' and ']'", 
				target));
	}

	// strip off array marker and trim again.
	target[lastPos] = '\0';
	lastPos = rtrim(target = ltrim(target+1));
	// if we had an empty vector lastPos = 0
	if (lastPos > 0 ) {
	  lastPos -= 1;

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
				THROW(APSException,
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
	}

	return (result);
}

//
//-------------------------- retrieve functions -----------------------------
// getName
//
//string	ParameterSetImpl::getName() const
//{
//	string fullKeyName = begin()->first;
//	char*	firstPoint = strchr(fullKeyName.c_str(), '.');
//
//	return(fullKeyName.substr(0, firstPoint - fullKeyName.c_str()));
//}

//
// getVersionNr
//
//string	ParameterSetImpl::getVersionNr() const
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
vector<bool> ParameterSetImpl::getBoolVector(const string& theKey) const
{
	LOG_TRACE_CALC_STR("getBoolVector(" << theKey << ")");

	// get destroyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<bool>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToBool(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getBoolVector(key, value)
//
vector<bool> ParameterSetImpl::getBoolVector(const string& theKey, const vector<bool>& theValue) const
{
	LOG_TRACE_CALC_STR("getBoolVector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<int16> ParameterSetImpl::getInt16Vector(const string& theKey) const
{
	LOG_TRACE_CALC_STR("getInt16Vector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<int16>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToInt16(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getInt16Vector(key, value)
//
vector<int16> ParameterSetImpl::getInt16Vector(const string& theKey, const vector<int16>& theValue) const
{
	LOG_TRACE_CALC_STR("getInt16Vector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<uint16> ParameterSetImpl::getUint16Vector(const string& theKey) const
{
	LOG_TRACE_CALC_STR("getUint16Vector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<uint16>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToUint16(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getUint16Vector(key, value)
//
vector<uint16> ParameterSetImpl::getUint16Vector(const string& theKey, const vector<uint16>& theValue) const
{
	LOG_TRACE_CALC_STR("getUint16Vector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<int32> ParameterSetImpl::getInt32Vector(const string& theKey) const
{
	LOG_TRACE_CALC_STR("getInt32Vector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<int32>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToInt32(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getInt32Vector(key, value)
//
vector<int32> ParameterSetImpl::getInt32Vector(const string& theKey, const vector<int32>& theValue) const
{
	LOG_TRACE_CALC_STR("getInt32Vector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<uint32> ParameterSetImpl::getUint32Vector(const string& theKey) const
{
	LOG_TRACE_CALC_STR("getUint32Vector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<uint32>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToUint32(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getUint32Vector(key, value)
//
vector<uint32> ParameterSetImpl::getUint32Vector(const string& theKey, const vector<uint32>& theValue) const
{
	LOG_TRACE_CALC_STR("getUint32Vector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<int64> ParameterSetImpl::getInt64Vector(const string& theKey) const
{
	LOG_TRACE_CALC_STR("getInt64Vector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<int64>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToInt64(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getInt64Vector(key, value)
//
vector<int64> ParameterSetImpl::getInt64Vector(const string& theKey, const vector<int64>& theValue) const
{
	LOG_TRACE_CALC_STR("getInt64Vector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<uint64> ParameterSetImpl::getUint64Vector(const string& theKey) const
{
	LOG_TRACE_CALC_STR("getUint64Vector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<uint64>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToUint64(string(elemPtrs[i])));
	}

	return (result);
}

//
//	getUint64Vector(key, value)
//
vector<uint64> ParameterSetImpl::getUint64Vector(const string& theKey, const vector<uint64>& theValue) const
{
	LOG_TRACE_CALC_STR("getUint64Vector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<float> ParameterSetImpl::getFloatVector(const string& theKey) const 
{
	LOG_TRACE_CALC_STR("getFloatVector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<float>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToFloat(string(elemPtrs[i])));
	}

	return (result);
}

//
// getFloatVector(key, value)
//
vector<float> ParameterSetImpl::getFloatVector(const string& theKey, const vector<float>& theValue) const 
{
	LOG_TRACE_CALC_STR("getFloatVector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<double> ParameterSetImpl::getDoubleVector(const string& theKey) const 
{
	LOG_TRACE_CALC_STR("getDoubleVector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<double>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToDouble(string(elemPtrs[i])));
	}

	return (result);
}

//
// getDoubleVector(key, value)
//
vector<double> ParameterSetImpl::getDoubleVector(const string& theKey, const vector<double>& theValue) const 
{
	LOG_TRACE_CALC_STR("getDoubleVector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<string> ParameterSetImpl::getStringVector(const string& theKey) const 
{
	LOG_TRACE_CALC_STR("getStringVector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<string>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(string(elemPtrs[i]));
	}

	return (result);
}

//
// getStringVector(key, value)
//
vector<string> ParameterSetImpl::getStringVector(const string& theKey, const vector<string>& theValue) const 
{
	LOG_TRACE_CALC_STR("getStringVector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
vector<time_t> ParameterSetImpl::getTimeVector(const string& theKey) const 
{
	LOG_TRACE_CALC_STR("getTimeVector(" << theKey << ")");

	// get destoyable copy of value part
	string		value(findKV(theKey)->second.c_str());	

	// parse value part as an array
	vector<char*> elemPtrs = splitVector(const_cast<char*>(value.c_str()));
	vector<time_t>	result;
	for (uint32	i = 0; i < elemPtrs.size(); ++i) {
		result.push_back(StringToTime_t(string(elemPtrs[i])));
	}

	return (result);
}

//
// getTimeVector(key, value)
//
vector<time_t> ParameterSetImpl::getTimeVector(const string& theKey, const vector<time_t>& theValue) const 
{
	LOG_TRACE_CALC_STR("getTimeVector(" << theKey << "," << theValue << ")");

        // Find key; if not found, return default value \a theValue
        const_iterator it = findKV(theKey, false);
        if (it == end()) return theValue;

	// get destroyable copy of value part
	string		value(it->second.c_str());	

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
// Writes the Key-Values pair from the current ParameterSetImpl to the given file
// thereby overwritting any file contents.
//
void ParameterSetImpl::writeFile(const string&	theFilename,
								    bool			append) const
{
	ofstream		paramFile;

	// Try to open the file
	LOG_TRACE_STAT_STR("Writing parameter file `" << theFilename << "'");
	paramFile.open(theFilename.c_str(), 
				   ofstream::out | (append ? ofstream::app : ofstream::trunc));
	if (!paramFile) {
		THROW (APSException, formatString("Unable to open file %s", theFilename.c_str()));
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
// Writes the Key-Values pair from the current ParameterSetImpl to the given 
// string.
//
void ParameterSetImpl::writeBuffer(string&	aBuffer) const
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
		THROW (APSException, aString + " is not an time value");
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
	string::size_type lastPoint = fullKeyName.rfind('.');
	if (lastPoint == string::npos) 
		return fullKeyName;
	else 
		return fullKeyName.substr(lastPoint+1);
}


//
// moduleName(fullKeyName)
//
// Returns the module hierarchy of key
//
string moduleName(const string& fullKeyName)
{
	string::size_type lastPoint = fullKeyName.rfind('.');
	if (lastPoint == string::npos) 
		return "";
	else 
		return fullKeyName.substr(0, lastPoint);
}

//
// keyPart(parameterline)
//
// Returns the key part of a parameter line.
//
string	keyPart	  (const string& parameterLine)
{
	string::size_type firstEqual = parameterLine.find('=');
	if (firstEqual == string::npos)
		return parameterLine;
	else
		return parameterLine.substr(0, firstEqual);
}

// Returns the value of a parameterline
string	valuePart   (const string& parameterLine)
{
	string::size_type firstEqual = parameterLine.find('=');
	if (firstEqual == string::npos)
		return parameterLine;
	else
		return parameterLine.substr(firstEqual+1);
}

// Returns the value of the index if the string contains an index otherwise
// 0 is returned. The second string contains the opening and closing chars
// that are used to indicate the index. The index must be a literal value
// not an expression.
int32 	indexValue (const string&	label, char	indexMarker[2])
{
	string::size_type	start = label.find_last_of(indexMarker[0]);
	if (start == string::npos) {
		return (0);
	}

	string::size_type	end = label.find(indexMarker[1], start);
	if (end == string::npos) {
		return(0);
	}

	return (strtol(label.data()+start+1, 0 ,0));

}

//
// locateModule(shortKey)
//
// Searches for a key ending in the given 'shortkey' and returns it full name.
// e.g: a.b.c.d.param=xxxx --> locateKey(d)-->a.b.c.
string	ParameterSetImpl::locateModule(const string&	shortKey) const
{
	const_iterator		iter = begin();
	const_iterator		eom  = end();
	while ((iter != eom)) {
		if (keyName(moduleName(iter->first)) == shortKey) {
			string prefix = moduleName(moduleName((iter->first)));
			if (prefix.length() > 0) {
				prefix += ".";
			}
			return (prefix);
		}
		iter++;
	}

	return ("");
}

    } // namespace APS
  } // namespace ACC
} // namespace LOFAR
