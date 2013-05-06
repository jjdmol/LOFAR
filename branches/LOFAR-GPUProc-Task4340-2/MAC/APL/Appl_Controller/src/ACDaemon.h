//#  ACDaemon.h: Daemon for launching Application Controllers
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_ACDAEMON_H
#define LOFAR_ACCBIN_ACDAEMON_H

// \file
// Daemon for launching Application Controllers

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/Exception.h>
#include <Common/Net/Socket.h>
#include <Common/Net/FdSet.h>
#include <Common/ParameterSet.h>
#include "ACRequestPool.h"


namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{


// The ACDaemon class implements a small daemon that wait for a request message
// and starts up an Application controller according to the request.
// The ACDaemon is fully controlled by the ParameterSet it receives during
// startup.
class ACDaemon
{
public:
	// Creates an ACDaemon object that start listening on the port mentioned
	// in the ParameterSet.
	explicit ACDaemon(const string&	progName);

	// Destructor.
	~ACDaemon();

	// Its normal (never ending) loop.
	void doWork() throw (Exception);

private:
	// Construct a parameterfile for the AC controller.
	void constructACFile(const ACRequest*	anACR,
						 const string&	    aFilename);

	void handlePingMessage();
	void handleACRequest();

	// Copying is not allowed
	ACDaemon(const ACDaemon&	that);

	// Copying is not allowed
	ACDaemon& operator=(const ACDaemon& that);

	//# --- Datamembers --- 
	// The listener socket to receive the requests on.
	Socket*			itsListener;

	// The ping socket to receive the still-alive packages on.
	Socket*			itsPingSocket;

	// The parameterSet that was received during start up.
	ParameterSet*	itsParamSet;

	// The list with current active Application Controllers
	ACRequestPool*	itsACPool;

	// File descriptor set of connected sockets
	FdSet			itsConnSet;

	// Name of the administration file forr surviving restarts.
	string			itsAdminFile;
};

// @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
