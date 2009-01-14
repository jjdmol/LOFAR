//# monitorStateReset
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
//#  $Id$
#uses "GCFCommon.ctl"
#uses "navFunct.ctl"


// This script needs to run on every CSU, CCU and MainCU
// it monitors if states needs to be reset this will be done in recursive mode
// down the LOFAR object tree for every state that is lower then the requested change.
// 

global bool isConnected=false;
global bool bDebug = true;
global dyn_string dpList;
global dyn_int    dpStates;
global dyn_string msgs;

main () {
  
  // Set the global statecolors/colornames.
  initLofarColors();


  // subscribe to the statechange update mechanism
  subscribeObjectStateReset();
}


///////////////////////////////////////////////////////////////////////////
//Function subscribeObjectStateReset
// 
// subscribes to the __resetObjectState DP of the database to monitor 
// possible stateResets
//
///////////////////////////////////////////////////////////////////////////
void subscribeObjectStateReset() {

  if (bDebug){
     DebugN("monitorStateResets.ctl:subscribeObjectStateChange|entered");
   }
  // Routine to connnect to the __resetObjectState point to trigger stateresets

  dpConnect("resetStateTriggered",false,"__resetObjectState.DPName",
            "__resetObjectState.stateNr",
            "__resetObjectState.message");
}

///////////////////////////////////////////////////////////////////////////
//Function resetStateTriggered
// 
// Callback where a trigger of __resetObjectState is handled.
//
// We have to see if the point asked is part of the database this
// ctl is running on. If so it can be handled. If not we just ignore it
//
// If this machine is the MainCU however, we might have to pass work to the different
// stations also whenever a DP resolves to be part of the station in a deeper
//
// The following appointments have been made about (re)setting states/childstates:
//      1)  The state change must be worse then the original state, otherwise do NOT touch
//      2)  if the state is not worse then the state must be either 10 or equal to the original state from the
//          point selected. Otherwise do NOT touch
//      3)  when looking at childStates, if status.leaf = TRUE then we should not touch the childstate
///////////////////////////////////////////////////////////////////////////
void resetStateTriggered(string dp1, string trigger,
                          string dp2, int state,
                          string dp3, string message) {
  // __resetObjectState change.
  // This point should have points like:
  //
  // LOFAR_PIC_Cabinet0_Subrack0_RSPBoard0_RCU0
  // 1 (= good)
  // a msg indicating extra comments on the state
  //
  // first find out if the involved DP resides on this system or not

  dynClear(dpList);
  dynClear(dpStates);
  dynClear(msgs);
  

    
  if (bDebug) DebugN("monitorStateResets.ctl:resetStateTriggered|entered with trigger:", trigger);
  string database=dpSubStr(trigger,DPSUB_SYS);
  
  if (getSystemName() == database) {
    if (bDebug) DebugN("monitorStateResets.ctl:resetStateTriggered|this DP resides on this system");
    
    string bareDP=dpSubStr(trigger,DPSUB_DP);
    int originatorState=-1;
    dpGet(trigger+".status.state",originatorState);
    if (bDebug) DebugN("monitorStateResets.ctl:resetStateTriggered|the original state is: "+ getStateColor(originatorState));
    
    // we might need alarmresets only
    if (state > 1000) {
      doAlarmReset(database,bareDP,state,originatorState,message);
    } else {
      doReset(database,bareDP,state,originatorState,message);
    }
  }
}

void doReset(string database,string bareDP,int state, int originatorState,string message) {
  //since this dp is part of this system we need to get a full list of all de dp's that belong to this dp tree, where
  // dp is the top.  We only are allowed to set a state higher, also keep a list of all the dp's we are setting.
  if (setStates(bareDP,state,originatorState,message)) {
    // now do the same for the childStates
    if (!setChildStates(bareDP,state,originatorState,message)) {
      return;
    } else {
      // since we were able to set the states and the childstates without errors
      // we need to trigger the alarmsystem to update the alarmsList.
      if (dpExists(MainDBName+DPNAME_NAVIGATOR + ".alarms")) {
        dpSetWait(MainDBName+DPNAME_NAVIGATOR + ".alarms.dpResetList",dpList,
                  MainDBName+DPNAME_NAVIGATOR + ".alarms.dpResetStates",dpStates,
                  MainDBName+DPNAME_NAVIGATOR + ".alarms.dpResetMsgs",msgs);
      } else {
        DebugTN("monitorStateResets.ctl:resetStateTriggered|ERROR!!!: Couldn't find "+MainDBName+DPNAME_NAVIGATOR + ".alarms");
      }    
    
      // and we also need to finish it up by setting the __navObjectState with the original dp
      // so the values will be set in the remainder of the tree
      if (dpExists("__navObjectState.DPName")) {
        dpSet("__navObjectState.DPName",database+bareDP+".status.state",
              "__navObjectState.stateNr",state,
              "__navObjectState.message",message,
              "__navObjectState.force",FALSE);
      } else {
        DebugTN("monitorStateResets.ctl:resetStateTriggered|ERROR!!!: Couldn't find __navObjectState.DPName");
      }
    }    
  } 
}  

