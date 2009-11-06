//
//  tStateResult.h: Test program to test all kind of usage of the GCF ports.
//
//  Copyright (C) 2006
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

#ifndef _TSTATE_RESULT_H_
#define _TSTATE_RESULT_H_

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

//
// Test if the framework responds in the right way on the return state
// that is returned by the task states.
//

class tStateResult : public GCFTask
{
public:
	tStateResult (string name);
	~tStateResult();

	// The test states
	GCFEvent::TResult initial 	 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult collecting (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult lastState	 (GCFEvent& e, GCFPortInterface& p);

private:
	GCFTCPPort*		itsServer;
	GCFTCPPort*		itsConn;
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
