// GCFCommon.ctl
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
// Common GCF functions
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// getStateColor      : return the colorName based on a state
// getStateName       : return the stateName based on a state
// getStateNumber     : return the stateNumber based on a stateName
// initLofarColors		: Initialise the color buffers.
// showChildState     : Prepare (connect to) the childState callback for a lofar hardware object
// showSelfState      : Prepare (connect to) the selfState callback for a lofar hardware object
// updateChildState   : Callback to update the selfstate light from a lofar hardware object
// updateSelfState    : Callback to update the childState border from a lofar hardware object

#uses "GCFLogging.ctl"

//the Database name & number for the Mainsystem (== MainCU)
const string   MainDBName        = "MCU001:";
const unsigned MainDBID          = 61;
const string   DPNAME_NAVIGATOR  = "__navigator";

global mapping stateColor;
global mapping stateName;
global mapping stateNumber;

///////////////////////////////////////////////////////////////////////////
//
// Function initLofarColors
//
// Initialise the color buffers.
//
///////////////////////////////////////////////////////////////////////////
void initLofarColors() {
  	// Set the global statecolors/colornames.
  	stateColor [0]  = "Lofar_off";
    stateColor [10] = "Lofar_operational";
    stateColor [20] = "Lofar_maintenance";
    stateColor [30] = "Lofar_test";
    stateColor [40] = "Lofar_suspicious";
    stateColor [43] = "Lofar_suspicious_went";
    stateColor [46] = "Lofar_suspicious_came";
    stateColor [50] = "Lofar_broken";
    stateColor [53] = "Lofar_broken_went";
    stateColor [56] = "Lofar_broken_came";
    
  	stateName [0]  = "off";
    stateName [10] = "operational";
    stateName [20] = "maintenance";
    stateName [30] = "test";
    stateName [40] = "suspicious";
    stateName [43] = "suspicious_went";
    stateName [46] = "suspicious_came";
    stateName [50] = "broken";
    stateName [53] = "broken_went";
    stateName [56] = "broken_came";
    
  	stateNumber ["off"]             = 0;
    stateNumber ["operational"]     = 10;
    stateNumber ["maintenance"]     = 20;
    stateNumber ["test"]            = 30;
    stateNumber ["suspicious"]      = 40;
    stateNumber ["suspicious_went"] = 43;
    stateNumber ["suspicious_came"] = 46;
    stateNumber ["broken"]          = 50;
    stateNumber ["broken_went"]     = 53;
    stateNumber ["broken_came"]     = 56;
    
}


const int OFF              = 0;
const int OPERATIONAL      = 10;
const int MAINTENANCE      = 20;
const int TEST             = 30;
const int SUSPICIOUS       = 40;
const int SUSPICIOUS_WENT  = 43;
const int SUSPICIOUS_CAME  = 46;
const int BROKEN           = 50;
const int BROKEN_WENT      = 53;
const int BROKEN_CAME      = 56;



///////////////////////////////////////////////////////////////////////////
//
// Function getStateColor(stateNr) : colorName
//
// helper function to return the colorName based on a state
//
// Added 3-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
string getStateColor(int aState) {
	if (mappingHasKey(stateColor,aState)) {
    	return stateColor[aState];
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
//
// Added 3-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
string getStateName(int aState) {
 	if (mappingHasKey(stateName,aState)) {
    	return stateName[aState];
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
//
// Added 20-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
int getStateNumber(string aState) {
  	if (aState != "") {
    	if (mappingHasKey(stateNumber,aState)) {
      	return stateNumber[aState];
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
  	// check if the required datapoint for this view are accessible
  	if (dpExists(aDP+".status.state")) {
          if (dpConnect("updateSelfState",aDP + ".status.state", aDP + ".status.state:_online.._invalid")==-1) {
            setValue("selfState.light","backCol","Lofar_invalid");
          }          
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
  	if (dpExists(aDP+".status.childState")) {
          if (dpConnect("updateChildState",aDP + ".status.childState", aDP + ".status.childState:_online.._invalid") == -1) {
      	    setValue("childStateBorder","foreCol","Lofar_invalid");
          } 
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

