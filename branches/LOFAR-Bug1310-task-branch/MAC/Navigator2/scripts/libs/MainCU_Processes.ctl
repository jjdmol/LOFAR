// MainCU_Processes.ctl
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
// MainCU_Processes_initList                        : prepares the processeslist
// MainCU_Processes_UpdateMainControllers           : updates all main db controllers depending on observations
// MainCU_Processes_UpdateStationControllers        : updates all stationdb controllers depending on observations
// MainCU_Processes_UpdateProcessesList             : prepares an updated treelist for the selectionboxes

#uses "GCFLogging.ctl"
#uses "GCFCommon.ctl"

global dyn_string result;
global dyn_string procList;
global string selectedObservation    = "";
global string selectedStation        = "";
global string obsBaseDP              = "";


// ****************************************
// Name : MainCU_Processes_initList
// ****************************************
// Description:  
//   prepares the processes list
//  
// ***************************************
void MainCU_Processes_initList() {
  selectedObservation="";
  selectedStation="";
  obsBaseDP="";
  
  dynClear(result);
  dynClear(procList);
  
  int z;
  dyn_dyn_anytype tab;
  //PermSW + PermSW_Daemons
  dpQuery("SELECT '_original.._value' FROM 'LOFAR_PermSW_*.status.state' ", tab);
  LOG_TRACE("MainCU_Processes.ctl:initList|Found: "+ tab);
  
  dyn_string aDS=navFunct_getDynString(tab, 2,1);
  dynSortAsc(aDS);
  
  for(z=1;z<=dynlen(aDS);z++){
    
    // strip .status.state from result
    string aS = dpSubStr(aDS[z],DPSUB_SYS_DP);

    // keep Path to work with
    string path=aS;
    

    // strip all including PermsSW out of the string
    strreplace(aS,syst+dpSubStr(baseDP,DPSUB_DP)+"_PermSW_","");


    // Remainder should be PermsSW Programs + Daemons  split on _ 
    dyn_string spl=strsplit(aS,"_");
    if (dynlen(spl) > 1) { // Daemon
      dynAppend(result,navFunct_dpStripLastElement(path)+","+spl[2]+","+path);
      dynAppend(procList,path);
    } else {   // Program
      dynAppend(result,","+spl[1]+","+path);
      if (spl[1] != "Daemons") {
        dynAppend(procList,path);
      }
    }
  }
  
  if (!dpExists(MainDBName+"LOFAR_PermSW_MACScheduler.activeObservations")) {
    setValue("activeObs","backCol","_dpdoesnotexist");
  } else {
    dpConnect("MainCU_Processes_ActiveObsCallback",true,"LOFAR_PermSW_MACScheduler.activeObservations");
  }
 
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".processesList",result);
  
}

// ****************************************
// Name : MainCU_Processes_UpdateMainControllers()
// ****************************************
// Description:  
//   sets selectedObservation and looks if the MainControllers need
//   to be updated.
//  
// ***************************************
void MainCU_Processes_UpdateMainControllers() {
  string newSelectedObservation=activeObs.getText(activeObs.selectedItem(),0);
  LOG_TRACE("MainCU_Processes.ctl:updateMainControllers|selected observation: "+ selectedObservation +" New: "+ newSelectedObservation);
  // check if selection is made, and the selection is indeed a new one
  if (newSelectedObservation != 0) {
    selectedStation="";
    selectedObservation = newSelectedObservation;
    observationName.text(selectedObservation);
    // get the real name from the selected Observation
    obsBaseDP=claimManager_nameToRealName("LOFAR_ObsSW_"+selectedObservation);   
    LOG_TRACE("MainCU_Processes.ctl:updateMainControllers|connecting to  Main Observation Ctrls");    
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","ObsCtrlPanel",
          DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(obsBaseDP));
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","OnlineCtrl_StorageApplPanel",
          DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(obsBaseDP));
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","OnlineCtrl_CorrelatorPanel",
          DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(obsBaseDP));
  }
}

