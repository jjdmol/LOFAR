// navFunct.ctl
//
//  Copyright (C) 2002-2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
// navFunct_acknowledgePanel                  : Returns acknowledge on a given action
// navFunct_bareDBName                        : Returns a DatabaseName without the : (if any)
// navFunct_CEPName2inputBuf                  : returns ionr from CEPname
// navFunct_checkEmailAddress                 : checks if an email address has the correct syntax
// navFunct_clearGlobalLists                  : clear all temporarily global hardware,observation and processes lists..
// navFunct_dpGetFullPathAsTypes              : Returns full dp path (maincu && station components) as dynstring)
// navFunct_dpGetLastElement                  : Returns last element from DP 
// navFunct_dpHasPanels                       : checkes if a given DP has loadable panels.
// navFunct_dpReachable                       : looks if the databpoint on a dist system is also reachable
// navFunct_dpStripLastElement                : Returns DP string without last element 
// navFunct_dynToString                       : Returns a dynArray as a , seperated string
// navFunct_fillHardwareLists                 : Fill g_StationList, g_CabinetList,g_SubrackList,g_RSPList,g_RCUList and g_TBBList
// navFunct_fillHardwareTree                  : Prepare the DP for HardwareTrees
// navFunct_fillObservationsList              : Fill g_observationList
// navFunct_fillObservationsTree              : Prepare the DP for ObservationTrees
// navFunct_fillPipelinesList                 : Fill g_pipelineList
// navFunct_fillPipelinesTree                 : Prepare the DP for PipelinesTrees
// navFunct_fillProcessesList                 : Fill g_processList
// navFunct_fillProcessesTree                 : Prepare the DP for ProcessTrees
// navFunct_fillStationLists                  : fill global lists with core/europe and remote stations
// navFunct_findFirstOne                      : Returns the number of a given array that is true for a certain range
// navFunct_formatInt                         : returns a string with the int preceeded by zeros
// navFunct_getArmFromStation                 : Returns the armposition code from a stationName
// navFunct_getDPFromTypePath                 : Returns Dpname derived from currentDatapoint,typeList and chosen type
// navFunct_getDynString                      : Returns a dynString from a dyn_dyn[index]
// navFunct_getGPUProcsForObservation         : Returns all the GPUProcs for a given observation
// navFunct_getHBABitmap                      : get the HBABitmap from a given observation on a given station
// navFunct_getInputBuffersForStation         : returns all the InputBuffers that are connected to a station
// navFunct_getLBABitmap                      : get the LBABitmap from a given observation on a given station
// navFunct_getLocusNodesForObservation       : returns all the LocusNOdes that are in use for an observation
// navFunct_getLogColor                       : returns the color that belongs to a log level
// navFunct_getLogLevel                       : returns the level from a logline
// navFunct_getPathLessOne                    : Returns path less last leaf/node
// navFunct_getReceiverBitmap                 : returns the stations receiverBitMap for a given observation
// navFunct_getRingFromStation                : Returns the ringName from a stationName
// navFunct_getStationFromDP                  : get the stationname out of a DP name (if any)
// navFunct_getStationInputForObservation     : Returns all stationInputs used for this observation
// navFunct_getWritersForObservation          : returns all the writers that are in use for an observation
// navFunct_giveFadedColor                    : returns faded color string between green and red depending on min,max and currentValue
// navFunct_handleUndockClick                 : undock window from navigator framework
// navFunct_hardware2Obs                      : Looks if a piece of hardware maps to an observation
// navFunct_hasAARTFAAC                       : checks if a given station belongs to the AARTFAAC stations
// navFunct_IONode2DPName                     : returns the DP name based on the ionode number.
// navFunct_isCoreStation                     : returns TRUE if the station is part of the Core stations
// navFunct_isHBA                             : returns true if the antenna is an International HBA antenna
// navFunct_isHBAZero                         : returns true if the antenna is a Core HBA Zero antenna
// navFunct_isHBAOne                          : returns true if the antenna is a Core HBA One antenna
// navFunct_isHBAInner                        : returns true if the antenna is a Remote HBA Inner antenna
// navFunct_isHBAOuter                        : returns true if the antenna is a Remote HBA Outer antenna
// navFunct_isInternationalStation            : returns TRUE if the station is part of the International stations
// navFunct_isLBA                             : returns true if the antenna is an International LBA antenna
// navFunct_isLBAInner                        : returns true if the antenna is a Core or Remote LBA Inner antenna
// navFunct_isLBAOuter                        : returns true if the antenna is a Core or Remote LBA Outer antenna
// navFunct_isRemoteStation                   : returns TRUE if the station is part of the Remote stations
// navFunct_isObservation                     : returns  true is a given observationnumber is an observation or false when it is a pipeline
// navFunct_listToDynString                   : puts [a,b,d] lists into dynstrings
// navFunct_locusNode2OSRack                  : Returns the OSRackNr for a given LocusNode
// navFunct_lofarDate2PVSSDate                : returns Lofardate Datestring 2000.11.19 [18:12:21[.888]] in PVSS format 2000.11.19 [18:12:21[.888]]
// navFunct_observationInPool                 : Look if a given observation is in a given pool (planned,active, finished)
// navFunct_observationNameToNumber           : Strips Observation from the name and returns the bare number
// navFunct_ObsToTemp                         : returns the temp observationname
// navFunct_printLastError                    : determines if there was an error, and if so prints the errorcode text and the error text
// navFunct_queryConnectObservations          : Queryconnect to keep track of all observations
// navFunct_receiver2Cabinet                  : Returns the CabinetNr for a RecieverNr
// navFunct_receiver2HBA                      : Returns the HBANr for a RecieverNr
// navFunct_receiver2RSP                      : Returns the RSPNr for a RecieverNr
// navFunct_receiver2Subrack                  : Returns the SubrackNr for a RecieverNr
// navFunct_RSP2Cabinet                       : Returns the CabinetNr for a given RSP
// navFunct_RSP2Subrack                       : Returns the SubrackNr for a given RSP
// navFunct_splitAction                       : Splits an actionstring into a dyn_string action + params
// navFunct_splitEvent                        : Splits an event string into the essentials
// navFunct_stationInObservation              : Look if a given station is used in an observation from a given pool (planned,active, finished)
// navFunct_stationNameToIONode               : returns the IONode belonging to a station
// navFunct_subrack2Cabinet                   : Returns the CabinetNr for a given Subrack
// navFunct_TBB2Cabinet                       : Returns the CabinetNr for a given TBB
// navFunct_TBB2Subrack                       : Returns the SubrackNr for a given TBB
// navFunct_TempToObs                         : returns the observationname from the temp
// navFunct_updateObservations                : Callback for the above query
// navFunct_waitObjectReady                   : Loops till object Ready or breaks out with error. 

#uses "GCFLogging.ctl"
#uses "GCFCommon.ctl"



global dyn_string oldActiveObservations;                        
global dyn_string oldPlannedObservations;                        
global dyn_string oldFinishedObservations;  
dyn_int HBAInnerCircle = makeDynInt(5,6,10,11,12,13,17,18,19,20,21,22,25,26,27,28,29,30,34,35,36,37,41,42);  // Telescopenrs in de Inner HBA (Remote only)
                     

// ****************************************
// Name : navFunct_splitEvent
// ****************************************
// Description:  
//   splits an event
//  
// Returns:
//   bool if succeeded
// ***************************************
bool navFunct_splitEvent(string event, string& aShape, string& anEvent) {

  if (event == "" || strtok(event,"|") < 0){
    return false;
  }
  
  
  dyn_string aStr = strsplit(event,"|");
  
  aShape     = aStr[1];
  anEvent    = aStr[2];
  return true;
}

// ****************************************
// Name : navFunct_splitAction
// ****************************************
// Description:  
//   Splits action string
//     into a dyn_string containing the action and all params
//     an actionString consists out of : seperated items. The first item is the action
//     all next (if any) consist out of the params needed for the action.
//
// Returns:
//   bool if succeeded
// ***************************************
bool navFunct_splitAction(string action,dyn_string& actionString) {

  LOG_DEBUG("navFunct.ctl:navFunct_splitAction|entered with Action: "+ action);
  if (action == "" ){
    return false;
  }
  
  actionString = strsplit(action,"|");

  return true;
}

// ****************************************
// Name : navFunct_QueryConnectObservations
// ****************************************
// Description:
//    Establish a query connect that gives us the current
//    status of all observations.
//    This information is needed to update the 'Planned', 'Running' and 'Finished'
//    observation tables
//
// Returns:
//    None
// ***************************************


void navFunct_queryConnectObservations()
{
  g_observations[ "DP"          ]    = makeDynString();                    
  g_observations[ "NAME"        ]    = makeDynString();
  g_observations[ "STATIONLIST" ]    = makeDynString();
  g_observations[ "SCHEDULE" ]       = makeDynString();
  if (dpExists(MainDBName+"LOFAR_PermSW_MACScheduler.activeObservations")) {
    oldPlannedObservations=makeDynString();
    oldFinishedObservations=makeDynString();
    oldActiveObservations=makeDynString();
    if (dpConnect("navFunct_updateObservations",true,MainDBName+"LOFAR_PermSW_MACScheduler.activeObservations",
                                                     MainDBName+"LOFAR_PermSW_MACScheduler.plannedObservations",
                                                     MainDBName+"LOFAR_PermSW_MACScheduler.finishedObservations") == -1) {
      LOG_ERROR( "navFunct.ctl:QueryConnectObservations|ERROR: Couldn't connect to MACScheduler!!! "  + getLastError() );
      if (g_initializing) {
        writeInitProcess("queryConnectObservationsFinished");
      }
    } 
  } else {
    if (!isStandalone()) LOG_ERROR( "navFunct.ctl:QueryConnectObservations|ERROR: MACScheduler points don't exist!!!");
    if (g_initializing) {
      writeInitProcess("queryConnectObservationsFinished");
    }

  }     
}

// ****************************************
// Name : avFunct_getReceiverBitmap
// ****************************************
// Description:
//    Search if on a given station a given observation has a receiverBitMap
//    This is used to determine what hardware is used for that observation on that station
//
// Returns:
//    The receiverbitMap 
// ***************************************
string navFunct_getReceiverBitmap(string db,string obsname) {
  string receiverBitmap;
  if (obsname == "") {
    return receiverBitmap;
  }


  string dp = claimManager_nameToRealName(obsname); 
  string aDP=navFunct_bareDBName(db)+":"+dpSubStr(dp,DPSUB_DP);

  if (dpExists(aDP+".receiverBitmap")) {
    dpGet(aDP+".receiverBitmap",receiverBitmap);
  }
  return receiverBitmap;
  
}

// ****************************************
// Name : avFunct_getHBABitmap
// ****************************************
// Description:
//    Search if on a given station a given observation has a HBABitMap
//    This is used to determine what hardware is used for that observation on that station
//
// Returns:
//    The HBABitMap 
// ***************************************
string navFunct_getHBABitmap(string db,string obsname) {
  string HBABitmap;
  if (obsname == "") {
    return HBABitmap;
  }


  string dp = claimManager_nameToRealName(obsname); 
  string aDP=navFunct_bareDBName(db)+":"+dpSubStr(dp,DPSUB_DP);

  if (dpExists(aDP+".HBABitmap")) {
    dpGet(aDP+".HBABitmap",HBABitmap);
  }
  return HBABitmap;
  
}

