//#  ControllerDefines.h: all kind of enumerations for controllers.
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

#ifndef APL_CONTROLLERDEFINES_H
#define APL_CONTROLLERDEFINES_H

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <GCF/TM/GCF_PortInterface.h>

namespace LOFAR { 
namespace APLCommon {

// Define errornumbers the controller may return.
enum
{
   CT_RESULT_NO_ERROR = 0, 
	CT_RESULT_LOST_CONNECTION,
	CT_RESULT_CLAIM_FAILED,
	CT_RESULT_CALSTART_FAILED,
	CT_RESULT_CALSTOP_FAILED,
	CT_RESULT_BEAMALLOC_FAILED,
	CT_RESULT_BEAMFREE_FAILED,
	CT_RESULT_MODESETUP_FAILED,
	CT_RESULT_ALLOC_FAILED,
	CT_RESULT_TRIGSETUP_FAILED,
	CT_RESULT_RECORD_FAILED,
	CT_RESULT_RELEASE_FAILED,
	CT_RESULT_ALREADY_REGISTERED,
	CT_RESULT_INVALID_ARGUMENT,
	CT_RESULT_CONFLICTING_ARGS,
	CT_RESULT_OUT_OF_SYNC,
	CT_RESULT_OBS_CONFLICT,
	CT_RESULT_MANUAL_ABORT,
  CT_RESULT_UNSPECIFIED
};

// Define numbers for the controllertypes. These numbers are translated to names,
// execuables, etc in the class ControllerName.
enum	{
	CNTLRTYPE_NO_TYPE = 0,				// no type defined yet
	CNTLRTYPE_SCHEDULERCTRL,			// MACscheduler
	CNTLRTYPE_OBSERVATIONCTRL,			// ObservationControl
	CNTLRTYPE_ONLINECTRL,				// OnlineControl
	CNTLRTYPE_OFFLINECTRL,				// OfflineControl
	CNTLRTYPE_BEAMDIRECTIONCTRL,		// BeamDirectionControl
	CNTLRTYPE_GROUPCTRL,				// RingControl
	CNTLRTYPE_STATIONCTRL,				// StationControl
	CNTLRTYPE_CLOCKCTRL,				// ClockControl
	CNTLRTYPE_BEAMCTRL,					// BeamControl
	CNTLRTYPE_CALIBRATIONCTRL,			// CalibrationControl
	CNTLRTYPE_TBBCTRL,					// TBBControl
	CNTLRTYPE_STATIONINFRACTRL,			// StationInfraControl
	CNTLRTYPE_TESTCTRL,					// StationControl
	
	CNTLRTYPE_NR_TYPES					// should always be the last
};

// Construct a uniq controllername from the controllerType, the instanceNr
// of the controller and the observationID.
// Note: the returned name is always the 'non-shared' name. To get the 'shared'
//		 name passed the result to 'sharedControllerName')
string	controllerName (uint16			cntlrType, 
						uint16			instanceNr, 
						uint32			ObservationNr,
						const string&	hostname = "");

// Convert a controller type to the coresponding node in the OTDB.
string	parsetNodeName (uint16		cntlrType);

// Convert the 'non-shared controllername' to the 'shared controller' name.
string	sharedControllerName (const string&	controllerName);

// Return name of the executable
string	getExecutable (uint16		cntlrType);

// return 'shared' bit of controllertype
bool	isSharedController(uint16		cntrlType) ;

// Get the ObservationNr from the controllername.
uint32	getObservationNr (const string&	ObservationName);

// Get the instanceNr from the controllername.
uint16	getInstanceNr (const string&	ObservationName);

// Get the controllerType from the controllername.
int32	getControllerType	(const string&	ObservationName);

// Get the name of the observation.
string 	observationName(int	obsID);

// Get the name of the parset of the observation.
string 	observationParset(int	obsID);

// Construct PS name solving markers line @observation@, @ring@, etc.
string	createPropertySetName(const string&		propSetMask,
							  const string&		controllerName,
							  const string&		realDPname = "REALDPNAME");

// Construct a message the matches the given CTState and send it on the port.
bool sendControlResult(GCF::TM::GCFPortInterface&	port,
					   uint16						signal,
					   const string&				cntlrName,
					   uint16						result);
};	// APLCommon
}; // LOFAR

#endif
