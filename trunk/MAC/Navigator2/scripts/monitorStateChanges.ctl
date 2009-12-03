//# monitorStateChanges.ctl
//#
//#  Copyright (C) 2007-2008
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
//#  $Id: monitorStateChanges.ctl,v 1.4 2007/06/26 08:59:29 coolen Exp $
#uses "GCFCommon.ctl"
#uses "navFunct.ctl"


// This script needs to run on every CSU, CCU and MainCU
// it monitors the state changes in the database, and will update the childstates accordingly
// 

global bool isConnected=false;
global bool bDebug = false;

main () {

  // Set the global statecolors/colornames.
  initLofarColors();

  // subscribe to the statechange update mechanism
  subscribeObjectStateChange();
}


///////////////////////////////////////////////////////////////////////////
//Function subscribeObjectStateChange
// 
// subscribes to the __navObjectState DP of the database to monitor 
// possible stateChanges, also checks if the scripts runs on a MCU*** 
// machine and subscribes to the needed statechanges there also.
//
// Added 26-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void subscribeObjectStateChange() {

  if (bDebug){
     DebugN("monitorStateChanges.ctl:subscribeObjectStateChange|entered");
   }
  // ROutine to connnect to the __navObjectState point to trigger statechanges
  // So that childState points can be set/reset accordingly.

  dpConnect("objectStateTriggered",false,"__navObjectState.DPName",
            "__navObjectState.stateNr",
            "__navObjectState.message",
            "__navObjectState.force");
  
  //Find out the Database we are running on
  string database=getSystemName();
  if (substr(database,0,3) == "MCU") {
    connectMainPoints();  
  }
}

 
///////////////////////////////////////////////////////////////////////////
//Function connectMainPoints
// 
// connect to the mainpoints to set the state's and childstate's to the max
// of the state's and childstate's of the ones in the stations
//
// Msg something like: Station:Dp.state=state
//                e.g. CS010:LOFAR_ObsSW_Oservation10.status.state=broken
//
// Points of interrest:
//    StnObservation  --> Station --> MCU*:LOFAR_ObsSW_Observation*_*_CS010.status.state/childState
//    StnPermSW       --> Station --> MCU*:LOFAR_PermSW_*_CS010.status.state/childState
//    StnPic          --> Station --> MCU*:LOFAR_PIC_*_CS010.status.state/childState
//
// Added 26-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void connectMainPoints() {

  // Routine to connect to _DistConnections.ManNums.
  // This point keeps a dyn_int array with all active distributed connections
  // and will generate a callback everytime a station goes off-, or on- line

  if (dpExists("_DistConnections.Dist.ManNums")) {
    dpConnect("distSystemTriggered",true,"_DistConnections.Dist.ManNums");
  } else {
    DebugN("monitorStateChanges.ctl:connectMainPoints|_DistConnections point not found, no trigger available for dist System updates.");  
  }

}


