// CEP_Processes.ctl
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
// global functions for the CEP_Processes panel
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// CEP_Processes_initList                        : prepares the processeslist
// CEP_Processes_UpdateCEPControllers        : updates all CEPdb controllers depending on observations
// CEP_Processes_UpdateProcessesList             : prepares an updated treelist for the selectionboxes

#uses "GCFLogging.ctl"
#uses "GCFCommon.ctl"
#uses "MainCU_Processes.ctl"

global dyn_string CEP_result;
global dyn_string CEP_procList;
global string CEP_selectedObservation    = "";
global string CEP_selectedStation        = "";
global string CEP_obsBaseDP              = "";



// ****************************************
// Name : CEP_Processes_initList
// ****************************************
// Description:  
//   prepares the processes list
//  
// ***************************************
bool CEP_Processes_initList() {
  dynClear(CEP_result);
  dynClear(CEP_procList);
  
  
  int z;
  dyn_dyn_anytype tab;
  //PermSW + PermSW_Daemons
  dpQuery("SELECT '_original.._value' FROM 'LOFAR_PermSW_*.status.state' REMOTE 'CCU001:'", tab);
  LOG_TRACE("CEP_Processes.ctl:initList|Found: "+ tab);
  
  dyn_string aDS=navFunct_getDynString(tab, 2,1);
  dynSortAsc(aDS);
  
  for(z=1;z<=dynlen(aDS);z++){
    
    // strip .status.state from CEP_result
    string aS = dpSubStr(aDS[z],DPSUB_SYS_DP);

    // keep Path to work with
    string path=aS;
    

    // strip all including PermsSW out of the string
    strreplace(aS,syst+dpSubStr(baseDP,DPSUB_DP)+"_PermSW_","");


    // Remainder should be PermsSW Programs + Daemons  split on _ 
    dyn_string spl=strsplit(aS,"_");
    if (dynlen(spl) > 1) { // Daemon
      dynAppend(CEP_result,navFunct_dpStripLastElement(path)+","+spl[2]+","+path);
      dynAppend(CEP_procList,path);
    } else {   // Program
      dynAppend(CEP_result,","+spl[1]+","+path);
      if (spl[1] != "Daemons") {
        dynAppend(CEP_procList,path);
      }
    }
  }
  
  LOG_DEBUG("CEP_Processes.ctl:initList|found procList: "+ CEP_procList);
  LOG_DEBUG("CEP_Processes.ctl:initList|found results: "+ CEP_result);
    
  if (!dpExists(MainDBName+"LOFAR_PermSW_MACScheduler.activeObservations")) {
    setValue("activeObs","backCol","Lofar_dpdoesnotexist");
  } else {
    if (dpConnect("CEP_Processes_ActiveObsCallback",true,"LOFAR_PermSW_MACScheduler.activeObservations") == -1) {
      LOG_ERROR("CEP_Processes.ctl:initList|couldn't connect to activeObservations.");
    } else {
      if (!navFunct_dpReachable("LOFAR_PermSW_MACScheduler.activeObservations")) { 
        setValue("activeObs","backCol","Lofar_dpOffline");
      }
    }        
  }
 
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".processesList",CEP_result);
}

// ****************************************
// Name : CEP_Processes_UpdateCEPControllers()
// ****************************************
// Description:  
//   sets CEP_selectedStation and looks if the CEPControllers need
//   to be updated.
//  
// ***************************************
bool CEP_Processes_UpdateCEPControllers() {
  LOG_TRACE("CEP_Processes.ctl:updateCEPControllers|entered, CEP_obsBaseDP: "+CEP_obsBaseDP);

  string newSelectedObservation=activeObs.getText(activeObs.selectedItem(),0);
  LOG_TRACE("CEP_Processes.ctl:updateMainControllers|selected observation: "+ CEP_selectedObservation +" New: "+ newSelectedObservation);
  // check if selection is made, and the selection is indeed a new one
  if (newSelectedObservation != 0) {
    selectedStation="";
    CEP_selectedObservation = newSelectedObservation;
    observationName.text(CEP_selectedObservation);
    // get the real name from the selected Observation
    CEP_obsBaseDP=claimManager_nameToRealName("LOFAR_ObsSW_"+CEP_selectedObservation);   
    
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","OnlineControl_BGPApplPanel",
        DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(CEP_obsBaseDP));
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.objectName","OnlineControl_StorageApplPanel",
        DPNAME_NAVIGATOR + g_navigatorID + ".updateTrigger.paramList",makeDynString(CEP_obsBaseDP));
  
  }
  
  // update StationTree
  LOG_DEBUG("CEP_Processes.ctl:UpdateCEPControllers|Starting updateStationTree");  
  CEP_Processes_UpdateStationTree();
  
  LOG_TRACE("CEP_Processes.ctl:UpdateCEPControllers|call UpdateProcessesList");
  if (!CEP_Processes_UpdateProcessesList()) {
    LOG_ERROR("CEP_Processes.ctl:UpdateCEPControllers|UpdateProcessesList returned false");
    return false;
  }
  return true;
}

