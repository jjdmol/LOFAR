// navConfig.ctl
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
// configuration storage functions for the Navigator.
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// navConfig_getNavigatorID		: returns the navigator ID
// navConfig_setNavigatorID		: sets the navigator ID
// navConfig_resetDP          :


#uses "GCFLogging.ctl"
#uses "navigator.ctl"


global string   DPTYPENAME_NAVIGATOR_INSTANCE   = "NavigatorInstance";
global int      g_navigatorID 			= 0;


//========================== Navigator ID and useCount ===========================

///////////////////////////////////////////////////////////////////////////
//
// Function navConfig_getNavigatorID() : ID
//  
// returns the navigator ID
//
///////////////////////////////////////////////////////////////////////////
int navConfig_getNavigatorID()
{
  return g_navigatorID;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfig_setNavigatorID(ID)
//  
// sets the navigator ID
//
///////////////////////////////////////////////////////////////////////////
void navConfig_setNavigatorID(int newID)
{
  LOG_TRACE("navConfig.ctl:navConfig_setNavigatorID|entered with ID: "+newID);

  // We need an unique ID for this instance of the user interface
  // We can use the myManId() which will give:
  // <manager type> + <manager ID>
  if (newID < 0) {
    g_navigatorID = myManId();
  } else {
    g_navigatorID = newID;
  }   
    
  // if there is no DP yet available, create one.
  if (! dpExists(DPNAME_NAVIGATOR + g_navigatorID)) {
    LOG_DEBUG("navConfig.ctl:navConfig_setNavigatorID|Creating new navigator configuration");
    dpCreate(DPNAME_NAVIGATOR + g_navigatorID, DPTYPENAME_NAVIGATOR_INSTANCE);
  }

  // We need to reset the fields in our datapoint
  // so that we don't have any old stuff ( from a previous session )
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_viewBox.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_viewBox.action");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_viewBox.selection");
  
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_viewSelection.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_viewSelection.action");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_viewSelection.selection");
  
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_topDetailSelection.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_topDetailSelection.action");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_topDetailSelection.selection");

  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_bottomDetailSelection.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_bottomDetailSelection.action");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_bottomDetailSelection.selection");
  
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_locator.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_locator.action");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_locator.selection");
  
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_progressBar.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_progressBar.action");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_progressBar.selection");
  
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_headLines.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_headLines.action");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_headLines.selection");
  
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_alerts.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_alerts.action");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_alerts.selection");
  
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_fastJumper.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_fastJumper.action");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".fw_fastJumper.selection");

  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".navigator.event");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".navigator.initiator");
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".navigator.selection");
  
  navConfig_resetDP(DPNAME_NAVIGATOR + g_navigatorID+".user");

  LOG_DEBUG("navConfig.ctl:navConfig_setNavigatorID|Navigator ID:", g_navigatorID);
}
  
///////////////////////////////////////////////////////////////////////////
//
// Function navConfig_resetDP
//  
// resets a datapoint
//
///////////////////////////////////////////////////////////////////////////
void navConfig_resetDP(string aDP) {
  if (dpSet(aDP,"") == -1) {
    LOG_DEBUG("navConfig.ctl:navConfig_resetDP|Error during dpSet : " + aDP + " Error: " + getLastError());
  }
}


