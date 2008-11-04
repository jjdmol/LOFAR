// navFunct.ctl
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
// navFunct_splitEvent                        : Splits an event string into the essentials
// navFunct_splitAction                       : Splits an actionstring into a dyn_string action + params
// navFunct_queryConnectObservations          : Queryconnect to keep track of all observations
// navFunct_queryConnectObservations_Callback : Callback for the above query
// navFunct_getArmFromStation                 : Returns the armposition code from a stationName
// navFunct_receiver2Cabinet                  : Returns the CabinetNr for a RecieverNr
// navFunct_receiver2Subrack                  : Returns the SubrackNr for a RecieverNr
// navFunct_receiver2RSP                      : Returns the RSPNr for a RecieverNr
// navFunct_RSP2Cabinet                       : Returns the CabinetNr for a given RSP
// navFunct_RSP2Subrack                       : Returns the SubrackNr for a given RSP
// navFunct_TBB2Cabinet                       : Returns the CabinetNr for a given TBB
// navFunct_TBB2Subrack                       : Returns the SubrackNr for a given TBB
// navFunct_subrack2Cabinet                   : Returns the CabinetNr for a given Subrack
// navFunct_dpStripLastElement                : Returns DP string without last element 
// navFunct_dpGetLastElement                  : Returns last element from DP 
// navFunct_dpGetFullPathAsTypes              : Returns full dp path (maincu && station components) as dynstring)
// navFunct_getDPFromTypePath                 : Returns Dpname derived from currentDatapoint,typeList and chosen type
// navFunct_dynToString                       : Returns a dynArray as a , seperated string
// navFunct_getDynString                      : Returns a dynString from a dyn_dyn[index]
// navFunct_bareDBName                        : Returns a DatabaseName without the : (if any)
// navFunct_findFirstOne                      : Returns the number of a given array that is true for a certain range
// navFunct_acknowledgePanel                  : Returns acknowledge on a given action
// navFunct_getPathLessOne                    : Returns path less last leaf/node
// navFunct_hardware2Obs                      : Looks if a piece of hardware maps to an observation
// navFunct_fillHardwareLists                 : Fill g_StationList, g_CabinetList,g_SubrackList,gRSPList,g_RCUList and g_TBBList
// navFunct_fillObservationsList              : Fill g_observationList
// navFunct_fillProcessesList                 : Fill g_processList
// navFunct_fillHardwareTree                  : Prepare the DP for HardwareTrees
// navFunct_fillProcessesTree                 : Prepare the DP for ProcessTrees
// navFunct_fillObservationsTree              : Prepare the DP for ObservationTrees
// navFunct)clearGlobalLists                  : clear all temporarily global hardware,observation and processes lists..

#uses "GCFLogging.ctl"
#uses "GCFCommon.ctl"
//#uses "navigator.ctl"

                            

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
  
  
  string strQuery = "SELECT '.claim.name:_original.._value', '.status.state:_original.._value', '.status.childState:_original.._value', '.stationList:_original.._value', '.receiverBitmap:_original.._value' FROM 'LOFAR_ObsSW_*' WHERE _DPT = \"Observation\" AND  '.claim.name:_original.._value' != \"\"";
  
  g_observations[ "DP"          ] = makeDynString();                    
  g_observations[ "NAME"        ] = makeDynString();
  g_observations[ "STATE"       ] = makeDynInt();
  g_observations[ "CHILDSTATE"  ] = makeDynInt();
  g_observations[ "STATIONLIST" ] = makeDynString();
  g_observations[ "RECEIVERBITMAP" ] = makeDynString();
  
  // Trigger a single query that gets an update when one 
  // observation changes
  dpQueryConnectSingle( "navFunct_queryConnectObservations_Callback", 1, "ident_observations", strQuery,50 );
}

// ****************************************
// Name : navFunct_queryConnectObservations_Callback
// ****************************************
// Description:
//    This is the callback that receives info about observations
//
// Returns:
//    None
// ***************************************

void navFunct_queryConnectObservations_Callback( 
  string strIdent,
  dyn_dyn_anytype aResult 
)
{
  int iPos;
  dyn_string strStationList;
  
  
  LOG_TRACE( "navFunct.ctl:navFunct_queryConnectObservations_Callback| Number of observations in message = " + dynlen( aResult ) );
  
  for( int t = 2; t <= dynlen( aResult ); t++)
  {  
    string strDP = aResult[t][1];
    
    // Is this an existing observation or a new one
    iPos = dynContains( g_observations[ "DP"         ], strDP );  
  
    if( iPos < 1 ){
      dynAppend( g_observations[ "DP"          ], strDP );
      iPos = dynlen( g_observations[ "DP" ] );
    }  

    // Station list is opened and closed with []  we have to strip those
    string stations = strltrim(strrtrim(aResult[t][5],"]"),"[");
    // Now store the values 
    g_observations[ "NAME"           ][iPos] = aResult[t][2];
    g_observations[ "STATE"          ][iPos] = aResult[t][3];
    g_observations[ "CHILDSTATE"     ][iPos] = aResult[t][4];
    g_observations[ "STATIONLIST"    ][iPos] = stations;
    g_observations[ "RECEIVERBITMAP" ][iPos] = aResult[t][6];
  }
  
}  

