//# gcfpa-cwd.ctl
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
//# GCF Connection watchdog
//# 

#uses "gcf-logging.ctl"
#uses "gcfpa-com.ctl"

const string GCF_WD_DP = "__gcf_wd";

////////////////////////////////////////////////////////////////////////////////
//
// Function main
//
// Starts the ConnectionWatchDog thread that monitor the (dis)connecting
// systems.
//
////////////////////////////////////////////////////////////////////////////////
main() {

   // prepare pa points
   PAPrep();

   // prepare watchdog
   addGlobal("gConnManList", DYN_DYN_STRING_VAR);	
   addGlobal("gDistSysList", DYN_UINT_VAR);	

   // start watchdog
   startThread("gcfConnectionWatchDog");

}

////////////////////////////////////////////////////////////////////////////////
//
// Function gcfConnectionWatchDog()
//
// Attach 'distSystemChanged' to the _DistManager.State.SystemNums datapoint
// to keep track of (dis)connecting system the are kept in gDistSysList.
//
////////////////////////////////////////////////////////////////////////////////
void gcfConnectionWatchDog() {
	LOG_DEBUG("GCF: Starting GCF connection watch-dog");

	retrieveManNums(getSystemId());
	dpGet("_DistManager.State.SystemNums", gDistSysList);

	for (int i = 1; i <= dynlen(gDistSysList); i++) {
		retrieveManNums(gDistSysList[i]);	 		
	}
	dpConnect("distSystemChanged", FALSE, "_DistManager.State.SystemNums");

	LOG_DEBUG("GCF: Watch-dog started");
}

////////////////////////////////////////////////////////////////////////////////
//
// Function retrieveManNums(sysNr)
//
// Install handlers for watching (dis)connecting UIs and APIs of the given system.
//
////////////////////////////////////////////////////////////////////////////////
void retrieveManNums(unsigned sysNr) {
	LOG_DEBUG("GCF: Add managers for (new) system " + getSystemName(sysNr) + ".");

	dyn_anytype manNums;
	string 		sysName = getSystemName(sysNr);

	dpGet(sysName + "_Connections.Ui.ManNums", manNums);
	addManagers(sysNr, manNums, "Ui");
	dpConnect("uiConnectionsChanged", FALSE, sysName + "_Connections.Ui.ManNums");

	dpGet(sysName + "_Connections.Api.ManNums", manNums);
	addManagers(sysNr, manNums, "Api");
	dpConnect("apiConnectionsChanged", FALSE, sysName + "_Connections.Api.ManNums");
}

