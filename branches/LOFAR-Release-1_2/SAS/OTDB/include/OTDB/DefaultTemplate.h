//#  DefaultTemplate.h: Structure containing the metadata of a tree.
//#
//#  Copyright (C) 2002-2004
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
//#  $Id: DefaultTemplate.h 7791 2006-03-14 09:16:19Z overeem $

#ifndef LOFAR_OTDB_DEFAULT_TEMPLATE_H
#define LOFAR_OTDB_DEFAULT_TEMPLATE_H

// \file
// Structure containing the metadata of a tree.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBtypes.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <pqxx/pqxx>

using namespace boost::posix_time;

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class OTDBconnection;


// A DefaultTemplate structure contains the major info of a tree in the database.
// The the last few fields will be empty for PIC trees.
class DefaultTemplate {
public:
	DefaultTemplate() : itsTreeID(0) {};
	~DefaultTemplate() {};

	treeIDType		treeID() const 		{ return (itsTreeID); }
	string			name;	

	// Show treeinfo
	ostream& print (ostream& os) const;

	// Friends may change the data references keys.
	friend	class OTDBconnection;

private:
//# Prevent changing the database keys
	DefaultTemplate(treeIDType	aTreeID) : itsTreeID(aTreeID) {};
	DefaultTemplate(const pqxx::result::tuple&	row);

	treeIDType		itsTreeID;
};

//#
//# operator<<
//#
inline ostream& operator<< (ostream&			os,
							const DefaultTemplate		aDefaultTemplate)
{
	return (aDefaultTemplate.print(os));
}


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