// ****************************************
// Name : avFunct_getLBABitmap
// ****************************************
// Description:
//    Search if on a given station a given observation has a LBABitMap
//    This is used to determine what hardware is used for that observation on that station
//
// Returns:
//    The LBABitMap 
// ***************************************
string navFunct_getLBABitmap(string db,string obsname) {
  string LBABitmap;
  if (obsname == "") {
    return LBABitmap;
  }


  string dp = claimManager_nameToRealName(obsname); 
  string aDP=navFunct_bareDBName(db)+":"+dpSubStr(dp,DPSUB_DP);

  if (dpExists(aDP+".LBABitmap")) {
    dpGet(aDP+".LBABitmap",LBABitmap);
  }
  return LBABitmap;
  
}


void navFunct_updateObservations(string dp1, dyn_string active,
                                 string dp2, dyn_string planned,
                                 string dp3, dyn_string finished) {
  dyn_string stationList;
  int iPos=1;

  
  bool update=false;
  
  if ((dynlen(active) != dynlen(oldActiveObservations)) ||
      (dynlen(planned) != dynlen(oldPlannedObservations)) ||
      (dynlen(finished) != dynlen(oldFinishedObservations))) {
    update = true;
  }
  if (!update) {  
    for (int i=1; i <= dynlen(active); i++) {
      if (dynContains(oldActiveObservations,active[i]) < 1) {
        update = true;
        break;
      }
    }
  }
  if (!update) {
    for (int i=1; i <= dynlen(planned); i++) {
      if (dynContains(oldPlannedObservations,planned[i]) < 1) {
        update = true;
        break;
      }
    }
  }
  if (!update) {
    for (int i=1; i <= dynlen(finished); i++) {
      if (dynContains(oldFinishedObservations,finished[i]) < 1) {
        update = true;
        break;
      }
    }
  }
  if (!update) {
    if (g_initializing) {
      writeInitProcess("queryConnectObservationsFinished");
    }
    return;
  }
  oldPlannedObservations = planned;
  oldActiveObservations = active;
  oldFinishedObservations = finished;
  
  LOG_DEBUG("navFunct.ctl:navFunct_updateObservations|triggered.....");

  // Clear mapping
  mapping observationsNew;
  observationsNew[ "DP"          ]    = makeDynString();                    
  observationsNew[ "NAME"        ]    = makeDynString();
  observationsNew[ "STATIONLIST" ]    = makeDynString();
  observationsNew[ "SCHEDULE" ]       = makeDynString();
  
  for (int i = 1; i<= dynlen(active); i++) {
    string dp = claimManager_nameToRealName("LOFAR_ObsSW_"+active[i]);
    if (dp != "") {
      iPos=dynAppend(observationsNew[ "DP"          ] , dp);
      dpGet(dp+".stationList",stationList);
      
      observationsNew[ "NAME"           ][iPos]  = "LOFAR_ObsSW_"+active[i];
      observationsNew[ "STATIONLIST"    ][iPos]  = stationList;
      observationsNew[ "SCHEDULE"       ][iPos]  = "active";
    }      
  }

  for (int i = 1; i<= dynlen(planned); i++) {
    string dp = claimManager_nameToRealName("LOFAR_ObsSW_"+planned[i]);
    if (dp != "") {
      iPos=dynAppend(observationsNew[ "DP"          ] , dp);
      dpGet(dp+".stationList",stationList);
      
      observationsNew[ "NAME"           ][iPos]  = "LOFAR_ObsSW_"+planned[i];
      observationsNew[ "STATIONLIST"    ][iPos]  = stationList;
      observationsNew[ "SCHEDULE"       ][iPos]  = "planned";
    }      
  }

  for (int i = 1; i<= dynlen(finished); i++) {
    string dp = claimManager_nameToRealName("LOFAR_ObsSW_"+finished[i]);
    if (dp != "") {
      iPos=dynAppend(observationsNew[ "DP"          ] , dp);
      dpGet(dp+".stationList",stationList);
      
      observationsNew[ "NAME"           ][iPos]  = "LOFAR_ObsSW_"+finished[i];
      observationsNew[ "STATIONLIST"    ][iPos]  = stationList;
      observationsNew[ "SCHEDULE"       ][iPos]  = "finished";
    }      
  }

  // all gathering done, set the global mapping;  
  g_observations=observationsNew;


  // check if tabCtrl has a Panel loaded (indicating init has passed and panels are loaded)
  // in that case a Navigator event should be issued after a change in MACScheduler observations, 
  // so the involved objects can update themselves.
  
  if (tabCtrlHasPanel) {
    string eventDp     = DPNAME_NAVIGATOR + g_navigatorID + ".fw_viewBox.event";
    string selectionDp = DPNAME_NAVIGATOR + g_navigatorID + ".fw_viewBox.selection";
    string event="navigator|Reload";
    string selection = "navFunct_updateObservations.ctl";
    
    if (dpExists(eventDp) && dpExists(selectionDp)) {
       dpSet(eventDp,event,selectionDp,selection);
    } else {
      LOG_ERROR("navFunct_updateObservations.ctl:navPanel_setEvent| "+eventDp +" or " +selectionDp + " Does not exist yet");     
    }
  }
  
  if (g_initializing) {
      writeInitProcess("queryConnectObservationsFinished");
  }

}

//*******************************************
// Name: Function navFunct_getRingFromStation
// *******************************************
// 
// Description:
//   Will return the ringName based upon the stationName. 
//   for now (CS1-20 fase) it will return Core, later something smart has to be
//   done to give the other ringnames correctly.
//   It is needed to get the correct datapoints in constructions like:
//   LOFAR_PermSW_Core_CS011.state when you get a CS011:LOFAR_PermSW.state
//   change.
//
// Returns:
//    the name of the ring
// *******************************************
string navFunct_getRingFromStation(string stationName) {

  	string ringName="";
  	if (substr(stationName,0,2) == "CS") {
    	  ringName="Core";
  	} else if (substr(stationName,0,2) == "RS") {
    	  ringName="Remote";
        } else {
          ringName="Europe";
        }
  	return ringName;
}

//*******************************************
// Name: Function navFunct_getArmFromStation
// *******************************************
// 
// Description:
//   Will return the armName based upon the stationName. 
//
// Returns:
//    the name of the arm
// *******************************************
string navFunct_getArmFromStation(string stationName) {

  	int armName="";
  	if (substr(stationName,0,2) == "RS") {
    	  armName=substr(stationName,2,1);
  	}
  	return armName;
}
// *******************************************
// Name : showMapping
// *******************************************
// Description:
//    Prints all key value pairs in the mapping
//
// Returns:
//    None
// *******************************************
void showMapping(mapping aM,string name) {
  if (g_logLevel > LOGLEVEL_DEBUG ) return;
  LOG_DEBUG( "navFunct.ctl:showMapping|Local mapping "+name +" contains now: " );
  for (int i = 1; i <= mappinglen(aM); i++) { 
    LOG_DEBUG("navFunct.ctl:showMapping|mappingGetKey", i, " = "+mappingGetKey(aM, i));  
    LOG_DEBUG("  mappingGetValue", i, " = "+mappingGetValue(aM, i));
  }
}

// *******************************************
// Name : showDynArray
// *******************************************
// Description:
//    Prints all values in a dyn_anytype
//
// Returns:
//    None
// *******************************************
void showDynArray(dyn_anytype anArray,string name) {
  DebugN( "navFunct.ctl:showDynArray|array "+name +" contains now: " );
  for (int i = 1; i <= dynlen(anArray); i++) { 
  	DebugN("navFunct.ctl:showDynArray|", i, " = "+anArray[i]);  
  }
}

// ****************************************
// Name : navFunct_receiver2HBA
// ****************************************
// Description:
//    Returns the HBANr to which a receiver is connected 
//
// Returns:
//    The HBANr
// ***************************************

int navFunct_receiver2HBA(int receiverNr) {
  return floor(receiverNr/2);
}


// ****************************************
// Name : navFunct_receiver2Cabinet
// ****************************************
// Description:
//    Returns the cabinetNr to which a receiver is connected 
//
// Returns:
//    The cabinetnr
// ***************************************

int navFunct_receiver2Cabinet(int receiverNr) {
  return floor(receiverNr/64);
}

// ****************************************
// Name : navFunct_locusNode2OSRack
// ****************************************
// Description:
//    Returns the OSRackNr to which a LocusNode is connected 
//
// Returns:
//    The OSRacknr
// ***************************************

int navFunct_locusNode2OSRack(int nodeNr) {
  // each rack has 12 locusnodes except rack 4, that rack has only 
  // 4 nodes.  Locusnodes start counting at 001.
  nodeNr-=1;
  if (nodeNr >= 53) nodeNr+=8;
  int i = floor(nodeNr/12);
  return i;
}


// ****************************************
// Name : navFunct_receiver2Subrack
// ****************************************
// Description:
//    Returns the subrackNr to which a receiver is connected 
//
// Returns:
//    The subracknr
// ***************************************

int navFunct_receiver2Subrack(int receiverNr) {
  return floor(receiverNr/32);
}

// ****************************************
// Name : navFunct_receiver2RSP
// ****************************************
// Description:
//    Returns the RSPNr to which a receiver is connected 
//
// Returns:
//    The RSPnr
// ***************************************
int navFunct_receiver2RSP(int receiverNr) {
  return floor(receiverNr/8);
}

// ****************************************
// Name : navFunct_receiver2TBB
// ****************************************
// Description:
//    Returns the TBBNr to which a receiver is connected 
//
// Returns:
//    The TBBnr
// ***************************************
int navFunct_receiver2TBB(int receiverNr) {
  return floor(receiverNr/16);
}

// ****************************************
// Name : navFunct_RSP2Cabinet
// ****************************************
// Description:
//    Returns the cabinetNr to which a RSPBoard is connected 
//
// Returns:
//    The cabinetnr
// ***************************************

int navFunct_RSP2Cabinet(int rspNr) {
  return floor(rspNr/8);
}

// ****************************************
// Name : navFunct_RSP2Subrack
// ****************************************
// Description:
//    Returns the subrackNr to which a RSPBoard is connected 
//
// Returns:
//    The Subracknr
// ***************************************

int navFunct_RSP2Subrack(int rspNr) {
  return floor(rspNr/4);
}

// ****************************************
// Name : navFunct_TBB2Cabinet
// ****************************************
// Description:
//    Returns the cabinetNr to which a TBBoard is connected 
//
// Returns:
//    The cabinetnr
// ***************************************

int navFunct_TBB2Cabinet(int tbbNr) {
  return floor(tbbNr/4);
}

// ****************************************
// Name : navFunct_TBB2Subrack
// ****************************************
// Description:
//    Returns the subrackNr to which a TBBoard is connected 
//
// Returns:
//    The Subracknr
// ***************************************

int navFunct_TBB2Subrack(int tbbNr) {
  return floor(tbbNr/2);
}

// ****************************************
// Name : navFunct_subrack2Cabinet
// ****************************************
// Description:
//    Returns the cabinetNr to which a subrack is connected 
//
// Returns:
//    The cabinetnr
// ***************************************

