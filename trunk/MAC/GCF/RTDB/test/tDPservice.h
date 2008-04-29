//  tDPservice.h: Definition of the DPservice task class.
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

#ifndef _RTDB_TPROPERTYSET_H_
#define _RTDB_TPROPERTYSET_H_

#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/DPservice.h>

namespace LOFAR {
 namespace GCF {
  namespace RTDB {

class tDPservice : public GCFTask
{
public:

	tDPservice (const string& name);
	virtual ~tDPservice();

	GCFEvent::TResult final			 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult createDPS		 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult WriteTest		 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult ReadTest		 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult WriteTest2	 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult WriteErrorTest (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult ReadErrorTest	 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult QueryTest		 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult CancelQueryTest(GCFEvent& e, GCFPortInterface& p);
#if 0
	GCFEvent::TResult DeletePS		(GCFEvent& e, GCFPortInterface& p);
#endif

private:
	PVSSservice*		itsPVSSservice;
	DPservice*			itsDPservice;
	GCFTimerPort*		itsTimerPort;
};

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR

#endif
