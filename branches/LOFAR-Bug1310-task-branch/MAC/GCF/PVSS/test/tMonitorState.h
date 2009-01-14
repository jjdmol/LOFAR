//  tMonitorState.h: Test prog for manual testing PVSS queries.
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

#ifndef _TMONITORSTATE_H
#define _TMONITORSTATE_H

#include <GCF/TM/GCF_Control.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/PVSSresponse.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {
/**
 * The tMonitorState task receives ECHO_PING events from the Ping task and
 * returns an ECHO_ECHO event for each ECHO_PING event received.
 */
class tMonitorState : public GCFTask
{
public:
	tMonitorState (const string& name);
	virtual ~tMonitorState();

	GCFEvent::TResult initial  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult watching (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult final    (GCFEvent& e, GCFPortInterface& p);

private:
	PVSSservice* 	itsService;;
	PVSSresponse*	itsResponse;
	GCFTimerPort*	itsTimerPort;
};

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR

#endif
