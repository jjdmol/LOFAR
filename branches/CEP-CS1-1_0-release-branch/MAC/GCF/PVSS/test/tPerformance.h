//
//  tPerformance.h: Definition of the tService task class.
//
//  Copyright (C) 2003
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

#ifndef _TPERFORMANCE_H_
#define _TPERFORMANCE_H_

#include <GCF/TM/GCF_Control.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/PVSSresponse.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {
/**
 * The tPerformance task receives ECHO_PING events from the Ping task and
 * returns an ECHO_ECHO event for each ECHO_PING event received.
 */
class tPerformance : public GCFTask
{
 public:

  /**
   * The constructor for the tPerformance task.
   * @param name The name of this task. By differentiating in the name, multiple
   * instances of the same task can be created and addressed.
   */
  tPerformance (const string& name);

  virtual ~tPerformance();

  GCFEvent::TResult initial  	 (GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult final    	 (GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1cleanup (GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1create	 (GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1setvalue(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1getvalue(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1delete	 (GCFEvent& e, GCFPortInterface& p);

 private:

  /**
   * The tPerformance task acts as a server for Ping tasks to use. Event from the Ping
   * task are received on the server port. And reply events to the Ping task
   * are sent through the server port.
   */
	PVSSservice* 	itsService;
	PVSSresponse*	itsResponse;
	GCFTimerPort*	itsTimerPort;
};

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR

#endif
