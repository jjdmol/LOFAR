//#  TemplateUnion.h: Implements a map of Key-Value pairs.
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


//# Description of class.
// The TemplateUnion class is a ParameterCollection that is used during runtime
// to feed an application with its runtime values.
// The restrictions of this collections are:
// 1. The firstline must be a versionnr key with a valid versionnumber.
// 2. All values must be filled in.
//
class TemplateUnion : public ParameterCollection
{
public:
	// Default constructable;
	TemplateUnion();
	~TemplateUnion();

	// Define a conversion function from base class to this class
	TemplateUnion(const ParameterCollection& that);

	// Allow reading a file for backwards compatibility
	explicit TemplateUnion(const string&	theFilename);

	// Copying is allowed.
	TemplateUnion(const TemplateUnion& that);
	TemplateUnion& 	operator=(const TemplateUnion& that);

	bool check(string&	errorReport) const;

	friend std::ostream& operator<<(std::ostream& os, const ParameterCollection &thePS);
};

} // namespace ACC
} // namespace LOFAR

#endif
