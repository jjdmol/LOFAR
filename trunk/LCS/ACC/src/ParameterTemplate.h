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
//#  Note: This source is best read with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACC_PARAMETERTEMPLATE_H
#define LOFAR_ACC_PARAMETERTEMPLATE_H

// \file ParameterTemplate.h
// Collection of parametersdefinitions.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <ACC/ParameterCollection.h>

namespace LOFAR {
  namespace ACC {
// \addtogroup ACC
// @{

//# Description of the class.
// The ParameterTemplate class is a ParameterCollection that contains the
// definition of all parameters of a module that can be set by the 'outside'
// world. In a ParameterTemplate the value-field can be used the define a 
// default value, a value range or both.
//
// The restrictions of a ParameterTemplate are: <br>
// 1) It must contain a \c versionnr key with a valid versionnumber.<br>
// 2) It should contain a \c qualification key. When it is missing the
//    value 'development' is assumed. <br>
// 3) The first part of the keyname must be the same for all keys because they
//    all belong to the same module. <br>
// 4) All versionnumber references must have a valid format. <br>
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

	// Check if the contents of the ParameterTemplate meets the restrictions.
	// The \c errorReport string contains the violations that were found.
	bool check(string&	errorReport) const;

	// Returns the qualification string of the ParameterTemplate.
	string	getQualification() const;

	friend std::ostream& operator<<(std::ostream& os, const ParameterCollection &thePS);
};

// @} addgroup
} // namespace ACC
} // namespace LOFAR

#endif
