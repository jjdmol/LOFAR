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
//#  $Id$
#uses "nav_fw/gcf-util.ctl"


// This script needs to run on every CSU, CCU and MainCU
// it monitors the state changes in the database, and will update the childstates accordingly
// 
main () {

  // Set the global statecolors/colornames.
  stateColor = makeDynString("Lofar_off","Lofar_operational","Lofar_maintenance","Lofar_test","Lofar_suspicious","Lofar_broken");
  stateName = makeDynString("off","operational","maintenance","test","suspicious","broken");
  subscribePSStateChange();
}


///////////////////////////////////////////////////////////////////////////
//Function subscribePSStateChange
// 
// subscribes to the __pa_PSState DP of the database to monitor 
// possible stateChanges
//
// Added 26-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void subscribePSStateChange() {

  // ROutine to connnect to the __pa_PSState point to trigger statechanges
  // So that chilcState points can bes set/reset accordingly.

  dpConnect("PSStateTriggered",true,"__pa_PSState.");
  
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
// basicly means that: 

// 
// 2b discussed.....  what and how to do with rings & stations on MCU...
// We probably will have to think about something smart with the Station points
// in the MainCU... (not sure if they can be reached from here...)
// Maybe an extra point in de stations can be made where the trigger for the maincu will be placed
// when the mainCU connects to this one only and just sets the state/childstate all should be in control again.
// And the same subroutines can be used all over.  However, if we connect there we have to know what to do with 
// vanishing systems, and connecting systems.... In general the MainCU's database will contain all Station Databases
// that ever connected to it, so dpAccessible could be used to check if a database is online atm when needed.
// and query will only return values from connected systems.  will dpQueryConnectAll however also still monitors a system that
// has been offline and came back on again ?
// Also Observations will be added and removed on a daily basis. all names unique...   what to do there?
//
// Msg something like: Station:Dp.state=state
//                e.g. CS010:LOFAR_ObsSW_Oservation10.state=broken
//
// Points of interrest:
//    StnObservation  --> Station --> MCU*:LOFAR_ObsSW_Observation*_*_CS010.state/childState
//    StnPermSW       --> Station --> MCU*:LOFAR_PermSW_*_CS010.state/childState
//    StnPic          --> Station --> MCU*:LOFAR_PIC_*_CS010.state/childState
//
// Added 26-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void connectMainPoints() {

  // To be defined/discussed

}


///////////////////////////////////////////////////////////////////////////
//Function PSStateTriggered
// 
// Callback where a trigger of __pa_PSState is handled.
//
// Added 26-3-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
void PSStateTriggered(string dp1, string trigger) {
  // __ps_PSState change.
  // This point should have points like:
  // LOFAR_PIC_Cabinet0_Subrack0_RSPBoard0_RCU0.state=bad
  // This State should be set to childState=bad in all upper (LOFAR-PIC_Cabinet0_Subrack0_RSPBoard0) treenodes
  // ofcourse we need to monitor if this statechange is allowed.


  LOG_INFO("Entered StateChangeTriggered with trigger:", trigger);

  int i,j;
  string state    = "";
  string datapoint = "";
  string element   = "";
  
  dyn_string vals= strsplit(trigger,"=");
	
  if (dynlen(vals) >1 ) {	
    state = vals[2];
    // strip element name and datapoint
    dyn_string aS=strsplit(vals[1],".");
    if (dynlen(aS) > 0 ) {	
     datapoint = aS[1];
    }
    if (dynlen(aS) > 1 ) {	
     element = aS[2];
    }


    LOG_INFO("state:  ",state," DP: " ,datapoint, " Element: ",element);
    // if all needed values are available we can start doing the major update.
    if (state != "" != "" & datapoint != "" & element != "" & dpExists(datapoint+"."+element)){

      // If element is state just set the state (might have been done by the callerprocess, 
      // we still need to discuss this. When this is done we have to strip the last treenode, 
      // and start updating the childStates for the remaining nodes. If the element is childState,
      // only the childState updates needs 2b done.
      if (element == "state") {
        // set the state value of this dp, to avoid unneed update chains, check the value. If it
        // is the same don't update it
        int aVal;
        dpGet(datapoint+".state",aVal);
        if (aVal != getStateNumber(state) & getStateNumber(state) > -1) {
          dpSet(datapoint+".state",getStateNumber(state));
            
          //strip the last _xxx from the datapoint
          dyn_string points=strsplit(datapoint,"_");
          datapoint="";
          for (int j=1; j < dynlen(points); j++ ) {
            if (j>1) datapoint+="_";
            datapoint+=points[j];
          }
        } else {
          return;
        }

      } else if (element != "childState") {
        LOG_ERROR("Error. unknown element in stateChange trigger: ", element);
        return;
      }

      //set childState if needed, if succeeded set childState for one level less also
      if ( setChildState(datapoint,getStateNumber(state))) {

        //strip the last _xxx from the datapoint
        dyn_string points=strsplit(datapoint,"_");
        datapoint="";
        for (int j=1; j < dynlen(points); j++ ) {
          if (j>1) datapoint+="_";
          datapoint+=points[j];
        }
        if (datapoint != "" & state > -1) {
          dpSet("__pa_PSState.",datapoint+".childState="+state);
        }
          
      } else {
        LOG_INFO("SetChildState not done");
      }

    } else {
      LOG_ERROR("result: not complete command, or database could not be found.");
    }
  }
}


bool setChildState(string Dp,int state) {
//
// set childState to the MAX value of the given state and all the states and childState's of it's direct children.
//
  

  LOG_INFO("setChildState reached with: ",Dp," stateVal: ", state);

  if (state < 0 || Dp == "") return false;

  dyn_dyn_anytype tab;
  int z;
  int aVal;

  // take present value for comparing later.
  dpGet(Dp+".childState",aVal);
  LOG_INFO(Dp+".childState = "+aVal);


  // Query for all state and childState in the tree from this datapoint down.
  // Only the first level will be examined.
  // By sorting descending we can shorten the time needed for evaluation. If the top element's state value = lower or equal
  // to the value we need to set, we can leave the loop and set the new state value. In case it is bigger we can take that value
  // and set it as the new value (if > the original value)
  string query = "SELECT '_original.._value' FROM '{"+Dp+"_*.childState,"+Dp+"_*.state}' SORT BY 1 DESC";
  dpQuery(query, tab);
 
  dyn_string aS=strsplit(Dp,"_");
  int maxElements= dynlen(aS)+1; 
  int foundElements=0;

  for(z=2;z<=dynlen(tab);z++) {
    dyn_string aStr=strsplit((string)tab[z][1],"_");
    foundElements=dynlen(aStr);
    if(foundElements <= maxElements) {
      LOG_INFO("Have to check DP: ",tab[z][1], " state: ", tab[z][2]);

      // check if found state > new state
      if ((int)tab[z][2] > state) {
        state=(int)tab[z][2];
      }
      // check if state != oldVal
      if (state != aVal & state > -1 ) {
        LOG_INFO("state not equal oldstate(",aVal,") so set ",Dp+".childState to: ",state);
        dpSet(Dp+".childState",state);
        return true;
      }
      return false;
    }
  }
  return false;
}