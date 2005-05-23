//#  OTDBeventList.cc: Collection of OTDBevents.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include<Common/LofarLogger.h>
#include<OTDB/OTDBeventList.h>

namespace LOFAR {
  namespace OTDB {

//
// OTDBeventList()
//
OTDBeventList::OTDBeventList(const OTDBcontrol&		control,
							 const mEventStatus		eventStatusSet,
							 const ptime&			periodBegin,
							 const ptime&			periodEnd)
{}

//
// ~OTDBeventList()
//
OTDBeventList::~OTDBeventList()
{}


//
// copy operator
//
OTDBeventList::OTDBeventList(const OTDBeventList& that)
  : vector<OTDBevent> (that)
{}

//
// operator= copying
//
OTDBeventList& OTDBeventList::operator=(const OTDBeventList& that)
{
	if (this != &that) {
		vector<OTDBevent>::operator= (that);
	}
	return (*this);
}


  } // namespace OTDB
} // namespace LOFAR
