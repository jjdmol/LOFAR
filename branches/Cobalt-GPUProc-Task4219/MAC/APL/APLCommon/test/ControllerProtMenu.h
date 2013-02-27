//#  ControllerProtMenu.h: Interface between MAC and SAS.
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
//#  $Id: ControllerProtMenu.h 12371 2008-12-23 13:18:31Z loose $

#ifndef ControllerProtMenu_H
#define ControllerProtMenu_H

//# GCF Includes
#include <GCF/TM/GCF_Control.h>

//# Common Includes
#include <Common/LofarLogger.h>

// forward declaration

namespace LOFAR {
	namespace Test {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;

class ControllerProtMenu : public GCFTask
{
public:
	ControllerProtMenu();
	~ControllerProtMenu();

   	GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult claim_state   (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult prepare_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult run_state     (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult suspend_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult release_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult finish_state  (GCFEvent& e, GCFPortInterface& p);

private:
	// avoid copying
	ControllerProtMenu(const ControllerProtMenu&);
   	ControllerProtMenu& operator=(const ControllerProtMenu&);

	void	_doActionMenu();

	// pointer to listener and childport
	GCFTCPPort*			itsListener;
	GCFTCPPort*			itsChildPort;
	string				itsControllerName;
};

  };//Test
};//LOFAR
#endif
