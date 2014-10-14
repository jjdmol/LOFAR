//#  ControllerInfo.h: Administrative description of a controller
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

#ifndef APLCOMMON_CONTROLLERINFO_H
#define APLCOMMON_CONTROLLERINFO_H

// \file
// Administrative description of a controller.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/CTState.h>
#include <GCF/TM/GCF_PortInterface.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
	using GCF::TM::GCFPortInterface;
  namespace APLCommon {

// \addtogroup APLCommon
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class ChildControl;


// class_description
// ChildControl is actually the internal controller administration of the
// ChildControl class. To prevent double administration of the controllers
// in the main-task the administration is partially made accesable.
class ControllerInfo
{
public:
	// Allow default construction and destruction for use in lists
	ControllerInfo() {};
	~ControllerInfo() {};

	string&				getName()		{	return (cntlrName);		};
	uint32				getInstanceNr()	{	return (instanceNr);	};
	uint16				getType()		{	return (cntlrType);		};
	GCFPortInterface*	getPort()		{	return (port);			};

	// Give ChildControl full access to the data.
	friend class ChildControl;

private:
	// Copying is not allowed
//	ControllerInfo(const ControllerInfo&	that);
//	ControllerInfo& operator=(const ControllerInfo& that);

	//# --- Datamembers ---
	string				cntlrName;		// uniq name of the controller
	uint32				instanceNr;		// for nonshared controllers
	OTDBtreeIDType		obsID;			// observation tree the cntlr belongs to
	GCFPortInterface*	port;			// connection with the controller
	uint16				cntlrType;		// type of controller
	string				hostname;		// host the controller runs on
	CTState::CTstateNr	requestedState;	// the state the controller should have
	time_t				requestTime;	// time of requested state
	CTState::CTstateNr	currentState;	// the state the controller has
	time_t				establishTime;	// time the current state was reached
	uint16				result;			// error nr of last action
	bool				inResync;		// controller in resync-cycle.
	time_t				startTime;		// time the controller must be active
	time_t				stopTime;		// time the controller must be stopped
	// --- for use in action list ---
	time_t				retryTime;		// time the request must be retried
	uint32				nrRetries;		// nr of retries performed
};


// @}
  } // namespace APLCommon
} // namespace LOFAR

#endif
