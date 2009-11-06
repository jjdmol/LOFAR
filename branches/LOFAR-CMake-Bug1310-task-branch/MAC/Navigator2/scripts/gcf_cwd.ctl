//# gcf_cwd.ctl
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
// distSystemChanged 					 - Eventhandle when a system goes on or offline
// fillWatchDog                - helper function to fill the lists into the watchdog point  


#uses "GCFLogging.ctl"

const string CWD_DP = "__gcf_cwd";
global mapping g_connections;

////////////////////////////////////////////////////////////////////////////////
//
// Function main
//
// Starts the ConnectionWatchDog that monitor the (dis)connecting of systems.
// in __gcf_cwd 4 dynamic lists are maintained that keep track of each system
// that is on or off line. The systemID is kept, a boolean that indicates if
// a system is on or offline, and the last time up and last time down is kept
// to be able to do time based restoration later.
//
////////////////////////////////////////////////////////////////////////////////
main() {
 	LOG_DEBUG("gcf_cwd.ctl:main|Starting GCF connection watch-dog");
        
  // init the mapping
  g_connections[ "SYSTEM"   ] = makeDynInt();
  g_connections[ "NAME"     ] = makeDynString();
  g_connections[ "UP"       ] = makeDynBool();
  g_connections[ "DOWNTIME" ] = makeDynTime();
  g_connections[ "UPTIME"   ] = makeDynTime();
  
	// retrieve old settings
  
	fillGlobalList(g_connections[ "SYSTEM"],
                       g_connections[ "NAME"],
                       g_connections[ "UP" ], 
                       g_connections[ "DOWNTIME"], 
                       g_connections[ "UPTIME"]);
	dpConnect("distSystemChanged", TRUE, "_DistManager.State.SystemNums");

	LOG_DEBUG("gcf_cwd.ctl:main|Watch-dog started");
} 

////////////////////////////////////////////////////////////////////////////////
//
// EventHandler distSystemChanged(dp, dyn newDistSysList)
//
////////////////////////////////////////////////////////////////////////////////
void distSystemChanged(string dp, dyn_int newDistSysList) {
	LOG_DEBUG("gcf_cwd.ctl:distSystemChanged|distSystemChanged: ", dp, newDistSysList);	
       
  int  iPos;
	// check all current systems and update mapping
 	for (int i = 1; i <= dynlen(newDistSysList); i++) {
 		iPos=dynContains ( g_connections[ "SYSTEM" ],newDistSysList[i]);        
    bool new=false;

   	// if the system was not yet available just add
    if (iPos < 1) {
      dynAppend( g_connections[ "SYSTEM" ],newDistSysList[i]);
      iPos=dynlen(g_connections[ "SYSTEM" ]);
      new = true;
    }
    
    // now store the values
    g_connections[ "NAME" ][iPos]   = getSystemName(newDistSysList[i]);
    g_connections[ "UP" ][iPos]     = true;
    g_connections[ "UPTIME" ][iPos] = getCurrentTime();
 	}
        
    // now we also have to check if there were systems available b4 that are not included in the triggerlist anymore
  // because that indicates that a system did go offline

  for (int i = 1; i <= dynlen(g_connections[ "SYSTEM"]);i++) {
    if (dynContains(newDistSysList,g_connections[ "SYSTEM" ][i]) < 1) {
      g_connections[ "UP" ][i]     = false;
      g_connections[ "DOWNTIME" ][i] = getCurrentTime(); 
    }
  }

  // write configuration to datapoint
 	for (int i = 1; i <= dynlen(g_connections["SYSTEM"]); i++) {
          		fillWatchDog(g_connections[ "SYSTEM"],
                                     g_connections[ "NAME"],
                                     g_connections[ "UP" ], 
                                     g_connections[ "DOWNTIME"], 
                                     g_connections[ "UPTIME"]);
  }
}

void fillWatchDog(
  dyn_int systems, dyn_string names,dyn_bool up,
  dyn_time downtime, dyn_time uptime 
)
{
  dpSet("__gcf_cwd.systemID",systems,"__gcf_cwd.name",names,"__gcf_cwd.online",up,"__gcf_cwd.lastUptime",uptime,"__gcf_cwd.lastDowntime",downtime);
}

void fillGlobalList(
  dyn_int &systems, dyn_string &names,dyn_bool &up,
  dyn_time &downtime, dyn_time &uptime 
)
{
  dpGet("__gcf_cwd.systemID",systems,"__gcf_cwd.name",names,"__gcf_cwd.online",up,"__gcf_cwd.lastUptime",uptime,"__gcf_cwd.lastDowntime",downtime);
}   
