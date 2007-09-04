//# gcf-panel-util.ctl
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
//# General panel util functions:

const int OFF         = 0;
const int OPERATIONAL = 1;
const int MAINTENANCE = 2;
const int TEST        = 3;
const int SUSPICIOUS  = 4;
const int BROKEN      = 5;



///////////////////////////////////////////////////////////////////////////
//
// Function getStateColor(stateNr) : colorName
//
// helper function to return the colorName based on a state
// dyn_arrays start count at 1!!!
//
// Added 3-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
string getStateColor(int aState) {
	if (aState < dynlen(stateColor) & aState >= 0) {
    	return stateColor[aState+1];
  	} 
  	else { 
		return "";
	}
}


///////////////////////////////////////////////////////////////////////////
//
// Function getStateName(stateNr) : name
//
// helper function to return the stateName based on a state
// dyn_arrays start count at 1!!!
//
// Added 3-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
string getStateName(int aState) {
 	if (aState < dynlen(stateName) & aState >= 0) {
    	return stateName[aState+1];
  	} 
  	else { 
    	return "";
  	}
}

///////////////////////////////////////////////////////////////////////////
//
// Function getStateNumber(stateName) : stateNr
//
// helper function to return the stateNumber based on a stateName
// dyn_arrays start count at 1!!!
//
// Added 20-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
int getStateNumber(string aState) {
  	if (aState != "") {
    	for (int i = 1; i <= dynlen(stateName); i++) {
      		if (aState == stateName[i]) {
        		return (i-1);
      		}
    	}
  	}
  	return (-1);
}

///////////////////////////////////////////////////////////////////////////
//
// Function showSelfState
//
// Prepare (connect to) the selfState callback for a lofar hardware object 
//
// Added 3-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void showSelfState(string aDP) {
  	// check if the requiered datapoint for this view are accessible
  	if (dpAccessable(aDP+".state")) {
    	dpConnect("updateSelfState",aDP + ".state", aDP + ".state:_online.._invalid");
  	} 
  	else {
    	setValue("selfState.light","backCol","_dpdoesnotexist");
  	}
}

///////////////////////////////////////////////////////////////////////////
//
// Function showChildState
//
// Prepare (connect to) the childState callback for a lofar hardware object 
//
// Added 3-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void showChildState(string aDP) {
  	// check if the requiered datapoint for this view are accessible
  	if (dpAccessable(aDP+".childState")) {
    	dpConnect("updateChildState",aDP + ".childState", aDP + ".childState:_online.._invalid");
  	} 
  	else {
    	setValue("childStateBorder","foreCol","_dpdoesnotexist");
  	}
}


///////////////////////////////////////////////////////////////////////////
//
// Function updateChildState
//
// Callback to update the childState border from a lofar hardware object
//
// Added 3-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
updateChildState(string dp1, int state, string dp2, bool invalid) {
  	string SymbolCol;
	
  	if (invalid) {
    	SymbolCol = "Lofar_invalid";
  	}	
  	else {
    	SymbolCol = getStateColor(state);
  	}
  	setValue("childStateBorder", "foreCol", SymbolCol);
}

///////////////////////////////////////////////////////////////////////////
//
// Function updateSelfState
//
// Callback to update the selfstate light from a lofar hardware object
//
// Added 3-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
updateSelfState(string dp1, int state, string dp2, bool invalid) {
  	string SymbolCol;

  	if (invalid) {
    	SymbolCol = "Lofar_invalid";
  	}
  	else {
    	SymbolCol = getStateColor(state);
  	}
  	setValue("selfState.light", "backCol", SymbolCol);
}

///////////////////////////////////////////////////////////////////////////
//
// Function checkDpType
//
// check if the panel is placed on the right datatype
// aDP        = found datapointtype
// aCheckType = required DPT
//
// Added 8-5-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
checkDpType(string aDp, string aCheckType){
  	LOG_DEBUG("CheckType : ",aCheckType);
  	LOG_DEBUG("DP        : ",$datapoint);
  	LOG_DEBUG("dpTypeName: ",dpTypeName($datapoint));

  	if (aCheckType != dpTypeName($datapoint)){
    	ChildPanelOnCentralModal("objects/nav_fw/warning_wrong_datatype.pnl",
		                         "WRONG datapointtype",
	                     	     makeDynString("$datapointType:" + aCheckType,
	                             "$typeName:" +dpTypeName($datapoint)));
   	}
}