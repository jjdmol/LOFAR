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
//#  $Id$

#ifndef ACC_PARAMETERSET_H
#define ACC_PARAMETERSET_H

#include <lofar_config.h>

//# Includes
#include <ACC/ParameterCollection.h>

namespace LOFAR {
  namespace ACC {


//# Description of the class.
// The ParameterSet class is a ParameterCollection that is used during runtime
// to feed an application with its runtime values.
// The restrictions of this collections are:
// 1. The firstline must be a versionnr key with a valid versionnumber.
// 2. All values must be filled in.
//
class ParameterSet : public ParameterCollection
{
public:
	// Default constructable;
	ParameterSet();
	~ParameterSet();

	// Define a conversion function from base class to this class
	ParameterSet(const ParameterCollection& that);

	// Allow reading a file for backwards compatibility
	explicit ParameterSet(const string&	theFilename);

	// Copying is allowed.
	ParameterSet(const ParameterSet& that);
	ParameterSet& 	operator=(const ParameterSet& that);

	// Check if the contents is a valid ParameterSet.
	bool check(string&	errorReport) const;

	friend std::ostream& operator<<(std::ostream& os, const ParameterCollection &thePS);

private:
};

} // namespace ACC
} // namespace LOFAR

#endif
