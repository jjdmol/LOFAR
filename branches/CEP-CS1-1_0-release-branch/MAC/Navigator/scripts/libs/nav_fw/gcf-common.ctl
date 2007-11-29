//# gcf-common.ctl
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

//#
//# Common GCF functions
//# 

#uses "nav_fw/gcf-logging.ctl"

global dyn_int g_distributedSystems;
global int g_distSysVarSem = 0;
global mapping stateColor;
global mapping stateName;
global mapping stateNumber;

///////////////////////////////////////////////////////////////////////////
//
// Function gcfStartFollowDistSystems()
//
// Connect the 'followDistributedSystems' function to the administration
// of the DistManager of PVSS to keep our g_distributedSystems variable
// equal to the list of connected systems.
//
///////////////////////////////////////////////////////////////////////////
void gcfStartFollowDistSystems()
{
	LOG_DEBUG("gcfStartFollowDistSystems.");
	g_distSysVarSem = 0;
	dpConnect("followDistributedSystems", "_DistManager.State.SystemNums");
}

///////////////////////////////////////////////////////////////////////////
//
// EventHandler followDistributedSystems(sysnumber)
//
// Event: A system has (dis)connected to our database.
//
// Copy the contents of _DistManager.State.SystemNums to our own
// g_distributedSystems variable.
//
///////////////////////////////////////////////////////////////////////////
void followDistributedSystems(string dp, dyn_int value)
{
	LOG_DEBUG("followDistributedSystems: ",value);
	while (g_distSysVarSem > 0) {
		// wait until the "semaphore" is freed by the dpAccessable method
		delay(0, 10); 
	}
	g_distributedSystems = value;
}

///////////////////////////////////////////////////////////////////////////
//
// Function lockDistSystemSemaphore
//
//
///////////////////////////////////////////////////////////////////////////
void lockDistSystemSemaphore() {
	g_distSysVarSem++;
}

///////////////////////////////////////////////////////////////////////////
//
// Function unlockDistSystemSemaphore
//
//
///////////////////////////////////////////////////////////////////////////
void unlockDistSystemSemaphore() {
	g_distSysVarSem--;
}

///////////////////////////////////////////////////////////////////////////
//
// Function initLofarColors
//
// Initialise the color buffers.
//
// Added 4-4-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void initLofarColors() {
  	// Set the global statecolors/colornames.
  	stateColor [0]  = "Lofar_off";
    stateColor [10] = "Lofar_operational";
    stateColor [20] = "Lofar_maintenance";
    stateColor [30] = "Lofar_test";
    stateColor [46] = "Lofar_suspicious";
    stateColor [56] = "Lofar_broken";
    
  	stateName [0]  = "off";
    stateName [10] = "operational";
    stateName [20] = "maintenance";
    stateName [30] = "test";
    stateName [46] = "suspicious";
    stateName [56] = "broken";
    
  	stateNumber ["off"]             = 0;
    stateNumber ["operational"]     = 10;
    stateNumber ["maintenance"]     = 20;
    stateNumber ["test"]            = 30;
    stateNumber ["suspicious"]      = 46;
    stateNumber ["broken"]          = 56;
    
}