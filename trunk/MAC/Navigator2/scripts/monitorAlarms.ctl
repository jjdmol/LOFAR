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
  dpQueryConnectAll( "objectStateCallback",true,"objectState",query,20); 
  
  // also connect to the dpResetList dp to receive lists if dp's that need to be cleared from the global list and thus from the
  // datapoint in the database
  if (dpExists(DPNAME_NAVIGATOR +  ".alarms.dpResetList")) {
    dpConnect("resetTriggered",DPNAME_NAVIGATOR +  ".alarms.dpResetList",
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
  
  while (occupied) {
    delay(0,25);    
  }
  
  occupied = true;
  int iPos;
  
  if (bDebug) DebugTN( "monitorAlarms.ctl:objectStateCallback| Number of states in callback = " + dynlen( aResult ) );
  if (bDebug) DebugTN("Result: "+ aResult);
  
  if (dynlen(aResult) <= 0) {
    occupied = false;
    return;
  } 
  
 
  //get the stateNr and DP that go with this message
  time aTime     = getCurrentTime();
  string aDP     = aResult[2][2];
  int state      = (int)aResult[3][2];
  string message = aResult[4][2];
  bool force     = aResult[5][2];
  int aStatus    = CAME;
  
  if (aDP==""){
    if (bDebug) DebugN("Return on empty dp");
    occupied = false;
    return;
  }
  
  // Is this an existing DP or a new one
  iPos = dynContains( g_alarms[ "DPNAME"         ], aDP );

  
  if (bDebug) DebugN("iPos:" + iPos);
  if (bDebug) DebugN("dpnames in global:"+g_alarms[ "DPNAME"         ]);
  if(iPos>0 && bDebug)  DebugN("floor g_alarms: "+floor(g_alarms["STATE"][iPos]/10));
  if (bDebug) DebugN("floor state: "+floor(state/10));
  if (bDebug) DebugN("force: "+force);
  
  // check if existing dp.
  // if it exists, check if new state 1st digit > oldState 1st digit && force, otherwise return
  if (iPos > 0 && ((floor(g_alarms["STATE"][iPos]/10) >= floor(state/10))&& !force)) {
    if (bDebug) DebugN("return on condition mismatch");
    occupied = false;
    return;
  }
    
    
  // if state < 40 we might need to remove an ACK'ed or WENT ALARM
  // if status != ACK then we need to keep it and let ACK command remove it
  // otherwise it can't be an alarm, so return.
  if (state < 40)  {
    bool changed = false;
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
      // rewrite database  
      if (dpExists(DPNAME_NAVIGATOR + ".alarms")) {
        setAlarms(DPNAME_NAVIGATOR + ".alarms",
                  g_alarms[ "TIME"],g_alarms[ "DPNAME"   ],g_alarms[ "MESSAGE"  ],g_alarms[ "STATE"    ],g_alarms[ "STATUS"   ]);
      } else {
        DebugTN("monitorAlarms.ctl:objectStateCallback|Couldn't write alarms to navigator");
      }
    }
    if (bDebug) DebugN("return on state < 40");   
    occupied = false;
    return;
  }
    
  
  // in the remainder of the cases, the state was 
  if( iPos < 1 ){
    if (bDebug) DebugN("Need to append new alarm");
    dynAppend( g_alarms[ "DPNAME"          ], aDP );
    iPos = dynlen( g_alarms[ "DPNAME" ] );
    if (state == BROKEN) aStatus=ACK;
    if (state == BROKEN_WENT) aStatus=WENT;
    if (state == BROKEN_CAME) aStatus=CAME;
    if (state == SUSPICIOUS) aStatus=ACK;
    if (state == SUSPICIOUS_WENT) aStatus=WENT;
    if (state == SUSPICIOUS_CAME) aStatus=CAME;
    
  } else {
    if (bDebug) DebugN("Need to examine if alarm update is needed");
    // it was an existing DP, so we have to compare the status.
     
    int oldState=g_alarms["STATE"][iPos];

    // check broken ranges first
    if (state >= BROKEN ) {
      if (oldState == BROKEN) {
        if (state == BROKEN) {
        	aStatus = ACK;
        } else if (state == BROKEN_CAME) {
          aStatus = CAME;
        } else {
          DebugTN("monitorAlarms.ctl:objectStateCallback|Someone wants to set : "+ aDP+ " state from BROKEN to BROKEN_WENT again, that should not be possible");
          aStatus = WENT;
        }
      } else if (oldState  == BROKEN_WENT) {
        if (state == BROKEN) {
          aStatus = ACK;
        } else if (state == BROKEN_WENT) {
          aStatus = WENT;
        } else {
          DebugTN("monitorAlarms.ctl:objectStateCallback|Someone wants to set : "+ aDP+ " state from BROKEN_WENT to BROKEN_CAME again, leaving last WENT not ACK'ed");
          aStatus = CAME;
        }        
      } else if (oldState == BROKEN_CAME) {
        if (state == BROKEN) {
          aStatus = ACK;
        } else if (state == BROKEN_WENT) {
          aStatus = WENT;
        } else {
          aStatus = CAME;
        }        
  		}
   	} else if (state >= SUSPICIOUS) {
      if (oldState == SUSPICIOUS) {
        if (state == SUSPICIOUS) {
        	aStatus = ACK;
        } else if (state == SUSPICIOUS_CAME) {
          aStatus = CAME;
        } else {
          DebugTN("monitorAlarms.ctl:objectStateCallback|Someone wants to set : "+ aDP+ " state from SUSPICIOUS to SUSPICIOUS_WENT again, that should not be possible");
          aStatus = WENT;
        }
      } else if (oldState  == SUSPICIOUS_WENT) {
        if (state == SUSPICIOUS) {
          aStatus = ACK;
        } else if (state == SUSPICIOUS_WENT) {
          aStatus = WENT;
        } else {
          DebugTN("monitorAlarms.ctl:objectStateCallback|Someone wants to set : "+ aDP+ " state from SUSPICIOUS_WENT to SUSPICIOUS_CAME again, leaving last WENT not ACK'ed");
          aStatus = CAME;
        }        
      } else if (oldState == SUSPICIOUS_CAME) {
        if (state == SUSPICIOUS) {
          aStatus = ACK;
        } else if (state == SUSPICIOUS_WENT) {
          aStatus = WENT;
        } else {
          aStatus = CAME;
        }        
      }
    }
  }
  if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Datapoint - "+aDP);
  if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Time      - "+aTime);
  if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: State     - "+state);
  if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Message   - "+message);
  if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Status    - "+aStatus);
  if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Found: Force     - "+force);
  // Now store the values 
  g_alarms[ "TIME"    ][iPos] = aTime;
  g_alarms[ "STATE"   ][iPos] = state;
  g_alarms[ "MESSAGE" ][iPos] = message;
  g_alarms[ "STATUS"  ][iPos] = aStatus;
    
  
  storeAlarms();
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
            g_alarms["TIME"][iPos] = aTime;
            g_alarms["STATE"][iPos]=aStateList[i];
            g_alarms["MESSAGE"][iPos]=aMsgList[i];
            g_alarms["STATUS"][iPos]=stateToStatus(aStateList[i]);
            changed=true;
          }
        } else {
          iPos=dynAppend(g_alarms["DPNAME" ],aDPList[i]);
          g_alarms["TIME"][iPos] = aTime;
          g_alarms["STATE"][iPos]=aStateList[i];
          g_alarms["MESSAGE"][iPos]=aMsgList[i];
          g_alarms["STATUS"][iPos]=stateToStatus(aStateList[i]);
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
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Storing the alarms in db");
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
