//  testQuery.h: Definition of the DPservice task class.
//
//  Copyright (C) 2007
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//

#ifndef _RTDB_TESTQUERY_H_
#define _RTDB_TESTQUERY_H_

#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/DPservice.h>

namespace LOFAR {
 namespace GCF {
  namespace RTDB {

class testQuery : public GCFTask
{
public:

	testQuery (const string& name);
	virtual ~testQuery();

	GCFEvent::TResult final		 	(GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult initial	 	(GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult waitForChanges(GCFEvent& e, GCFPortInterface& p);

private:
	DPservice*			itsDPservice;
	uint32				itsQueryID;
	GCFTimerPort*		itsTimerPort;
};

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR

#endif
