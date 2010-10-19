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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACC_TEMPLATEUNION_H
#define LOFAR_ACC_TEMPLATEUNION_H

// \file TemplateUnion.h
// Implements a map of Key-Value pairs.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <ACC/ParameterCollection.h>

namespace LOFAR {
  namespace ACC {
// \addtogroup ACC
// @{


//# Description of class.
// The TemplateUnion class is a ParameterCollection that is used by SAS to fill
// in the runtime values for an application.
// The restrictions of this collections are:<br>
// 1. It must contain a \c versionnr key with a valid versionnumber.<br>
// 2. All values must be filled in.<br>
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

	// Check if the contents of the TemplateUnion meets the restrictions.
	// The \c errorReport string contains the violations that were found.
	bool check(string&	errorReport) const;

	friend std::ostream& operator<<(std::ostream& os, const ParameterCollection &thePS);
};

// @} addgroup
} // namespace ACC
} // namespace LOFAR

#endif