int navFunct_subrack2Cabinet(int subrackNr) {
  return floor(subrackNr/2);
}

// ****************************************
// Name : navFunct_dpStripLastElement
// ****************************************
// Description:
//    Strip last element from given dp string
// Returns:  
//    the given dp without the last . or _ 
//    split element
// ***************************************

string navFunct_dpStripLastElement(string dp) {
  // first check if dp contains a .
  string ret="";
  if (strtok(dp,".") > -1) {
    dyn_string dpAr= strsplit(dp,".");
    if (dynlen(dpAr) > 0) {
      ret=dp;
      strreplace(ret,"."+dpAr[dynlen(dpAr)],"");
    }
  } else if (strtok(dp,"_") > -1) {
    dyn_string dpAr= strsplit(dp,"_");
    if (dynlen(dpAr) > 0) {
      ret=dp;
      strreplace(ret,"_"+dpAr[dynlen(dpAr)],"");
    }
  }
  return ret;
}

// ****************************************
// Name : navFunct_dpGetLastElement
// ****************************************
// Description:
//    get last element from given dp string
// Returns:  
//    the last . or _ split element
//
// ***************************************

string navFunct_dpGetLastElement(string dp) {
  // first check if dp contains a .
  string ret=dp;
  if (strtok(dp,".") > -1) {
    dyn_string dpAr= strsplit(dp,".");
    ret = dpAr[dynlen(dpAr)];
  } else if (strtok(dp,"_") > -1) {
    dyn_string dpAr= strsplit(dp,"_");
    ret = dpAr[dynlen(dpAr)];
  }
  return ret;
}
// ****************************************
// Name : navFunct_dpGetFullPathAsTypes
// ****************************************
// Description:
//    Determines full path (as seen from Maincu start) and returns
//    a dynstring with the typesinvolved
// Returns:  
//    Returns dynstring filles with the dpTypes involved)
// ****************************************
dyn_string navFunct_dpGetFullPathAsTypes(string aDp){
  
  dyn_string typePath;
  if (aDp == "") {
    return  typePath;
  }

  // strip dp into system and bare dp
  string systemName = dpSubStr(aDp,DPSUB_SYS);
  if (systemName == "") {
    systemName=getSystemName();
  }
  string dp         = dpSubStr(aDp,DPSUB_DP);
  
  // if system is in a station we need to give the maincu path also first.
  // station:                          MainCU
  // stnLOFAR                          LOFAR
  // stnLOFAR_stnPIC                   LOFAR_PIC_Core_LOFAR_
  // stnLOFAR_stnPermSW                LOFAR_PermSW
  // stnLOFAR_stnObsSW                 LOFAR_ObsSW
  // stnLOFAR_stnObsSW_stnObservation  LOFAR_ObsSW_Observation
  // CEPLOFAR                          LOFAR
  // CEPLOFAR_CEPPIC                   LOFAR_PIC
  // CEPLOFAR_CEPPermSW                LOFAR_PermSW
  // CEPLOFAR_CEPObsSW                 LOFAR_ObsSW
  // CEPLOFAR_CEPObsSW_CEPObservation  LOFAR_ObsSW_Observation
  // 
  int index=0;
  if (systemName != MainDBName) {
    typePath[++index] = dpTypeName("LOFAR");
    if (dp=="LOFAR"){
      typePath[++index] = dpTypeName("LOFAR_PIC");
      typePath[++index] = dpTypeName("LOFAR_PIC_"+navFunct_getRingFromStation(getSystemName()));
    }
    if (strpos(dp,"PIC") >=0) {
      typePath[++index] = dpTypeName("LOFAR_PIC");
      typePath[++index] = dpTypeName("LOFAR_PIC_"+navFunct_getRingFromStation(getSystemName()));
    }
    
    if (strpos(dp,"PermSW") >=0) {
      typePath[++index] = dpTypeName("LOFAR_PermSW");
    }
    if (strpos(dp,"ObsSW") >=0) {
      typePath[++index] = dpTypeName("LOFAR_ObsSW");
    }        
    if (strpos(dp,"Observation") >=0) {
      typePath[++index] = dpTypeName("LOFAR_ObsSW_Observation");
    }

  }        
      
      
  
  dyn_string splitted = strsplit(dp,"_");
  string start;  
  for (int i=1; i <= dynlen(splitted); i++)  {
    start+=splitted[i];
    typePath[i+index] = dpTypeName(systemName+start);
    start+="_";
    
  }
  
  return typePath;
}

// ****************************************
// Name : navFunct_getDPFromTypePath
// ****************************************
// Description:
//    Determines Dpname derived from currentDatapoint,typeList 
//    and chosen type
// Returns:  
//    Returns Dpname 
// ****************************************
string navFunct_getDPFromTypePath(dyn_string typeList,int choice) {
  LOG_DEBUG("navFunct.ctl:navFunct_getDPFromTypePath|typeList: "+ typeList+" choice: "+choice); 
  if (dynlen(typeList) == "" || choice <=0) {
    return  g_currentDatapoint;
  }

  
  LOG_DEBUG("CurDP: "+g_currentDatapoint);
  // strip dp into system and bare dp
  string systemName = dpSubStr(g_currentDatapoint,DPSUB_SYS);
  if (systemName == "") {
    systemName=getSystemName();
  }
  string dp         = dpSubStr(g_currentDatapoint,DPSUB_DP);  
  string newDatapoint=systemName;
  
  dyn_string splitted = strsplit(dp,"_");
  
 
  // when we are in the Main database the new datapoint is the old datatpoint upto and including the choice.
  if (systemName == getSystemName() ) {
    for (int i=1; i<= choice; i++) {
      if (i>1) {
        newDatapoint+="_";
      }
      newDatapoint +=splitted[i];
    }
  } else if (systemName == CEPDBName) {
    //determine breakpoint in typeList
    int cepLofarIdx = dynContains(typeList,"CEPLOFAR");
    // check if backwards will leave CEP
    if (choice >= cepLofarIdx) { // no it won't leave CEP
      for (int i=1; i<= choice-cepLofarIdx+1; i++) {
        if (i>1) {
          newDatapoint+="_";
        }
        newDatapoint +=splitted[i];
      }
    } else {   //   yes we will jump back to maincu level
      if (ACTIVE_TAB == "Hardware") {
        newDatapoint=MainDBName+"LOFAR_PIC_Remote";
      } else if (ACTIVE_TAB == "Observations") {
        newDatapoint=MainDBName+"LOFAR_ObsSW";
      } else if (ACTIVE_TAB == "Processes") {
        newDatapoint=MainDBName+"LOFAR_PermSW";
      }
    }
  } else {
    //determine breakpoint in typeList
    int stnLofarIdx = dynContains(typeList,"StnLOFAR");
    // check if backwards will leave the station.
    if (choice >= stnLofarIdx) { // no it won't leave the station
      for (int i=1; i<= choice-stnLofarIdx+1; i++) {
        if (i>1) {
          newDatapoint+="_";
        }
        newDatapoint +=splitted[i];
      }
    } else {   //   yes we will jump back to maincu level
      if (ACTIVE_TAB == "Hardware") {
        newDatapoint=MainDBName+"LOFAR_PIC_Europe";
      } else if (ACTIVE_TAB == "Observations") {
        newDatapoint=MainDBName+"LOFAR_ObsSW";
      } else if (ACTIVE_TAB == "Processes") {
        newDatapoint=MainDBName+"LOFAR_PermSW";
      }
    }
  }    
  return newDatapoint;
  
  
}

// ****************************************
// Name : navFunct_dynToString
// ****************************************
// Description:
//    Make a string from a dynString
// Returns:  
//    Returns a string containing the , seperated dynString
// ****************************************
string navFunct_dynToString (dyn_anytype aDynArray) {
  string aString="";
  for (int i = 1 ; i<= dynlen(aDynArray) ; i++) {
    if(i>1) {
      aString += ",";
    }
    aString += aDynArray[i];
  }
  return aString;
}

// ****************************************
// Name : navFunct_getDynString
// ****************************************
// Description:
//    Make a dyn_string from a given index at a dyn_dyn_anytype
// Returns:  
//    Returns a dynString
// ****************************************
dyn_string navFunct_getDynString(dyn_dyn_anytype tab, int start,int idx) {
  dyn_string aS;
  for (int i=start; i<= dynlen(tab); i++) {
    dynAppend(aS,tab[i][idx]);
  }
  return aS;
}


// ****************************************
// Name : navFunct_bareDBName
// ****************************************
// Description:
//    Returns a DatabaseName without the : (if any)
//
// 
// Returns:  
//    Returns a string containing the name
// ****************************************
string navFunct_bareDBName(string aDBName) {
  strreplace(aDBName,":","");
  return aDBName;
}


// ****************************************
// Name : navFunct_findFirstOne
// ****************************************
// Description:
//   finds the first position of 1 in the receiverleist 
//   starting at start index. 
// 
// Returns:  
//    Returns the position of the 1st one found in the array
// ****************************************
int navFunct_findFirstOne(string receiverList, int start) {

  if ( receiverList == "" ) {
    return -1;
  }
  
  // make a substring from the startindex onwards
  string aS= substr(receiverList,start);
  
  // find the first occurance of "1"
  int pos = strpos(aS,"1");

  // return startIndex + found pos
  if (pos < 0) {
    return pos;
  } else {
    return (start+pos);
  }
}

// ****************************************
// Name : navFunct_acknowledgePanel 
// ****************************************
// Description:
//    Acknowledge an action
//
// 
// Returns:  
//    Returns a bool
// ****************************************
bool navFunct_acknowledgePanel(string text) {
  dyn_float dreturnf;   // Variables for the return values
 
  dyn_string dreturns;
  bool  retVal=false;

  
  ChildPanelOnReturn("vision/MessageInfo","Confirm",makeDynString("$1:"+text,"$2:Yes","$3:No"),10,10,dreturnf, dreturns);
 
  // The code below is executed first when the child panel is closed
  if (dreturns == "true") retVal=true;
  return retVal;
}

///////////////////////////////////////////////////////////////////////////
//Function navFunct_getPathLessOne
// 
// Returns the given path string less the last item. Paths can contain
// _ and . seperated items Like in:
// LOFAR_PIC_Cabinet0_Subrack0_RSPBoard0.AP0 (In fact the . seperated members
// are nested elements, but we need to tread them as a Path member since they
// can contain states and or childStates
//
//
///////////////////////////////////////////////////////////////////////////
string navFunct_getPathLessOne(string path) {
  
  string returnVal="";
  dyn_string aS;
  
  // look if there is a . in the pathname, 
  // if so strip the last one plus point and return the result
  // and we are done
  
  aS = strsplit(path,'.');
  if (dynlen(aS) > 1) {
  	returnVal = aS[1];
  	for (int i=2; i< dynlen(aS);i++) {
      returnVal += "."+aS[i];
    }
    return returnVal;
  }
  
  // if no . found then look if there is a _ in the pathname, 
  // if so strip the last one plus _ and return the result
  // and we are done
  
  aS = strsplit(path,'_');
  if (dynlen(aS) > 1) {
  	returnVal = aS[1];
  	for (int i=2; i< dynlen(aS);i++) {
      returnVal += "_"+aS[i];
    }
    return returnVal;
  }
  
  // otherwise return empty string
  
  return returnVal;
}