///////////////////////////////////////////////////////////////////////////
//Function distSystemTriggered
// 
// Callback function that is triggered by the (dis)appearance of a Distributed system.
//
///////////////////////////////////////////////////////////////////////////
void distSystemTriggered(string dp1, dyn_int systemList) {
  
  if (bDebug) DebugN("monitorStateChanges.ctl:distSystemTriggered|entered");

  // Check all states from LOFAR_PermSW.state in all remote stations.
  // if one of them changes, and the state is not equal to old state, update the MCU one
  string queryPermSW = "SELECT '_original.._value' FROM '{LOFAR_PermSW.status.state,LOFAR_PermSW.status.childState}' REMOTE ALL WHERE _DPT = \"StnPermSW\" SORT BY 1 DESC";

  // Check all states from LOFAR_PIC.state in all remote stations.
  // if one of them changes, and the state is not equal to old state, update the MCU one
  string queryPIC = "SELECT '_original.._value' FROM '{LOFAR_PIC.status.state,LOFAR_PIC.status.childState}' REMOTE ALL WHERE _DPT = \"StnPIC\" SORT BY 1 DESC";

  // Check all states from LOFAR_ObsSW_Observation*.state in all remote stations.
  // if one of them changes, and the state is not equal to old state, update the MCU one
  string queryObservation = "SELECT '_original.._value' FROM '{LOFAR_ObsSW_Observation*.status.state,LOFAR_ObsSW_Observation*.status.childState}' REMOTE ALL WHERE _DPT = \"StnObservation\" SORT BY 1 DESC";

  if (bDebug) DebugN("monitorStateChanges.ctl:distSystemTriggered|isConnected: "+isConnected);
  if (isConnected) {
    if (dpQueryDisconnect("stationStateTriggered","PermSW") < 0) {
      if (bDebug) DebugN("monitorStateChanges.ctl:distSystemTriggered|disconnect PermSW: "+getLastError());
    }
    if (dpQueryDisconnect("stationStateTriggered","PIC") < 0) {
      if (bDebug) DebugN("monitorStateChanges.ctl:distSystemTriggered|disconnect PIC: "+getLastError());
    }
    if (dpQueryDisconnect("stationStateTriggered","Observation") < 0) {
      if (bDebug) DebugN("monitorStateChanges.ctl:distSystemTriggered|disconnect Observation: "+getLastError());
    }
  }

  if (dynlen(systemList) > 0 ) {
    dpQueryConnectSingle("stationStateTriggered",false,"PermSW",queryPermSW);
    dpQueryConnectSingle("stationStateTriggered",false,"PIC",queryPIC);
    dpQueryConnectSingle("stationStateTriggered",false,"Observation",queryObservation);
    isConnected=true;
  } else {
    isConnected=false;
  }    
}