bool doAlarmReset(string database,string DP,int state, int originatorState,string message) {
  string branch="PIC";
  bool changed=false;
  // select all WENT states
  string query = "SELECT '_online.._value' FROM '" + DP + "*.**.status.state' WHERE '_online.._value' == 56  OR  '_online.._value' == 46";
  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|composed query: "+query);
    
  dyn_dyn_anytype tab;
    
  if (dpQuery(query,tab)<0) {
    DebugTN("monitorStateResets.ctl:doAlarmReset|ERROR during dpQuery");
    return false;
  }
  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|results query: "+tab);
    
  // prepare a large dynarray with existing values and one with the values belonging to that dp
  if (dynlen(tab) < 2) {
    return true;
  }
  dyn_string stationNames;
  dyn_string dpMsgs;
  
  for(int z=2;z<=dynlen(tab);z++){
    // in the process of searching through all dpnames we also have to monitor if a reset has to be tranferred from MainCU
    // to all or a station. for now this is only when the DPName is of DPType "Station"
    // if found we will add the name to a list so we can trigger all stations later
    if (dpTypeName(dpSubStr(tab[z][1],DPSUB_DP)) == "Station") {
      string aS = navFunct_getStationFromDP(tab[z][1]);
      if (dpExists(aS) ) {
        dynAppend(stationNames,aS);
      }
    }
    
    if (dpExists(tab[z][1]) ){
      string msgDp=navFunct_dpStripLastElement(tab[z][1])+".message";
      int newstate=-1;
      if (tab[z][2] == 56) {
        newstate=50;
      } else if (tab[z][2]==46) {
        newstate=40;    
      } else {
        DebugN("monitorStateResets.ctl:doAlarmReset|ERROR: wrong alarm state:"+tab[z][2]);
      }   
      dynAppend(dpList,tab[z][1]);
      dynAppend(dpStates,newstate);
      dynAppend(dpMsgs,msgDp);
      dynAppend(msgs,message);
    }
  }
  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|Found dps   : "+dpList);
  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|Found states: "+dpStates);
  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|Found msgdps: "+dpMsgs);
  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|Found msgs  : "+msgs);
  
  // if stationNames are found then we need to trigger the stations reset also
  for (int i=1; i<= dynlen(stationNames);i++) {
      if (dpSetWait(stationNames[i]+"__resetObjectState.DPName",stationNames[i]+"LOFAR_"+branch,
                    stationNames[i]+"__resetObjectState.stateNr",state,
                    stationNames[i]+"__resetObjectState.message",message) < 0) {
        DebugTN("monitorStateResets.ctl:setState|ERROR during dpSetWait for all stations");
      }
  }  

  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|Found involved stations:"+stationNames);
    
  if (dynlen(dpList) > 0) {
    changed=true;
    if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|found dp's to set: "+dpList);
    if (dpSetWait(dpList,dpStates,dpMsgs,msgs)){
      DebugTN("monitorStateResets.ctl:doAlarmReset|ERROR during dpSet");
    }
  }
  
  // and same for childstate
  string query = "SELECT '_online.._value' FROM '" + DP + "*.**.status.childState' WHERE '_online.._value' == 56  OR  '_online.._value' == 46";
  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|composed query: "+query);
    
  dyn_dyn_anytype tab;
    
  if (dpQuery(query,tab)<0) {
    DebugTN("monitorStateResets.ctl:doAlarmReset|ERROR during dpQuery");
    return false;
  }
  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|results query: "+tab);
    
  // prepare a large dynarray with existing values and one with the values belonging to that dp
  if (dynlen(tab) < 2) {
    return true;
  }
  dynClear(dpList);
  dynClear(dpStates);
  dynClear(dpMsgs);
  dynClear(msgs);
  
  for(int z=2;z<=dynlen(tab);z++){
    if (dpExists(tab[z][1]) ){
      string msgDp=navFunct_dpStripLastElement(tab[z][1])+".message";
      int newstate=-1;
      if (tab[z][2] == 56) {
        newstate=50;
      } else if (tab[z][2]==46) {
        newstate=40;    
      } else {
        DebugN("monitorStateResets.ctl:doAlarmReset|ERROR: wrong alarm state:"+tab[z][2]);
      }   
      dynAppend(dpList,tab[z][1]);
      dynAppend(dpStates,newstate);
      dynAppend(dpMsgs,msgDp);
      dynAppend(msgs,message);
    }
  }
  if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|Found involved stations:"+stationNames);
    
  if (dynlen(dpList) > 0) {
    changed=true;
    if (bDebug) DebugN("monitorStateResets.ctl:doAlarmReset|found dp's to set: "+dpList);
    if (dpSetWait(dpList,dpStates,dpMsgs,msgs)){
      DebugTN("monitorStateResets.ctl:doAlarmReset|ERROR during dpSet");
    }
  }
  
  if (changed) {  
    // and we also need to finish it up by setting the __navObjectState with the original dp
    // so the values will be set in the remainder of the tree
    if (dpExists("__navObjectState.DPName")) {
      dpSet("__navObjectState.DPName",database+DP+".status.state",
            "__navObjectState.stateNr",state,
            "__navObjectState.message",message,
            "__navObjectState.force",FALSE);
    } else {
      DebugTN("monitorStateResets.ctl:resetStateTriggered|ERROR!!!: Couldn't find __navObjectState.DPName");
    }
  }
}
    