// ****************************************
// Name: navFunct_hardware2Obs   
// ****************************************
//     tries to determine if a certain piece of hardware resides 
//     in an observation
//           
// Returns:
//     true or false
// ****************************************
bool navFunct_hardware2Obs(string stationName, string observation, 
                           string objectName, string strData, int intData) {
  bool flag = false;
  

  if (strpos(observation,"LOFAR_ObsSW_") < 0) {
    observation = "LOFAR_ObsSW_"+observation;
  }

  
  // remove : from station name if there
  strreplace(stationName,":","");
  
  // check if Observation is available in list and get the stationList and receiverBitmap 
  int iPos = dynContains( g_observations[ "NAME"         ], observation );
  if (iPos <=0) {
    LOG_DEBUG("navFunct.ctl:navFunct_hardware2Obs|observation: "+ observation+" not in g_observations: "+g_observations["NAME"]);     
    return flag;
  }
  
  
  dyn_string obsStations = navFunct_listToDynString(g_observations[ "STATIONLIST"    ][iPos]);
  string receiverBitmap = navFunct_getReceiverBitmap(stationName,observation);
  string HBABitmap = navFunct_getHBABitmap(stationName,observation);
  string LBABitmap = navFunct_getLBABitmap(stationName,observation);  
  

  
  // expand stationList with virtual groups (Core, Remote & Europe), so they can be highlighted when available
  bool coreFound  = false;
  bool remoteFound = false;
  bool europeFound = false;
  for (int i=1; i<= dynlen(obsStations); i++) {
    if (!coreFound && navFunct_getRingFromStation(obsStations[i])=="Core") {
      coreFound=true;
    } else if (!remoteFound && navFunct_getRingFromStation(obsStations[i])=="Remote") {
      remoteFound=true;
    } else if (!europeFound && navFunct_getRingFromStation(obsStations[i])=="Europe") {
      europeFound=true;
    } 
    if (coreFound && remoteFound && europeFound) {
      break;
    }
  }
  
  if (coreFound) {
    dynAppend(obsStations,"Core");
  }
  if (remoteFound) {
    dynAppend(obsStations,"Remote");
  }
  if (europeFound) {
    dynAppend(obsStations,"Europe");
  }


  // if station is not in stationList return false
  if (!dynContains(obsStations,stationName)) {
    return flag;
  } else {
    
 
    if (objectName == "Station") {
      flag = true;
    } else if (objectName == "Cabinet") {
      int i = navFunct_findFirstOne(receiverBitmap,intData*64);
      if (i >= 0 && i < (intData+1)*64) {
        flag = true;
      }  
    } else if (objectName == "Subrack") {
      int i = navFunct_findFirstOne(receiverBitmap,intData*32);
      if (i >= 0 && i < (intData+1)*32) {
        flag = true;
      }  
    } else if (objectName == "RSP" || objectName == "RSPBoard") {
      int i = navFunct_findFirstOne(receiverBitmap,intData*8);
      if (i >= 0 && i < (intData+1)*8) {
        flag = true;
      }  
    } else if (objectName == "TBB" || objectName == "TBBoard") {
      int i = navFunct_findFirstOne(receiverBitmap,intData*16);
      if (i >= 0 && i < (intData+1)*16) {
        flag = true;
      } 
    } else if (objectName == "RCU") {
      if (receiverBitmap != "" && receiverBitmap[intData] == "1") {
        flag = true;
      }
    } else if (objectName == "HBA") {
      if (HBABitmap != "" && HBABitmap[intData] == "1") {
        flag = true;
      }        
    } else if (objectName == "LBA") {
      if (LBABitmap != "" && LBABitmap[intData] == "1") {
        flag = true;
      }        
    } else if (objectName == "SPU") {
      flag = true;
    } else if (objectName == "Clock") {
      flag = true;
    } else {
      LOG_DEBUG("navFunct.ctl:navFunct_hardware2Obs|ERROR, Unknow hardware searched: "+ objectName);
    }
  }
  return flag;
}

// ****************************************
// Name: navFunct_fillHardwareLists   
// ****************************************
//     Fill Hardware lists based on Observations/Pipelines or processes, 
//     depending on what list is filled by a panel
//     also fill the db Point with the new tree          
// ****************************************
void navFunct_fillHardwareLists() {
  LOG_DEBUG("navFunct.ctl:navFunct_fillHardwareLists| Entered");     
  LOG_DEBUG("navFunct.ctl:navFunct_fillHardwareLists| g_observationsList: "+g_observationsList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillHardwareLists| g_pipelinesList: "+g_pipelinesList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillHardwareLists| g_processesList: "+g_processesList);     

  dynClear(strHighlight);
  dynClear(highlight);
  // fill hardware based on available observations
  if (dynlen(g_observationsList) > 0) {
    for (int i=1; i<= dynlen(g_observationsList); i++) {
      // get stationlist for this observation
      string obsName = "LOFAR_ObsSW_"+g_observationsList[i];
      int iPos = dynContains( g_observations[ "NAME"         ], obsName);
      if (iPos <=0) {
        LOG_DEBUG("navFunct.ctl:navFunct_fillHardwareLists|ERROR: Observation "+ obsName+" not in g_observations.");     
        continue;
      }
  
      dyn_string obsStations = navFunct_listToDynString(g_observations[ "STATIONLIST"    ][iPos]);
    
      // fill (unique) results in g_stationList
      for (int j=1; j<= dynlen(obsStations); j++) {
        if (!dynContains(g_stationList,obsStations[j])) {
          dynAppend(g_stationList,obsStations[j]);
        }
      }
    }

    // or based on available pipelines
  } else if (dynlen(g_pipelinesList) > 0) {
    for (int i=1; i<= dynlen(g_pipelinesList); i++) {

     // To be done
      
    }
    
  // or based on processes

  } else {
    // loop over all claimed processes in the global list and
    // check if the database is not the MainCU
    // if it is not the mainCU, and it is not allready in the global hardware lists, add it. 
    for (int i=1; i <= dynlen(g_processesList);i++) {
      string database=navFunct_bareDBName(dpSubStr(g_processesList[i],DPSUB_SYS));
      if (database != navFunct_bareDBName(MainDBName) &&
          !dynContains(g_stationList,database)) {
        dynAppend(g_stationList,database);
      }
    }
  }

  dynSortAsc(g_stationList);

  // now prepare the hardwareTree    
  navFunct_fillHardwareTree();  
}  

// ****************************************
// Name: navFunct_fillObservationList   
// ****************************************
//     Fill Observation lists based on hardware or processes, 
//     depending on what list is filled by a panel
//     also fill the db Point with the new tree          
// ****************************************

