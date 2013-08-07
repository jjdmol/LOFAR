//# monitorAlarms.ctl
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
#uses "GCFAlarm.ctl"
#uses "navFunct.ctl"



// This script needs to run the MainCU
// it monitors the state changes in the database, and will update the alarms accordingly
// 

bool bDebug=false;
bool occupied=false;

main () {

  // Set the global statecolors/colornames.
  initLofarColors();

  // initialize the AlarmSystem
  initCtrlAlarmSystem();
}

///////////////////////////////////////////////////////////////////////////
//
// Function initAlarmSystem
//
// Initializes the AlarmSystem for the main ctrl'er that runs on the MainCu
// connect to distmanager to be able to check
// if systems came or went ofline. If came or went do a queryconnect again on all
// NCFObjectStates to be able to check if state changes were issued.
// 
///////////////////////////////////////////////////////////////////////////
void initCtrlAlarmSystem()  {
  if (bDebug) DebugTN("monitorAlarms.ctl:initAlarmSystem|entered");
  g_alarms[ "TIME"     ] = makeDynTime();                    
  g_alarms[ "DPNAME"   ] = makeDynString();
  g_alarms[ "MESSAGE"  ] = makeDynString();
  g_alarms[ "STATE"    ] = makeDynInt();
  g_alarms[ "STATUS"   ] = makeDynInt();
  
  // read all stored alarms.
  readAlarms();
  
  // Routine to connect to _DistConnections.ManNums.
  // This point keeps a dyn_int array with all active distributed connections
  // and will generate a callback everytime a station goes off-, or on- line

 
  if (dpExists("_DistConnections.Dist.ManNums")) {
    dpConnect("distSystemTriggered",true,"_DistConnections.Dist.ManNums");
  } else {
    if (bDebug) DebugTN("monitorAlarms.ctl:initCtrlAlarmSystem|_DistConnections point not found, no trigger available for dist System updates.");  
  } 
  if (bDebug) DebugTN("monitorAlarms.ctl:initCtrlAlarmSystem|Nr alarms in global after init: "+ dynlen(g_alarms["TIME"]));      
  showMapping(g_alarms,"g_alarms"); 
}

// *******************************************
// Name : distSystemTriggered
// *******************************************
// Description:
// Callback that a distSystem has connected/disconnected
//
//
// Returns:
//    None
// *******************************************
void distSystemTriggered(string dp1, dyn_int systemList) {
  
  string query = "SELECT '_original.._value' FROM '{__navObjectState.DPName,__navObjectState.stateNr,__navObjectState.message,__navObjectState.force}' REMOTE ALL WHERE _DPT = \"NCFObjectState\" SORT BY 0";
  
  
  if (isConnected) {
    dpQueryDisconnect("objectStateCallback","objectState"); 
    dpDisconnect("resetTriggered",DPNAME_NAVIGATOR +  ".alarms.dpResetList",
                                  DPNAME_NAVIGATOR +  ".alarms.dpResetStates",
                                  DPNAME_NAVIGATOR +  ".alarms.dpResetMsgs");
    dpDisconnect("rereadTriggered",DPNAME_NAVIGATOR +  ".alarms.rereadAlarms");
  }
  
  // also connect to the resetTrigger dp to receive a trigger that the internal global list needs a reread
  // (in case ACK did remove an alarm for example)
  if (dpExists(DPNAME_NAVIGATOR +  ".alarms.rereadAlarms")) {
    dpConnect("rereadTriggered",false,DPNAME_NAVIGATOR +  ".alarms.rereadAlarms");
  } else {
    DebugTN("monitorAlarms.ctl:distSystemTriggered|Couldn't connect to"+DPNAME_NAVIGATOR +  ".alarms.rereadAlarms");
  }  

  // Connect to the dp elements that we use to receive
  // a new claim in the MainDB
  // dpQueryConnectAll( "objectStateCallback",true,"objectState",query,20);   test DISCARDING
  dpQueryConnectAll( "objectStateCallback",true,"objectState",query); 
  
  // also connect to the dpResetList dp to receive lists if dp's that need to be cleared from the global list and thus from the
  // datapoint in the database
  if (dpExists(DPNAME_NAVIGATOR +  ".alarms.dpResetList")) {
    dpConnect("resetTriggered",false,DPNAME_NAVIGATOR +  ".alarms.dpResetList",
                                     DPNAME_NAVIGATOR +  ".alarms.dpResetStates",
                                     DPNAME_NAVIGATOR +  ".alarms.dpResetMsgs");
  } else {
    DebugTN("monitorAlarms.ctl:distSystemTriggered|Couldn't connect to"+DPNAME_NAVIGATOR +  ".alarms.dpResetList");
  }
  
  
  isConnected = true; 
}

