//#  APCmdImpl.h: the implementation of the AP commands
//#
//#  Copyright (C) 2002-2005
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

#ifndef ACC_APCMDIMPL_H
#define ACC_APCMDIMPL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <ACC/ProcessControl.h>

namespace LOFAR {
  namespace ACC {

//# Description of class.
class APCmdImpl : public ProcessControl 
{
public:
	// Default constructable
	APCmdImpl();

	// Destructor
	virtual ~APCmdImpl();

	// Command to control the application processes.
	virtual bool	define 	 () 							  const;
	virtual bool	init 	 () 							  const;
	virtual bool	run 	 () 							  const;
	virtual bool	pause  	 (const	string&		condition) 	  const;
	virtual bool	quit  	 () 							  const;
	virtual bool	snapshot (const string&		destination)  const;
	virtual bool	recover  (const string&		source) 	  const;
	virtual bool	reinit	 (const string&		configID)	  const;

	// Define a generic way to exchange info between client and server.
	string	askInfo   (const string& 	keylist) const;

protected:
	// Copying is not allowed
	APCmdImpl(const APCmdImpl& that);
	APCmdImpl& 	operator=(const APCmdImpl& that);
};


  } // namespace ACC
} // namespace LOFAR

#endif
