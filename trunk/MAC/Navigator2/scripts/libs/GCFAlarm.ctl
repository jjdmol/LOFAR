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




///////////////////////////////////////////////////////////////////////////
//
// Function initAlarmSystem
//
// Initializes the AlarmSystem, connect to distmanager to be able to check
// if systems came or went ofline. If came or went do a queryconnect again on all
// NCFObjectStates to be able to check if state changes were issued.
// 
///////////////////////////////////////////////////////////////////////////
void initAlarmSystem()  {
  LOG_TRACE("GCFAlarm.ctl:initAlarmSystem|entered");
  g_alarms[ "TIME"     ] = makeDynTime();                    
  g_alarms[ "DPNAME"   ] = makeDynString();
  g_alarms[ "MESSAGE"  ] = makeDynString();
  g_alarms[ "STATE"    ] = makeDynInt();
  g_alarms[ "STATUS"   ] = makeDynInt();
  
  // Routine to connect to _DistConnections.ManNums.
 	// This point keeps a dyn_int array with all active distributed connections
 	// and will generate a callback everytime a station goes off-, or on- line

  // read initial alarms from database.
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".alarms")) {
    getAlarms(DPNAME_NAVIGATOR + g_navigatorID + ".alarms",
              g_alarms[ "TIME"],g_alarms[ "DPNAME"   ],g_alarms[ "MESSAGE"  ],g_alarms[ "STATE"    ],g_alarms[ "STATUS"   ]);
  } else {
    LOG_ERROR("GCFAlarm.ctl:initAlarmSystem|Couldn't get alarms from navigator instance");
  }
  
  if (dpExists("_DistConnections.Dist.ManNums")) {
    dpConnect("distSystemTriggered",true,"_DistConnections.Dist.ManNums");
  } else {
    LOG_DEBUG("GCFAlarm.ctl:initAlarmSystem|_DistConnections point not found, no trigger available for dist System updates.");  
  } 
  LOG_DEBUG("GCFAlarm.ctl:initAlarmSystem|Nr alarms in global after init: "+ dynlen(g_alarms["TIME"]));      
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
  if (dpGet(dp+".time",times,dp+".datapoint",names,dp+".message",messages,dp+".state",state,dp+".status",status)) {
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
  
  if (dpSet(dp+".time",times,dp+".datapoint",names,dp+".message",messages,dp+".state",state,dp+".status",status)) {
    LOG_DEBUG("GCFAlarm.ctl:setAlarms|Error setting alarms in database: ", getLastError());  
  }
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
  
  string query = "SELECT '_original.._value' FROM '{__navObjectState.DPName,__navObjectState.stateNr,__navObjectState.message}' REMOTE ALL WHERE _DPT = \"NCFObjectState\" SORT BY 0";
  
  
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
  
  LOG_DEBUG( "GCFAlarm.ctl:objectStateCallback| Number of states in callback = " + dynlen( aResult ) );
  LOG_DEBUG("Result: "+ aResult);
  
  if (dynlen(aResult) <= 0) {
    return;
  }
  //get the stateNr and DP that go with this message This will need to be done in one call later though!!
  time aTime     = getCurrentTime();
  string aDP     = aResult[2][2];
  int state      = (int)aResult[3][2];
  string message = aResult[4][2];
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
      if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".alarms")) {
        setAlarms(DPNAME_NAVIGATOR + g_navigatorID + ".alarms",
                  g_alarms[ "TIME"],g_alarms[ "DPNAME"   ],g_alarms[ "MESSAGE"  ],g_alarms[ "STATE"    ],g_alarms[ "STATUS"   ]);
      } else {
        LOG_ERROR("GCFAlarm.ctl:objectStateCallback|Couldn't write alarms to navigator instance");
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
          LOG_ERROR("GCFAlarm.ctl:objectStateCallback|Someone wants to set : "+ aDP+ " state from BROKEN to BROKEN_WENT again, that should not be possible");
          aStatus = WENT;
        }
      } else if (oldState  == BROKEN_WENT) {
        if (state == BROKEN) {
          aStatus = ACK;
        } else if (state == BROKEN_WENT) {
          aStatus = WENT;
        } else {
          LOG_ERROR("GCFAlarm.ctl:objectStateCallback|Someone wants to set : "+ aDP+ " state from BROKEN_WENT to BROKEN_CAME again, leaving last WENT not ACK'ed");
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
          LOG_ERROR("GCFAlarm.ctl:objectStateCallback|Someone wants to set : "+ aDP+ " state from SUSPICIOUS to SUSPICIOUS_WENT again, that should not be possible");
          aStatus = WENT;
        }
      } else if (oldState  == SUSPICIOUS_WENT) {
        if (state == SUSPICIOUS) {
          aStatus = ACK;
        } else if (state == SUSPICIOUS_WENT) {
          aStatus = WENT;
        } else {
          LOG_ERROR("GCFAlarm.ctl:objectStateCallback|Someone wants to set : "+ aDP+ " state from SUSPICIOUS_WENT to SUSPICIOUS_CAME again, leaving last WENT not ACK'ed");
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
  LOG_DEBUG("GCFAlarm.ctl:objectStateCallback|Found: Datapoint - "+aDP);
  LOG_DEBUG("GCFAlarm.ctl:objectStateCallback|Found: Time      - "+aTime);
  LOG_DEBUG("GCFAlarm.ctl:objectStateCallback|Found: State     - "+state);
  LOG_DEBUG("GCFAlarm.ctl:objectStateCallback|Found: Message   - "+message);
  LOG_DEBUG("GCFAlarm.ctl:objectStateCallback|Found: Status    - "+aStatus);
  // Now store the values 
  g_alarms[ "TIME"    ][iPos] = aTime;
  g_alarms[ "STATE"   ][iPos] = state;
  g_alarms[ "MESSAGE" ][iPos] = message;
  g_alarms[ "STATUS"  ][iPos] = aStatus;
    
  
  // fill initial alarms from database.
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".alarms")) {
    LOG_DEBUG("GCFAlarm.ctl:objectStateCallback|Storing the alarsm in db");
    setAlarms(DPNAME_NAVIGATOR + g_navigatorID + ".alarms",
              g_alarms[ "TIME"],g_alarms[ "DPNAME"   ],g_alarms[ "MESSAGE"  ],g_alarms[ "STATE"    ],g_alarms[ "STATUS"   ]);
  } else {
    LOG_ERROR("GCFAlarm.ctl:objectStateCallback|Couldn't write alarms to navigator instance");
  }
}

