//# gcfpa-prep.ctl
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
//# Prepares the PVSS database for the PropertyAgent
//# 

////////////////////////////////////////////////////////////////////////////////
//
// Function Main.
//
// Create the base types needed for PA, Dist and Point Trigger in the databse
//
////////////////////////////////////////////////////////////////////////////////
main() {
	dyn_dyn_string xxdepes;
	dyn_dyn_int xxdepei;
	dyn_string types;

	types = dpTypes("GCF*");
  dynAppend(types,"NCFObjectState");
	handleType(types, "NCFObjectState", DPEL_STRING);
	handleType(types, "GCFPaPsEnabled", DPEL_STRING);
	handleType(types, "GCFPaPsIndication", DPEL_STRING);
	handleType(types, "GCFDistPort", DPEL_BLOB);
	if (!dpExists("__gcfportAPI_DPAserver")) {
		dpCreate("__gcfportAPI_DPAserver", "GCFDistPort");
	}
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
	dpCreate("__pa_PSIndication", "GCFPaPsIndication");
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
      		if (dpName != getSystemName() + "__gcfportAPI_DPAserver") {
      			dpDelete(dpName);
      			DebugTN(dpName + " deleted");
      		}
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
	DebugTN("handleType: ", types, type, dpelType);
	if (dynContains(types, type)) {
		deleteDPs(type);
	}
	dyn_dyn_string xxdepes;
	dyn_dyn_int xxdepei;
	xxdepes[1] = makeDynString (type);
	xxdepei[1] = makeDynInt (dpelType);
	dpTypeCreate(xxdepes,xxdepei);
	DebugN("Add type " + type);
}