// ****************************************
// Name : CEP_Processes_UpdateProcessesList()
// ****************************************
// Description:  
//   takes the general CEP_results and adds the (changed) 
//   observation CEP ctrl'ers
//  
// ***************************************
bool CEP_Processes_UpdateProcessesList() {
  LOG_TRACE("CEP_Processes.ctl:updateProcessesList|entered selected observation: "+ CEP_selectedObservation);
  dyn_string list;
  
  // copy old CEP_results from rest of the panel to the new list
  list=CEP_result;
  g_processesList=CEP_procList;
  
  int z;
  dyn_dyn_anytype tab;
  // if an observation is chosen
  if(CEP_selectedObservation != "") {
    // get the real name from the selected Observation
    string CEPObsDP=claimManager_nameToRealName("LOFAR_ObsSW_"+CEP_selectedObservation);
    
    if (strtok(CEP_selectedStation,":") < 0) { 
      CEP_selectedStation+=":";
    }
    if (strpos(CEPObsDP,"CCU001") < 0) {     
      CEPObsDP="CCU001:"+dpSubStr(CEPObsDP,DPSUB_DP);
    }    
    CEP_obsBaseDP = CEPObsDP;
    // add Observation 
    dynAppend(list,","+CEP_selectedObservation+","+CEPObsDP);

    //select all Ctrl under CEP:LOFAR_PermSW_'CEP_selectedObservation'
    string query="SELECT '_original.._value' FROM '"+CEPObsDP+"_*.status.state' REMOTE 'CCU001:'";
    LOG_DEBUG("CEP_Processes.ctl:updateProcessesList|Query: "+ query);
    dpQuery(query, tab);
    LOG_TRACE("CEP_Processes.ctl:updateProcessesList|CEP Controllers Found: "+ tab);
      
    dyn_string aDS=navFunct_getDynString(tab, 2,1);
    dynSortAsc(aDS);
    for(z=1;z<=dynlen(aDS);z++){
    
      // strip .status.state from CEP_result
      string aS = dpSubStr(aDS[z],DPSUB_SYS_DP);

      // keep Path to work with
      string path=aS;
    
      
      // strip all including Observation out of the string
      strreplace(aS,CEPObsDP+"_","");

      
      // Remainder should be Ctrl Programs, split on _ 
      dyn_string spl=strsplit(aS,"_");
      if (dynlen(spl) > 1) { // low level Ctrl
        if (dynlen(spl) < 3) {
          dynAppend(list,navFunct_dpStripLastElement(path)+","+spl[2]+","+path);
          dynAppend(g_processesList,path);
        }
      } else {   // Ctrl
        dynAppend(list,CEPObsDP+","+spl[1]+","+path);
        if (spl[1] != "OnlineControl") {
          dynAppend(g_processesList,path);
        }
      }
    }
    
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".processesList",list);
    
    // set panel to ready
    g_objectReady=true;
  
    // trigger that the panel values are calculated and ready
    navPanel_setEvent("CEP_Processes.ctl:updateProcessesList","Update");
  }
  return true;
} 



CEP_Processes_ActiveObsCallback(string dp1, dyn_string activeObservations) {
  LOG_TRACE("CEP_Processes.ctl:activeObsCallback|Found: "+ activeObservations);

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
  CEP_selectedObservation=oldSelection;
  LOG_DEBUG("CEP_Processes.ctl:activeObsCallback|oldSelection: "+oldSelection);
  for (int i=1; i<= dynlen(activeObservations);i++) {
    string realName=claimManager_nameToRealName("LOFAR_ObsSW_"+activeObservations[i]);
    activeObs.appendItem("",realName,activeObservations[i]);
    activeObs.ensureItemVisible(realName);
    activeObs.setIcon(realName,0,"16_empty.gif");
    if (i==1) {
      newSelection=realName;
    }
  }
  
  if (strpos(newSelection,"CCU001") < 0) {     
      CEP_obsBaseDP="CCU001:"+dpSubStr(newSelection,DPSUB_DP);
  } else {   
      CEP_obsBaseDP = newSelection;
    }
  
  if ((oldSelection == newSelection) ||
      (oldSelection != "" && activeObs.itemExists(oldSelection))) {
    activeObs.setSelectedItem(oldSelection,true);
    CEP_selectedObservation=activeObs.getText(activeObs.selectedItem(),0); 
    observationName.text(CEP_selectedObservation);
    LOG_DEBUG("CEP_Processes.ctl:activeObsCallback|Selection: "+CEP_selectedObservation);
    // nothing further needed
    return;
  } else {
    activeObs.setSelectedItem(newSelection,true);    
    CEP_selectedObservation=activeObs.getText(activeObs.selectedItem(),0); 
  }
  
  observationName.text(CEP_selectedObservation);
  LOG_DEBUG("CEP_Processes.ctl:activeObsCallback|Selection: "+CEP_selectedObservation);
  
  // something has changed, so update Main Controllers
  LOG_DEBUG("CEP_Processes.ctl:activeObsCallback|Starting updateCEPControllers");
  CEP_Processes_UpdateCEPControllers(); 
}

CEP_Processes_UpdateStationTree() {

  // empty the table
  stationTree.clear();
  
  
  LOG_DEBUG("CEPProcesses.ctl:CEP_Processes_UpdateStationTree|Found CEP_selectedObservation: "+CEP_selectedObservation);
  if (CEP_selectedObservation == "") {
    return;
  }
  
  LOG_DEBUG("CEP_Processes.ctl:CEP_Processes_UpdateStationTree|Found CEP_obsBaseDP: "+CEP_obsBaseDP);
  if (dpExists(CEP_obsBaseDP)) {
    // look if that name is available in the Observation List
    int j = dynContains(g_observations["DP"],CEP_obsBaseDP);
    if ( j > 0) {
      // get the Stationlist from that observation
      string sts=g_observations["STATIONLIST"][j];
      LOG_DEBUG("CEP_Processes.ctl:CEP_Processes_UpdateStationTree|Found Stationlist for this Observation: "+ sts);
      // add stations if not allready there
      dyn_string stations = navFunct_listToDynString(sts);
      for (int k=1; k<= dynlen(stations);k++) {
        if (!stationTree.itemExists(stations[k])) {
          stationTree.appendItem("",stations[k],stations[k]);
          stationTree.ensureItemVisible(stations[k]);
          stationTree.setIcon(stations[k],0,"16_empty.gif");

        }
      }
    }
  }
}  
