// Station_Processes.ctl
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
// global functions for the mainProcesses panel
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// Station_Processes_initList                        : prepares the processeslist
// Station_Processes_UpdateStationControllers        : updates all stationdb controllers depending on observations
// Station_Processes_UpdateProcessesList             : prepares an updated treelist for the selectionboxes

#uses "GCFLogging.ctl"
#uses "GCFCommon.ctl"

global dyn_string station_result;
global string station_selectedObservation    = "";
global string station_selectedStation        = "";
global string station_obsBaseDP              = "";


// ****************************************
// Name : Station_Processes_initList
// ****************************************
// Description:  
//   prepares the processes list
//  
// ***************************************
bool Station_Processes_initList() {
  station_selectedObservation="";
  station_selectedStation=dpSubStr(g_currentDatapoint,DPSUB_SYS);
  station_obsBaseDP="";
  
  dynClear(station_result);
  
  int z;
  dyn_dyn_anytype tab;
  //PermSW + PermSW_Daemons
  dpQuery("SELECT '_original.._value' FROM 'LOFAR_PermSW_*.status.state' REMOTE '" +station_selectedStation + "'", tab);
  LOG_TRACE("Station_Processes.ctl:initList|Found: "+ tab);
  

  dyn_string aDS=navFunct_getDynString(tab, 2,1);
  dynSortAsc(aDS);
  
  for(z=1;z<=dynlen(aDS);z++){
    
    // strip .status.state from station_result
    string aS = dpSubStr(aDS[z],DPSUB_SYS_DP);

    // keep Path to work with
    string path=aS;
    

    // strip all including PermsSW out of the string
    strreplace(aS,syst+dpSubStr(baseDP,DPSUB_DP)+"_PermSW_","");


    // Remainder should be PermsSW Programs + Daemons  split on _ 
    dyn_string spl=strsplit(aS,"_");
    if (dynlen(spl) > 1) { // Daemon
      station_result[z]=navFunct_dpStripLastElement(path)+","+spl[2]+","+path;
    } else {   // Program
      station_result[z]=","+spl[1]+","+path;
    }
  }
 
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".processesList",station_result);
  
  // trigger that the panel values are calculated and ready
  navPanel_setEvent("Station_Processes.ctl:initList","Update");
  return true;
}

// ****************************************
// Name : Station_Processes_UpdateStationControllers()
// ****************************************
// Description:  
//   sets station_selectedStation and looks if the StationControllers need
//   to be updated.
//  
// ***************************************
bool Station_Processes_UpdateStationControllers() {
  string newSelectedObservation=activeStationObs.getText(activeStationObs.selectedItem(),0);
  string newSelectedStation=stationTree.getText(stationTree.selectedItem(),0);

  stationDBName.text(station_selectedStation);

  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","BeamCtrlPanel",
        DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(station_obsBaseDP,station_selectedStation));
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","CalCtrlPanel",
        DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(station_obsBaseDP,station_selectedStation));
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","TBBCtrlPanel",
        DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(station_obsBaseDP,station_selectedStation));
    
  if (!Station_Processes_UpdateProcessesList()) {
    LOG_ERROR("Station_Processes.ctl:UpdateStationControllers|UpdateProcessesList returned false");
    return false;
  }
  return true;
}

// ****************************************
// Name : Station_Processes_UpdateProcessesList()
// ****************************************
// Description:  
//   takes the general station_results and adds the (changed) 
//   observation and station ctrl'ers
//  
// ***************************************
bool Station_Processes_UpdateProcessesList() {
  dyn_string list;
  
  // copy old station_results from rest of the panel to the new list
  list=station_result;
  
  int z;
  int idx=dynlen(list)+1;  // start index for adding new station_results
  dyn_dyn_anytype tab;

  // if an observation is chosen
  if(station_selectedObservation != "") {
    // get the real name from the selected Observation
    string obsDP=claimManager_nameToRealName("LOFAR_ObsSW_"+station_selectedObservation);
    obsDP=station_selectedStation+dpSubStr(obsDP,DPSUB_DP); 
    
    // add Observation 
    list[idx++]=","+station_selectedObservation+","+obsDP;

    //select all Ctrl under Station:LOFAR_PermSW_'station_selectedObservation'
    string query="SELECT '_original.._value' FROM '"+obsDP+"_*.status.state' REMOTE '"+station_selectedStation+"'";
    dpQuery(query, tab);
    LOG_TRACE("Station_Processes.ctl:updateProcessesList|Station Controllers Found: "+ tab);
      
    dyn_string aDS=navFunct_getDynString(tab, 2,1);
    dynSortAsc(aDS);
    for(z=1;z<=dynlen(aDS);z++){
    
      // strip .status.state from station_result
      string aS = dpSubStr(aDS[z],DPSUB_SYS_DP);

      // keep Path to work with
      string path=aS;
    
      
      // strip all including Observation out of the string
      strreplace(aS,obsDP+"_","");

      // Remainder should be Ctrl Programs, split on _ 
      dyn_string spl=strsplit(aS,"_");
      if (dynlen(spl) > 1) { // low level Ctrl
        list[idx++]=navFunct_dpStripLastElement(path)+","+spl[2]+","+path;
      } else {   // Ctrl
        list[idx++]=obsDP+","+spl[1]+","+path;
      }
    }
    
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".processesList",list);
    
    // trigger that the panel values are calculated and ready
    navPanel_setEvent("Station_Processes.ctl:updateProcessesList","Update");
  }
  return true;
} 

Station_Processes_UpdateStationTree() {

  // empty the table
  stationTree.clear();
  
  
  LOG_DEBUG("Station_Processes.ctl:Station_Processes_UpdateStationTree|Found station_selectedObservation: "+station_selectedObservation);
  if (station_selectedObservation == "") {
    return;
  }
  
  LOG_DEBUG("Station_Processes.ctl:Station_Processes_UpdateStationTree|Found station_obsBaseDP: "+station_obsBaseDP);
  if (station_obsBaseDP != "") {
    // look if that name is available in the Observation List
    int j = dynContains(g_observations["DP"],station_obsBaseDP);
    if ( j > 0) {
      // get the Stationlist from that observation
      string sts=g_observations["STATIONLIST"][j];
      LOG_DEBUG("Station_Processes.ctl:Station_Processes_UpdateStationTree|Found Stationlist for this Observation: "+ sts);
      // add stations if not allready there
      dyn_string stations = strsplit(sts,",");
      for (int k=1; k<= dynlen(stations);k++) {
        if (!stationTree.itemExists(stations[k])) {
          stationTree.appendItem("",stations[k],stations[k]);
          stationTree.ensureItemVisible(stations[k]);
          stationTree.setIcon(stations[k],0,"16_empty.gif");
          if (k==1) {
            stationTree.setSelectedItem(stations[k],true);
          }
        }
      }
      if (!Station_Processes_UpdateStationControllers()) {
        LOG_ERROR("Station_Processes.ctl:UpdateStationTree|UpdateStationControllers returned false");
        return false;
      }
    }
  }
}


  