// *******************************************
// Name : objectStateCallback
// *******************************************
// Description:
// Fill the global alarm mapping and the alarm database
//
//
// Returns:
//    None
// *******************************************
void objectStateCallback(string ident, dyn_dyn_anytype aResult) {
  
  if (bDebug) DebugTN( "monitorAlarms.ctl:objectStateCallback| Number of states in callback = " + dynlen( aResult ) );
  if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Result: "+ aResult);
  if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|occupied: "+ occupied);
  while (occupied) {
    delay(0,25);    
  }
  
  occupied = true;
  
  
  if (dynlen(aResult) <= 0) {
    occupied = false;
    return;
  } 
  
  bool changed = false;
  int iPos;
 
  // Loop through all alarms in this callback
  for (int nr = 2; nr < dynlen (aResult);nr+=4) {
    //get the stateNr and DP that go with this message
    
    time aTime     = getCurrentTime();
    string aSystem = dpSubStr(aResult[nr][1],DPSUB_SYS);
    string aDP     = aResult[nr][2];
    if (strpos(aDP,":") < 0) {
      string aS=aSystem+aDP;
      aDP=aS;
    }
    // check if DPName contains a System name, otherwise get the name from dppart
    
    int state      = (int)aResult[nr+1][2];
  
    if (state == 60) {
      continue;
    }
    
    
    string message = aResult[nr+2][2];
    bool force     = aResult[nr+3][2];
    int aStatus    = CAME;
  
    if (aDP==""){
      continue;
    }
  
    // Is this an existing DP or a new one
    iPos = dynContains( g_alarms[ "DPNAME"         ], aDP );

  
    // check if existing dp.
    // if it exists, check if new state 1st digit > oldState 1st digit && force, otherwise return
//    if (iPos > 0 && ((floor(g_alarms["STATE"][iPos]/10) >= floor(state/10))&& !force)) {
    if (iPos > 0 && g_alarms["STATE"][iPos] >= state && !force) {
    	if (bDebug) DebugN("monitorAlarms.ctl:objectStateCallback|return on condition mismatch");
    	if (bDebug) DebugN("Found existing dp: ",aDP);
    	if (bDebug) DebugN("state ",g_alarms["STATE"][iPos]);
      	continue;
    }
    
    
    // if state < 40 we might need to remove an ACK'ed or WENT ALARM
    // if status != ACK then we need to keep it and let ACK command remove it
    // otherwise it can't be an alarm, so return.
    if (state < 40)  {
      if (iPos > 0 && g_alarms["STATUS"][iPos] == ACK ) {
        dynRemove(g_alarms["DPNAME" ],iPos);
        dynRemove(g_alarms["TIME"   ],iPos);
        dynRemove(g_alarms["STATE"  ],iPos);
        dynRemove(g_alarms["MESSAGE"],iPos);
        dynRemove(g_alarms["STATUS" ],iPos);
        changed = true;


      } else if (iPos > 0 && g_alarms["STATUS"][iPos] == CAME) {
        g_alarms["STATUS"][iPos] = WENT;
        g_alarms["STATE"][iPos] = g_alarms["STATE"][iPos]-3;
        changed = true;
      }   
      if (changed) {
        storeAlarms();
      }
      continue;
    }
    
    // in the remainder of the cases the state was an alarm
    if( iPos < 1 ){
      if (bDebug) DebugN("monitorAlarms.ctl:objectStateCallback|Need to append new alarm");
      dynAppend( g_alarms[ "DPNAME"          ], aDP );
      iPos = dynlen( g_alarms[ "DPNAME" ] );
      if (state == BROKEN) aStatus=ACK;
      if (state == BROKEN_WENT) aStatus=WENT;
      if (state == BROKEN_CAME) aStatus=CAME;
      if (state == SUSPICIOUS) aStatus=ACK;
      if (state == SUSPICIOUS_WENT) aStatus=WENT;
      if (state == SUSPICIOUS_CAME) aStatus=CAME;
    
    } else {
      if (bDebug) DebugN("monitorAlarms.ctl:objectStateCallback|Need to examine if alarm update is needed");
      // it was an existing DP, so we have to compare the status.
     
      int oldState=g_alarms["STATE"][iPos];
      time oldTime = g_alarms["TIME"][iPos];

      // check broken ranges first
      if (state >= BROKEN ) {
        if (oldState == BROKEN) {
          if (state == BROKEN) {
            aStatus = ACK;
            aTime = oldTime;
          } else if (state == BROKEN_CAME) {
            aStatus = CAME;
          } else {
            aStatus = WENT;
          }
        } else if (oldState  == BROKEN_WENT) {
          if (state == BROKEN) {
            aStatus = ACK;
          } else if (state == BROKEN_WENT) {
            aTime = oldTime;
            aStatus = WENT;
          } else {
            aStatus = CAME;
          }        
        } else if (oldState == BROKEN_CAME) {
          if (state == BROKEN) {
            aStatus = ACK;
          } else if (state == BROKEN_WENT) {
            aStatus = WENT;
          } else {
            aTime = oldTime;
            aStatus = CAME;
          }
        }
      } else if (state >= SUSPICIOUS) {
        if (oldState == SUSPICIOUS) {
          if (state == SUSPICIOUS) {
            aTime = oldTime;
            aStatus = ACK;
          } else if (state == SUSPICIOUS_CAME) {
            aStatus = CAME;
          } else {
            aStatus = WENT;
          }
        } else if (oldState  == SUSPICIOUS_WENT) {
          if (state == SUSPICIOUS) {
            aStatus = ACK;
          } else if (state == SUSPICIOUS_WENT) {
            aTime = oldTime;
            aStatus = WENT;
          } else {
            aStatus = CAME;
          }        
        } else if (oldState == SUSPICIOUS_CAME) {
          if (state == SUSPICIOUS) {
            aStatus = ACK;
          } else if (state == SUSPICIOUS_WENT) {
            aStatus = WENT;
          } else {
            aStatus = CAME;
            aTime = oldTime;
          }        
        }
      }
    }
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Datapoint - "+aDP);
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Time      - ",aTime);
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: State     - "+state);
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Message   - "+message);
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Status    - "+aStatus);
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Force     - "+force);
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|iPos:            - "+iPos);
    // Now store the values 
    g_alarms[ "TIME"    ][iPos] = aTime;
    g_alarms[ "STATE"   ][iPos] = state;
    g_alarms[ "MESSAGE" ][iPos] = message;
    g_alarms[ "STATUS"  ][iPos] = aStatus;
    changed=true;
    // check if the alarm was a suspicious_came or an alarm_came
    // if this is true then we need to send an email to the observers
    // the station tests generate 100's of alarms all the time, 
    // the system should know this but the mailnotification becomes spam with these amounts
    // so we will skip these for now.
    if (strpos(message,"stationtest:") < 0) {
      if (state == SUSPICIOUS_CAME ) {
        sendMail("SUSPICIOUS_CAME",aDP,aTime,message);
      } else if (state == BROKEN_CAME) {
        sendMail("BROKEN_CAME",aDP,aTime,message);        
      }
    }
  }
  // store all alarms if changed
  if (changed) {
    storeAlarms();
  }
  occupied = false;
}

