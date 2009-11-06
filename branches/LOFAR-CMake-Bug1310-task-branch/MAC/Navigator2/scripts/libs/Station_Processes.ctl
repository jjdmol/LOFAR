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
#uses "MainCU_Processes.ctl"

global dyn_string station_result;
global dyn_string station_procList;
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
  station_selectedObservation=selectedObservation;
  station_selectedStation=syst;
  station_obsBaseDP="";
  
  
  dynClear(station_result);
  dynClear(station_procList);
  
  int z;
  dyn_dyn_anytype tab;
  //PermSW + PermSW_Daemons
  string query="SELECT '_original.._value' FROM 'LOFAR_PermSW_*.status.state' REMOTE '" +station_selectedStation + "'";
  LOG_TRACE("Station_Processes.ctl:initList|Query: "+ query);
  
  dpQuery(query, tab);
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
      dynAppend(station_result,navFunct_dpStripLastElement(path)+","+spl[2]+","+path);
      dynAppend(station_procList,path);
    } else {   // Program
      dynAppend(station_result,","+spl[1]+","+path);
      if (spl[1] != "Daemons") {
        dynAppend(station_procList,path);
      }
    }
  }
  LOG_TRACE("Station_Processes.ctl:initList|station_result composed: "+ station_result);
  
  if (!dpExists(MainDBName+"LOFAR_PermSW_MACScheduler.activeObservations")) {
    setValue("activeStationObs","backCol","_dpdoesnotexist");
  } else {
    dpConnect("Station_Processes_ActiveStationObsCallback",true,"LOFAR_PermSW_MACScheduler.activeObservations");
  }

 
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".processesList",station_result);
  
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
  string newSelectedStation=stationTree.getText(stationTree.selectedItem(),0);
  LOG_TRACE("Station_Processes.ctl:updateStationControllers|selected station: "+ station_selectedStation +" New: "+ newSelectedStation);

  // check if selection is made, and the selection is indeed a new one
  if (newSelectedStation != 0) {
    station_selectedStation = newSelectedStation;
    stationDBName.text(station_selectedStation);
    
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","BeamCtrlPanel",
          DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(station_obsBaseDP,station_selectedStation));
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","CalCtrlPanel",
          DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(station_obsBaseDP,station_selectedStation));
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","TBBCtrlPanel",
          DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(station_obsBaseDP,station_selectedStation));
  }
    
  LOG_TRACE("Station_Processes.ctl:UpdateStationControllers|call UpdateProcessesList");
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
  LOG_TRACE("Station_Processes.ctl:updateProcessesList|entered selected observation: "+ station_selectedObservation);
  dyn_string list;
  
  // copy old station_results from rest of the panel to the new list
  list=station_result;
  g_processesList=station_procList;
  
  int z;
  dyn_dyn_anytype tab;

  // if an observation is chosen
  if(station_selectedObservation != "") {
    // get the real name from the selected Observation
    string obsDP=claimManager_nameToRealName("LOFAR_ObsSW_"+station_selectedObservation);
    
    if (strtok(station_selectedStation,":") < 0) {      
      obsDP=station_selectedStation+":"+dpSubStr(obsDP,DPSUB_DP);
    } else { 
      obsDP=station_selectedStation+dpSubStr(obsDP,DPSUB_DP);
    }    
    // add Observation 
    dynAppend(list,","+station_selectedObservation+","+obsDP);

    //select all Ctrl under Station:LOFAR_PermSW_'station_selectedObservation'
    string query="SELECT '_original.._value' FROM '"+obsDP+"_*.status.state' REMOTE '"+station_selectedStation+"'";
    LOG_DEBUG("Station_Processes.ctl:updateProcessesList|Query: "+ query);
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
        dynAppend(list,navFunct_dpStripLastElement(path)+","+spl[2]+","+path);
        dynAppend(g_processesList,path);
      } else {   // Ctrl
        dynAppend(list,obsDP+","+spl[1]+","+path);
        if (spl[1] != "OnlineCtrl") {
          dynAppend(g_processesList,path);
        }
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
  if (dpExists(station_obsBaseDP)) {
    // look if that name is available in the Observation List
    int j = dynContains(g_observations["DP"],station_obsBaseDP);
    if ( j > 0) {
      // get the Stationlist from that observation
      string sts=g_observations["STATIONLIST"][j];
      LOG_DEBUG("Station_Processes.ctl:Station_Processes_UpdateStationTree|Found Stationlist for this Observation: "+ sts);
      // add stations if not allready there
      dyn_string stations = navFunct_listToDynString(sts);
      for (int k=1; k<= dynlen(stations);k++) {
        if (!stationTree.itemExists(stations[k])) {
          stationTree.appendItem("",stations[k],stations[k]);
          stationTree.ensureItemVisible(stations[k]);
          stationTree.setIcon(stations[k],0,"16_empty.gif");

          if (stations[k] == selectedStation) {
            stationTree.setSelectedItem(stations[k],true);
            station_selectedStation=stations[k];
          }
        }
      }
      LOG_TRACE("Station_Processes.ctl:updateStationTree|calling UpdateStationControllers. selected ststion: "+station_selectedStation);
      if (!Station_Processes_UpdateStationControllers()) {
        LOG_ERROR("Station_Processes.ctl:UpdateStationTree|UpdateStationControllers returned false");
        return false;
      }
    }
  }
}