void navFunct_fillObservationsList() {
  LOG_DEBUG("navFunct.ctl:navFunct_fillObservationsLists| Entered");     
  LOG_DEBUG("navFunct.ctl:navFunct_fillObservationsLists| g_stationsList: "+g_stationList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillObservationsLists| g_processesList: "+g_processesList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillObservationsLists| g_pipelinesList: "+g_pipelinesList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillObservationsLists| g_observationsList: "+g_observationsList);     
  dynClear(strHighlight);
  dynClear(highlight);
  

  // check if processList is filled
  if (dynlen(g_processesList) > 0) {
    // loop over all claimed processes in the global list and
    // check if the dp contains an observation
    // if it is an observation, and it is not allready in the global observation lists, add it. 
    for (int i=1; i <= dynlen(g_processesList);i++) {
      // check if the dptype is of type (Stn)Observation
      string process = navFunct_getPathLessOne(g_processesList[i]);
      // check if it is an existing databasePoint
      if (dpExists(process) ) {
        if (dpTypeName(process) == "Observation" || dpTypeName(process) == "StnObservation" || dpTypeName(process) == "CEPObservation") {
          // get the real observation name
          int iPos = dynContains(g_observations["DP"],dpSubStr(process,DPSUB_DP));
          if (iPos > 0) {
            string observation = g_observations["NAME"][iPos];
            strreplace(observation,"LOFAR_ObsSW_","");
          
            if (!dynContains(g_observationsList,observation)) {
              dynAppend(g_observationsList,observation);
            }
          }
        }
      }
    }
  // otherwise hardware  
  } else {

    // we need to expand stationgroup terms like Europe,Remote and Core so that these stations are also involved
    // in the process of looking what observations are valid.      
    dyn_string stationList;
    dyn_string aTemp;

    for (int k=1;k<= dynlen(g_stationList); k++) {
      if (g_stationList[k] == "Europe") {
        aTemp=europeStations;
        dynAppend(stationList,aTemp);
      } else if (g_stationList[k] == "Remote") {
        aTemp=remoteStations;
        dynAppend(stationList,aTemp);
      } else if (g_stationList[k] == "Core") {
        aTemp=coreStations;
        dynAppend(stationList,aTemp);
      } else {
        dynAppend(stationList,g_stationList[k]);
      }
    }
    
     
    
    // check all available observations
    for (int i = 1; i <= dynlen(g_observations["NAME"]); i++) {
      // only observations!!
      if (g_observations["STATIONLIST"][i] == "[]") continue;
      bool found=false;
      string shortObs=g_observations["NAME"][i];
      strreplace(shortObs,"LOFAR_ObsSW_","");
    
      // If we are have more entries in the station list we assume we are looking at a panel that has only stations
      // involved, so we  do not need to look at more hardware, in other cases we have to look if at least one piece
      // of each hardwareType also is needed for the observation to decide if it needs 2b in the list
      if ( dynlen(g_observations["NAME"]) < i) break;
      
      if (dynlen(stationList) > 1) {           
        // loop through stationList
        for (int j=1; j<= dynlen(stationList); j++) {
          // safe escape if map was updated while in this loop    
          if ( dynlen(g_observations["NAME"]) < i || dynlen(stationList) < j) break;
          //test if station is used in the observation
          if (navFunct_hardware2Obs(stationList[j], g_observations["NAME"][i],"Station",stationList[j],0)) {
            if (!dynContains(g_observationsList, shortObs)){
              dynAppend(g_observationsList,shortObs);
              found=true;
            }
          }
          if (found) break;
        }
      } else {
        string station="";
        if (dynlen(stationList) >=1) station = stationList[1];
        found = false;
        // check cabinets
        for (int c = 1; c<=dynlen(g_cabinetList); c++) {
          if (navFunct_hardware2Obs(station, g_observations["NAME"][i],"Cabinet","",g_cabinetList[c])) {
            // we found one involved cabinet, so obs can be included and we can skip the rest
            found = true;
          }
          if (found) break;
        }
        
        
        if (!found) {
          // check subracks
          for (int c = 1; c<=dynlen(g_subrackList); c++) {
            if (navFunct_hardware2Obs(station, g_observations["NAME"][i],"Subrack","",g_subrackList[c])) {
              // we found one involved subrack, so obs can be included and we can skip the rest
              found = true;
            }
            if (found) break;
          }
        }
        
        if (!found) {
          // check RSPBoards
          for (int c = 1; c<=dynlen(g_RSPList); c++) {
            if (navFunct_hardware2Obs(station, g_observations["NAME"][i],"RSPBoard","",g_RSPList[c])) {
              // we found one involved rspBoard, so obs can be included and we can skip the rest
              found = true;
            }
            if (found) break;
          }
        }

        if (!found) {
          // check TBBoards
          for (int c = 1; c<=dynlen(g_TBBList); c++) {
            if (navFunct_hardware2Obs(station, g_observations["NAME"][i],"TBBoard","",g_TBBList[c])) {
              // we found one involved TBBoard, so obs can be included and we can skip the rest
              found = true;
            }
            if (found) break;
          }
        }

        if (!found) {
          // check RCU
          for (int c = 1; c<=dynlen(g_RCUList); c++) {
            if (navFunct_hardware2Obs(station, g_observations["NAME"][i],"RCU","",g_RCUList[c])) {
              // we found one involved RCU, so obs can be included and we can skip the rest
              found = true;
            }
            if (found) break;
          }
        }

        if (!found) {
          // check HBAAntenna
          for (int c = 1; c<=dynlen(g_HBAList); c++) {
            if (navFunct_hardware2Obs(station, g_observations["NAME"][i],"HBA","",g_HBAList[c])) {
              // we found one involved HBA, so obs can be included and we can skip the rest
              found = true;
            }
            if (found) break;
          }
        }

        if (!found) {
          // check LBAAntenna
          for (int c = 1; c<=dynlen(g_LBAList); c++) {
            if (navFunct_hardware2Obs(station, g_observations["NAME"][i],"LBA","",g_LBAList[c])) {
              // we found one involved LBA, so obs can be included and we can skip the rest
              found = true;
            }
            if (found) break;
          }
        }

        
        if (found && !dynContains(g_observationsList, shortObs)){
          dynAppend(g_observationsList,shortObs);
        }
      }      
    }
  }
  // now prepare the ObservationTree    
  navFunct_fillObservationsTree();  
}

// ****************************************
// Name: navFunct_fillPipelinesList   
// ****************************************
//     Fill Pipeline lists based on hardware or processes, 
//     depending on what list is filled by a panel
//     also fill the db Point with the new tree          
// ****************************************

void navFunct_fillPipelinesList() {
  LOG_DEBUG("navFunct.ctl:navFunct_fillPipelinesLists| Entered");     
  LOG_DEBUG("navFunct.ctl:navFunct_fillPipelinesLists| g_stationsList: "+g_stationList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillPipelinesLists| g_processesList: "+g_processesList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillPipelinesLists| g_pipelinesList: "+g_pipelinesList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillPipelinesLists| g_observationsList: "+g_observationsList);     
  dynClear(strHighlight);
  dynClear(highlight);
  // now prepare the ObservationTree    
  navFunct_fillPipelinesTree();  
}
  
// ****************************************
// Name: navFunct_fillProcessesList   
// ****************************************
//     Fill Processes lists based on hardware or observations, 
//     depending on what list is filled by a panel
//     also fill the db Point with the new tree          
// ****************************************

void navFunct_fillProcessesList() {
  dynClear(strHighlight);
  dynClear(highlight);
  LOG_DEBUG("navFunct.ctl:navFunct_fillProcesseLists| Entered");     
  LOG_DEBUG("navFunct.ctl:navFunct_fillProcesseLists| g_stationsList: "+g_stationList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillProcesseLists| g_processesList: "+g_processesList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillPipelinesLists| g_pipelinesList: "+g_pipelinesList);     
  LOG_DEBUG("navFunct.ctl:navFunct_fillProcesseLists| g_observationsList: "+g_observationsList);

  dynClear(strHighlight);
  dynClear(highlight);
  

  if (dynlen(g_stationList) > 0) {                      // else hardware based processes
    dyn_string stationList;
    dyn_string aTemp;

    // if the stationlist involves rings, we need to expand those rings to real stations
    for (int k=1;k<= dynlen(g_stationList); k++) {
      if (g_stationList[k] == "Europe") {
        aTemp=europeStations;
        dynAppend(stationList,aTemp);
      } else if (g_stationList[k] == "Remote") {
        aTemp=remoteStations;
        dynAppend(stationList,aTemp);
      } else if (g_stationList[k] == "Core") {
        aTemp=coreStations;
        dynAppend(stationList,aTemp);
      } else {
        dynAppend(stationList,g_stationList[k]);
      }
    }
    
    // For each type of "station" (MCU,CCU and stations) we need to gather the involved running processes, 
    // Observation software if any active observations that involve this "stations" and the permanent software
    //

    for (int k=1;k<= dynlen(stationList); k++) {
      int z;
      dyn_dyn_anytype tab;
      //PermSW + PermSW_Daemons
      if (navFunct_dpReachable(stationList[k]+":")) {
        dpQuery("SELECT '_original.._value' FROM 'LOFAR_PermSW_*.process.error' REMOTE '"+stationList[k]+"'", tab);
        LOG_TRACE("navFunct.ctl:navFunct_fillProcessesList|Found: "+ tab);
  
        dyn_string aDS=navFunct_getDynString(tab, 2,1);
        dynSortAsc(aDS);
  
        for(z=1;z<=dynlen(aDS);z++){
    
          // strip .status.state from CEP_result
          string aS = dpSubStr(aDS[z],DPSUB_SYS_DP);
          dynAppend(g_processesList,aS);
        }
      }
    }
  }
  // now prepare the ProcessesTree    
  navFunct_fillProcessesTree(); 
}

// ****************************************
// Name: navFunct_fillHardwareTree   
// ****************************************
//     Fill Hardware Tree based on available Hardware in the global
//     hardwareLists
//
// ****************************************
void navFunct_fillHardwareTree() {
  dyn_string result;
  string connectTo="";
  string lvl="";
  string station ="";
  string dp="";
  
  // add Stations
  if (dynlen(g_stationList) > 1 ) {
    for (int i = 1; i <= dynlen(g_stationList); i++) {
      // for ease of selection we use stationlike objects on the main google hardware panels, such
      // as CCU001 , Remote and Core. Since those are obviously no stations (== databases) we need to set another point to
      // jump to when doubleclicked
      if (g_stationList[i]+":" == CEPDBName) {
        dp = CEPDBName+"LOFAR_PIC";
      } else if (g_stationList[i] == "Core") {
        dp = MainDBName+"LOFAR_PIC_Core";
      } else if (g_stationList[i] == "Remote") {
        dp = MainDBName+"LOFAR_PIC_Remote";
      } else {        
        dp = g_stationList[i]+":LOFAR";
      }
      dynAppend(result,connectTo+","+g_stationList[i]+","+dp);
    }
    lvl="Station";
  } else if (dynlen(g_stationList) == 1) { 
    dp = g_stationList[1]+":LOFAR";
    dynAppend(result,connectTo+","+g_stationList[1]+","+dp);
    station = g_stationList[1];
    connectTo=dp;
      
    // Different selection for CEPDBName (CCU001) as for "normal" stations
    if (g_stationList[1]+":" == CEPDBName) {
      string baseConnect=connectTo;
      
      // add CobaltRacks
      if (dynlen(g_cobaltRackList) > 0) {
        for (int i = 1; i <= dynlen(g_cobaltRackList); i++) {
          dp = station+":LOFAR_PIC_Cobalt";
          dynAppend(result,baseConnect+",Cobalt"+","+dp);
        }
      }

      //add CobaltNodes
      if (dynlen(g_cobaltNodeList) > 0) {
        for (int i = 1; i <= dynlen(g_cobaltNodeList); i++) {
          connectTo = station+":LOFAR_PIC_Cobalt";
          dp = station+":LOFAR_PIC_Cobalt_CBT"+navFunct_formatInt(g_cobaltNodeList[i],999);
          dynAppend(result,connectTo+",CBT"+navFunct_formatInt(g_cobaltNodeList[i],999)+","+dp);
        }
      }

      // add OSRacks
      if (dynlen(g_OSRackList) > 0) {
        for (int i = 1; i <= dynlen(g_OSRackList); i++) {
          dp = station+":LOFAR_PIC_OSRack"+g_OSRackList[i];
          dynAppend(result,baseConnect+",OSRack"+g_OSRackList[i]+","+dp);
        }
        lvl="OSRack";
      }
      
      //add Locusnodes
      if (dynlen(g_locusNodeList) > 0) {
        for (int i = 1; i <= dynlen(g_locusNodeList); i++) {
          int osRackNr=navFunct_locusNode2OSRack(g_locusNodeList[i]);
          if (lvl == "OSRack") {
            connectTo = station+":LOFAR_PIC_OSRack"+osRackNr;
          }
          dp = station+":LOFAR_PIC_OSRack"+osRackNr+"_LocusNode"+g_locusNodeList[i];
          dynAppend(result,connectTo+",LocusNode"+g_locusNodeList[i]+","+dp);
        }
        lvl="LocusNode";
      }
      
      
    } else {
      // add Cabinets
      if (dynlen(g_cabinetList) > 0) {
        for (int i = 1; i <= dynlen(g_cabinetList); i++) {
          dp = station+":LOFAR_PIC_Cabinet"+g_cabinetList[i];
          dynAppend(result,connectTo+",Cabinet"+g_cabinetList[i]+","+dp);
        }
        lvl="Cabinet";
      }
  
      // add UriBoards
      if (dynlen(g_uriBoardList) > 0) {
        for (int i = 1; i <= dynlen(g_uriBoardList); i++) {
          int cabinetNr=navFunct_uriBoard2Cabinet(g_uriBoardList[i]);
          if (lvl == "Cabinet") {
            connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr;
          }
          dp = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_URIBoard"+g_uriBoardList[i];
          dynAppend(result,connectTo+",URIBoard"+g_uriBoardList[i]+","+dp);
        }
      }

      // add UniBoards (only one per station for now)
      if (dynlen(g_uniBoardList) > 0) {
        int cabinetNr=1;
        if (lvl == "Cabinet") {
          connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr;
        }
        dp = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_UniBoard";
        dynAppend(result,connectTo+",UniBoard,"+dp);
      }

      // add fpgas
      if (dynlen(g_FPGAList) > 0) {
        for (int i = 1; i <= dynlen(g_FPGAList); i++) {
          connectTo = station+":LOFAR_PIC_Cabinet1_UniBoard";
          dp = station+":LOFAR_PIC_Cabinet1_UniBoard_FPGA"+g_FPGAList[i];
          dynAppend(result,connectTo+",FPGA"+g_FPGAList[i]+","+dp);
        }
      }

      // add Subracks
      if (dynlen(g_subrackList) > 0) {
        for (int i = 1; i <= dynlen(g_subrackList); i++) {
          int cabinetNr=navFunct_subrack2Cabinet(g_subrackList[i]);
          if (lvl == "Cabinet") {
            connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr;
          }
          dp = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_Subrack"+g_subrackList[i];
          dynAppend(result,connectTo+",Subrack"+g_subrackList[i]+","+dp);
        }
        lvl="Subrack";
      }
  
      // add RSPBoards
      if (dynlen(g_RSPList) > 0) {
        for (int i = 1; i <= dynlen(g_RSPList); i++) {
          int cabinetNr=navFunct_RSP2Cabinet(g_RSPList[i]);
          int subrackNr=navFunct_RSP2Subrack(g_RSPList[i]);
          if (lvl == "Cabinet") {
            connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr;
          } else if (lvl == "Subrack") {
            connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_Subrack"+subrackNr;
          }
          dp = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_Subrack"+subrackNr+"_RSPBoard"+g_RSPList[i];
          dynAppend(result,connectTo+",RSPBoard"+g_RSPList[i]+","+dp);
        }
        lvl="RSPBoard";
      }
  
      // add TBBoards
      if (dynlen(g_TBBList) > 0) {
        for (int i = 1; i <= dynlen(g_TBBList); i++) {
          int cabinetNr=navFunct_TBB2Cabinet(g_TBBList[i]);
          int subrackNr=navFunct_TBB2Subrack(g_TBBList[i]);
          if (lvl == "Cabinet") {
            connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr;
          } else if (lvl == "Subrack") {
            connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_Subrack"+subrackNr;
          }
          dp = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_Subrack"+subrackNr+"_TBBoard"+g_TBBList[i];
          dynAppend(result,connectTo+",TBBoard"+g_TBBList[i]+","+dp);
        }
        lvl="RSPBoard";
      }
  
      // add RCUs
      if (dynlen(g_RCUList) > 0) {
        for (int i = 1; i <= dynlen(g_RCUList); i++) {
          int cabinetNr=navFunct_receiver2Cabinet(g_RCUList[i]);
          int subrackNr=navFunct_receiver2Subrack(g_RCUList[i]);
          int rspNr=navFunct_receiver2RSP(g_RCUList[i]);
          if (lvl == "Cabinet") {
            connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr;
          } else if (lvl == "Subrack") {
            connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_Subrack"+subrackNr;
          } else if (lvl == "RSPBoard") {
            connectTo = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_Subrack"+subrackNr+"_RSPBoard"+rspNr;
          }
          dp = station+":LOFAR_PIC_Cabinet"+cabinetNr+"_Subrack"+subrackNr+"_RSPBoard"+rspNr+"_RCU"+g_RCUList[i];
          dynAppend(result,connectTo+",RCU"+g_RCUList[i]+","+dp);
        }
      }
      
      // add HBAAntennas
      if (dynlen(g_HBAList) > 0) {
        for (int i = 1; i <= dynlen(g_HBAList); i++) {
          connectTo = station+":LOFAR";
          string extrah = "";
           if (g_HBAList[i] < 10) extrah = "0";          
          dp = station+":LOFAR_PIC_HBA"+extrah+g_HBAList[i];
          dynAppend(result,connectTo+",HBA"+extrah+g_HBAList[i]+","+dp);
        }
      }
      // add LBAAntennas
      if (dynlen(g_LBAList) > 0) {
        for (int i = 1; i <= dynlen(g_LBAList); i++) {
          connectTo = station+":LOFAR";
          string extrah = "";
           if (g_LBAList[i] < 100) extrah = "0";          
           if (g_LBAList[i] < 10) extrah = "00";          
          dp = station+":LOFAR_PIC_LBA"+extrah+g_LBAList[i];
          dynAppend(result,connectTo+",LBA"+extrah+g_LBAList[i]+","+dp);
        }
      }
    }
  }
  
  LOG_DEBUG("navFunct.ctl:navFunct_fillHardwareTree|result: "+ result);     
  
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".hardwareList",result);
  
}  

// ****************************************
// Name: navFunct_fillProcessesTree   
// ****************************************
//     Fill Processes Tree based on available Processes in the global
//     processesLists
//
// ****************************************
void navFunct_fillProcessesTree() {

  if (dynlen(g_processesList) <=0) return;
  
  dyn_string result;
  string lvl="";
  string station ="";
  string dp="";
  
  dynSortAsc(g_processesList);

  for (int i = 1; i <= dynlen(g_processesList); i++) {
    // Analyze the datapointpath and see if all elements are available to be able to connect to them
    dp = g_processesList[i];
    string db = navFunct_bareDBName(dpSubStr(dp,DPSUB_SYS));
    dp = dpSubStr(dp,DPSUB_DP);
    // find out if on PermSW or ObsSW level 
    if (strpos(dp,"PermSW_") >= 0) {
      lvl = "PermSW";
      dp =substr(dp,13);
    } else {
      lvl = "ObsSW";
      dp =substr(dp,12);
    }
    dyn_string pathList = strsplit(dp,"_");
    // now we have the db name and the path in seperateparts
    string connectTo = db+":LOFAR_"+lvl;
    if (!dynContains(result,","+db+","+db+":LOFAR_"+lvl)) { 
      dynAppend(result,","+db+","+db+":LOFAR_"+lvl);
    }
    string fullProcessPath=connectTo;
    for (int j=1; j <= dynlen(pathList); j++) {
      fullProcessPath+="_"+pathList[j];
      if (strpos(pathList[j],"TempObs") > -1) {
        // Observation found, get real name in stead of Tempname
        string observation = strsplit(claimManager_realNameToName("LOFAR_ObsSW_"+pathList[j]),"_")[3];
        pathList[j] = observation;
      }
      if (!dynContains(result,connectTo+","+pathList[j]+","+fullProcessPath)) {
       dynAppend(result,connectTo+","+pathList[j]+","+fullProcessPath);
      }
      connectTo=fullProcessPath;      
    }
  }
  
  LOG_DEBUG("navFunct.ctl:navFunct_fillProcessesTree|result: "+ result); 
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".processesList",result);  
}