// ****************************************
// Name : MainCU_Processes_UpdateStationControllers()
// ****************************************
// Description:  
//   sets selectedStation and looks if the StationControllers need
//   to be updated.
//  
// ***************************************
void MainCU_Processes_UpdateStationControllers() {
  string newSelectedStation=stationTree.getText(stationTree.selectedItem(),0);

  LOG_TRACE("MainCU_Processes.ctl:updateStationControllers|selected station: "+ selectedStation +" New: "+ newSelectedStation);
  // check if selection is made, and the selection is indeed a new one
  if (newSelectedStation != 0) {  
    selectedStation = newSelectedStation;
    stationDBName.text(selectedStation);
    MainCU_Processes_UpdateProcessesList();

    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","BeamCtrlPanel",
          DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(obsBaseDP,selectedStation));
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","CalCtrlPanel",
          DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(obsBaseDP,selectedStation));
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","TBBCtrlPanel",
          DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(obsBaseDP,selectedStation));
    
  }
}

// ****************************************
// Name : MainCU_Processes_UpdateProcessesList()
// ****************************************
// Description:  
//   takes the general results and adds the (changed) 
//   observation and station ctrl'ers
//  
// ***************************************
void MainCU_Processes_UpdateProcessesList() {
  LOG_TRACE("MainCU_Processes.ctl:updateProcessesList|entered");
  dyn_string list;
  
  // copy old results from rest of the panel to the new list
  list=result;
  g_processesList = procList;
  
  
  int z;
  dyn_dyn_anytype tab;

  // if an observation is chosen
  if(selectedObservation != "") {
    // get the real name from the selected Observation
    string obsDP=claimManager_nameToRealName("LOFAR_ObsSW_"+selectedObservation); 
    //select all Ctrl under LOFAR_PermSW_'selectedObservation'
    dpQuery("SELECT '_original.._value' FROM '"+obsDP+"_*.status.state' ", tab);
    LOG_TRACE("MainCU_Processes:updateProcessesList|MainCu controllers Found: "+ tab);
    
    dyn_string aDS=navFunct_getDynString(tab, 2,1);
    dynSortAsc(aDS);
      // create an entry for the observation
    dynAppend(list,","+selectedObservation+","+obsDP);

    for(z=1;z<=dynlen(aDS);z++){
    
      // strip .status.state from result
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
    
    //same for station controllers (check if a station exists)
    if (selectedStation != "" && dpExists(selectedStation+":LOFAR") ){
      
      // strip system and add station
      string stationObsDP=selectedStation+":"+dpSubStr(obsDP,DPSUB_DP);
      // add station to selected Observation
      dynAppend(list,obsDP+","+selectedStation+","+stationObsDP);

      //select all Ctrl under Station:LOFAR_PermSW_'selectedObservation'
      dpQuery("SELECT '_original.._value' FROM '"+stationObsDP+"_*.status.state' REMOTE '"+selectedStation+":'", tab);
      LOG_TRACE("MainCU_Processes.ctl:updateProcessesList|Station Controllers Found: "+ tab);
      
      aDS=navFunct_getDynString(tab, 2,1);
      dynSortAsc(aDS);
      for(z=1;z<=dynlen(aDS);z++){
    
        // strip .status.state from result
        string aS = dpSubStr(aDS[z],DPSUB_SYS_DP);

        // keep Path to work with
        string path=aS;
    
      
        // strip all including Observation out of the string
        strreplace(aS,stationObsDP+"_","");


        // Remainder should be Ctrl Programs, split on _ 
        dyn_string spl=strsplit(aS,"_");
        if (dynlen(spl) > 1) { // low level Ctrl
          dynAppend(list,navFunct_dpStripLastElement(path)+","+spl[2]+","+path);
          dynAppend(g_processesList,path);
        } else {   // Ctrl
          dynAppend(list,stationObsDP+","+spl[1]+","+path);
          dynAppend(g_processesList,path);
        }
      }
    }
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".processesList",list);
    
    // trigger that the panel values are calculated and ready
    navPanel_setEvent("MainCU_Processes.ctl:updateProcessesList","Update");
  }
} 