//*******************************************
// Name: Function navFunct_getArmFromStation
// *******************************************
// 
// Description:
//   Will return the armName based upon the stationName. 
//   for now (CS1 fase) it will return Core, later something smart has to be
//   done to give the other ringnames correctly.
//   It is needed to get the correct datapoints in constructions like:
//   LOFAR_PermSW_Core_CS010.state when you get a CS010:LOFAR_PermSW.state
//   change.
//
// Returns:
//    the name of the arm
// *******************************************
string navFunct_getArmFromStation(string stationName) {

  	string armName="";
  	if (substr(stationName,0,2) == "CS") {
    	armName="Core";
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
  DebugN( "navFunct.ctl:showMapping|Local mapping "+name +" contains now: " );
  for (int i = 1; i <= mappinglen(aM); i++) { 
  	DebugN("navFunct.ctl:showMapping|mappingGetKey", i, " = "+mappingGetKey(aM, i));  
		DebugN("  mappingGetValue", i, " = "+mappingGetValue(aM, i));
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
  int index=0;
  if (systemName != MainDBName) {
    typePath[++index] = dpTypeName("LOFAR");
    if (dp=="LOFAR"){
      typePath[++index] = dpTypeName("LOFAR_PIC");
      typePath[++index] = dpTypeName("LOFAR_PIC_Core");
    }
    if (strpos(dp,"PIC") >=0) {
      typePath[++index] = dpTypeName("LOFAR_PIC");
      typePath[++index] = dpTypeName("LOFAR_PIC_Core");
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
  
  if (dynlen(typeList) == "" || choice <=0) {
    return  g_currentDatapoint;
  }

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
      newDatapoint=getSystemName()+"LOFAR";
      if (choice > 1) {
        newDatapoint += "_"+typeList[choice];  // add ObsSW, PermSW or PIC
        if (strpos(typeList[choice],"Ring") >= 0) {
          newDatapoint += "_" + getArmFromStation(systemName);
        }
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
  
  // check if Observation is available in list and get the corresponding 
  if (strpos(observation,"LOFAR_ObsSW_") < 0) {
    observation = "LOFAR_ObsSW_"+observation;
  }
  
  // remove : from station name if there
  strreplace(stationName,":","");
  
  int iPos = dynContains( g_observations[ "NAME"         ], observation );
  if (iPos <=0) {
    LOG_DEBUG("navFunct.ctl:navFunct_hardware2Obs|observation: "+ observation+" not in g_observations.");     
    return false;
  }
  
  dyn_string obsStations = strsplit(g_observations[ "STATIONLIST"    ][iPos],",");
  string receiverBitmap = g_observations[ "RECEIVERBITMAP" ][iPos];
  // if receiverBitmap == "" return false
  if (receiverBitmap == "") {
    return false;
  }  
 
  // if station is not in stationList return false
  if (!dynContains(obsStations,stationName)) {
    flag = false;
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
      if (receiverBitmap[intData] == "1") {
        flag = true;
      }
    } else if (objectName == "Antenna") {
      if (receiverBitmap[(intData*2)] == "1" || receiverBitmap[((intData*2)+1)] == "1") {
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
//     Fill Hardware lists based on Observations or processes, 
//     depending on what list is filled by a panel
//     also fill the db Point with the new tree          
// ****************************************
void navFunct_fillHardwareLists() {
  // fill hardware based on available observations
  if (dynlen(g_observationsList) > 0) {
    for (int i=1; i<= dynlen(g_observationsList); i++) {
      // get stationlist for this observation
      string obsName = "LOFAR_ObsSW_"+g_observationsList[i];
      int iPos = dynContains( g_observations[ "NAME"         ], obsName);
      if (iPos <=0) {
        LOG_DEBUG("navFunct.ctl:navFunct_fillHardwareLists|Observation: "+ obsName+" not in g_observations.");     
        return;
      }
  
      dyn_string obsStations = strsplit(g_observations[ "STATIONLIST"    ][iPos],",");
    
      // fill (unique) results in g_stationList
      for (int j=1; j<= dynlen(obsStations); j++) {
        if (!dynContains(g_stationList,obsStations[j])) {
          dynAppend(g_stationList,obsStations[j]);
        }
      }
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

  // check if processList is filled
  if (dynlen(g_processesList) > 0) {
    // loop over all claimed processes in the global list and
    // check if the dp contains an observation
    // if it is an observation, and it is not allready in the global observation lists, add it. 
    for (int i=1; i <= dynlen(g_processesList);i++) {
      // check if the dptype is of type (Stn)Observation
      string process = navFunct_getPathLessOne(g_processesList[i]);
      if (dpTypeName(process) == "Observation" || dpTypeName(process) == "StnObservation") {
        // get the real observation name
        int iPos = dynContains(g_observations["DP"],MainDBName+dpSubStr(process,DPSUB_DP));
        if (iPos > 0) {
          string observation = g_observations["NAME"][iPos];
          strreplace(observation,"LOFAR_ObsSW_","");
          
          if (!dynContains(g_observationsList,observation)) {
            dynAppend(g_observationsList,observation);
          }
        }
      }
    }
  // otherwise hardware  
  } else {
    // check all available observations
    for (int i = 1; i <= dynlen(g_observations["NAME"]); i++) {
      bool found=false;
      string shortObs=g_observations["NAME"][i];
      strreplace(shortObs,"LOFAR_ObsSW_","");
    
      // If we are have more entries in the staion list we assume we are looking at a panle that has only stations
      // involved, so we  do not need to look at more hardware, in other cases we have to look if at least one piece
      // of each hardwareType also is needed for the observation to decide if it needs 2b in the list
      
      if (dynlen(g_stationList) > 1) {           
        // loop through stationList
        for (int j=1; j<= dynlen(g_stationList); j++) {
        
          //test if station is used in the observation
          if ( navFunct_hardware2Obs(g_stationList[j], g_observations["NAME"][i],"Station",g_stationList[j],0)) {
            if (!dynContains(g_observationsList, shortObs)){
              dynAppend(g_observationsList,shortObs);
              found=true;
            }
          }
          if (found) break;
        }
      } else {
        string station = g_stationList[1];
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
// Name: navFunct_fillProcessesList   
// ****************************************
//     Fill Processes lists based on hardware or observations, 
//     depending on what list is filled by a panel
//     also fill the db Point with the new tree          
// ****************************************

void navFunct_fillProcessesList() {
  
  // to do
}

// ****************************************
// Name: navFunct_fillHardwareTree   
// ****************************************
//     Fill Hardware Tree based on available Hardware in the global
//     hardwareLists
//
// ****************************************
navFunct_fillHardwareTree() {
  dyn_string result;
  string connectTo="";
  string lvl="";
  string station ="";
  string dp="";
  
  // add Stations
  if (dynlen(g_stationList) > 1 ) {
    for (int i = 1; i <= dynlen(g_stationList); i++) {
      dp = g_stationList[i]+":LOFAR";
      dynAppend(result,connectTo+","+g_stationList[i]+","+dp);
    }
    lvl="Station";
  } else if (dynlen(g_stationList) == 1) { 
    dp = g_stationList[1]+":LOFAR";
    dynAppend(result,connectTo+","+g_stationList[1]+","+dp);
    station = g_stationList[1];
    connectTo=dp;
      
  
    // add Cabinets
    if (dynlen(g_cabinetList) > 0) {
      for (int i = 1; i <= dynlen(g_cabinetList); i++) {
        dp = station+":LOFAR_PIC_Cabinet"+g_cabinetList[i];
        dynAppend(result,connectTo+",Cabinet"+g_cabinetList[i]+","+dp);
      }
      lvl="Cabinet";
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
navFunct_fillProcessesTree() {
  //to do
}

// ****************************************
// Name: navFunct_fillObservationsTree   
// ****************************************
//     Fill Observations Tree based on available Observations in the global
//     hardwareLists
//
// ****************************************
navFunct_fillObservationsTree() {
  dyn_string result;
    
  // get lists of all active,planned and finished observations to compare later
  dyn_string aO;
  dyn_string pO;
  dyn_string fO;
  dpGet("LOFAR_PermSW_MACScheduler.activeObservations",aO);
  dpGet("LOFAR_PermSW_MACScheduler.plannedObservations",pO);
  dpGet("LOFAR_PermSW_MACScheduler.finishedObservations",fO);
    
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
      return;
    }
           
    string lvl="";
    // check in what lvl the searched obs is
    if (dynContains(aO,g_observationsList[i])>0) {
      lvl="active";
    } else if (dynContains(pO,g_observationsList[i])>0) {
      lvl="planned";
    } else if (dynContains(fO,g_observationsList[i])>0) {
      lvl="finished";
    }
          
    string aS=lvl+","+g_observationsList[i]+","+g_observations["DP"][iPos];
    if (!dynContains(result,aS) && lvl!=""){
        dynAppend(result,aS);
    }
  }
  
  LOG_DEBUG("navFunct.ctl:navFunct_fillObservationsTree|result: "+ result);     
  
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".observationsList",result);  
}

// ****************************************
// Name: navFunct_clearGlobalLists  
// ****************************************
//     Clear all global observations,hardware and processesLists
//
// ****************************************
navFunct_clearGlobalLists() {
  // empty global listings
  dynClear(g_stationList);
  dynClear(g_cabinetList);
  dynClear(g_subrackList);
  dynClear(g_RSPList);
  dynClear(g_TBBList);
  dynClear(g_RCUList);

  dynClear(g_observationsList);
  dynClear(g_processesList);
}
