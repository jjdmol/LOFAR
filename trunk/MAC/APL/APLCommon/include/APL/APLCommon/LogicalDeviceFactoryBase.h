//#  LogicalDeviceFactoryBase.h: Base class for logical device factories.
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

#ifndef LogicalDeviceFactoryBase_H
#define LogicalDeviceFactoryBase_H

//# Includes
#define BOOST_SP_USE_PTHREADS
#include <boost/shared_ptr.hpp>

//# local includes
#include "APL/APLCommon/APLCommonExceptions.h"

//# Common Includes
#include <GCF/TM/GCF_Task.h>


namespace LOFAR {
  namespace APLCommon {

// forward declaration
class LogicalDevice;

class LogicalDeviceFactoryBase
{
public:
	// Constructor and desctuctor
	LogicalDeviceFactoryBase() {}; 
	virtual ~LogicalDeviceFactoryBase() {};
      
	// The factory call.
	virtual boost::shared_ptr<LogicalDevice> createLogicalDevice(const string& taskName, 
																 const string& parameterFile,
																 GCF::TM::GCFTask* pStartDaemon)=0;

	virtual bool sharingAllowed() {
		return (false);
	}

protected:
	// protected copy constructor
	LogicalDeviceFactoryBase(const LogicalDeviceFactoryBase&);
	// protected assignment operator
	LogicalDeviceFactoryBase& operator=(const LogicalDeviceFactoryBase&);

private:
	ALLOC_TRACER_CONTEXT  
};

};//APLCommon
};//LOFAR
#endif