void resetTriggered(string dp1, dyn_string aDPList,
                    string dp2, dyn_int aStateList,
                    string dp3, dyn_string aMsgList) {
    while (occupied) {
      delay(0,25);    
    }
    occupied=true;
    
    if (bDebug) DebugTN("monitorAlarms.ctl:resetTriggered|resetList received: "+ aDPList);
    bool changed = false;
    int iPos=-1;
    time aTime     = getCurrentTime();    
    // we have to loop through the supplied dplists,
    // we need to check if the new state is an alarm,
    // if it is an alarm (40-56)
    // check if available in alarm list, if not, add it.
    // if it is, and it differs from the old, update the old one
    //
    // if the state is not an alarm, but the dp is in the alarm list
    // remove the alarm.
    
    // start checking over all supplied dps
    for (int i=1;i <= dynlen(aDPList); i++) {
      iPos = dynContains( g_alarms[ "DPNAME"         ], aDPList[i] );
    
      // check if state >= start alarm type states
      if (aStateList[i] >= SUSPICIOUS  ) {
        if (iPos > 0) {
          if (g_alarms["STATE"][iPos] != aStateList[i]) {
            if (g_alarms["STATE"][iPos]!=aStateList[i] || 
                g_alarms["STATUS"][iPos]!=stateToStatus(aStateList[i]) ||
                g_alarms["MESSAGE"][iPos]!=aMsgList[i] ) {
              g_alarms["TIME"][iPos] = aTime;
              g_alarms["STATE"][iPos]=aStateList[i];
              g_alarms["MESSAGE"][iPos]=aMsgList[i];
              g_alarms["STATUS"][iPos]=stateToStatus(aStateList[i]);
              // check if the alarm was a suspicious_came or an alarm_came
              // if this is true then we need to send an email to the observers
              if (aStateList[i] == SUSPICIOUS_CAME ) {
                sendMail("SUSPICIOUS_CAME",aDPList[i],aTime,aMsgList[i]);
              } else if (aStateList[i] == BROKEN_CAME) {
                sendMail("BROKEN_CAME",aDPList[i],aTime,aMsgList[i]);        
              }
              changed=true;
            }   
          }
        } else {
          iPos=dynAppend(g_alarms["DPNAME" ],aDPList[i]);
          g_alarms["TIME"][iPos] = aTime;
          g_alarms["STATE"][iPos]=aStateList[i];
          g_alarms["MESSAGE"][iPos]=aMsgList[i];
          g_alarms["STATUS"][iPos]=stateToStatus(aStateList[i]);
          // check if the alarm was a suspicious_came or an alarm_came
          // if this is true then we need to send an email to the observers
          if (aStateList[i] == SUSPICIOUS_CAME ) {
            sendMail("SUSPICIOUS_CAME",aDPList[i],aTime,aMsgList[i]);
          } else if (aStateList[i] == BROKEN_CAME) {
            sendMail("BROKEN_CAME",aDPList[i],aTime,aMsgList[i]);        
          }
          changed=true;
        }
      } else {
        // If it was in the list it can be removed now
        if (iPos > 0) {
          dynRemove(g_alarms["DPNAME" ],iPos);
          dynRemove(g_alarms["TIME"   ],iPos);
          dynRemove(g_alarms["STATE"  ],iPos);
          dynRemove(g_alarms["MESSAGE"],iPos);
          dynRemove(g_alarms["STATUS" ],iPos);
          changed=true;
        } 
      }
    }  
    
    if (changed) {
      storeAlarms();
    }
    occupied = false;  
}