///////////////////////////////////////////////////////////////////////////
//Function stationStateTriggered
// 
// Callback where a trigger of a changed is handled.
//
// Added 02-04-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void stationStateTriggered(string ident, dyn_dyn_anytype tab) {
  if (bDebug) DebugN("monitorStateChanges.ctl:stationStateTriggered|entered");

  string state    = "";
  string station = "";
  string armName = "";
  string datapoint = "";
  string element = "";


  for(int z=2;z<=dynlen(tab);z++){
    if (bDebug) DebugN("monitorStateChanges.ctl:stationStateTriggered|Found : ",tab[z]);

    state = tab[z][2];
    string dp=tab[z][1];
    station = dpSubStr(dp,DPSUB_SYS);
    strreplace(dp,station,"");
    strreplace(station,":","");

    // strip element name and datapoint
    datapoint = dpSubStr(dp,DPSUB_DP);
  
    // skip the point
    element = substr(dp,strlen(datapoint)+1,((strlen(dp))-(strlen(datapoint)))); 

    // get the RingName
    armName=navFunct_getRingFromStation(station);


    if (bDebug) DebugN("monitorStateChanges.ctl:stationStateTriggered|station : ",station, " DP: " ,datapoint," element: ",element);

    // if all needed values are available we can start doing the major update.
    if (state >-1 && datapoint != "" && station != "" && armName != "" && element != ""){
      if (ident == "PIC") {
	      setStates(datapoint + "_"+ armName + "_" + station,element,state,"",true,true);
      } else {
	      setStates(datapoint,element,state,"",true,true);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//Function objectStateTriggered
// 
// Callback where a trigger of __navObjectState is handled.
//
// Added 26-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void objectStateTriggered(string dp1, string trigger,
                          string dp2, int state,
                          string dp3, string message,
                          string dp4, bool force) {
  // __navObjectState change.
  // This point should have points like:
  //
  // LOFAR_PIC_Cabinet0_Subrack0_RSPBoard0_RCU0.status.state
  // 1 (= good)
  // a msg indicating extra comments on the state
  // true/false
  //
  // The force bool will be used to monitor if lowering a state > 10 is allowed.
  // false means NO, true means force a change.
  //                               
  // This State should be set to childState=bad in all upper (LOFAR-PIC_Cabinet0_Subrack0_RSPBoard0) treenodes
  // ofcourse we need to monitor if this statechange is allowed.
  // if force false we also monitor the first digit of the state (1-5) if the 1st digit of the current state matches, not
  // state change allowed (to avoid ACK'ed alarms go back to came for example)
  


  if (trigger == "") return;

  if (bDebug) DebugN("monitorStateChanges.ctl:objectStateTriggered|entered with trigger:", trigger);

  string datapoint = "";
  string element   = "";

  
  // the datapointname can hold _ and . seperated path elements.  however, after the last point there should be
  // an element state, or an element childState
  //
  // strip the system name
  datapoint = dpSubStr(trigger,DPSUB_DP_EL);

    
  int start=strpos(trigger,".status.state");
  if (start >= 0) {
  	element = "status.state";
  } else {
    start=strpos(trigger,".status.childState");
    if (start >= 0) {
   	  element = "status.childState";
    } else {
      DebugTN("monitorStateChanges.ctl:objectStateTriggered|ERROR: No state nor childState found in DPName");
      return;
    }
  }

  // strip the last status.state or status.childState from the datapoint name.
  // the remainder is used as path
  datapoint = substr(trigger,0,start);

  if (bDebug) DebugN("monitorStateChanges.ctl:objectStateTriggered|state:  " + state + " DP: " + datapoint + " Element: " + element + " Message: " + message);
  // if all needed values are available we can start doing the major update.
  if (state >= 0 && datapoint != "" && element != "" && dpExists(datapoint+"."+element)){

    setStates(datapoint,element,state,message,force,false);
  } else {
    DebugTN("monitorStateChanges.ctl:objectStateTriggered|result: Wrong datapoint or state. DP: " + datapoint +"."+element+ " State: "+state );
  }
}


///////////////////////////////////////////////////////////////////////////
//Function setStates
// 
// Does the setting of (child)state.
//
// datapoint      = the base datapoint that needs to be set
// element        = state or childState needs 2b checked 
// state          = new state
// message        = message if applied by __navObjectState
// force          = boolean indicating if states > 10 are allowed to be lowered or not and check first digit old against new state
// stationTrigger = check if the stationTrigger fired this. In that case the state/childState
//                  is a pure copy from the station to the MCU point, so childState should be 
//                  treated as state and just be copied, not evaluated.
//
// Added 3-4-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void setStates(string datapoint,string element,int state,string message,bool force,bool stationTrigger) {

  if (bDebug) DebugN("monitorStateChanges.ctl:setStates|entered with datapoint: "+ datapoint+" element: "+ element+ " state: "+state+" message: "+message);

  string dp;
  
  // If element is state just set the state (might have been done by the callerprocess, 
  // When this is done we have to strip the last treenode, 
  // and start updating the childStates for the remaining nodes. If the element is childState,
  // only the childState updates needs 2b done, unless stationTrigger = true, then childstate should be
  // handled the same way as state.
  if (element == "status.state" | stationTrigger) {
    // set the state value of this dp
    
    int aVal;
    dpGet(datapoint+"."+element,aVal);
    if (state > -1 && state != aVal) {
      if (force) {
        dpSet(datapoint+"."+element,state);
      } else {
        if (bDebug) DebugN("monitorStateChanges.ctl:setStates|original val: "+aVal+ " state to set to: " + state);
        if (aVal <=10 && state <= 0) {
          dpSet(datapoint+"."+element,state);
        } else if (aVal <= state && floor(aVal/10)!=floor(state/10) ){
          dpSet(datapoint+"."+element,state);
        } else {
          if (bDebug) DebugN("monitorStateChanges.ctl:setStates|State not changed because of force conditions");
          return;
        }
      }
      dpSet(datapoint+".status.message",message);
      
    } else {
      if (bDebug) DebugN("monitorStateChanges.ctl:setStates|Equal value or state < 0, no need to set new state");
    }
    
    // But we still need to trigger a new childState update
    dp = navFunct_getPathLessOne(datapoint);
    datapoint=dp;

  } else if (element != "status.childState") {
    DebugN("monitorStateChanges.ctl:setStates|Error. unknown element in stateChange trigger: ", element);
    return;
  }

  
  if (bDebug) DebugN( "monitorStateChanges.ctl:setStates|Continueing with setChildState passing path: "+datapoint);
  // set childState if needed, if succeeded set childState for one level less also
  // continue while true
  while ( setChildState(datapoint,state)) {
    if (bDebug) DebugN( "monitorStateChanges.ctl:setStates|Continueing with setChildState passing path: "+datapoint);
    dp = navFunct_getPathLessOne(datapoint);
    datapoint=dp;
  }
}


///////////////////////////////////////////////////////////////////////////
//Function setChildState
// 
// Does the setting of childstate based upon the highest state found in all 
// the direct children of a given datepoint, merged with the given state
//
// Added 26-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
bool setChildState(string Dp,int state) {

  if (bDebug) DebugN("monitorStateChanges.ctl:setChildState| entered with DP: "+Dp+" stateVal: "+ state);

  if (state < 0 || Dp == "") return false;

  dyn_dyn_anytype tab;
  int z;
  int aVal;

  // take present value for comparing later.
  dpGet(Dp+".status.childState",aVal);
  if (bDebug) DebugN("monitorStateChanges.ctl:setChildState|"+Dp+".status.childState = "+aVal);


  // Query for all state and childState in the tree from this datapoint down.
  // Only the first level will be examined.
  // By sorting descending we can shorten the time needed for evaluation. If the top element's state value = lower or equal
  // to the value we need to set, we can leave the loop and set the new state value. In case it is bigger we can take that value
  // and set it as the new value (if > the original value)

  // at the moment there is a bug in the sql approach from ETM, if the result Select is an empty table then the sort still
  // is executed. This give an error. So for now we do the query two times.
  // the 2nd one only if the first one gives a result > 1;
 
// =====>>>> Should be fixed in 3.7
//  string query = "SELECT '_original.._value' FROM '{"+Dp+"_*.status.childState,"+Dp+"_*.status.state}'";
//  int err = dpQuery(query, tab);

//  if (err < 0 | dynlen(tab)< 2) {
//    return true;
//  }

  string query = "SELECT '_original.._value' FROM '{"+Dp+"_*.status.childState,"+Dp+"_*.status.state}' SORT BY 1 DESC";
  if (bDebug) DebugN("monitorStateChanges.ctl:setChildState|Query: ",query);
  int err = dpQuery(query, tab);



   
  if (err < 0) {
    if (bDebug) DebugN("monitorStateChanges.ctl:setChildState|Error " + err + " while getting query.");
    return false;
  }
 
  dyn_string aS1=strsplit(Dp,"_");
  int maxElements= dynlen(aS1)+1; 
  int foundElements=0;

  if (bDebug) DebugN("monitorStateChanges.ctl:setChildState|max elements for DP  after _ split: "+maxElements);

  // first check if the last element still has . seperated elements
  dyn_string aS2=strsplit(aS1[dynlen(aS1)],".");
  if (dynlen(aS2) > 1) {
    maxElements+=dynlen(aS2)-1; 
  }

  if (bDebug) DebugN("monitorStateChanges.ctl:setChildState|max elements for DP  after . split: "+maxElements);

  for(z=2;z<=dynlen(tab);z++) {
    dyn_string aStr1=strsplit((string)tab[z][1],"_");
    foundElements=dynlen(aStr1);

    if (bDebug) DebugN("monitorStateChanges.ctl:setChildState|Working with dp: " +(string)tab[z][1]+ " that has "+ foundElements +" elements");

    // first check if the last element still has . seperated elements but skip the status.state status.childState
    dyn_string aStr2=strsplit(aStr1[dynlen(aStr1)],".");
    if (dynlen(aStr2) > 3) {
      foundElements+=dynlen(aStr2)-3; 
    }
    
    if (bDebug) DebugN("monitorStateChanges.ctl:setChildState|and after . check has " + foundElements +" elements");

    if(foundElements <= maxElements) {
      if (bDebug) DebugN("monitorStateChanges.ctl:setChildState|Have to check DP: ",tab[z][1], " state: ", tab[z][2]);

      // check if found state > new state
      if ((int)tab[z][2] > state) {
        state=(int)tab[z][2];
      }
      // check if state != oldVal
      if (state != aVal && state > -1 ) {
        if (bDebug) DebugN("monitorStateChanges.ctl:setChildState|state not equal oldstate(",aVal,") so set ",Dp+".status.childState to: ",state);
        dpSet(Dp+".status.childState",state);
        return true;
      }
      return false;
    }
  }

  return false;
}