// ****************************************
// Name: navFunct_fillObservationsTree   
// ****************************************
//     Fill Observations Tree based on available Observations in the global
//     hardwareLists
//
// ****************************************
void navFunct_fillObservationsTree() {
  dyn_string result;
    
    
  dyn_string result;
  dynAppend(result,",planned,planned");
  dynAppend(result,",active,active");
  dynAppend(result,",finished,finished");  
  
  //  loop over all involved observations
  for (int i = 1; i <= dynlen(g_observationsList); i++) {
    string obsName= "LOFAR_ObsSW_"+g_observationsList[i];
                    
    //check position in available observations
    int iPos = dynContains(g_observations["NAME"],obsName);
    if (iPos < 1) {
      LOG_DEBUG("navFunct.ctl:navFunct_fillObservationsTree|ERROR, couldn't find "+obsName+" in g_observations");
      continue;
    }
           
    
    string aS=g_observations["SCHEDULE"][iPos]+","+g_observationsList[i]+","+g_observations["DP"][iPos];
    if (!dynContains(result,aS)){
        dynAppend(result,aS);
    }
  }
  
  LOG_DEBUG("navFunct.ctl:navFunct_fillObservationsTree|result: "+ result);     
  
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".observationsList",result);  
}

// ****************************************
// Name: navFunct_fillObservationsTree   
// ****************************************
//     Fill Observations Tree based on available Observations in the global
//     hardwareLists
//
// ****************************************
void navFunct_fillPipelinesTree() {
  dyn_string result;
    
    
  dyn_string result;
  dynAppend(result,",planned,planned");
  dynAppend(result,",active,active");
  dynAppend(result,",finished,finished");  
  
  //  loop over all involved observations
  for (int i = 1; i <= dynlen(g_pipelinesList); i++) {
    string obsName= "LOFAR_ObsSW_"+g_pipelinesList[i];
                    
    //check position in available pipelines
    int iPos = dynContains(g_observations["NAME"],obsName);
    if (iPos < 1) {
      LOG_DEBUG("navFunct.ctl:navFunct_fillPipelinesTree|ERROR, couldn't find "+obsName+" in g_observations");
      continue;
    }
           
    
    string aS=g_observations["SCHEDULE"][iPos]+","+g_pipelinesList[i]+","+g_observations["DP"][iPos];
    if (!dynContains(result,aS)){
        dynAppend(result,aS);
    }
  }
  
  LOG_DEBUG("navFunct.ctl:navFunct_fillPipelinesTree|result: "+ result);     
  
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".pipelinesList",result);  
}

// ****************************************
// Name: navFunct_clearGlobalLists  
// ****************************************
//     Clear all global observations,hardware and processesLists
//
// ****************************************
void navFunct_clearGlobalLists() {
  // empty global listings
  dynClear(g_stationList);
  dynClear(g_cabinetList);
  dynClear(g_subrackList);
  dynClear(g_uriBoardList);
  dynClear(g_uniBoardList);
  dynClear(g_FPGAList);
  dynClear(g_RSPList);
  dynClear(g_TBBList);
  dynClear(g_RCUList);
  dynClear(g_HBAList);
  dynClear(g_LBAList);
  dynClear(g_OSRackList);
  dynClear(g_locusNodeList);
  dynClear(g_cobaltRackList);
  dynClear(g_cobaltNodeList);
  dynClear(g_cobaltNICList);

  dynClear(g_observationsList);
  dynClear(g_pipelinesList);
  dynClear(g_processesList);
}

// ****************************************
// Name: navFunct_listToDynString  
// ****************************************
//     puts [a,b,d] lists into dynstrings
//
// return:  a Dynstring with all the elements between , 
//          and after cutting away []
// ****************************************
dyn_string navFunct_listToDynString(string aS) {
  // check for []
  if (strpos(aS,"[") > -1) {
    aS=strltrim(aS,"[");
    aS=strrtrim(aS,"]");
  }
  return strsplit(aS,",");
}

// ****************************************
// Name: navFunct_fillStationLists  
// ****************************************
//     fills globals with available core/europe and remote stations
//
// ****************************************
void navFunct_fillStationLists() {
  coreStations = makeDynString("CS001","CS002","CS003","CS004","CS005","CS006","CS007",
                               "CS011","CS013","CS017",
                               "CS021","CS024","CS026","CS028",
                               "CS030","CS031","CS032",
                               "CS101","CS103",
                               "CS201",
                               "CS301","CS302",
                               "CS401",
                               "CS501");
//  coreStations = makeDynString("CS001","CS002","CS003","CS004","CS005","CS006","CS007",
//                               "CS011","CS012","CS013","CS014","CS016","CS017","CS018",
//                               "CS020","CS021","CS022","CS023","CS024","CS026","CS028",
//                               "CS030","CS031","CS032",
//                               "CS101","CS103",
//                               "CS201",
//                               "CS301","CS302",
//                               "CS401",
//                               "CS501");
  remoteStations = makeDynString("RS106",
                                 "RS205","RS208","RS210",
                                 "RS305","RS306","RS307","RS310",
                                 "RS406","RS407","RS409",
                                 "RS503","RS508","RS509");
//  remoteStations = makeDynString("RS104","RS106","RS107",
//                                 "RS205","RS206","RS207","RS208","RS210",
//                                 "RS306","RS307","RS308","RS309","RS310","RS311",
//                                 "RS404","RS406","RS407","RS408","RS409","RS410","RS411","RS412","RS413",
//                                 "RS503","RS506","RS507","RS508","RS509");
//  europeStations = makeDynString("DE601","DE602","DE603","DE604","DE605","DE609","FR606","PL610","PL611","PL612","SE607","UK608");
  europeStations = makeDynString("DE601","DE602","DE603","DE604","DE605","DE609","FR606","PL610","PL611","PL612","SE607","UK608");
  superTerpStations = makeDynString("CS002","CS003","CS004","CS005","CS006","CS007");
  cs0nnCoreStations = makeDynString("CS001",
                                    "CS011","CS013","CS017",
                                    "CS021","CS024","CS026","CS028",
                                    "CS030","CS031","CS032");
  csx01CoreStations = makeDynString("CS101","CS201","CS301","CS401","CS501","CS103","CS302");
  
}


