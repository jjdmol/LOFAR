//#  ACCmdImpl.h: the implementation of the AC commands
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

#ifndef ACC_ACCMDIMPL_H
#define ACC_ACCMDIMPL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <ACC/ApplControl.h>

namespace LOFAR {
  namespace ACC {

// Description of class.
class ACCmdImpl : public ApplControl
{
public:
	// Default constructable
	ACCmdImpl();

	// Destructor
	virtual ~ACCmdImpl();

	// Commands to control the application
	virtual bool	boot 	 (const time_t		scheduleTime,
							  const string&		configID) const;
	virtual bool	define 	 (const time_t		scheduleTime) const;
	virtual bool	init 	 (const time_t		scheduleTime) const;
	virtual bool	run 	 (const time_t		scheduleTime) const;
	virtual bool	pause  	 (const time_t		scheduleTime,
							  const time_t		waitTime,
							  const	string&		condition) const;
	virtual bool	quit  	 (const time_t		scheduleTime) const;
	virtual bool	shutdown (const time_t		scheduleTime) const;
	virtual bool	snapshot (const time_t		scheduleTime,
							  const string&		destination) const;
	virtual bool	recover  (const time_t		scheduleTime,
							  const string&		source) const;

	virtual bool	reinit	 (const time_t		scheduleTime,
							  const string&		configID) const;
	virtual bool	replace	 (const time_t		scheduleTime,
							  const string&		processList,
							  const string&		nodeList,
							  const string&		configID) const;

	// Define a generic way to exchange info between client and server.
	string	askInfo   (const string& 	keylist) const;

private:
	// Copying is not allowed
	ACCmdImpl(const ACCmdImpl& that) : ApplControl(*this) { operator= (that); };
	ACCmdImpl& 	operator=(const ACCmdImpl& that) { return (*this); };

};

  } // namespace ACC
} // namespace LOFAR

#endif
