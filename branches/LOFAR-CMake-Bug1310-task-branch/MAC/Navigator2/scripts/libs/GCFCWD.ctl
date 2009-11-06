//# GCFWatchDog
//#
//#  Copyright (C) 2002-2004
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
//#  $Id:$

//#
//# Connection watchdog
//# 

// Functions:
// Main                        - starts the watchdog and connects to the distmanager DP
// distSystemChanged 	       - Eventhandle when a system goes on or offline
// fillWatchDog                - helper function to fill the lists into the watchdog point  


#uses "GCFCommon.ctl"

const string CWD_DP = "__gcf_cwd";
global mapping g_connections;


////////////////////////////////////////////////////////////////////////////////
//
// Function main
//
// Starts the ConnectionWatchDog that monitor the (dis)connecting of systems.
// in __gcf_cwd, 5 dynamic lists are maintained that keep track of each system
// that is on or off line. The systemID is kept, a boolean that indicates if
// a system is on or offline, and the last time up and last time down is kept
// to be able to do time based restoration later.
//
////////////////////////////////////////////////////////////////////////////////
GCFCWD_Init() {
  LOG_DEBUG("GCFCWD.ctl:main|Starting GCF connection watch-dog");
        
  // init the mapping
  g_connections[ "SYSTEM"   ] = makeDynInt();
  g_connections[ "NAME"     ] = makeDynString();
  g_connections[ "UP"       ] = makeDynBool();
  g_connections[ "DOWNTIME" ] = makeDynTime();
  g_connections[ "UPTIME"   ] = makeDynTime();
  
  // retrieve old settings
  
  dpConnect("GCFCWD_connectWD", TRUE, CWD_DP+".systemID",
                                      CWD_DP+".name",
                                      CWD_DP+".online",
                                      CWD_DP+".lastUptime",
                                      CWD_DP+".lastDowntime");


  LOG_DEBUG("GCFCWD.ctl:GCFCWD_Init|Watch-dog started");
} 

////////////////////////////////////////////////////////////////////////////////
//
// EventHandler GCFCWD_connectWD
//
////////////////////////////////////////////////////////////////////////////////
void GCFCWD_connectWD(string dp1, dyn_int systemID,
                      string dp2, dyn_string name,
                      string dp3, dyn_bool up,
                      string dp4, dyn_time upTime,
                      string dp5, dyn_time downTime) {
  LOG_DEBUG("GCFCWD.ctl:GCFCWD_connectWD|watchdog triggered systemchange");
  LOG_DEBUG("GCFCWD.ctl:GCFCWD_connectWD|systemID: ",systemID);	
  LOG_DEBUG("GCFCWD.ctl:GCFCWD_connectWD|name: ",name);	
  LOG_DEBUG("GCFCWD.ctl:GCFCWD_connectWD|up: ",up);	
  LOG_DEBUG("GCFCWD.ctl:GCFCWD_connectWD|upTime: ",upTime);	
  LOG_DEBUG("GCFCWD.ctl:GCFCWD_connectWD|downTime: ",downTime);	
       
  int  iPos;
  // check all current systems and update mapping
  for (int i = 1; i <= dynlen(systemID); i++) {
    iPos=dynContains ( g_connections[ "SYSTEM" ],systemID[i]);

    // if the system was not yet available just add
    if (iPos < 1) {
      dynAppend( g_connections[ "SYSTEM" ],systemID[i]);
      iPos=dynlen(g_connections[ "SYSTEM" ]);
    }  
    // now store the values    
    g_connections[ "NAME" ][iPos]     = name[i];
    g_connections[ "UP" ][iPos]       = up[i];
    g_connections[ "UPTIME" ][iPos]   = upTime[i];
    g_connections[ "DOWNTIME" ][iPos] = downTime[i];
  }
}