void rereadTriggered(string dp1, bool aB) {
  if (bDebug) DebugTN("monitorAlarms.ctl:rereadTriggered|reread internal alarmList");
  while (occupied) {
    delay(0,25);    
  }
  occupied=true;
    
  readAlarms();
  occupied=false;
}


void storeAlarms() {
  // store alarms in database.
  if (dpExists(DPNAME_NAVIGATOR + ".alarms")) {
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Storing the alarms in db: "+g_alarms);
    setAlarms(DPNAME_NAVIGATOR + ".alarms",
              g_alarms[ "TIME"],g_alarms[ "DPNAME"   ],g_alarms[ "MESSAGE"  ],g_alarms[ "STATE"    ],g_alarms[ "STATUS"   ]);
  } else {
    DebugTN("monitorAlarms.ctl:objectStateCallback|Couldn't write alarms to navigator ");
  }
}


void readAlarms() {
  mappingClear(g_alarms);
    
  // read initial alarms from database.
  if (dpExists(DPNAME_NAVIGATOR +  ".alarms")) {
    getAlarms(DPNAME_NAVIGATOR +  ".alarms",
              g_alarms[ "TIME"],g_alarms[ "DPNAME"   ],g_alarms[ "MESSAGE"  ],g_alarms[ "STATE"    ],g_alarms[ "STATUS"   ]);
  } else {
    DebugTN("monitorAlarms.ctl:initCtrlAlarmSystem|Couldn't get alarms from navigator");
  }
}

private void sendMail(string state, string aDP,time aTime,string message) {

  int ret;
  dyn_string email_cont;
  string aS="";
  
  string t = formatTime("%c",aTime);

  email_cont[1] = "observer@astron.nl";
  email_cont[2] = "lofarsys@control.lofar";
  aS="LOFAR_ALARM "+dpSubStr(aDP,DPSUB_SYS)+" "+message+" "+state;
  email_cont[3] = aS;
  aS="This is a generated message, replying is useless.\n\n New alarm from LOFAR: \n\n time     : "+t+"\n status   : "+state+"\n datapoint: "+aDP+"\n message  : "+message; 
  email_cont[4] = aS;
  

  // sending the message
  if (bDebug) DebugTN("sending msg to observer: ");
  emSendMail ("smtp.lofar.eu","mcu001.control.lofar", email_cont, ret);
  if (bDebug) DebugTN("SendMail return value: "+ret);
  
}
