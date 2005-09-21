//#  ParameterSet.h: Implements a map of Key-Value pairs.
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_APS_PARAMETERSET_H
#define LOFAR_APS_PARAMETERSET_H

// \file ParameterSet.h
// Implements a map of Key-Value pairs.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_sstream.h>
#include <Common/StringUtil.h>
#include <APS/KVpair.h>

namespace LOFAR {
  namespace ACC {
    namespace APS {
// \addtogroup APS
// @{

const char PC_QUAL_STABLE[]   = { "stable"        };
const char PC_QUAL_TEST[]     = { "test"          };
const char PC_QUAL_DEVELOP[]  =	{ "development"   };
const char PC_KEY_VERSIONNR[] = { "versionnr"     };
const char PC_KEY_QUAL[]      =	{ "qualification" };

//# Description of class.
// The ParameterSet class is a key-value implementation of the type
// map<string, string>. 
// This means that values are stored as a string which allows easy merging and
// splitting of ParameterSets because no conversions have to be done.
// A couple of getXxx routines are provided to convert the strings to the 
// desired type.
//
class ParameterSet : public map <string, string>
{
public:
	typedef map<string, string>::iterator			iterator;
	typedef map<string, string>::const_iterator		const_iterator;

	// \name Construction and Destruction
	// A ParameterSet can be constructed as empty collection, can be
	// read from a file or copied from another collection. 
	// @{

	// Create an empty collection.
	ParameterSet();

	// Destroy the contents.
	~ParameterSet();

	// The ParameterSet may be construction by reading a param. file.
	explicit ParameterSet(const string&	theFilename);

	// Copying is allowed.
	ParameterSet(const ParameterSet& that);

	// Copying is allowed.
	ParameterSet& 	operator=(const ParameterSet& that);
	//@}


	// \name Merging or appending collections
	// An existing collection can be extended/merged with another collection.
	// @{

	// Adds the Key-Values pair in the given file to the current collection.
	void	adoptFile      (const string&               theFilename);

	// Adds the Key-Values pair in the given buffer to the current collection.
	void	adoptBuffer    (const string&               theBuffer);

	// Adds the Key-Values pair in the given collection to the current 
	// collection.
	void	adoptCollection(const ParameterSet&	theCollection);
	// @}


	// \name Saving the collection
	// The map of key-value pair can be saved in a file or a string.
	// @{

	// Writes the Key-Values pair from the current ParCollection to the file.
	void	writeFile   (const string& theFilename, bool append = false) const;

	// Writes the Key-Values pair from the current ParCollection to the 
	// string buffer.
	void	writeBuffer (      string& theBuffer) const;
	//@}

	// \name Make subsets
	// A subset from the current collection can be made based on the prefix
	// of the keys in the collection.
	// @{

	// Creates a subset from the current ParameterSet containing all the 
	// parameters that start with the given baseKey. The baseKey is cut off 
	// from the Keynames in the created subset, the optional prefix is put
	// before all keys in the subset.
	ParameterSet	makeSubset(const string& baseKey,
								   const string& prefix = "") const;
	// @}

	
	// \name Handling single key-value pairs
	// Single key-value pairs can ofcourse be added, replaced or removed from 
	// a collection.
	// @{

	// Add the given pair to the collection. When the \c aKey does not exist 
	// in the collection an exception is thrown.
	void	add    (const string& aKey, const string& aValue);
	void	add    (const KVpair& aPair);

	// Replaces the given pair in the collection. If \c aKey does not exist in
	// the collection the pair is just added to the collection.
	void	replace(const string& aKey, const string& aValue);
	void	replace(const KVpair& aPair);

	// Removes the pair with the given key. Removing a non-existing key is ok.
	void	remove (const string& aKey);
	// @}

	// \name Searching and retrieving
	// The following functions support searching the collection for existance
	// of given keys an the retrieval of the corresponding value. In the getXxx
	// retrieve functions the stored string-value is converted to the wanted
	// type.
	// @{

	// Checks if the given Key is defined in the ParameterSet.
	bool	isDefined (const string& searchKey) const
				{ return (find(searchKey) != end()); };

	// Return the 'metadata' from the parameterCollection.
//	string  getName          () const;

	// Return the 'metadata' from the parameterCollection.
//	string  getVersionNr     () const;

	// Returns the value as a boolean.
	bool	getBool  (const string& aKey) const;
	int16	getInt16 (const string& aKey) const;
	uint16	getUint16(const string& aKey) const;
	int32	getInt32 (const string& aKey) const;
	uint32	getUint32(const string& aKey) const;
#if HAVE_LONG_LONG
	int64	getInt64 (const string& aKey) const;
	uint64	getUint64(const string& aKey) const;
#endif
	float	getFloat (const string& aKey) const;
	double	getDouble(const string& aKey) const;
	string	getString(const string& aKey) const;
	// Returns the value as a time value (seconds since 1970).
	time_t	getTime  (const string& aKey) const;