// ****************************************
// Name: navFunct_getStationFromDP  
// ****************************************
//     tries to get the station name out 
//     of a given DP (like from DP's as:
//     LOFAR_PIC_Remote_RS002
//     it will return RS002:
//
//     Returns the station Names DB
// ****************************************
string navFunct_getStationFromDP(string aDPName) {
  string aS = dpSubStr(aDPName,DPSUB_DP);
  string station="";
  if (strpos(aS,"Core") > -1 ||
      strpos(aS,"Remote") > -1 ||
      strpos(aS,"Europe") > -1) {
    dyn_string aDS = strsplit(aS,"_");
    aS = aDS[dynlen(aDS)]+":";
  }
  return aS;
}

// ****************************************
// Name: navFunct_dpReachable 
// ****************************************
//  Tries to determine if a (dist) system is (still)
//  reachable.  This because if a dist system allready was
//  up, but goed down, a reload of a panel gives dpExists = true
//  and a dpConnect gives no erro, but just no callback. 
//  so there is no way to determine if a dist system is still reachable.
//
//     Returns true if system is still reachable
// ****************************************
bool navFunct_dpReachable(string aDP) {
  if (!dpExists(aDP)) return false;
  
  string sys = dpSubStr(aDP,DPSUB_SYS);
  // check if system is in our active connections.
  if (sys == MainDBName) {
    return true;
  }
  int iPos = dynContains ( g_connections[ "NAME" ],sys);
  if (iPos <= 0 ){
    return false;
  }
  
  // return state of the connection
  return g_connections[ "UP" ][iPos];
}

// ****************************************
// Name: navFunct_dpHasPanels 
// ****************************************
//  Tries to determine if the g_currentDatapoint
//  has viewable panels.
//     Returns true if it does
// ****************************************
bool navFunct_dpHasPanels(string dp) {
  string panelConfigDP="";
  if (dpExists(dp)) {
    panelConfigDP=ACTIVE_USER+"."+dpTypeName(dp)+"_"+ACTIVE_TAB;
  }
  
  if (dpExists(panelConfigDP)) {
    return true;
  } else {
    return false;   
  }
}

// ****************************************
// Name: navFunct_waitObjectReady
// ****************************************
//  Waits a given time in ms to see if g_objectReady is true
//  if not true within a given time, it will issue an error
//  sets it true and returns. Name can be used to show the program where the call came from
//
// 
// ****************************************
void navFunct_waitObjectReady(int timer,string name) {
  int retry=0;
  while (!g_objectReady) {
    delay(0,50);
    retry+=50;
    if (retry >= timer) {
      LOG_ERROR("navFunct.ctl:navFunct_waitObjectReady|retry called by: "+name+" longer then timer, we will try to continue");
      g_objectReady=true;
      return;
    }
  }
  return;
} 



string navFunct_ObsToTemp(string dp){
  int pos=strpos(dp,"Observation");
  if ( pos > -1) {
    string aDB=dpSubStr(dp,DPSUB_SYS);
    string bareDP=substr(dp,strlen(aDB));
    string aS2="";
    int nr=-1;
    int err = sscanf(bareDP,"LOFAR_ObsSW_Observation%d_%s",nr,aS2);
    if (aS2 != "") {
      dp=aDB+claimManager_nameToRealName("LOFAR_ObsSW_Observation"+nr)+"_"+aS2;
    } else {
      dp=aDB+claimManager_nameToRealName("LOFAR_ObsSW_Observation"+nr);
    }
  }
  return dp;
}

string navFunct_TempToObs(string dp){
  int pos=strpos(dp,"TempObs");
  if ( pos > -1) {
    string front=substr(dp,0,pos+11);
    string end = substr(dp,pos+11);
    string aDB=dpSubStr(front,DPSUB_SYS);
    string bareDP=dpSubStr(front,DPSUB_DP);
    dp=aDB+claimManager_realNameToName(bareDP)+end;
  }
  return dp;
}

// returns color for loglines
dyn_string navFunct_getLogColor(string msg, string level ){
  string col="white";
  dyn_string d1=makeDynString(msg,col);
  string txt=msg;
  if (level != "") {
    txt = level;
  } 
  if (strpos(txt,"ERROR") >= 0 || strpos(txt,"FATAL")>=0) {
    col=getStateColor(BROKEN);
  } else if (strpos(txt,"WARN") >=0) {
    col=getStateColor(SUSPICIOUS);
  }      
  return makeDynString(msg,col); 
}

string navFunct_getLogLevel(string aMsg) {
  dyn_string msgParts;
  string lvl="";

  // we need to cut out all \r and \n from the string
  strreplace(aMsg,"\r","");
  strreplace(aMsg,"\n","");
    
  msgParts = strsplit(aMsg,"|");
  
  if (dynlen(msgParts) >= 2) {
    lvl = msgParts[2];
  }
  return lvl;
}

// returns true if a system is currently online, else false
// only can check the dist systems, so we need to assume that the server system
// will be online in this case. since the navigator won't work when it isn't this assumption can be 
// safely made
bool navFunct_isOnline(int syst) {
  // check if asked for server system
  if (syst == MainDBID) return true;
  
  int iPos = dynContains(g_connections["SYSTEM"],syst);
  if (iPos > 0) {
    return g_connections["UP"][iPos];
  } else {
    LOG_ERROR("navFunct.ctl:navFunct_isOnline|System not found in g_connections "+ syst);
    return false;
  }
}

// ****************************************
// Name: navFunct_formatInt
// ****************************************
// val = the value to be formatted
// maxval = the maximum value to determine the format
//  so a val of 1 will be formatted"
//  maxval 9   -  1
//  maxval 99  -  01
//  maxval 999 -  001
// Returns:
//     the intval as string preceeded with zeros 
// or "" if error
// ****************************************
string navFunct_formatInt(int val,int maxval) {
  if (val > maxval) 
    return "";

  int nr = val;  
  // have to avoid loop when nr = 0
  if (nr == 0) nr = 1;
  string ret="";
  while (nr < maxval) {
    if (nr*10 > maxval) break;
    nr*=10;
    ret+="0";
  }
  ret+=val;
  return ret;
}
    
// ***************************
// navFunct_lofarTime2PVSSTime
// ***************************
// inDate : the input datestring in the format 2000.11.19[ 18:12:21[.888]]
// outDate: the date in PVSS format 2000.11.19[ 18:12:21[.888]] or empty string
//
// Returns true or false in case of an error
// ***************************
bool navFunct_lofarDate2PVSSDate(string inDate, time& t) {
  string outDate="";
  string date ="";
  string tm   = "";
  string mSec = "";
  t=0;

  if (inDate == "") return false;
  // expects the date in   2000-05-19T10:22:12.123  format, so check this
  dyn_string splittedDate = strsplit(inDate,"T");
  if (dynlen(splittedDate) != 2) return false;
  dyn_string splittedTime = strsplit(splittedDate[2],".");
  if (dynlen(splittedTime) >= 2) {      // mSec available
    mSec=splittedTime[2];
  }
  if (dynlen(strsplit(splittedTime[1],":")) != 3 ) return false;
  tm = splittedTime[1];

  dyn_string spl_date=strsplit(splittedDate[1],"-");
  if (dynlen(spl_date) != 3) return false;
  if (strlen(spl_date[2]) > 2) return false;
  // change - into . notation
  date=splittedDate[1];
  strreplace(date,"-",".");
  
  // the date must exactly be of form YY.MM.DD HH:MM:SS.msec
  
  if (tm   == "") tm = "00:00:00";
  if (mSec == "") mSec = "000";
  outDate += date;
  outDate += " "+tm;
  outDate += "."+mSec;

  t = scanTimeUTC(outDate);
    
  return true;
}

// ***************************
// navFunct_giveFadedColor
// ***************************
// minValue     :  Minimal Value for the given currentValue
// maxValue     :  Maximal Value for the given currentValue
// currentValue :  The currentValue
//
// returns:     : String containing the RGB color values between green and red in the form: {0,255,0}
// ***************************

string navFunct_giveFadedColor(int minValue, int maxValue,int currentValue) {
  
  string error = "navFunct.ctl:givefadedColor|ERROR: ";
  if (maxValue < minValue || currentValue < minValue || currentValue > maxValue) {
    if (maxValue < minValue) {
      error+="maxvalue ("+ maxValue+") < minValue ("+minValue+")";
    } else {
      error += "currentValue ("+currentValue+") out of bounds ("+minValue+") <> ("+maxValue+")";
    }
    LOG_ERROR(error);
    return "{0,0,0}";
  }
  
  int r,g,b,perc;
  string color="";
  float step;

  step = (maxValue-minValue)/100;
  
  perc = currentValue/step;
  
  if (perc < 50) {
    r = 0+(255-0) * perc/50.0;
    g = 255+(255-255) * perc/50.0;
    b = 0;
  } else {
    r = 255+(255-255) * (perc-50.0)/50.0;
    g = 255+(0-255) * (perc-50.0)/50.0;
    b = 0;    
  }      
  color="{"+r+","+g+","+b+"}";
  return color;
}

// ***************************
// navFunct_isObservation
// ***************************
// obsName : the observation in question
//
// Returns true if observation false when it is a pipeline
// ***************************
bool navFunct_isObservation(string obsName) {
  bool isObs = true;
  int iPos = dynContains( g_observations[ "NAME"         ], "LOFAR_ObsSW_"+obsName );
   if (iPos <=0) {
     LOG_DEBUG("navFunct.ctl:navFunct_isObservations|observation: "+ obsName+" not in g_observations : "+g_observations[ "NAME" ]);     
     isObs=false;
     return isObs;
  }
  
  
  dyn_string stations = navFunct_listToDynString(g_observations[ "STATIONLIST"    ][iPos]);

  if (dynlen(stations)< 1) {
    isObs=false;
  }
  
  return isObs;
}

// ***************************
// navFunct_getStationInputForObservation
// ***************************
// obsName : the observation in question
//
// Returns a dyn_string containing all StationInput used by this observation
// ***************************
// 
dyn_string navFunct_getStationInputForObservation(string obsName) {
  //  we only need the number from the observation
  if (strpos(obsName,"Observation") >= 0) {
    strreplace(obsName,"Observation","");
  }
  dyn_string stationInput;
  dyn_dyn_anytype tab;
  if (!navFunct_dpReachable(CEPDBName)) return stationInput;

  string query="SELECT '_online.._value' FROM 'LOFAR_*_CobaltStationInput*.observationName' REMOTE '"+CEPDBName+"' WHERE '_online.._value' == \""+obsName+"\"";
  dpQuery(query,tab);
  for(int z=2;z<=dynlen(tab);z++) {
    string dp = dpSubStr(tab[z][1],DPSUB_SYS_DP);
    dynAppend(stationInput,dp);
  }
  dynSort(stationInput);
  return stationInput;
}