Station_Processes_ActiveStationObsCallback(string dp1, dyn_string activeObservations) {
  LOG_TRACE("Station_Processes.ctl:activeObsCallback|Found: "+ activeObservations);
  
  // wait a few milisecs to give the g_observationlist time to refill
  delay(0,500);  // empty the table
  
    // empty the table
  activeStationObs.clear();
  station_obsBaseDP="";  
  
  // if the active observations list changes the list here should be changed also.
  // iterate over the found entries and fill the table
  // if no previous Observation selected, set selection to the first on the list, also
  // set it here when previous selection disappeared from the list.
  // otherwise nothing will be changed in the selection
  
  // check if this station is involved in an active observation, if so it will be added to the list

  string newSelection="";
  string oldSelection =activeStationObs.selectedItem();
  if (oldSelection != "") {
    station_selectedObservation=activeStationObs.getText(activeStationObs.selectedItem(),0); 
  }
  LOG_DEBUG("Station_Processes.ctl:activeObsCallback|oldSelection: "+oldSelection+ " station_selectedObservation: "+station_selectedObservation);
   
  bool first=true;
  for (int i=1; i<= dynlen(activeObservations);i++) {
    LOG_DEBUG("Station_Processes.ctl:activeObsCallback|checking g_observations for: "+"LOFAR_ObsSW_"+activeObservations[i]);
    string realName=claimManager_nameToRealName("LOFAR_ObsSW_"+activeObservations[i]);
    int j = dynContains(g_observations["NAME"],"LOFAR_ObsSW_"+activeObservations[i]);
    if ( j > 0) {
      LOG_DEBUG("Station_Processes.ctl:activeObsCallback|checking stationList for: "+station_selectedStation);      
      // get the Stationlist from that observation
      string sts=g_observations["STATIONLIST"][j];
      LOG_DEBUG("Station_Processes.ctl:activeObsCallback|Stations found: "+sts);
      dyn_string stations = navFunct_listToDynString(sts);
      for (int k=1; k<= dynlen(stations);k++) { 
        if (stations[k] == station_selectedStation) {
          if (station_obsBaseDP == "") {
            station_obsBaseDP=realName;
          }
          activeStationObs.appendItem("",realName,activeObservations[i]);
          activeStationObs.ensureItemVisible(realName);
          activeStationObs.setIcon(realName,0,"16_empty.gif");
          if (station_selectedObservation == activeObservations[i]) {
            newSelection=realName;
            station_obsBaseDP=realName;
            first=false;
          }
          if (first) {
            newSelection=realName;
            station_obsBaseDP=realName;
            first=false;
          }
        }
      }
    }
  }
  
  LOG_DEBUG("Station_Processes.ctl:activeObsCallback|oldSelection: "+oldSelection+" newSelection: "+newSelection);
  
  if ((oldSelection == newSelection) ||
      (oldSelection != "" && activeStationObs.itemExists(oldSelection))) {
    activeStationObs.setSelectedItem(oldSelection,true);    
    station_selectedObservation=activeStationObs.getText(activeStationObs.selectedItem(),0); 
    LOG_DEBUG("Station_Processes.ctl:activeObsCallback|Selection: "+station_selectedObservation);
  } else {
    activeStationObs.setSelectedItem(newSelection,true);    
    station_selectedObservation=activeStationObs.getText(activeStationObs.selectedItem(),0); 
  }
  
  LOG_DEBUG("Station_Processes.ctl:activeObsCallback|Selection: "+station_selectedObservation);
  
  // something has changed, so update Main Controllers
  LOG_DEBUG("Station_Processes.ctl:activeObsCallback|Starting updateStationTree");  
  // something has changed, so update Station Tree
  Station_Processes_UpdateStationTree();
   
}

  
