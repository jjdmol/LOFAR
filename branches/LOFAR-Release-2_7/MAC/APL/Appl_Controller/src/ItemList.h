//#  ItemList.h: one line description
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_ITEMLIST_H
#define LOFAR_ACCBIN_ITEMLIST_H

// \file
// Class that can substract and array of items from a parameter collection.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_vector.h>
#include <Common/ParameterSet.h>


namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{


// Class that can substract and array of items from a parameter collection.
class ItemList : public vector <string>
{
public:
	typedef vector<string>::iterator		iterator;
	typedef vector<string>::const_iterator	const_iterator;

	// Constructs a vector of Values from the vector of keys with the name
	// 'prefix' from the given ParameterSet. ( prefix[n] = )
	ItemList(const ParameterSet&	aPS,
			 const string&			prefix);

	bool isInList(const string&	aItemName) const;

private:
	
};

// @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
