//#  ParameterCollection.h: Implements a map of Key-Value pairs.
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
//#  Abstract:
//#
//#	 This class implements a container of key-value pairs. The KV pairs can
//#  be read from a file or be merged with the values in another file.
//#  The class also support the creation of subsets.
//#
//#  $Id$

#ifndef ACC_PARAMETERCOLLECTION_H
#define ACC_PARAMETERCOLLECTION_H

#include <lofar_config.h>

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_sstream.h>

namespace LOFAR {
  namespace ACC {

const char PC_QUAL_STABLE[]   = { "stable"        };
const char PC_QUAL_TEST[]     = { "test"          };
const char PC_QUAL_DEVELOP[]  =	{ "development"   };
const char PC_KEY_VERSIONNR[] = { "versionnr"     };
const char PC_KEY_QUAL[]      =	{ "qualification" };

//# Description of class.
// The ParameterCollection class is an implementation of a map <string, string>. 
// This means that Values are stored as a string which allows easy merging and
// splitting of ParameterCollections because no conversions have to be done.
// A couple of getXxx routines are provided to convert the strings to the 
// desired type.
//
class ParameterCollection : public map <string, string>
{
public:
	typedef map<string, string>::iterator			iterator;
	typedef map<string, string>::const_iterator		const_iterator;

	// Default constructable.
	ParameterCollection();
	~ParameterCollection();

	// The ParameterCollection may be construction by reading a param. file.
	explicit ParameterCollection(const string&	theFilename);

	// Copying is allowed.
	ParameterCollection(const ParameterCollection& that);
	ParameterCollection& 	operator=(const ParameterCollection& that);

	// Adds the Key-Values pair in the given file to the current ParCollection.
	void	adoptFile      (const string&               theFilename);
	void	adoptBuffer    (const string&               theBuffer);
	void	adoptCollection(const ParameterCollection&	theCollection);

	// Writes the Key-Values pair from the current ParCollection to the file.
	void	writeFile   (const string& theFilename) const;
	void	writeBuffer (      string& theBuffer) const;

	// Creates a subset from the current ParameterCollection containing all the 
	// parameters that start with the given baseKey. The baseKey is cut off 
	// from the Keynames in the created subset, the optional prefix is put
	// before all keys in the subset.
	ParameterCollection	makeSubset(const string& baseKey,
								   const string& prefix = "") const;

	// Routines for adding/replacing single pairs.
	void	add    (const string& aKey, const string& aValue);
	void	replace(const string& aKey, const string& aValue);
	void	remove (const string& aKey);

	// Checks if the given Key is defined in the ParameterCollection.
	bool	isDefined (const string& searchKey) const
				{ return (find(searchKey) != end()); };

	// Return the 'metadata' from the parameterCollection.
	string  getName          () const;
	string  getVersionNr     () const;

	// Return the keyvalues as a C++ type.
	bool	getBool  (const string& theKey) const;
	int		getInt   (const string& theKey) const;
	double	getDouble(const string& theKey) const;
	float	getFloat (const string& theKey) const;
	string	getString(const string& theKey) const;
	time_t	getTime  (const string& theKey) const;

	// Allow printing the whole parameter collection.
	friend std::ostream& operator<<(std::ostream& os, const ParameterCollection &thePS);

private:
	void	readFile   (const string& theFile, const	bool merge);
	void	readBuffer (const string& theFile, const	bool merge);
	void	addStream  (istream&	inputStream, const	bool merge);
};

//# -------------------- Global functions --------------------
// Check if the given string is a valid versionnumber (x.y.z)
bool	isValidVersionNr   (const string& versionNr);

// Check if the given string is a valid versionnumber reference. This may be
// x.y.z or the words stable or development.
bool	isValidVersionNrRef(const string& versionNr);

// Returns the value of the given string or 0 if it is not a valid seqnr
uint32	seqNr(const string& aString);

// Returns the last part of a fullkeyname being the keyname (a.k.o. dirname).
string	keyName	   (const string& fullKeyName);

// Returns everthing except the last part of a fullkeyname being the modulename
// the key belongs to. (a.k.o. basename).
string	moduleName (const string& fullKeyName);

// Returns the keypart of a parameterline
string	keyPart	   (const string& parameterLine);

// Returns the value of a parameterline
string	valuePart  (const string& parameterLine);

// Returns the value of the index if the string contains an index otherwise
// 0 is returned.
int32 	indexValue (const string&	label, char	indexMarker[2]);

} // namespace ACC
} // namespace LOFAR

#endif
