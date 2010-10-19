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

bool bDebug=true;

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

  // read initial alarms from database.
  if (dpExists(DPNAME_NAVIGATOR +  ".alarms")) {
    getAlarms(DPNAME_NAVIGATOR +  ".alarms",
              g_alarms[ "TIME"],g_alarms[ "DPNAME"   ],g_alarms[ "MESSAGE"  ],g_alarms[ "STATE"    ],g_alarms[ "STATUS"   ]);
  } else {
    DebugTN("monitorAlarms.ctl:initCtrlAlarmSystem|Couldn't get alarms from navigator");
  }
  
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
  }
  
  // Connect to the dp elements that we use to receive
  // a new claim in the MainDB
  dpQueryConnectAll( "objectStateCallback",true,"objectState",query,20); 
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
  int iPos;
  
  if (bDebug) DebugTN( "monitorAlarms.ctl:objectStateCallback| Number of states in callback = " + dynlen( aResult ) );
  if (bDebug) DebugTN("Result: "+ aResult);
  
  if (dynlen(aResult) <= 0) {
    return;
  } 
  
  if (dynlen(aResult) > 2) {
    DebugTN("ERROR: More results found, software handles only 1 now!!!!!!!!");
  }
  
  //get the stateNr and DP that go with this message
  time aTime     = getCurrentTime();
  string aDP     = aResult[2][2];
  int state      = (int)aResult[3][2];
  string message = aResult[4][2];
  bool force     = aResult[5][2];
  int aStatus    = CAME;
  
  // Is this an existing Timestamp DP state combo or a new one
  iPos = dynContains( g_alarms[ "DPNAME"         ], aDP );
  

  // if state < 40 we might need to remove an ACK'ed alarm 
  // if status != ACK then we need to keep it and let ACK command remove it
  // otherwise it can't be an alarm, so return.
  if (state < 40)  {
    bool changed = false;
    if (iPos > 0 && g_alarms["STATUS"][iPos] == ACK) {
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
         
    return;
  }
    
  if( iPos < 1 ){
    dynAppend( g_alarms[ "DPNAME"          ], aDP );
    iPos = dynlen( g_alarms[ "DPNAME" ] );
    if (state == BROKEN) aStatus=ACK;
    if (state == BROKEN_WENT) aStatus=WENT;
    if (state == BROKEN_CAME) aStatus=CAME;
    if (state == SUSPICIOUS) aStatus=ACK;
    if (state == SUSPICIOUS_WENT) aStatus=WENT;
    if (state == SUSPICIOUS_CAME) aStatus=CAME;
    
  } else {
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
          if (bDebug) DebugTNTN("monitorAlarms.ctl:objectStateCallback|Someone wants to set : "+ aDP+ " state from BROKEN to BROKEN_WENT again, that should not be possible");
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
    
  
  // fill initial alarms from database.
  if (dpExists(DPNAME_NAVIGATOR + ".alarms")) {
    if (bDebug) DebugTN("monitorAlarms.ctl:objectStateCallback|Storing the alarms in db");
    setAlarms(DPNAME_NAVIGATOR + ".alarms",
              g_alarms[ "TIME"],g_alarms[ "DPNAME"   ],g_alarms[ "MESSAGE"  ],g_alarms[ "STATE"    ],g_alarms[ "STATUS"   ]);
  } else {
    DebugTN("monitorAlarms.ctl:objectStateCallback|Couldn't write alarms to navigator ");
  }
}
