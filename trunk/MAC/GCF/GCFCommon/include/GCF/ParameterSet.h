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
//#	 This class implements a container of key-value pairs. The KV pairs can
//#  be read from a file or be merged with the values in another file.
//#  The class also support the creation of subsets.
//#
//#  $Id$

#ifndef PARAMETERSET_H
#define PARAMETERSET_H

#include <lofar_config.h>

//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
//#include <Common/lofar_strstream.h> // provides the wrong istringstream version
#include <sstream>
using std::istringstream;
using std::ostringstream;

using namespace LOFAR;

namespace GCF
{

//# Description of class.
// The ParameterSet class is an implementation of a map <string, string>. This
// means that the Value is stored as a string which allows easy merging and
// splitting of ParameterSets because no conversions have to be done.
// A couple of getXxx routines convert the strings to the desired type.
//
class ParameterSet : public map <string, string>
{
public:
	typedef map<string, string>::iterator			iterator;
	typedef map<string, string>::const_iterator		const_iterator;

  static ParameterSet* instance();
  
	// Default constructable;
	ParameterSet();
	~ParameterSet();

	// The ParameterSet may be construction by reading a parameter file.
	explicit ParameterSet(const string&	theFilename);
	
	// Copying is allowed.
	ParameterSet(const ParameterSet& that);
	ParameterSet& 	operator=(const ParameterSet& that);

	// Adds the Key-Values pair in the given file to the current ParameterSet.
	void			adoptFile  (const string& theFilename);
	void			adoptBuffer(const string& theBuffer);

	// Writes the Key-Values pair from the current ParameterSet to the file.
	void			writeFile   (const string& theFilename) const;
	void			writeBuffer (const string& theBuffer) const;

	// Creates a subset from the current ParameterSet containing all the 
	// parameters that start with the given baseKey. The baseKey is cut off 
	// from the Keynames in the created subset.
	ParameterSet	makeSubset(const string& baseKey) const;

	// Checks if the given Key is defined in the ParameterSet.
	bool	isDefined (const string& searchKey) const
				{ return (find(searchKey) != end()); };

	int		getInt   (const string& theKey) const;
	double	getDouble(const string& theKey) const;
	float	getFloat(const string& theKey) const;
	string	getString(const string& theKey) const;

	friend std::ostream& operator<<(std::ostream& os, const ParameterSet &thePS);

private:
	void	readFile   (const string& theFile, const	bool merge);
	void	readBuffer (const string& theFile, const	bool merge);
	void	addStream  (istream&	inputStream, const	bool merge);
 
  static ParameterSet* _pInstance;
  ALLOC_TRACER_CONTEXT;
};

} // namespace GCF
using namespace GCF;
#endif
