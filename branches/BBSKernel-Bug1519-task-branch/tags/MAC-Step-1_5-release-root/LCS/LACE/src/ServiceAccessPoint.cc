//#  ServiceAccessPoint.cc: Class for a point of IO.
//#
//#  Copyright (C) 2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
//#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <Common/LofarLogger.h>
#include <LACE/ServiceAccessPoint.h>

namespace LOFAR {
  namespace LACE {

//
// ServiceAccessPoint()
//
ServiceAccessPoint::ServiceAccessPoint() :
	itsHandle    (0),
	itsIsBlocking(false)
{}


//
// ~ServiceAccessPoint()
//
ServiceAccessPoint::~ServiceAccessPoint()
{}

//
// setBlocking(blocking)
//
int	 ServiceAccessPoint::setBlocking(bool	blocking)
{
	ASSERTSTR(itsHandle, "Handle must be initialized before setting (non)blocking mode");

	itsIsBlocking = blocking;
	LOG_DEBUG_STR("setBlocking(" << itsHandle << "," << ((blocking) ? "true" : "false") << ")");


	int		block = itsIsBlocking ? 0 : 1;
	return (fcntl (itsHandle, F_SETFL, itsIsBlocking ? 0 : O_NONBLOCK));
//	return (ioctl (itsHandle, FIONBIO, &block));
}


//
// setHandle(handle)
//
void ServiceAccessPoint::setHandle(SAPHandle	aHandle)
{
	// adopt handler and if not nill assure that blocking matches internal setting.
	itsHandle = aHandle;
	if (itsHandle) {
		setBlocking(itsIsBlocking);
	}
}

  } // namespace LACE
} // namespace LOFAR
