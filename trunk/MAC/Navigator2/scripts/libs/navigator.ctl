// navigator.ctl
//
//  Copyright (C) 2002-2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
///////////////////////////////////////////////////////////////////
// global functions for the navigator
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// navigator_handleEventInitialize	: initializes the navigator
// navigator_handleEventTerminate	: terminates the navigator
// navigator_handleEventClose		: closes the navigator
// navigator_initializing               : returns false if init ready, else true
// navigator_clearWorkDPs               : clear the work Datapoints

#uses "GCFCWD.ctl"
#uses "GCFLogging.ctl"
#uses "claimManager.ctl"
#uses "GCFCommon.ctl"
#uses "GCFAlarm.ctl"
#uses "navCtrl.ctl"
#uses "navConfig.ctl"
#uses "navFunct.ctl"
#uses "navTabCtrl.ctl"

global bool       g_initializing         = true;
global string     g_currentDatapoint     = "LOFAR";
global dyn_string g_observationsList;  // holds active observations
global dyn_string g_processesList;     // holds active software
global mapping    g_observations;      // 
global dyn_string g_stationList;       // holds valid stations for choices in the viewBox
global dyn_string g_cabinetList;       // holds valid cabinets for choices in the viewBox
global dyn_string g_subrackList;       // holds valid subracks for choices in the viewBox
global dyn_string g_RSPList;           // holds valid RSP's for choices in the viewBox
global dyn_string g_TBBList;           // holds valid TBB's for choices in the viewBox
global dyn_string g_RCUList;           // holds valid RCU's for choices in the viewBox
global dyn_string strPlannedObs;
global dyn_string strHighlight;        // contains highlight info for mainpanels
global dyn_string highlight;           // contains highlight info for objects

//======================== Action Handlers =======================================
//
// The following functions are used to handle the 'actions' that are 
// generated by the mainpanel.
//

void navigator_handleEventInitialize()
{
  LOG_TRACE("navigator.ctl:navigator_handleEventInitialize|entered");
  g_initializing = true;

  // Set the global statecolors/colornames.
  initLofarColors();
  
  // Init the connection Watchdog
  GCFCWD_Init();

  // first thing to do: get a new navigator ID
  // check the commandline parameter:
  int navID = 0;
  if (isDollarDefined("$ID")) {
    navID = $ID;
  }
  // make sure there is a __navigator<id> datapoint of the type GCFNavigatorInstance.
  navConfig_setNavigatorID(navID); 

  // Do a dpQueryConnectSingle() so that we get a permanent list of claims
  // we can use this to translate a claimed name into a real datapoint name
  claimManager_queryConnectClaims();
  
  // initialize the logSystem
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".logger")) {
    initLog(DPNAME_NAVIGATOR + g_navigatorID + ".logger");
  } else {
    DebugN("ERROR: Logsystem hasn't been found.");
  }

  // set user to root for now, has to be taken from PVSS login later
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".user")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".user",getUserName());
    ACTIVE_USER=getUserName();
  }

  // Clear the workDatapoints
  navigator_clearWorkDPs();  
        
  // Initilaize the alarm system
  initAlarmSystem();

  // set initialized ready
  g_initializing = false;

  LOG_TRACE("navigator.ctl:navigator_handleEventInitialize|end");
}

///////////////////////////////////////////////////////////////////////////
//
// Function navigator_handleEventTerminate()
//
// NOTE: it is NOT possible to call dpGet in the terminate handler!
//
///////////////////////////////////////////////////////////////////////////
void navigator_handleEventTerminate()
{
  LOG_TRACE("navigator.ctl:navigator_handleEventTerminate|entered");
}

///////////////////////////////////////////////////////////////////////////
//
// Function navigator_handleEventClose()
//
// de-initializes the navigator
// NOTE: it is NOT possible to call dpGet in the terminate handler!
//
///////////////////////////////////////////////////////////////////////////
void navigator_handleEventClose()
{
  LOG_TRACE("navigator.ctl:navigator_handleEventClose|entered");

    
  PanelOff();
}

///////////////////////////////////////////////////////////////////////////
//
// Function navigator_initializing()
//
// returns false if init is ready, else false
//
///////////////////////////////////////////////////////////////////////////
bool navigator_initializing() {
  return g_initializing;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navigator_clearWorkDPs()
//
// clears all workpoints that can contain residues from previous runs.
//
///////////////////////////////////////////////////////////////////////////
void navigator_clearWorkDPs() {
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".observationsList")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".observationsList",makeDynString(",LOFAR,LOFAR"));
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".hardwareList")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".hardwareList",makeDynString(",LOFAR,LOFAR"));
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".processesList")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".processesList",makeDynString(""));
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".trigger")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".trigger",false);
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".objectTrigger")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".objectTrigger",false);
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_viewBox.action")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".fw_viewBox.action","");
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_viewSelection.action")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".fw_viewSelection.action","");
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_topDetailSelection.action")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".fw_topDetailSelection.action","");
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_bottomDetailSelection.action")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".fw_bottomDetailSelection.action","");
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_locator.action")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".fw_locator.action","");
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_progressBar.action")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".fw_progressBar.action","");
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_headLines.action")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".fw_headLines.action","");
  }
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_alerts.action")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".fw_alerts.action","");
  }

}
