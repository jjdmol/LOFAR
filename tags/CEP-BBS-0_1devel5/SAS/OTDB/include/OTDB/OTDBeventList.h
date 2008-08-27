//#  OTDBeventList.h: Collection of OTDBevents.
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

#ifndef LOFAR_OTDB_OTDBEVENTLIST_H
#define LOFAR_OTDB_OTDBEVENTLIST_H

// \file
// Collection of OTDBevents.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_datetime.h>
#include <Common/lofar_vector.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBevent.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class OTDBcontrol;

typedef		int32		mEventStatus;

// Gets an collection of OTDBevents from the database.
// 
class OTDBeventList : vector<OTDBevent>
{
public:
	OTDBeventList(const OTDBcontrol&		control,
				  const mEventStatus		eventStatusSet,
				  const ptime&				periodBegin,
				  const ptime&				periodEnd);
	~OTDBeventList();

private:
	// Copying is not allowed
	OTDBeventList(const OTDBeventList&	that);

	OTDBeventList& operator=(const OTDBeventList& that);

	//# --- Datamembers ---
	//# -- none --
};

//# --- Inline functions ---



// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
