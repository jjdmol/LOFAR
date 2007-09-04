//# gcfnav-pmlinterface.ctl
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
//# This PVSS control script provides an interface to the gcf pml scripts. It hides 
//# the administration of clientID's from the caller
//#

#uses "nav_fw/gcf-logging.ctl"
#uses "nav_fw/gcfpa-pml.ctl"

global unsigned g_PAclientId = 0;  // client ID used during all communication with the PML

global string    NAVPML_DPNAME_ENABLED   = "__enabled";
global string    NAVPML_ENABLED_PERM     =  "permanent";
global string    NAVPML_ENABLED_TEMP     =  "temporary";
global string    NAVPML_ENABLED_AUTOLOAD =  "autoloaded";
global string    NAVPML_ENABLED_PERM_AUTOLOADED =  "perm-autoloaded";
global string    NAVPML_ENABLED_TEMP_AUTOLOADED =  "temp-autoloaded";

///////////////////////////////////////////////////////////////////////////
//
// Function navPMLinitialize
//
// Open the connection with the PML
//
///////////////////////////////////////////////////////////////////////////
void navPMLinitialize() {
/*	LOG_DEBUG("navPMLinitialize");

	// Create communication DP with PA and attach callback function
	if (g_PAclientId == 0) {
		g_PAclientId = gcfInit("pmlCallback");
	}
	else {
		LOG_WARN("PML already initialized");
	}

	if (g_PAclientId == 0) {
		LOG_FATAL("PML not properly initialized");
	}
	else {
		dpConnect("HandlePanelMessage", false, DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_MESSAGE);
		LOG_INFO("PML initialized. Using ID:", g_PAclientId);
	}
*/
}  

////////////////////////////////////////////////////////////////////////////////
//
// EventHanler HandlePanelMessage(dp, message)
//
// Unload PS if PS is not of type 'autoloaded'
//
////////////////////////////////////////////////////////////////////////////////
void HandlePanelMessage(string dp, string msg) {
/*	LOG_DEBUG("HandlePanelMessage: ", dp, msg);

	dyn_string splittedMsg = strsplit(msg, "|");  
	if ((dynlen(splittedMsg) == 2) && (splittedMsg[1] == "PML_UNLOAD")) {
		string datapoint = splittedMsg[2];
		if (!navPMLisAutoLoaded(datapoint)) {
			LOG_TRACE("PML unload propertyset", g_PAclientId, datapoint);
			gcfUnloadPS(g_PAclientId, datapoint);
		}
	}

*/
}

///////////////////////////////////////////////////////////////////////////
//
// Function navPMLterminate
//
// Closes the connection with the PML
//
///////////////////////////////////////////////////////////////////////////
void navPMLterminate(bool inTerminate = false) {
/*	LOG_DEBUG("navPMLterminate");

	if (g_PAclientId != 0) {
		gcfLeave(g_PAclientId, inTerminate);
		g_PAclientId = 0;
	}
*/
}  

///////////////////////////////////////////////////////////////////////////
//
// Function navPMLloadPropertySet
//
// Loads the propertyset of the datapoint
//
///////////////////////////////////////////////////////////////////////////
void navPMLloadPropertySet(string datapoint) {
/*	LOG_DEBUG("navPMLloadPropertySet", datapoint);

	navPMLCorrectDp(datapoint);
	if (!navPMLisAutoLoaded(datapoint)) {
		gcfLoadPS(g_PAclientId, datapoint);
	}
*/
}

///////////////////////////////////////////////////////////////////////////
//
// Function navPMLunloadPropertySet
//
// Unloads the propertyset of the datapoint.
//
///////////////////////////////////////////////////////////////////////////
bool navPMLunloadPropertySet(string datapoint) {
/*	LOG_DEBUG("navPMLunloadPropertySet", datapoint);
	
	string navigatorMessagePoint=DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_MESSAGE;

	navPMLCorrectDp(datapoint);
	if (dpExists(datapoint) & dpExists(navigatorMessagePoint)) {
		// in this way the context can be switched from the terminated panel to a still running 
		// panel (the navigator)
		dpSet(navigatorMessagePoint, "PML_UNLOAD|" + datapoint);
	}
	else {
		LOG_TRACE("navPMLunloadPropertySet -- Datapoint does not exist", datapoint);
	}
*/
}