bool setStates(string DP,int state, int originatorState,string message) {
  string branch="PIC";
  string query = "SELECT '_online.._value' FROM '" + DP + "*.**.status.state' WHERE '_online.._value' < " + state + " OR  '_online.._value' == "+OPERATIONAL + " OR '_online.._value' == "+ originatorState;
  if (bDebug) DebugN("monitorStateResets.ctl:setStates|composed query: "+query);
    
  dyn_dyn_anytype tab;
    
  if (dpQuery(query,tab)<0) {
    DebugTN("monitorStateResets.ctl:setState|ERROR during dpQuery");
    return false;
  }
  if (bDebug) DebugN("monitorStateResets.ctl:setStates|results query: "+tab);
    
  // prepare a large dynarray with existing values and one with the values belonging to that dp
  if (dynlen(tab) < 2) {
    return true;
  }
  dyn_string stationNames;
  dyn_string dpMsgs;
  
  for(int z=2;z<=dynlen(tab);z++){
    // in the process of searching through all dpnames we also have to monitor if a reset has to be tranferred from MainCU
    // to all or a station. for now this is only when the DPName is of DPType "Station"
    // if found we will add the name to a list so we can trigger all stations later
    if (dpTypeName(dpSubStr(tab[z][1],DPSUB_DP)) == "Station") {
      string aS = navFunct_getStationFromDP(tab[z][1]);
      if (dpExists(aS) ) {
        dynAppend(stationNames,aS);
      }
    }
    if (dpExists(tab[z][1]) ){
      string msgDp=navFunct_dpStripLastElement(tab[z][1])+".message";

      dynAppend(dpList,tab[z][1]);
      dynAppend(dpStates,state);
      dynAppend(dpMsgs,msgDp);
      dynAppend(msgs,message);
    }
  }
  
  // if stationNames are found then we need to trigger the stations reset also
  for (int i=1; i<= dynlen(stationNames);i++) {
      if (dpSetWait(stationNames[i]+"__resetObjectState.DPName",stationNames[i]+"LOFAR_"+branch,
                    stationNames[i]+"__resetObjectState.stateNr",state,
                    stationNames[i]+"__resetObjectState.message",message) < 0) {
        DebugTN("monitorStateResets.ctl:setState|ERROR during dpSetWait for all stations");
        return false;
      }
  }  

  if (bDebug) DebugN("monitorStateResets.ctl:setStates|Found involved stations:"+stationNames);
    
  if (dynlen(dpList) > 0) {
    if (bDebug) DebugN("monitorStateResets.ctl:setState|found dp's to set: "+dpList);
    if (dpSetWait(dpList,dpStates,dpMsgs,msgs)){
      DebugTN("monitorStateResets.ctl:setState|ERROR during dpSet");
      return false;
    }
  } else {
    return true;
  }
  return true;
}

bool setChildStates(string DP,int state, int originatorState, string message) {
  string query = "SELECT '_online.._value' FROM '" + DP + "*.**.status.childState' WHERE '.status.leaf:_online.._value' == 0 AND '.status.childState:_online.._value' < " + state + " OR  '.status.childState:_online.._value' == "+OPERATIONAL+ " OR  '.status.childState:_online.._value' == "+originatorState;
  if (bDebug) DebugN("monitorStateResets.ctl:setChildStates|composed statequery: "+query);
    
  dyn_dyn_anytype tab;
    
  if (dpQuery(query,tab)<0) {
    DebugTN("monitorStateResets.ctl:setChildState|ERROR during dpQuery");
    return false;
  }
  if (bDebug) DebugN("monitorStateResets.ctl:setChildState|results query: "+tab);
  dyn_string valDPs;
  dyn_string msgDPs;
  dyn_string msgStr;
  dyn_anytype values;
    
  // prepare a large dynarray with existing values and one with the values belonging to that dp
  if (dynlen(tab) < 2) {
    return true;
  }

  for(int z=2;z<=dynlen(tab);z++){
    if (dpExists(tab[z][1]) ){
      string msgDp=navFunct_dpStripLastElement(tab[z][1])+".message";
      
      dynAppend(valDPs,tab[z][1]);
      dynAppend(values,state);
      dynAppend(msgDPs,msgDp);
      dynAppend(msgStr,message);
    }
  }
    
  // if dps found, do an update of all values in one go.
  if (dynlen(valDPs) > 0) {
    if (bDebug) DebugN("monitorStateResets.ctl:setChildState|found dp's to set: "+valDPs);
    if (dpSetWait(valDPs,values,msgDPs,msgStr)){
      DebugTN("monitorStateResets.ctl:setChildState|ERROR during dpSet");
      return false;
    }
  } else {
    return true;
  }
  return true;
}
