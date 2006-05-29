//#  LogicalDeviceStarter.h: one line description
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_CUDAEMONS_LOGICALDEVICESTARTER_H
#define LOFAR_CUDAEMONS_LOGICALDEVICESTARTER_H

// \file
// one line description.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <APS/ParameterSet.h>

namespace LOFAR {
  using ACC::APS::ParameterSet;
  namespace CUDaemons {

    // \addtogroup CUDaemons
    // @{

    //# Forward Declarations
    //#class forward;


// Description of class.
class LogicalDeviceStarter
{
public:
	explicit LogicalDeviceStarter (ParameterSet*	aParSet);
	~LogicalDeviceStarter();

	int32 createLogicalDevice (const string&	ldTypeName,
							   const string&	taskname,
							   const string& 	parentHost,
							   const string& 	parentService);

	int32	error()	{ return (itsError); }

private:
	// Copying is not allowed
	LogicalDeviceStarter();
	LogicalDeviceStarter (const LogicalDeviceStarter& that);
	LogicalDeviceStarter& operator= (const LogicalDeviceStarter& that);

private:
	//# Datamembers
	typedef struct {
		string		name;
		string		executable;
		bool		shared;
	} LDstart_t;

	vector<LDstart_t>		itsProgramList;
	int32					itsError;
};

    // @}

  } // namespace CUDaemons
} // namespace LOFAR

#endif
