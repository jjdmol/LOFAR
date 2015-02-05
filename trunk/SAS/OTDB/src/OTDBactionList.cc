//#  OTDBactionList.cc: Collection of OTDBactions.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include<Common/LofarLogger.h>
#include<OTDB/OTDBactionList.h>

namespace LOFAR {
  namespace OTDB {

//
// OTDBactionList()
//
OTDBactionList::OTDBactionList(const OTDBcontrol&		control,
							   const OTDBeventList&		eventList,
							   mActionStatus			actionStatusSet,
							   mOriginator				originatorSet,
							   ptime					periodBegin,
							   ptime					periodEnd)
{}

//
// ~OTDBactionList()
//
OTDBactionList::~OTDBactionList()
{}


//
// copy operator
//
OTDBactionList::OTDBactionList(const OTDBactionList& that)
  : vector<OTDBaction> (that)
{}

//
// operator= copying
//
OTDBactionList& OTDBactionList::operator=(const OTDBactionList& that)
{
	if (this != &that) {
		vector<OTDBaction>::operator= (that);
	}
	return (*this);
}


  } // namespace OTDB
} // namespace LOFAR