///////////////////////////////////////////////////////////////////////////
//
// Function navPMLconfigurePropertySet
//
// Configures the propertyset of the datapoint
//
///////////////////////////////////////////////////////////////////////////
void navPMLconfigurePropertySet(string psScope, string psApcName) {
/*	LOG_DEBUG("navPMLconfigurePropertySet", psScope, psApcName);

	navPMLCorrectDp(psScope);  
	gcfConfigurePS(g_PAclientId, psScope, psApcName);
*/
}

///////////////////////////////////////////////////////////////////////////
//
// pmlCallback - this function is called by the PropertyAgent when a (un)load request
//               has finished. 
// response array contents:
// [1] : response ("loaded", "unloaded", "configured", "gone")
// [2] : datapoint name
// [3] : result ("OK", "failed")
//
///////////////////////////////////////////////////////////////////////////
void pmlCallback(dyn_string response) {
/*  LOG_DEBUG("pmlCallback:", LOG_DYN(response));
*/
}

///////////////////////////////////////////////////////////////////////////
//
// Function navPMLisAutoLoaded
//
// Returns true if the propertyset is auto loaded
//
///////////////////////////////////////////////////////////////////////////
bool navPMLisAutoLoaded(string datapoint) {
/*	LOG_DEBUG("navPMLisAutoLoaded", datapoint);

	// DP must exist ofcourse
	if (!dpAccessable(datapoint)) {
		LOG_DEBUG("navPMLloadPropertySet -- Datapoint does not exist", datapoint);
		return FALSE;
	}

	// check if the propertyset is autoloaded by GCF
	if (dpAccessable(datapoint + NAVPML_DPNAME_ENABLED)) {
		string enabled = "";
		dpGet(datapoint + NAVPML_DPNAME_ENABLED + ".", enabled);
		// DP is autoloaded if 'autoloaded' is somewhere in the value.
		if (strpos(enabled, NAVPML_ENABLED_AUTOLOAD) >= 0) {
			return TRUE;
		}
	}

	return FALSE;
*/
  return true;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navPMLisTemporary
//
// Returns true if the propertyset is temporary
//
///////////////////////////////////////////////////////////////////////////
bool navPMLisTemporary(string datapoint) {
/*	LOG_DEBUG("navPMLisTemporary: ", datapoint);

	bool temporary = false;

	// check if the propertyset is temporary by GCF
	// add __enabled to orig DP
	if (strpos(datapoint, NAVPML_DPNAME_ENABLED) < 0) {
		datapoint += NAVPML_DPNAME_ENABLED;
	}

	// get contents of DP__enabled point and check for 'temp'
	if (dpAccessable(datapoint)) {
		string enabled = "";
		dpGet(datapoint + ".", enabled);
		LOG_TRACE("navPMLisTemporary[content enabled]", enabled);
		if (strpos(enabled, NAVPML_ENABLED_TEMP) == 0 || strpos(enabled, NAVPML_ENABLED_TEMP_AUTOLOADED) == 0) {
			temporary = true;
		}
	}

	LOG_TRACE("navPMLisTemporary[T/F]", temporary);

	return temporary;
*/
  return false;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navPMLCorrectDp
//
// Cut off the element name path (if specified) and/or the
// NAVPML_DPNAME_ENABLED indicator
//
///////////////////////////////////////////////////////////////////////////
void navPMLCorrectDp(string& dpName) {
/*
	LOG_DEBUG("navPMLCorrectDp: ", dpName);

	// don't use the PVSS function dpSubStr here, because the dpName could 
	// not exists, which than results in an empty string
	dyn_string splittedDpName = strsplit(dpName, ".");
	dpName = splittedDpName[1];
	// does name contain __enabled?
	int enabledPos = strpos(dpName, NAVPML_DPNAME_ENABLED);
	if (enabledPos > 0) {
		dpName = substr(dpName, 0, enabledPos);	// cut it off.
	}  
*/
}