MainCU_Processes_UpdateStationTree() {

  LOG_TRACE("MainCU_Processes.ctl:updateStationTree|entered. Selected Observation: "+selectedObservation);
    // empty the table
  stationTree.clear();
  
  
  if (selectedObservation == "") {
    return;
  }
  
  LOG_DEBUG("MainCU_Processes.ctl:updateStationTree|obsBaseDP: "+obsBaseDP);
  if (dpExists(obsBaseDP)) {
    // look if that name is available in the Observation List
    int j = dynContains(g_observations["DP"],obsBaseDP);
    if ( j > 0) {
      // get the Stationlist from that observation
      string sts=g_observations["STATIONLIST"][j];
      LOG_DEBUG("MainCU_Processes.ctl:UpdateStationTree|Found Stationlist for this Observation: "+ sts);
      // add stations if not allready there

      dyn_string stations = navFunct_listToDynString(sts);

      for (int k=1; k<= dynlen(stations);k++) {
        if (!stationTree.itemExists(stations[k])) {
          stationTree.appendItem("",stations[k],stations[k]);
          stationTree.ensureItemVisible(stations[k]);
          stationTree.setIcon(stations[k],0,"16_empty.gif");
          if (k==1) {
            stationTree.setSelectedItem(stations[k],true);
            selectedStation=stations[k];
          }
        }
      }
      LOG_TRACE("MainCU_Processes.ctl:updateStationTree|calling UpdateStationControllers. selected ststion: "+selectedStation);
      MainCU_Processes_UpdateStationControllers();
    }
  }
}

MainCU_Processes_ActiveObsCallback(string dp1, dyn_string activeObservations) {
  LOG_TRACE("MainCU_Processes.ctl:activeObsCallback|Found: "+ activeObservations);

  // wait a few milisecs to give the g_observationlist time to refill
  delay(0,500);  // empty the table
  activeObs.clear();
  
  // if the active observations list changes the list here should be changed also.
  // iterate over the found entries and fill the table
  // if no previous Observation selected, set selection to the first on the list, also
  // set it here when previous selection disappeared from the list.
  // otherwise nothing will be changed in the selection

  int idx=-1;
  string newSelection="";
  string oldSelection=activeObs.selectedItem();
  selectedObservation=oldSelection;
  LOG_DEBUG("MainCU_Processes.ctl:activeObsCallback|oldSelection: "+oldSelection);
  for (int i=1; i<= dynlen(activeObservations);i++) {
    string realName=claimManager_nameToRealName("LOFAR_ObsSW_"+activeObservations[i]);
    activeObs.appendItem("",realName,activeObservations[i]);
    activeObs.ensureItemVisible(realName);
    activeObs.setIcon(realName,0,"16_empty.gif");
    if (i==1) {
      newSelection=realName;
    }
  }
  
  if ((oldSelection == newSelection) ||
      (oldSelection != "" && activeObs.itemExists(oldSelection))) {
    activeObs.setSelectedItem(oldSelection,true);
    selectedObservation=activeObs.getText(activeObs.selectedItem(),0); 
    observationName.text(selectedObservation);
    LOG_DEBUG("MainCU_Processes.ctl:activeObsCallback|Selection: "+selectedObservation);
    // nothing further needed
    return;
  } else {
    activeObs.setSelectedItem(newSelection,true);    
    selectedObservation=activeObs.getText(activeObs.selectedItem(),0); 
  }
  
  observationName.text(selectedObservation);
  LOG_DEBUG("MainCU_Processes.ctl:activeObsCallback|Selection: "+selectedObservation);
  
  // something has changed, so update Main Controllers
  LOG_DEBUG("MainCU_Processes.ctl:activeObsCallback|Starting updateMainControllers");
  MainCU_Processes_UpdateMainControllers(); 
  LOG_DEBUG("MainCU_Processes.ctl:activeObsCallback|Starting updateStationTree");
  MainCU_Processes_UpdateStationTree(); 
   
}


  
