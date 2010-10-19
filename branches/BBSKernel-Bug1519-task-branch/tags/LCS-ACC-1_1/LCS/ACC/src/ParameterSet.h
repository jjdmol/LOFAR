//#  ParameterSet.h: ParameterCollectin filled with runtime values.
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
//#	 Defines a class the should contain fully filled ParamaterCollections
//#  to be used in runtime to feed the applications with information.
//#
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACC_PARAMETERSET_H
#define LOFAR_ACC_PARAMETERSET_H

// \file ParameterSet.h
// ParameterCollection filled with runtime values.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <ACC/ParameterCollection.h>

namespace LOFAR {
  namespace ACC {
// \addtogroup ACC
// @{


//# Description of the class.
// The ParameterSet class is a ParameterCollection that is used during runtime
// to feed an application with its runtime values.<br>
// The restrictions of this collections are:<br>
// 1) It must contain a \c versionnr key with a valid versionnumber.
// 2) All values must be filled in.<br>
// This can be checked with the \c check method.
//
// Since ParameterSet is derived from ParameterCollection all manipulation
// methods of this base class are available. The only extra functionality
// ParameterSet offers is the \c check method the checks if the ParameterSet
// is conform the restrictions.
class ParameterSet : public ParameterCollection
{
public:
	// Default constructable;
	ParameterSet();
	~ParameterSet();

	// Define a conversion function from base class to this class
	ParameterSet(const ParameterCollection& that);

	// Allow reading a file for backwards compatibility; OBSOLETE!
	explicit ParameterSet(const string&	theFilename);

	// Copying is allowed.
	ParameterSet(const ParameterSet& that);
	ParameterSet& 	operator=(const ParameterSet& that);

	// Check if the contents of the Parameterset meets the restrictions. The
	// \c errorReport string contains the violations that were found.
	bool check(string&	errorReport) const;

	friend std::ostream& operator<<(std::ostream& os, const ParameterCollection &thePS);

private:
};

// @} addgroup
} // namespace ACC
} // namespace LOFAR

#endif
