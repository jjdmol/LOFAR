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
#include <Common/StringUtil.h>
#include <cstdlib>

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
	thePS.writeStream(os);
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
  iterator          pos    = subSet->begin();

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
    pos = subSet->insert(pos, make_pair(prefix + it->first.substr(base.size()),
                                        it->second));
  }
  
  return (subSet);
}

//
// subtractSubset(fullPrefix)
//
// Removes all the keys the start with the given prefix from the parset.
//
void ParameterSetImpl::subtractSubset(const string& fullPrefix) 
{
	LOG_TRACE_CALC_STR("subtractSubSet(" << fullPrefix << ")");

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
    // Cannot adopt itself.
    if (&theCollection != this) {
	const_iterator		newItem = theCollection.begin();

	while (newItem != theCollection.end()) {
		replace(thePrefix+newItem->first, newItem->second);
		++newItem;
	}
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

	readStream(paramFile, prefix, merge);

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

	readStream(iss, prefix, merge);

}

//
// readStream
// (private)
//
// Disentangles the stream and adds the Key-Values pair to the current 
// ParameterSetImpl.
//
void ParameterSetImpl::readStream(istream&	inputStream, 
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
			// line must have an equal-character of course.
			char* separator = strstr(paramLine, "=");
			if (!separator) {
				THROW (APSException,formatString("No '=' found in %s", paramLine));
			}
			*separator = '\0';					// terminate key string
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
                ///		if ((valLen > 0) && (*valueStr == '"' || *valueStr == '\'') &&
                ///									(*valueStr == valueStr[valLen])) {
                ///			valueStr[valLen] = '\0';
                ///			++valueStr;
                ///		}
	
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
			result = insert(std::make_pair(prefixedKey,
                                                       ParameterValue(valueStr,
                                                                      false))); 
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
void ParameterSetImpl::add(const string& aKey, const ParameterValue& aValue)
{
  if (!insert(make_pair(aKey, aValue)).second) {
    THROW (APSException, formatString("add:Key %s double defined?", aKey.c_str()));
  }
}

//
// replace (key, value)
//
void ParameterSetImpl::replace(const string& aKey, const ParameterValue& aValue)
{
  (*this)[aKey] = aValue;
}

//
// remove (key)
//
void ParameterSetImpl::remove(const string& aKey)
{
  // remove any existed value
  erase(aKey);
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

#define PARAMETERSETIMPL_GETVECTOR(TPC,TPL) \
vector<TPL> ParameterSetImpl::get##TPC##Vector(const string& aKey, \
                                               bool expandable) const \
{ \
  ParameterValue value (findKV(aKey)->second); \
  if (expandable) value = value.expand(); \
  return value.get##TPC##Vector(); \
} \
 \
vector<TPL> ParameterSetImpl::get##TPC##Vector(const string& aKey, \
                                               const vector<TPL>& aValue, \
                                               bool expandable) const   \
{ \
  const_iterator it = findKV(aKey,false); \
  if (it == end()) return aValue; \
  ParameterValue value (it->second); \
  if (expandable) value = value.expand(); \
  return value.get##TPC##Vector(); \
}

PARAMETERSETIMPL_GETVECTOR (Bool, bool)
PARAMETERSETIMPL_GETVECTOR (Int, int)
PARAMETERSETIMPL_GETVECTOR (Uint, uint)
PARAMETERSETIMPL_GETVECTOR (Int64, int64)
PARAMETERSETIMPL_GETVECTOR (Uint64, uint64)
PARAMETERSETIMPL_GETVECTOR (Float, float)
PARAMETERSETIMPL_GETVECTOR (Double, double)
PARAMETERSETIMPL_GETVECTOR (String, string)
PARAMETERSETIMPL_GETVECTOR (Time, time_t)


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
	writeStream(paramFile);

	// Close the file
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
	ostringstream oss;
	writeStream(oss);
	aBuffer = oss.str();
}

//
// writeStream
//
// Writes the Key-Value pairs from the current ParameterSetImpl to the given
// output stream.
//
void ParameterSetImpl::writeStream(ostream&	os) const
{
	// Write all the pairs to the file
	const_iterator		curPair = begin();
	while (curPair != end()) {
		// Key can always be written.
		os << curPair->first << "=";
                os << curPair->second.get() << endl;
		curPair++;
	}
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
