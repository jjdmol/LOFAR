//#  ProcessControl.h: Defines the I/F of Process Control.
//#
//#  Copyright (C) 2004
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

#ifndef ACC_PROCESSCONTROL_H
#define ACC_PROCESSCONTROL_H

#include <lofar_config.h>

//# Includes
#include <ACC/DH_ProcessControl.h>

namespace LOFAR {
  namespace ACC {

//# Description of class.
// The ProcessControl class defines the interface of the Application
// processes. All functions in this class are abstract and need to be
// implemented on both the client and the server-side.
//
class ProcessControl 
{
public:
	// Destructor
	virtual ~ProcessControl() { };

	// Command to control the application processes.
	virtual bool	define 	 (const time_t		scheduleTime) const = 0;
	virtual bool	init 	 (const time_t		scheduleTime) const = 0;
	virtual bool	run 	 (const time_t		scheduleTime) const = 0;
	virtual bool	pause  	 (const time_t		scheduleTime,
							  const time_t		waitTime,
							  const	string&		condition) 	  const = 0;
	virtual bool	quit  	 (const time_t		scheduleTime) const = 0;
	virtual bool	snapshot (const time_t		scheduleTime,
							  const string&		destination)  const = 0;
	virtual bool	recover  (const time_t		scheduleTime,
							  const string&		source) 	  const = 0;

	virtual bool	reinit	 (const time_t		scheduleTime,
							  const string&		configID)	  const = 0;

	// Define a generic way to exchange info between client and server.
	virtual string	askInfo   (const string& 	keylist) const = 0;

protected:
	// Not default constructable
	ProcessControl() {}
	// Copying is also not allowed
	ProcessControl(const ProcessControl& that) { operator=(that); };
	ProcessControl& 	operator=(const ProcessControl& that) { return (*this); };
};


  } // namespace ACC
} // namespace LOFAR

#endif
