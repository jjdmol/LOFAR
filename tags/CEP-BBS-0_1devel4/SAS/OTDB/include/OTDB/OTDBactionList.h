//#  OTDBactionList.h: Collection of OTDBactions.
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
//#  $Id$

#ifndef LOFAR_OTDB_OTDBACTIONLIST_H
#define LOFAR_OTDB_OTDBACTIONLIST_H

// \file
// Collection of OTDBactions.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_vector.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBaction.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class OTDBcontrol;
class OTDBeventList;

typedef		int32		mActionStatus;
typedef		int32		mOriginator;

// Collection of OTDBactions.
// ...
class OTDBactionList : public vector<OTDBaction>
{
public:
	OTDBactionList (const OTDBcontrol&		control,
					const OTDBeventList&	eventList,
					mActionStatus			actionStatusSet,
					mOriginator				originatorSet,
					ptime					periodBegin,
					ptime					periodEnd);
	~OTDBactionList();

private:
	// Copying is not allowed
	OTDBactionList(const OTDBactionList&	that);

	OTDBactionList& operator=(const OTDBactionList& that);

	//# --- Datamembers ---
};

//# --- Inline functions ---



// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
