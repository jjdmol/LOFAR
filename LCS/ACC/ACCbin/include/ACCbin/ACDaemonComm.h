//#  ACDaemonComm.h: Communication with ACDaemon
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

#ifndef LOFAR_ACCBIN_ACDAEMONCOMM_H
#define LOFAR_ACCBIN_ACDAEMONCOMM_H

// \file
// Small class that implements the communication to the ACDaemon (from the
// Application Controller).

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/Net/Socket.h>
#include <ALC/ACRequest.h>

using namespace LOFAR::ACC::ALC;

namespace LOFAR {
  namespace ACC {

// \addtogroup ACCbin
// @{

//# used in ping command
#define	AC_ALIVE_SIGN	' '
#define AC_LEAVING_SIGN	'~'

// Implements the communication from the Application Controller to the ACDaemon.
// The Application Controller needs to send pings to the ACDaemon at regular
// intervals and must unregister at the daemon when it stops.
class ACDaemonComm
{
public:
	ACDaemonComm(const string&	host,
				 const string&	port,
				 const string&  ID);
	~ACDaemonComm();

	bool	sendPing();
	void	unregister();

private:
	// Copying is not allowed
	ACDaemonComm(const ACDaemonComm&	that);
	ACDaemonComm& operator=(const ACDaemonComm& that);

	//# --- Datamembers ---
	Socket*		itsDaemonSocket;
	char		itsPingID [ACREQUESTNAMESIZE+1];
};

// @}	addtogroup
  } // namespace ACC
} // namespace LOFAR

#endif
