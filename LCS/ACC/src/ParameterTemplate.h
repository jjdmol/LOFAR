//#  ParameterTemplate.h: Collection of parametersdefinitions.
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
//#	 Defines a class the should contain all parameters that the 'outside' world
//#  can set in the module the template belongs to.
//#
//#  $Id$

#ifndef ACC_PARAMETERTEMPLATE_H
#define ACC_PARAMETERTEMPLATE_H

#include <lofar_config.h>

//# Includes
#include <ACC/ParameterCollection.h>

namespace LOFAR {
  namespace ACC {


//# Description of the class.
// The ParameterTemplate class is a ParameterCollection that contains the
// definition of all parameters of a module that can be set by the 'outside'
// world. In a ParameterTemplate the value-field can be used the define a 
// default value, a value range or both.
//
// The restrictions of a ParameterTemplate are:
// 1. The firstline must be a versionnr key with a valid versionnumber.
// 2. The secondline should be the qualification key. When it is missing the
//    value 'development' is assumed.
// 3. The first part of the keyname must be the same for all keys because they
//    all belong to the same module.
//
class ParameterTemplate : public ParameterCollection
{
public:
	// Default constructable;
	ParameterTemplate();
	~ParameterTemplate();

	// Define a conversion function from base class to this class
	ParameterTemplate(const ParameterCollection& that);

	// Allow reading a file for backwards compatibility
	explicit ParameterTemplate(const string&	theFilename);

	// Copying is allowed.
	ParameterTemplate(const ParameterTemplate& that);
	ParameterTemplate& 	operator=(const ParameterTemplate& that);

	// Check if the contents is a valid ParameterTemplate.
	bool check(string&	errorReport) const;
	string	getQualification() const;

	friend std::ostream& operator<<(std::ostream& os, const ParameterCollection &thePS);
};

} // namespace ACC
} // namespace LOFAR

#endif
