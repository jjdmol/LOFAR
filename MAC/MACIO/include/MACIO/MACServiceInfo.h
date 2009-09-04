//#  MACServiceInfo.h: Contains all information about the MAC services.
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
//#  $Id$

#ifndef MACIO_SERVICEINFO_H
#define MACIO_SERVICEINFO_H

// \file MACServiceInfo.h
// Contains all information about the MAC services.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes

// Avoid 'using namespace' in headerfiles

namespace LOFAR {

// \addtogroup MAC
// @{

// The only well-known ports of whole MAC.
#define	MAC_EC_PORT						10000
#define	SHM_INFOSERVER_PORT				23990
#define	MAC_CODELOGGING_PORT			23999
#define	MAC_SERVICEBROKER_PORT			24000

// Define names for the services
#define MAC_SVCMASK_RSPDRIVER			"RSPDriver%s:acceptor_v3.1"
#define	MAC_SVCMASK_CALSERVER			"CalServer%s:acceptor_v2"
#define MAC_SVCMASK_BEAMSERVER			"BeamServer%s:acceptor_v2.1"
#define MAC_SVCMASK_TBBDRIVER			"TBBDriver%s:acceptor"
//#define MAC_SVCMASK_RSPCTLFE			"RSPCtlFE%s:acceptor"

// Define names for the daemons
#define MAC_SVCMASK_SERVICEBROKER		"ServiceBroker%s:v1.0"
#define MAC_SVCMASK_STARTDAEMON			"StartDaemon%s:v1.0"
#define	MAC_SVCMASK_LOGPROC				"LogProcessor%s:v1.0"
#define MAC_SVCMASK_KVTLOGGER			"KVTLogger%s:v1.0"

// Define names for all controllers
#define	MAC_SVCMASK_SCHEDULERCTRL		"MACScheduler%s:v1.0"
#define	MAC_SVCMASK_OBSERVATIONCTRL		"Observation%s:v1.0"
#define	MAC_SVCMASK_BEAMDIRECTIONCTRL	"BeamDirection%s:v1.0"
#define	MAC_SVCMASK_GROUPCTRL			"RingControl%s:v1.0"
#define	MAC_SVCMASK_STATIONCTRL			"StationControl%s:v1.0"
#define	MAC_SVCMASK_CLOCKCTRL			"ClockCtrl%s:v1.0"
#define	MAC_SVCMASK_BEAMCTRL			"BeamCtrl%s:v1.0"
#define	MAC_SVCMASK_CALIBRATIONCTRL		"CalibrationCtrl%s:v1.0"
#define	MAC_SVCMASK_TBBCTRL				"TBBCtrl%s:v1.0"
#define	MAC_SVCMASK_STATIONINFRACTRL	"StationInfraCtrl%s:v1.0"
#define	MAC_SVCMASK_MACINFOSERVER		"MACInfoServer%s:v1.0"
#define	MAC_SVCMASK_SHMSESSION			"MISSession%s:v1.0"


// Define names for GCF test applications
#define MAC_SVCMASK_GCFTEST_ST3SERVER	"ST3%s:server"
#define MAC_SVCMASK_GCFTEST_ST3PROVIDER	"ST3%s:provider"
#define MAC_SVCMASK_APLTEST_CTLRMENU	"ControllerTestMenu%s:v1.0"

// @}
} // namespace LOFAR

#endif
