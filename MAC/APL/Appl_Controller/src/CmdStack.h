//#  CmdStack.h: Time-ordered stack of DH_ApplControl structures
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_CMDSTACK_H
#define LOFAR_ACCBIN_CMDSTACK_H

// \file
// Time-ordered stack of DH_ApplControl structures used by the Application
// Controller to hold future commands for a while.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <time.h>
#include <ALC/DH_ApplControl.h>
#include <Common/lofar_map.h>

using namespace LOFAR::ACC::ALC;

namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{

// Time-ordered stack of DH_ApplControl structure pointers.
class CmdStack
{
public:
	typedef map<time_t,	DH_ApplControl*>					DHACStack;
	typedef map<time_t, DH_ApplControl*>::iterator			iterator;
	typedef map<time_t, DH_ApplControl*>::const_iterator	const_iterator;

	CmdStack ();
	~CmdStack();

	// Add the given DH_ApplControl pointer to the stack with the given time.
	void			add(time_t	scheduleTime, DH_ApplControl*	aDHAC);

	// Remove the top element from the stack and return a pointer to it.
	DH_ApplControl*	pop();

	// Returns true is the time of the top-element lays in the past.
	bool			timeExpired();

	// Removes all command from the Stack
	void			clear();

private:
	// Who wants to copy a CmdStack?
	CmdStack(const CmdStack&	that);

	// Who wants to copy a CmdStack?
	CmdStack& operator= (const CmdStack&	that);

	//# --- DataMembers ---
	// The map used for the storage.
	DHACStack		itsStack;
};

inline void CmdStack::clear()
{
	itsStack.clear();
}

// @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
