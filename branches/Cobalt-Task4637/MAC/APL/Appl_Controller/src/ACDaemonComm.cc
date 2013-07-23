//#  ACDaemonComm.cc: Communication from AC to ACD.
//#
//#  Copyright (C) 2005
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include"ACDaemonComm.h"

namespace LOFAR {
  namespace ACC {

ACDaemonComm::ACDaemonComm(const string&	host,
				 		   const string&	port,
				 		   const string&  	ID) :
	itsDaemonSocket(new Socket("DaemonPing", host, port, Socket::UDP))
{
	ASSERTSTR(itsDaemonSocket->ok(), "Can't open ping socket with ACdaemon");

	itsPingID[0] = AC_ALIVE_SIGN;
	strncpy (&itsPingID[1], ID.c_str(), ACREQUESTNAMESIZE-1);
	itsPingID [ACREQUESTNAMESIZE] = '\0';
}

ACDaemonComm::~ACDaemonComm()
{
	if (itsDaemonSocket) {
		delete itsDaemonSocket;
	}
}

bool	ACDaemonComm::sendPing()
{
	return (itsDaemonSocket->write(itsPingID, ACREQUESTNAMESIZE+1) == 
														ACREQUESTNAMESIZE+1);
}

void	ACDaemonComm::unregister()
{
	itsPingID[0] = AC_LEAVING_SIGN;
	sendPing();
}



  } // namespace ACC
} // namespace LOFAR