// ***************************
// navFunct_getInputBuffersForStation
// ***************************
// station : the station in question
//
// Returns a dyn_string containing all InputBuffers used by this station
// ***************************
// 
dyn_string navFunct_getInputBuffersForStation(string station) {
  dyn_string inputBuffers;
  dyn_dyn_anytype tab;
  
  string stUp = strtoupper(station);
  
  if (!navFunct_dpReachable(CEPDBName)) return inputBuffers;
  string query="SELECT '_online.._value' FROM 'LOFAR_PermSW_"+stUp+"*_CobaltStationInput.observationName' REMOTE '"+CEPDBName+"'";
  dpQuery(query,tab);
  for(int z=2;z<=dynlen(tab);z++) {
    string dp = dpSubStr(tab[z][1],DPSUB_SYS_DP);
    dynAppend(inputBuffers,dp);
  }
  dynSort(inputBuffers);
  return inputBuffers;
}

// ***************************
// navFunct_getGPUProcsForObservation
// ***************************
// obsName : the observation in question
//
// Returns a dyn_string containing all GPUProcs used by this observation
// ***************************
// 
dyn_string navFunct_getGPUProcsForObservation(string obsName) {
  //  we only need the number from the observation
  if (strpos(obsName,"Observation") >= 0) {
    strreplace(obsName,"Observation","");
  }
  dyn_string gpuprocs;
  dyn_dyn_anytype tab;
  if (!navFunct_dpReachable(CEPDBName)) return gpuprocs;
  string query="SELECT '_online.._value' FROM 'LOFAR_*_CobaltGPUProc*.observationName' REMOTE '"+CEPDBName+"' WHERE '_online.._value' == \""+obsName+"\"";
  dpQuery(query,tab);
  for(int z=2;z<=dynlen(tab);z++) {
    string dp=dpSubStr(tab[z][1],DPSUB_SYS_DP);
    // avoid doubles
    if (!dynContains(gpuprocs,dp) ) dynAppend(gpuprocs,dp);
  }
  dynSort(gpuprocs);
  return gpuprocs;
}

// ***************************
// navFunct_getLocusNodesForObservation
// ***************************
// obsName : the observation in question
//
// Returns a dyn_int containing all LocusNodeNumbers used by this observation
// ***************************
// 
dyn_int navFunct_getLocusNodesForObservation(string obsName) {

  // The locusNodes have no observationName connected, but the adders know to 
  // which locusnode they are connected, so we can collect these together with their locusnode point
  dyn_string adders = navFunct_getAddersForObservation(obsName);
  
  dyn_int locusnodes;

  for(int z=1;z<=dynlen(adders);z++) {
    int ln;
    dpGet(adders[z]+".locusNode",ln);
    dynAppend(locusnodes,ln);
  }
  dynsort(locusnodes);
  return locusnodes;
}

// ***************************
// navFunct_getWritersForObservation
// ***************************
// obsName : the observation in question
//
// Returns a dyn_string containing all Writers used by this observation
// ***************************
// 
dyn_string navFunct_getWritersForObservation(string obsName) {
  //  we only need the number from the observation
  if (strpos(obsName,"Observation") >= 0) {
    strreplace(obsName,"Observation","");
  }
  dyn_string writers;
  dyn_dyn_anytype tab;

  if (!navFunct_dpReachable(CEPDBName)) return writers;
  string query="SELECT '_online.._value' FROM 'LOFAR_*_Writer*.observationName' REMOTE '"+CEPDBName+"' WHERE '_online.._value' == \""+obsName+"\"";
  
  dpQuery(query,tab);
  for(int z=2;z<=dynlen(tab);z++) {
    dynAppend(writers,dpSubStr(tab[z][1],DPSUB_SYS_DP));
  }
  return writers;
}

// ***************************
// navFunct_observationInPool
// ***************************
// observation : the station in question
// pool    : the pool to search into
//
// Returns true if the pool contains this observation
// ***************************
// 
bool navFunct_observationInPool(string obs,string pool) {
  for (int i = 1; i <= dynlen(g_observations["SCHEDULE"]); i++ ) {
    if (g_observations["SCHEDULE"][i] == pool && dynContains(g_observations["NAME"],"LOFAR_ObsSW_"+obs)) {
      return true;
    }
  }
  return false;
}


// ***************************
// navFunct_stationInObservation
// ***************************
// station : the station in question
// pool    : the pool to search into
//
// Returns true if a station is used in an observation defined in the pool
// ***************************
// 
bool navFunct_stationInObservation(string station,string pool) {
  for (int i = 1; i <= dynlen(g_observations["SCHEDULE"]); i++ ) {
    if (g_observations["SCHEDULE"][i] == pool) {
      if (strpos(g_observations["STATIONLIST"][i],station) >= 0) {
        return true;
      }
    }
  }
  return false;
}

// **********************
// navFunct_isCoreStation
// **********************
// stationName:  the station in question
//
// Returns TRUE if the station is part of the Core stations
// **********************
//
bool navFunct_isCoreStation(string stationName) {
  if (strpos(stationName,"CS") == 0) {
    return true;
  }
  return false;
}

// **********************
// navFunct_isInternationalStation
// **********************
// stationName:  the station in question
//
// Returns TRUE if the station is part of the International stations
// **********************
//
bool navFunct_isInternationalStation(string stationName) {
  if (strpos(stationName,"CS") == 0 || strpos(stationName,"RS") == 0) {
    return false;
  }
  return true;
}

// **********************
// navFunct_isRemoteStation
// **********************
// stationName:  the station in question
//
// Returns TRUE if the station is part of the Remote stations
// **********************
//
bool navFunct_isRemoteStation(string stationName) {
  if (strpos(stationName,"RS") == 0) {
    return true;
  }
  return false;
}

// **********************
// navFunct_isHBA
// **********************
// stationName:  the station in question
// antennaNr  :  the antennanr in question
//
// returns true if the antenna is an International HBA antenna
// **********************
//
bool navFunct_isHBA(string stationName,int antennaNr) {
  if (navFunct_isInternationalStation(stationName) && antennaNr < 96) {
    return true;
  }
  return false;
}

// **********************
// navFunct_isHBAZero
// **********************
// stationName:  the station in question
// antennaNr  :  the antennanr in question
//
// returns true if the antenna is a Core HBA Zero antenna
// **********************
//
bool navFunct_isHBAZero(string stationName,int antennaNr) {
  if (navFunct_isCoreStation(stationName) && antennaNr < 24) {
    return true;
  }
  return false;
}

// **********************
// navFunct_isHBAOne
// **********************
// stationName:  the station in question
// antennaNr  :  the antennanr in question
//
// returns true if the antenna is a Core HBA One antenna
// **********************
//
bool navFunct_isHBAOne(string stationName,int antennaNr) {
  if (navFunct_isCoreStation(stationName) && antennaNr > 23) {
    return true;
  }
  return false;
}

// **********************
// navFunct_isHBAInner
// **********************
// stationName:  the station in question
// antennaNr  :  the antennanr in question
//
// returns true if the antenna is a Remote HBA Inner antenna
// **********************
//
bool navFunct_isHBAInner(string stationName,int antennaNr) {
  if (navFunct_isRemoteStation(stationName) && dynContains(HBAInnerCircle,antennaNr)) {
    return true;
  }
  return false;
}

// **********************
// navFunct_isHBAOuter
// **********************
// stationName:  the station in question
// antennaNr  :  the antennanr in question
//
// returns true if the antenna is a Remote HBA Outer antenna
// **********************
//
bool navFunct_isHBAOuter(string stationName,int antennaNr) {
  if (navFunct_isRemoteStation(stationName) && !dynContains(HBAInnerCircle,antennaNr)) {
    return true;
  }
  return false;
}

// **********************
// navFunct_isLBA
// **********************
// stationName:  the station in question
// antennaNr  :  the antennanr in question
//
// returns true if the antenna is an International LBA antenna
// **********************
//
bool navFunct_isLBA(string stationName,int antennaNr) {
  if (navFunct_isInternationalStation(stationName) && antennaNr < 96) {
    return true;
  }
  return false;
}

// **********************
// navFunct_isLBAInner
// **********************
// stationName:  the station in question
// antennaNr  :  the antennanr in question
//
// returns true if the antenna is a Core or Remote LBA Inner antenna
// **********************
//
bool navFunct_isLBAInner(string stationName,int antennaNr) {
  if ((navFunct_isCoreStation(stationName) || navFunct_isRemoteStation(stationName))&& antennaNr < 48) {
    return true;
  }
  return false;
}

// **********************
// navFunct_hasAARTFAAC
// **********************
// stationName:  the station in question
//
// returns true if the station has AARTFAAC capabilities
// **********************
//
bool navFunct_hasAARTFAAC(string stationName) {
  stationName = navFunct_bareDBName(stationName);
  string aartfaacDP = stationName+":LOFAR_PIC_StationInfo.AARTFAAC";
  bool hasAARTFAAC=false;
  if (dpExists(aartfaacDP)) {
    dpGet(aartfaacDP,hasAARTFAAC);
  }
  return hasAARTFAAC;
}

// ****************************************
// Name : navFunct_uriBoard2Cabinet
// ****************************************
// Description:
//    Returns the cabinetNr to which an uriBoard is connected 
//
// Returns:
//    The cabinetnr
// ***************************************

int navFunct_uriBoard2Cabinet(int uriBoardNr) {
  return floor(uriBoardNr/2);
}

// ****************************************
// Name : navFunct_observationNameToNumber
// ****************************************
// Description:
//    Returns: the observationName without "observation"
// ***************************************

int navFunct_observationNameToNumber(string obsname) {
  return substr(obsname,strpos(obsname,"Observation")+strlen("Observation"));
}

// ****************************************
// Name : navFunct_handleUndockClick()
// ****************************************
// Description:
//    tries to undock to current panel in mainView
//    from the navigator
// ***************************************

void navFunct_handleUndockClick() {
      
  ModuleOnWithPanel(ACTIVE_TAB+"_Undocked", -1, -1, 0, 0, 1, 1, "", g_activePanel, ACTIVE_TAB+":"+g_currentDatapoint, makeDynString("$undocked:" + true));
}

// ****************************************
// Name : navFunct_checkEmailAddress
// ****************************************
// Description:
//    checks an emailAddress for its correct syntax
//    
// ***************************************

bool navFunct_checkEmailAddress(string anAddress)
{
  // an email should be in the form xxxx@yyyy.zz
  // can be improved obviously
  if (anAddress == "") return FALSE;
  dyn_string part1 = strsplit(anAddress,"@");
  if (dynlen(part1) != 2 || part1[1] == "" || part1[2] == "" ) return FALSE;
  dyn_string part2 = strsplit(part1[2],".");
  if (dynlen(part2) != 2 || part2[1] == "" || part2[2] == "" ) return FALSE;
  return TRUE;
}

void navFunct_printLastError(string place, dyn_errClass err)
{
   if(dynlen(err) > 0) 
   { 
     string dpName = getErrorDpName(err); // get the dpname relatedto the error
     string errText = getErrorText(err); //get the error text 
     LOG_ERROR(place+"| Error : " + dpName + " : " + errText);
   } 
}
