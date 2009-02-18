// GCFAlarm.ctl
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
// GCF Alarm functions
///////////////////////////////////////////////////////////////////
//
//
// Functions and procedures
//
// initAlarmSystem      : initializes the AlarmSystem
// getAlarmStatus       : gives the string representation from the alarmStatus
// getAlarms            : get the priorly set and still open alarms from the database
// setAlarms            : writes all alarsm to database
// distSystemTriggered  : Callback that a distSystem has connected/disconnected
// objectStateCallback  : Fill the global alarm mapping and the alarm database

#uses "GCFLogging.ctl"
#uses "GCFCommon.ctl"

// Mapping where we try to keep all alarms to be able to access them faster from the panels
global mapping    g_alarms;

// needed for dpDisconnect after a connect and a resync when dist systems go off or online
global bool isConnected = false;

const int CAME = 0;
const int WENT = 1;
const int ACK  = 2;

// *******************************************
// Name : initNavigatorAlarms
// *******************************************
// Description:
// initializes the Navigator Alarmsystem
// Keeps a global mapping that contain all current alarms
// connects to the __navigator.alarms point te be triggered when alarms change
// so the mapping can be kept up te date.
//
//

// *******************************************
void initNavigatorAlarms(){
  LOG_DEBUG("GCFAlarm.ctl.ctl:initAlarmSystem|entered");
  g_alarms[ "TIME"     ] = makeDynTime();                    
  g_alarms[ "DPNAME"   ] = makeDynString();
  g_alarms[ "MESSAGE"  ] = makeDynString();
  g_alarms[ "STATE"    ] = makeDynInt();
  g_alarms[ "STATUS"   ] = makeDynInt();
  
  // Routine to connect to _DistConnections.ManNums.
  // This point keeps a dyn_int array with all active distributed connections
  // and will generate a callback everytime a station goes off-, or on- line

  // connect to alarm point and read initial alarms from database.
  if (dpExists(DPNAME_NAVIGATOR +  ".alarms")) {
    if (dpConnect("alarmSystemTriggered",true,DPNAME_NAVIGATOR +  ".alarms.time")== -1) {
      LOG_ERROR("GCFAlarm.ctl:initCtrlAlarmSystem|Couldn't connect to alarm point, alarms will not be updated");  
    }
  } else {
    LOG_ERROR("GCFAlarm.ctl:initCtrlAlarmSystem|Couldn't connect to alarm point, alarms will not be updated");  
  } 
}

// *******************************************
// Name : alarmSystemTriggered
// *******************************************
// Description:
// callback when the __navigator.alarm point is changed.
// this callback will rewrite the global mapping for the navigator
//
void alarmSystemTriggered(string dp1, dyn_time times) {
  mappingClear(g_alarms);
  if (dpExists(DPNAME_NAVIGATOR +  ".alarms")) {
    getAlarms(DPNAME_NAVIGATOR +  ".alarms",
              g_alarms[ "TIME"],g_alarms[ "DPNAME"   ],g_alarms[ "MESSAGE"  ],g_alarms[ "STATE"    ],g_alarms[ "STATUS"   ]);
  } else {
    LOG_ERROR("GCFAlarm.ctl:alarmSystemTriggered|Couldn't get alarms from navigator");
  }
  
  //set trigger so panels (if any) know mapping is (re)synced
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID+".alarmsTrigger")) {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID+".alarmsTrigger",true);
  }  
  LOG_DEBUG("GCFAlarm.ctl:alarmSystemTriggered|Nr alarms in global after update: "+ dynlen(g_alarms["TIME"]));      
  showMapping(g_alarms,"g_alarms");

  
}

// *******************************************
// Name : getAlarmStatus
// *******************************************
// Description:
// gives the string representation from the alarmStatus
//
//
// Returns:
//    string representing the Status
// *******************************************
string getAlarmStatus(int status){

  if (status == 0) {
    return "CAME";
  } else if (status == 1) {
    return "WENT";
  } else if (status == 2) {
    return "ACK";
  }
  return "UNKNOWN";
}

// *******************************************
// Name : getAlarms
// *******************************************
// Description:
// get the priorly set and still open alarms from the database
//
//
// Returns:
//    None
// *******************************************
void getAlarms(string dp,dyn_time &times,dyn_string &names, dyn_string &messages, dyn_int &state, dyn_int &status) {
  if (dpGet(dp+".time",times,dp+".datapoint",names,dp+".message",messages,dp+".state",state,dp+".status",status) < 0) {
    LOG_DEBUG("GCFAlarm.ctl:getAlarms|Error getting alarms from database: ", getLastError());  
  }
}

// *******************************************
// Name : setAlarms
// *******************************************
// Description:
// writes all alarms to database
//
//
// Returns:
//    None
// *******************************************
void setAlarms(string dp,dyn_time times,dyn_string names, dyn_string messages, dyn_int state, dyn_int status) {
  
  if (dpSet(dp+".time",times,dp+".datapoint",names,dp+".message",messages,dp+".state",state,dp+".status",status) < 0) {
    LOG_DEBUG("GCFAlarm.ctl:setAlarms|Error setting alarms in database: ", getLastError());  
  }
}

// *******************************************
// Name : stateToStatus
// *******************************************
// Description:
// returns the alarmstatus derived from the state
//
//
// Returns:
//    the derived status
// *******************************************
int stateToStatus(int aState) {
  if (aState == SUSPICIOUS || aState == BROKEN) {
    return ACK;
  } else if (aState == SUSPICIOUS_CAME || aState == BROKEN_CAME) {
    return CAME;
  } else if (aState == SUSPICIOUS_WENT || aState == BROKEN_WENT) {
    return WENT;
  }
  return -1;
}





