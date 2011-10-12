//  tPVSSinfo.h: Test prog for manual testing PVSS queries.
//
//  Copyright (C) 2008
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

#ifndef _TPVSSINFO_H
#define _TPVSSINFO_H

#include <GCF/TM/GCF_Control.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/PVSSresponse.h>

namespace LOFAR {
 namespace GCF {
  using TM::GCFTask;
  using TM::GCFTimerPort;
  namespace PVSS {
/**
 * The tPVSSinfo task receives ECHO_PING events from the Ping task and
 * returns an ECHO_ECHO event for each ECHO_PING event received.
 */
class tPVSSinfo : public GCFTask
{
public:
	tPVSSinfo (const string& name);
	virtual ~tPVSSinfo();

	GCFEvent::TResult initial  (GCFEvent& e, GCFPortInterface& p);

private:
	PVSSservice* 	itsService;;
	PVSSresponse*	itsResponse;
	GCFTimerPort*	itsTimerPort;
};

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR

#endif