////////////////////////////////////////////////////////////////////////////////
//
// Function addManagers(sysNr, dyn mgrNrs, mgrType)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void addManagers(unsigned sysNr, dyn_anytype manNums, string manType) {
	LOG_DEBUG("addManagers: ", sysNr, manNums, manType);

	dyn_string manItem;
	string 	   systemId = (string) sysNr;
	for (int i = 1; i <= dynlen(manNums); i++) {
		manItem = makeDynString(sysNr, manType, manNums[i]);	
		LOG_TRACE("GCF: Add mananger: " + getSystemName(sysNr) + manType + ":" + manNums[i]);
		gConnManList[dynlen(gConnManList) + 1] = manItem;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// EventHandler distSystemChanged(dp, dyn newDistSysList)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void distSystemChanged(string dp, dyn_uint newDistSysList) {
  	LOG_DEBUG("distSystemChanged: ", dp, newDistSysList);	

  	for (int i = 1; i <= dynlen(gDistSysList); i++) {
    	if (!dynContains(newDistSysList, gDistSysList[i])) {	
      		remoteSystemGone(gDistSysList[i]);
    	}
  	}

  	for (int i = 1; i <= dynlen(newDistSysList); i++) {
    	if (!dynContains(gDistSysList, newDistSysList[i])) {
      		dpSet(GCF_WD_DP + ".sys", "c" + newDistSysList[i] + ":");
      		retrieveManNums(newDistSysList[i]);
    	}
  	}

  	gDistSysList = newDistSysList;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function remoteSystemGone(sysNr)
//
//
//
////////////////////////////////////////////////////////////////////////////////
void remoteSystemGone(unsigned sysNr) {
	LOG_DEBUG("GCF: System " + getSystemName(sysNr) + " gone.");	

	string msg = "d" + sysNr + ":";
	dpSet(GCF_WD_DP + ".sys", msg);

	for (int i = 1; i <= dynlen(gConnManList); i++) {
		if (gConnManList[i][1] == sysNr) {						
			dynRemove(gConnManList, i);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// EventHandler uiConnectionsChanged
//
//
//
////////////////////////////////////////////////////////////////////////////////
void uiConnectionsChanged(string dp, dyn_uint value) {
	connectionsChanged(dp, value, "Ui");
}

////////////////////////////////////////////////////////////////////////////////
//
// EventHandler apiConnectionsChanged
//
//
//
////////////////////////////////////////////////////////////////////////////////
void apiConnectionsChanged(string dp, dyn_uint value) {
  	connectionsChanged(dp, value, "Api");
}

////////////////////////////////////////////////////////////////////////////////
//
// EventHandler connectionsChanged
//
//
//
////////////////////////////////////////////////////////////////////////////////
void connectionsChanged(string dp, dyn_uint value, string manType) {
	LOG_DEBUG("connectionsChanged: ", dp, value, manType);

	string		sysNr = getSystemId(dpSubStr(dp, DPSUB_SYS));
	dyn_string	msg;
	int			i, j;
	bool		manNumFound = false;
	dyn_string	newItem;

	for (i = 1; i <= dynlen(value); i++) {
		for (j = 1; j <= dynlen(gConnManList); j++) {
			if (gConnManList[j][1] == sysNr && 
					gConnManList[j][2] == manType && 
					value[i] == gConnManList[j][3]) {
				manNumFound = true;
				break;	
			}
		} 
		if (!manNumFound) {
			newItem = makeDynString(sysNr, manType, value[i]);
			LOG_TRACE("GCF: Add mananger: " + getSystemName(sysNr) + manType + ":" + value[i]);
			gConnManList[dynlen(gConnManList) + 1] = newItem;
		}
		else {
			manNumFound = false;
		}
	}

	for (i = 1; i <= dynlen(gConnManList); i++) {
		if (gConnManList[i][1] == sysNr && 
				gConnManList[i][2] == manType && 
				!dynContains(value, gConnManList[i][3])) {
			// a (remote) manager is disconnected from PVSS so inform the local property agent
			msg = "d" + sysNr + ":" + manType + ":" + gConnManList[i][3] + ":";
			LOG_TRACE("GCF: Remove mananger: " + msg);
			dpSet(GCF_WD_DP + ".man", msg);
			dynRemove(gConnManList, i);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Create the base types needed for PA, Dist and Point Trigger in the database
//
////////////////////////////////////////////////////////////////////////////////
void PAPrep() {
	dyn_dyn_string xxdepes;
	dyn_dyn_int xxdepei;
	dyn_string types;

	types = dpTypes("*");
	handleType(types, "NCFObjectState", DPEL_STRING);
	string type = "GCFWatchDog";
	if (dynContains(types, type)) {
		deleteDPs(type);
	}
	xxdepes[1] = makeDynString (type, "");
	xxdepes[2] = makeDynString ("", "sys");
	xxdepes[3] = makeDynString ("", "man");
	xxdepei[1] = makeDynInt (DPEL_STRUCT);
	xxdepei[2] = makeDynInt (0, DPEL_STRING);
	xxdepei[3] = makeDynInt (0, DPEL_STRING);
	dpTypeCreate(xxdepes,xxdepei);
	DebugN("Add type " + type);
	dpCreate("__gcf_wd", type);
	dpCreate("__navObjectState", "NCFObjectState");
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: deleteDPs(string)
//
// delete all DP's from given type and delete the type when finished
//
////////////////////////////////////////////////////////////////////////////////
deleteDPs(string type) {
  	DebugTN("deleteDPs: ", type);
  	string dpName;
  	dyn_string names = dpNames("*",type);  
  	int i, len;
  	len = dynlen(names);
  	if (len > 0) {
     	  for (i = 1; i <= len; i++) {
      		dpName = names[i];
      			dpDelete(dpName);
      			DebugTN(dpName + " deleted");
    	  }
  	}
  	dpTypeDelete(type);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function handleType(dyn_string,string,int)
//
//  handle dynstring types to be created
//
////////////////////////////////////////////////////////////////////////////////
void handleType(dyn_string types, string type, int dpelType) {
//	DebugTN("handleType: ", types, type, dpelType);
//	if (dynContains(types, type)) {
//		deleteDPs(type);
//	}
	dyn_dyn_string xxdepes;
	dyn_dyn_int xxdepei;
	xxdepes[1] = makeDynString (type);
	xxdepei[1] = makeDynInt (dpelType);
	// create type if not allready there
	if (!dynContains(types, type)) {
		dpTypeCreate(xxdepes,xxdepei);
		DebugN("Add type " + type);
	}
}
	