	// Returns the value as an integer
	vector<bool>	getBoolVector  (const string& aKey) const;
	vector<int16>	getInt16Vector (const string& aKey) const;
	vector<uint16>	getUint16Vector(const string& aKey) const;
	vector<int32>	getInt32Vector (const string& aKey) const;
	vector<uint32>	getUint32Vector(const string& aKey) const;
#if HAVE_LONG_LONG
	vector<int64>	getInt64Vector (const string& aKey) const;
	vector<uint64>	getUint64Vector(const string& aKey) const;
#endif
	vector<float>	getFloatVector (const string& aKey) const;
	vector<double>	getDoubleVector(const string& aKey) const;
	vector<string>	getStringVector(const string& aKey) const;
	vector<time_t>	getTimeVector  (const string& aKey) const;

	// @}

	// \name Printing
	// Mostly for debug purposes the collection can be printed.
	// @{

	// Allow printing the whole parameter collection.
	friend std::ostream& operator<<(std::ostream& os, const ParameterSet &thePS);
	// @}

private:
	// \name Implementation of the 'adopt' methods
	// The 'adopt' methods are implemented in the addStream method. The 'read'
	// methods do some preprocessing so the 'adopt' method can use the
	// \c addStream method.
	// @{
	void	readFile   (const string& theFile, const	bool merge);
	void	readBuffer (const string& theFile, const	bool merge);
	void	addStream  (istream&	inputStream, const	bool merge);
	// @}

	const_iterator	findKV(const string& aKey) const;
};

//# -------------------- Global functions --------------------
// Checks if the given string is a valid versionnumber (x.y.z)
bool	isValidVersionNr   (const string& versionNr);

// Checks if the given string is a valid versionnumber reference. This may be
// of the form \c x.y.z or the words \c stable, \c test or \c development
// (defined as \c PC_QUAL_STABLE, \c PC_QUAL_TEST and \c PC_QUAL_DEVELOP).
bool	isValidVersionNrRef(const string& versionNr);

// Returns the value of the given string or 0 if it is not a valid seqnr
//uint32	seqNr(const string& aString);

// When a hierarchical keyname is passed to \c fullKeyName the methods returns
// the last part of the keyname. For example:
// \code
// moduleName("base.sub.leaf")
// \endcode
// returns \c "leaf". When a keyname without dots is passed the whole key
// is returned.<br>
// \c moduleName is a kind of \c dirname function for keys.
string	keyName	   (const string& fullKeyName);

// When a hierarchical keyname is passed to \c moduleName the methods returns
// all but the last part of the keyname. For example:
// \code
// moduleName("base.sub.leaf")
// \endcode
// returns \c "base.sub". When a keyname without dots is passed and empty string
// is returned.<br>
// \c moduleName is a kind of \c basename function for keys.
string	moduleName (const string& fullKeyName);

// Returns the raw keypart of a parameterline that contains a key-value pair.
// The returned string is \e not trimmed for whitespace.
string	keyPart	   (const string& parameterLine);

// Returns the raw value-part of a parameterline that contains a key-value pair.
// This means that the string is \e not trimmed for whitespace and that comments
// at the end of the line are also returned.<br>
// It simply returns everything behind the first \c = sign.
string	valuePart  (const string& parameterLine);

// Returns the value of the index if the string contains an index otherwise
// 0 is returned. The \c indexMarker argument must be used to pass the two chars
// that are used to delimeter the index. The index must be a literal value
// not an expression. For example:
// \code
//  indexValue("label{25}andmore", "{}");
// \endcode
// returns the value 25. When more indexdelimiters are found in the string the
// last pair is used.
int32 	indexValue (const string&	label, char	indexMarker[2]);

vector<char*>	splitVector(char*	target);
time_t StringToTime_t (const string& aString);
// @} addgroup

//#	getBool(key)
inline bool ParameterSet::getBool(const string& aKey) const
{
	return (StringToBool(findKV(aKey)->second));
}

//#	getInt16(key)
inline int16 ParameterSet::getInt16(const string& aKey) const
{
	return (StringToInt16(findKV(aKey)->second));
}

//#	getUint16(key)
inline uint16 ParameterSet::getUint16(const string& aKey) const
{
	return (StringToUint16(findKV(aKey)->second));
}

//#	getInt32(key)
inline int32 ParameterSet::getInt32(const string& aKey) const
{
	return (StringToInt32(findKV(aKey)->second));
}

//#	getUint32(key)
inline uint32 ParameterSet::getUint32(const string& aKey) const
{
	return (StringToUint32(findKV(aKey)->second));
}

#if HAVE_LONG_LONG
//#	getInt64(key)
inline int64 ParameterSet::getInt64(const string& aKey) const
{
	return (StringToInt64(findKV(aKey)->second));
}

//#	getUint64(key)
inline uint64 ParameterSet::getUint64(const string& aKey) const
{
	return (StringToUint64(findKV(aKey)->second));
}
#endif

//#	getFloat(key)
inline float ParameterSet::getFloat (const string& aKey) const
{
	return (StringToFloat(findKV(aKey)->second));
}

//#	getDouble(key)
inline double ParameterSet::getDouble(const string& aKey) const
{
	return (StringToDouble(findKV(aKey)->second));
}
//#	getString(key)
inline string ParameterSet::getString(const string& aKey) const
{
	return (findKV(aKey)->second);
}

//#	getTime(key)
inline time_t ParameterSet::getTime(const string& aKey) const
{
	return (StringToTime_t(findKV(aKey)->second));
}

    } // namespace APS
  } // namespace ACC
} // namespace LOFAR

#endif
