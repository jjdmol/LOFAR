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
//#  $Id$

#ifndef ACC_CMDSTACK_H
#define ACC_CMDSTACK_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <time.h>
#include <ACC/ACCommand.h>
#include <Common/lofar_map.h>

namespace LOFAR {
  namespace ACC {

// Description of class.
class CmdStack
{
public:
	typedef map<time_t,	ACCommand>					ACCmdStack;
	typedef map<time_t, ACCommand>::iterator		iterator;
	typedef map<time_t, ACCommand>::const_iterator	const_iterator;

	CmdStack ();
	~CmdStack();

	void		add(time_t	scheduleTime, ACCommand*	aCmd);
	ACCommand	pop();
	bool		timeExpired();

private:
	CmdStack(const CmdStack&	that);
	CmdStack& operator= (const CmdStack&	that);

	ACCmdStack		itsStack;
};

  } // namespace ACC
} // namespace LOFAR

#endif
