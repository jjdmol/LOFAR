//#  OTDBcontrol.h: Special connectiontype for events and actions.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef LOFAR_OTDB_OTDBCONTROL_H
#define LOFAR_OTDB_OTDBCONTROL_H

// \file
// Special connectiontype for events and actions.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBconnection.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class ...;


// Special connectiontype for events and actions.
// ...
class OTDBcontrol : public OTDBconnection
{
public:
	OTDBcontrol (const string&	username,
				 const string&	passwd,
				 const string&	database);
	~OTDBcontrol();

	actionID addAction (eventIDType			anEventID,
						actionType			anActionType,
			  			const string&		description);

private:
	// Copying is not allowed
	OTDBcontrol(const OTDBcontrol&	that);
	OTDBcontrol& operator=(const OTDBcontrol& that);

	//# --- Datamembers ---
	...
};

//# --- Inline functions ---

// ... example
//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const OTDBcontrol& aOTDBcontrol)
{	
	return (c.print(os));
}


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
