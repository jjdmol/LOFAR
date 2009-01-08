// navCtrl.ctl
//
//  Copyright (C) 2002-2004  // TabChanoged: The Tab has changed, so a new panle needs to be initialized and put in place
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
// Ctrl scripts to handle the different event/selection/actions
// between the navigator Objects
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// navCtrl_handleViewBoxEvent                : handles viewBox
// navCtrl_handleViewSelectionEvent          : handles viewSelection
// navCtrl_handleTopDetailSelectionEvent     : handles topDetailSelection
// navCtrl_handleBottomDetailSelectionEvent  : handles bottomDetailSelection
// navCtrl_handleLocatorEvent                : handles locator
// navCtrl_handleProgressBarEvent            : handles progressBar
// navCtrl_handleHeadLinesEvent              : handles headLines
// navCtrl_handleAlertEvent                  : handles alert
// navCtrl_handleFastJumperEvent             : handles fastJumper

#uses "navigator.ctl"

global string   ACTIVE_TAB                = "Observations";
global string   ACTIVE_USER;

global string   VIEWBOXACTIONDP;
global string   VIEWSELECTIONACTIONDP;
global string   TOPDETAILSELECTIONACTIONDP;
global string   BOTTOMDETAILSELECTIONACTIONDP;
global string   LOCATORACTIONDP;
global string   PROGRESSBARACTIONDP;
global string   HEADLINESACTIONDP;
global string   ALERTSACTIONDP; 
global string   FASTJUMPERACTIONDP; 


///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleViewBoxEvent
//
// handles all interactions after an event from the viewBox
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleViewBoxEvent(string dp,string value){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleViewBoxEvent|entered with dp: " + dp + " and value: " + value);
  string aShape;
  string anEvent;
  dyn_string aSelection;
  
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_viewBox.selection")) {
    dpGet(DPNAME_NAVIGATOR + g_navigatorID+".fw_viewBox.selection", aSelection);
  } else {
    LOG_WARN("navCtrl.ctl:navCtrl_handleViewBoxEvent| Error getting selection from : " + DPNAME_NAVIGATOR + g_navigatorID+".fw_viewBox.selection");
    return;
  }      
  
  // split the event into shape and event
  if (!navFunct_splitEvent(value,aShape,anEvent) ) {
    LOG_WARN("navCtrl.ctl:navCtrl_handleViewBoxEvent| Error splitting event: " + value);
    return;
  }

  // ok so now we have all essentials
  // aShape contains the shape that initiated the event
  // anEvent contains the initial event
  // aSelection contains the Selections that belongs to the event  
  LOG_INFO("navCtrl.ctl:navCtrl_handleViewBoxEvent| Found shape    : " + aShape);
  LOG_INFO("navCtrl.ctl:navCtrl_handleViewBoxEvent| Found event    : " + anEvent);
  LOG_INFO("navCtrl.ctl:navCtrl_handleViewBoxEvent| Found selection: " + aSelection);
  navCtrl_handleNavigatorEvent(aSelection,anEvent,aShape); 
  
  // depending on the event received, actions need to be taken
  
 
  // TabChanged: The Tab has changed, so a new panel needs to be initialized and put in place
  if (anEvent == "ChangePanel") {
    if (navTabCtrl_showView()) {
        
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
      
      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);

      // inform headLines Object
      dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);
   
    }
    return;
  }
  
  if (anEvent == "Reload") {
    dpSet(VIEWBOXACTIONDP,"Reload");
  }
  
  if (anEvent == "Update") {
    
    // The Viewbox signals that he is ready, so the global lists should be filled also,
    // determine the missing parts and fill those based on the lists filled.
    // so if g_observationsList is filled, fill hardware and processes based on this
    // if g_processesList is filled, fill hardware and observations list based on this
    // and if g_harwareList is filled, fill observation and processes List based on this
    
    if (dynlen(g_observationsList) > 0) {
      navFunct_fillObservationsTree();
      navFunct_fillHardwareLists();
      navFunct_fillProcessesList();
    } else if (dynlen(g_processesList) > 0) {
      navFunct_fillProcessesTree();
      navFunct_fillHardwareLists();
      navFunct_fillObservationsList();
    } else if (dynlen(g_stationList) > 0) {
      navFunct_fillHardwareTree();
      navFunct_fillProcessesList();
      navFunct_fillObservationsList();
    }
    
    dpSet(TOPDETAILSELECTIONACTIONDP,"Update");
    dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Update");
    dpSet(LOCATORACTIONDP,"Update");
    dpSet(FASTJUMPERACTIONDP,"Update");
    dpSet(PROGRESSBARACTIONDP,"Update");
    dpSet(HEADLINESACTIONDP,"Update");

    return;
  }  
  
  if (anEvent == "EventClick") {
    
    // The viewbox gave an eventclick, this means we want to trigger the hightlight sequence
    // for a piece of hardware, or an observation, or a process
    // in case of a piece of hardware, we want to highlight the hardware itself,
    // the processes involved with this piece of hardware and the observations that
    // use this piece of hardware. In all cases the global lists should have been filled prior to this
    // command, so if needed we can also use these lists to determine the involved parties.
    

    
    dynClear(highlight);
    
    
        
        
    // ACTIVE_TAB can be used to see if we are looking at Hardware, processes or observations
    
    for (int i=1;i<= dynlen(aSelection);i++){
      dyn_string sel = strsplit(aSelection[i],"|"); 
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent|sel: "+sel); 
      for (int j = 1; j <= dynlen(sel); j++) {
        if (!dynContains(highlight,sel[j])) {
          dynAppend(highlight,sel[j]);
        }
         
    
        // if ACTIVE_TAB is hardware, we also want to look for all involved processes and observations
        if (ACTIVE_TAB == "Hardware") {
          // check all available observations
          for (int i = 1; i <= dynlen(g_observationsList);i++) {
            string longObs="LOFAR_ObsSW_"+g_observationsList[i];
        
            string station = g_stationList[1];
            string choice="";
            string strSearch="";
            int    iSearch=-1;

            // if more stations in lists, we are on a multiple station panel, and no other hardware is selectable
            if (dynlen(g_stationList) > 1) {
              station = sel[j];
              choice = "Station";
              strSearch=sel[j];
            } else {
              // in all other cases we need to determine the station involved in the panel
              // and the hardware that triggered the click
          
          
              if (strpos(sel[j],"Cabinet") > -1) {
                string c=sel[j];
                strreplace(c,"Cabinet","");
                iSearch=c;
                choice = "Cabinet";
              } else if (strpos(sel[j],"Subrack") > -1) {
                string c=sel[j];
                strreplace(c,"Subrack","");
                iSearch=c;
                choice = "Subrack";  
              } else if (strpos(sel[j],"RSPBoard") > -1) {
                string c=sel[j];
                strreplace(c,"RSPBoard","");
                iSearch=c;
                choice = "RSPBoard";
              } else if (strpos(sel[j],"TBBoard") > -1) {
                string c=sel[j];
                strreplace(c,"TBBoard","");
                iSearch=c;
                choice = "TBBoard";
              } else if (strpos(sel[j],"RCU") > -1) {
                string c=sel[j];
                strreplace(c,"RCU","");
                iSearch=c;
                choice = "RCU"; 
              }
            }
            

            if (navFunct_hardware2Obs(station, longObs,choice,strSearch,iSearch)){
              if (!dynContains(highlight,g_observationsList[i])) {
                // seems this observation is involved, add it to the list
                dynAppend(highlight,g_observationsList[i]);
              }
            }
          }
        } else if (ACTIVE_TAB == "Observations") {
          // We need to check what stations are involved in the Observations
          string longObs="LOFAR_ObsSW_"+sel[j];
          int iPos = dynContains(g_observations["NAME"],longObs);
          if (iPos > 0) {
            string s = g_observations["STATIONLIST"][iPos];
            dyn_string stations=navFunct_listToDynString(s);
            for (int k=1; k<= dynlen(stations); k++) {
              if (!dynContains(highlight,stations[k])) {
                dynAppend(highlight,stations[k]);
              }
            }
          }
        }
      }         
    }      
                         
    // we now have a list of items that need to be highlighted
    LOG_TRACE("navCtrl.ctl:navCtrl_handleViewBoxEvent| highlightList contains now: "+highlight);                          
    if (dynlen(highlight) > 0) {
      LOG_TRACE("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Kick object highlight trigger");
      dpSet(DPNAME_NAVIGATOR + g_navigatorID+".objectTrigger",true);
    }
    
    // inform headLines Object
    dpSet(HEADLINESACTIONDP,"ChangeInfo|"+aSelection);
    return;
  }
}

///////////////////////////////////////////////////////////////////////////
//
// Function handleViewSelectionEvent
//
// handles all interactions after an event from the viewSelection
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleViewSelectionEvent(string dp,string value){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleViewSelectionEvent|entered with dp: " + dp + " and value: " + value);

  string aShape;
  string anEvent;
  dyn_string aSelection;
  
  
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_viewSelection.selection")) {
    dpGet(DPNAME_NAVIGATOR + g_navigatorID+".fw_viewSelection.selection", aSelection);
  } else {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleViewSelectionEvent| Error getting selection from : " + DPNAME_NAVIGATOR + g_navigatorID+".fw_viewSelection.selection");
    return;
  }      
  
  // split the event into shape and event
  if (!navFunct_splitEvent(value,aShape,anEvent) ) {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleViewSelectionEvent| Error splitting event: " + value);
    return;
  }

  // ok so now we have all essentials
  // aShape contains the shape that initiated the event
  // anEvent contains the initial event
  // aSelection contains the Selections that belongs to the event  
  LOG_INFO("navCtrl.ctl:navCtrl_handleViewSelectionEvent| Found shape    : " + aShape);
  LOG_INFO("navCtrl.ctl:navCtrl_handleViewSelectionEvent| Found event    : " + anEvent);
  LOG_INFO("navCtrl.ctl:navCtrl_handleViewSelectionEvent| Found selection: " + aSelection);
 	navCtrl_handleNavigatorEvent(aSelection,anEvent,aShape); 
        
  // depending on the event received, actions need to be taken

  // Initialized:  The Viewbox is ready with its first panel, all other 
  // navigator object can be initialized now.
  if (anEvent == "Initialized") {
    dpSet(TOPDETAILSELECTIONACTIONDP,"Initialize");
    dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Initialize");
    dpSet(LOCATORACTIONDP,"Initialize");
    dpSet(FASTJUMPERACTIONDP,"Initialize");
    dpSet(PROGRESSBARACTIONDP,"Initialize");
    dpSet(HEADLINESACTIONDP,"Initialize");
    dpSet(ALERTSACTIONDP,"Initialize");
    return;
  }
  
  //ChangePanel
  if (anEvent == "ChangePanel") {
   
    if (navTabCtrl_showView()) {
        
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+g_currentDatapoint);

      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+g_currentDatapoint);
    }
  }  
  
  // TabChanged: The Tab has changed, so a new panel needs to be initialized and put in place
  if (anEvent == "TabChanged") {
    if (aSelection != ACTIVE_TAB) {
      navTabCtrl_removeView();
      
      ACTIVE_TAB = aSelection;
      if (navTabCtrl_showView()) {
        
        // change locator
        dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);

        // change fastJumper
        dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);

        // inform headLines Object
        dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);
        
        // update selectors   
        dpSet(TOPDETAILSELECTIONACTIONDP,"Update");
        dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Update");      }
    }
    return;
  } 
  LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewSelectionEvent| ACTIVE_TAB now        : " + ACTIVE_TAB);
}

///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleTopDetailSelectionEvent
//
// handles all interactions after an event from the topDetailSelection
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleTopDetailSelectionEvent(string dp,string value){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleTopSelectionEvent|entered with dp: " + dp + " and value: " + value);
  
  navCtrl_handleDetailSelectionEvent(dp,value,"top");
}

///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleBottomDetailSelectionEvent
//
// handles all interactions after an event from the bottomDetailSelection
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleBottomDetailSelectionEvent(string dp,string value){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleBottomDetailSelectionEvent|entered with dp: " + dp + " and value: " + value);
  
  navCtrl_handleDetailSelectionEvent(dp,value,"bottom");
}

///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleDetailSelectionEvent
//
// handles all interactions after an event from the top or bottom DetailSelection
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleDetailSelectionEvent(string dp,string value,string target){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|entered with dp: " + dp + " and value: " + value +" for target: " + target);
  
  string aShape;
  string anEvent;
  dyn_string aSelection;
  
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_"+target+"DetailSelection.selection")) {
    dpGet(DPNAME_NAVIGATOR + g_navigatorID+".fw_"+target+"DetailSelection.selection", aSelection);
  } else {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Error getting selection from : " + DPNAME_NAVIGATOR + g_navigatorID+".fw_"+target+".DetailSelectionselection");
    return;
  }      
  
  // split the event into shape and event
  if (!navFunct_splitEvent(value,aShape,anEvent) ) {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Error splitting event: " + value);
    return;
  }

  // ok so now we have all essentials
  // aShape contains the shape that initiated the event
  // anEvent contains the initial event
  // aSelection contains the Selections that belongs to the event  
  LOG_INFO("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Found shape    : " + aShape);
  LOG_INFO("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Found event    : " + anEvent);
  LOG_INFO("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Found selection: " + aSelection);
  navCtrl_handleNavigatorEvent(aSelection,anEvent,aShape);
  
  string typeSelector="";
  string observationType="";
  string selection="";
                      
  for (int i=1;i<= dynlen(aSelection);i++){
    dyn_string sel = strsplit(aSelection[i],"|");   
    if (dynlen(sel) > 1) {
      if (sel[1] == "Observations") {
        typeSelector=sel[1];
        if (dynlen(sel) >= 2 ) observationType=sel[2];
        if (dynlen(sel) >= 3 ) selection=sel[3];
      } else {
        typeSelector=sel[1];
        observationType="";
        if (dynlen(sel) >= 2 ) selection=sel[2];      
      }
    }    
  }
  
  if (anEvent == "ChangeSelection") {
    if (target == "bottom") {
      dpSet(BOTTOMDETAILSELECTIONACTIONDP,"update");
    } else {
      dpSet(TOPDETAILSELECTIONACTIONDP,"update");
    }
  }
  
  // Fill highlight string        
  string action = "Highlight";
  if (anEvent == "EventClick") { 
    // Empty highlight string
    dynClear(highlight);
    for (int i=1;i<= dynlen(aSelection);i++){
      dyn_string sel = strsplit(aSelection[i],"|"); 
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|sel: "+sel); 
      if (dynlen(sel) > 1) {
        if (sel[1] == "Observations") {
          typeSelector=sel[1];
          observationType=sel[2];
          selection=sel[3];
          if (!dynContains(highlight,sel[3])) {
            dynAppend(highlight,sel[3]);
          }
          action+="|"+sel[1]+"|"+sel[2]+"|"+sel[3];
          
          // if selection == observation, add involved hardware
          // loop through global hardware lists and add all hardware that is used by this observation
          
          // if more then one station available only stations need to be examend.
          string longObs="LOFAR_ObsSW_"+sel[3];
          string station="";  
          if (dynlen(g_stationList) > 1) {
            for (int k=1; k<= dynlen(g_stationList); k++) {
              if (navFunct_hardware2Obs(g_stationList[k], longObs,"Station",g_stationList[k],0)) {
                if (!dynContains(highlight,g_stationList[k])) {
                  dynAppend(highlight,g_stationList[k]);
                }
              }
            }
          } else {
            station=g_stationList[1];
            for (int k = 1; k<= dynlen(g_cabinetList); k++) {
              if (navFunct_hardware2Obs(station, longObs,"Cabinet","",g_cabinetList[k])) {
                if (!dynContains(highlight,g_cabinetList[k])) {
                  dynAppend(highlight,"Cabinet"+g_cabinetList[k]);
                }
              }
            }
            for (int k = 1; k<= dynlen(g_subrackList); k++) {
              if (navFunct_hardware2Obs(station, longObs,"Subrack","",g_subrackList[k])) {
                if (!dynContains(highlight,g_subrackList[k])) {
                  dynAppend(highlight,"Subrack"+g_subrackList[k]);
                }
              }
            }
            for (int k = 1; k<= dynlen(g_RSPList); k++) {
              if (navFunct_hardware2Obs(station, longObs,"RSPBoard","",g_RSPList[k])) {
                if (!dynContains(highlight,g_RSPList[k])) {
                  dynAppend(highlight,"RSPBoard"+g_RSPList[k]);
                }
              }
            }
            for (int k = 1; k<= dynlen(g_TBBList); k++) {
              if (navFunct_hardware2Obs(station, longObs,"TBBoard","",g_TBBList[k])) {
                if (!dynContains(highlight,g_TBBList[k])) {
                  dynAppend(highlight,"TBBoard"+g_TBBList[k]);
                }
              }
            }
            for (int k = 1; k<= dynlen(g_RCUList); k++) {
              if (navFunct_hardware2Obs(station, longObs,"RCU","",g_RCUList[k])) {
                if (!dynContains(highlight,g_RCUList[k])) {
                  dynAppend(highlight,"RCU"+g_RCUList[k]);
                }
              }
            }
          }             
        } else if (sel[1] == "Hardware") {  // Hardware
          if (!dynContains(highlight,sel[2])) {
            dynAppend(highlight,sel[2]);
          }

          // check all available observations
          for (int i = 1; i <= dynlen(g_observationsList);i++) {
            string longObs="LOFAR_ObsSW_"+g_observationsList[i];
        
            string station = g_stationList[1];
            string choice="";
            string strSearch="";
            int    iSearch=-1;

          
          
            if (strpos(sel[2],"Cabinet") > -1) {
              string c=sel[2];
              strreplace(c,"Cabinet","");
              iSearch=c;
              choice = "Cabinet";
            } else if (strpos(sel[2],"Subrack") > -1) {
              string c=sel[2];
              strreplace(c,"Subrack","");
              iSearch=c;
              choice = "Subrack";  
            } else if (strpos(sel[2],"RSPBoard") > -1) {
              string c=sel[2];
              strreplace(c,"RSPBoard","");
              iSearch=c;
              choice = "RSPBoard";
            } else if (strpos(sel[2],"TBBoard") > -1) {
              string c=sel[2];
              strreplace(c,"TBBoard","");
              iSearch=c;
              choice = "TBBoard";
            } else if (strpos(sel[2],"RCU") > -1) {
              string c=sel[2];
              strreplace(c,"RCU","");
              iSearch=c;
              choice = "RCU"; 
            } else {
              station = sel[2];
              choice = "Station";
              strSearch=sel[2];
            }
            

            if (navFunct_hardware2Obs(station, longObs,choice,strSearch,iSearch)){
              if (!dynContains(highlight,g_observationsList[i])) {
                // seems this observation is involved, add it to the list
                dynAppend(highlight,g_observationsList[i]);
              }
            }
          }
          action+="|"+sel[1]+"|"+sel[2];
        } else { // processes
          typeSelector=sel[1];
          observationType="";
          selection=sel[2];
          if (!dynContains(highlight,sel[2])) {
            dynAppend(highlight,sel[2]);
          }
          
          
          
          action+="|"+sel[1]+"|"+sel[2];
        }
      }          
    }   
    LOG_TRACE("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| highlight contains now: "+highlight);                          
    if (dynlen(highlight) > 0) {
      LOG_TRACE("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Kick trigger");
      dpSet(DPNAME_NAVIGATOR + g_navigatorID+".objectTrigger",true);
         
      // Also prepare actions for the different objects that can do something
      // with this highlight.
      // the Detail selection events need to highlight the choices involved also
      dpSet(TOPDETAILSELECTIONACTIONDP,action);
      dpSet(BOTTOMDETAILSELECTIONACTIONDP,action);
      
          
    }
  }
  
  if (anEvent == "ChangePanel") {

    //check if a tab change should be initiated
    if (ACTIVE_TAB != typeSelector && typeSelector != "") {
      LOG_DEBUG("Active tab should be changed to : "+ typeSelector);
      ACTIVE_TAB = typeSelector;
      navTabCtrl_setSelectedTab(typeSelector);
    }
    
    if (dpExists(selection)) {
      g_currentDatapoint=selection;
    }
    
    if (navTabCtrl_showView()) {
        
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);

      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);
    }
  }       
}

///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleLocatorEvent
//
// handles all interactions after an event from the locator
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleLocatorEvent(string dp,string value){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleLocatorEvent|entered with dp: " + dp + " and value: " + value);

  string aShape;
  string anEvent;
  dyn_string aSelection;
  
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_locator.selection")) {
    dpGet(DPNAME_NAVIGATOR + g_navigatorID+".fw_locator.selection", aSelection);
  } else {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleLocatorEvent| Error getting selection from : " + DPNAME_NAVIGATOR + g_navigatorID+".fw_locator.selection");
    return;
  }      
  
  // split the event into shape and event
  if (!navFunct_splitEvent(value,aShape,anEvent) ) {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleLocatorEvent| Error splitting event: " + value);
    return;
  }

  // ok so now we have all essentials
  // aShape contains the shape that initiated the event
  // anEvent contains the initial event
  // aSelection contains the Selections that belongs to the event  
  LOG_INFO("navCtrl.ctl:navCtrl_handleLocatorEvent| Found shape    : " + aShape);
  LOG_INFO("navCtrl.ctl:navCtrl_handleLocatorEvent| Found event    : " + anEvent);
  LOG_INFO("navCtrl.ctl:navCtrl_handleLocatorEvent| Found selection: " + aSelection);
  navCtrl_handleNavigatorEvent(aSelection,anEvent,aShape); 
  
  // depending on the event received, actions need to be taken
  if (anEvent == "ChangePanel") {
    if (navTabCtrl_showView()) {
      
      //clear old highlights
      dynClear(strHighlight);        
      
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
      
      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);

    }
  }
  
}
 
///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleProgressBarEvent
//
// handles all interactions after an event from the progressBar
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleProgressBarEvent(string dp,string value){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleProgressBarEvent|entered with dp: " + dp + " and value: " + value);

  string aShape;
  string anEvent;
  dyn_string aSelection;
  
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_progressBar.selection")) {
    dpGet(DPNAME_NAVIGATOR + g_navigatorID+".fw_progressBar.selection", aSelection);
  } else {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleProgressBarEvent| Error getting selection from : " + DPNAME_NAVIGATOR + g_navigatorID+".fw_progressBar.selection");
    return;
  }      
  
  // split the event into shape and event
  if (!navFunct_splitEvent(value,aShape,anEvent) ) {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleProgressBarEvent| Error splitting event: " + value);
    return;
  }

  // ok so now we have all essentials
  // aShape contains the shape that initiated the event
  // anEvent contains the initial event
  // aSelection contains the Selections that belongs to the event  
  LOG_INFO("navCtrl.ctl:navCtrl_handleProgressBarEvent| Found shape    : " + aShape);
  LOG_INFO("navCtrl.ctl:navCtrl_handleProgressBarEvent| Found event    : " + anEvent);
  LOG_INFO("navCtrl.ctl:navCtrl_handleProgressBarEvent| Found selection: " + aSelection);
 	navCtrl_handleNavigatorEvent(aSelection,anEvent,aShape); 
  
}

///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleHeadLinesEvent
//
// handles all interactions after an event from the headLines
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleHeadLinesEvent(string dp,string value){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleHeadLinesEvent|entered with dp: " + dp + " and value: " + value);

  string aShape;
  string anEvent;
  dyn_string aSelection;
  
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_headLines.selection")) {
    dpGet(DPNAME_NAVIGATOR + g_navigatorID+".fw_headLines.selection", aSelection);
  } else {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleHeadLinesEvent| Error getting selection from : " + DPNAME_NAVIGATOR + g_navigatorID+".fw_headLines.selection");
    return;
  }      
  
  // split the event into shape and event
  if (!navFunct_splitEvent(value,aShape,anEvent) ) {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleHeadLinesEvent| Error splitting event: " + value);
    return;
  }

  // ok so now we have all essentials
  // aShape contains the shape that initiated the event
  // anEvent contains the initial event
  // aSelection contains the Selections that belongs to the event  
  LOG_INFO("navCtrl.ctl:navCtrl_handleHeadLinesEvent| Found shape    : " + aShape);
  LOG_INFO("navCtrl.ctl:navCtrl_handleHeadLinesEvent| Found event    : " + anEvent);
  LOG_INFO("navCtrl.ctl:navCtrl_handleHeadLinesEvent| Found selection: " + aSelection);
 	navCtrl_handleNavigatorEvent(aSelection,anEvent,aShape); 
  
}

///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleAlertsEvent
//
// handles all interactions after an event from the Alert
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleAlertsEvent(string dp,string value){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleAlertsEvent|entered with dp: " + dp + " and value: " + value);

  string aShape;
  string anEvent;
  dyn_string aSelection;
  
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_alerts.selection")) {
    dpGet(DPNAME_NAVIGATOR + g_navigatorID+".fw_alerts.selection", aSelection);
  } else {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleAlertsEvent| Error getting selection from : " + DPNAME_NAVIGATOR + g_navigatorID+".fw_alerts.selection");
    return;
  }      
  
  // split the event into shape and event
  if (!navFunct_splitEvent(value,aShape,anEvent) ) {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleAlertsEvent| Error splitting event: " + value);
    return;
  }

  // ok so now we have all essentials
  // aShape contains the shape that initiated the event
  // anEvent contains the initial event
  // aSelection contains the Selections that belongs to the event  
  LOG_INFO("navCtrl.ctl:navCtrl_handleAlertsEvent| Found shape    : " + aShape);
  LOG_INFO("navCtrl.ctl:navCtrl_handleAlertsEvent| Found event    : " + anEvent);
  LOG_INFO("navCtrl.ctl:navCtrl_handleAlertsEvent| Found selection: " + aSelection);
 	navCtrl_handleNavigatorEvent(aSelection,anEvent,aShape); 
        
       
  // Check for top tab and change when needed, also set the new active DP panel if available 
  if (anEvent == "ChangeTab") {
    if (ACTIVE_TAB != aSelection) {
      navTabCtrl_setSelectedTab(aSelection);
      ACTIVE_TAB = aSelection;
    }
    if (navTabCtrl_showView()) {
        
      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);

      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);

      // inform headLines Object
      dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);
    }
  }

}

///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleFastJumperEvent
//
// handles all interactions after an event from the fastJumper
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleFastJumperEvent(string dp,string value){
  LOG_TRACE("navCtrl.ctl:navCtrl_handleFastJumperEvent|entered with dp: " + dp + " and value: " + value);

  string aShape;
  string anEvent;
  dyn_string aSelection;
  
  if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".fw_fastJumper.selection")) {
    dpGet(DPNAME_NAVIGATOR + g_navigatorID+".fw_fastJumper.selection", aSelection);
  } else {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleFastJumperEvent| Error getting selection from : " + DPNAME_NAVIGATOR + g_navigatorID+".fw_fastJumper.selection");
    return;
  }      
  
  // split the event into shape and event
  if (!navFunct_splitEvent(value,aShape,anEvent) ) {
    LOG_WARNING("navCtrl.ctl:navCtrl_handleFastJumperEvent| Error splitting event: " + value);
    return;
  }

  // ok so now we have all essentials
  // aShape contains the shape that initiated the event
  // anEvent contains the initial event
  // aSelection contains the Selections that belongs to the event  
  LOG_INFO("navCtrl.ctl:navCtrl_handleFastJumperEvent| Found shape    : " + aShape);
  LOG_INFO("navCtrl.ctl:navCtrl_handleFastJumperEvent| Found event    : " + anEvent);
  LOG_INFO("navCtrl.ctl:navCtrl_handleFastJumperEvent| Found selection: " + aSelection);
  navCtrl_handleNavigatorEvent(aSelection,anEvent,aShape); 
  // depending on the event received, actions need to be taken
  
  if (anEvent == "ChangePanel") {
    if (navTabCtrl_showView()) {
      
      //clear old highlights
      dynClear(strHighlight);        
      
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
      
      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);

    }
  }
  
}

navCtrl_highlightCheckGroups() {
  // check highlightlist for Core and Remote
  // if available, add the involved Core and/or Remote Stations
  if (dynContains(highlight,"Core")) {
    for (int i=1; i<= dynlen(coreStations); i++) {
      dynAppend(highlight,coreStations[i]);
    }
  }

  if (dynContains(highlight,"Remote")) {
    for (int i=1; i<= dynlen(remoteStations); i++) {
      dynAppend(highlight,remoteStations[i]);
    }
  }
}
///////////////////////////////////////////////////////////////////////////
//
// Function navCtrl_handleNavigatorEvent
//
// write event, selection and initiator into db (for testing only)
//
///////////////////////////////////////////////////////////////////////////
void navCtrl_handleNavigatorEvent(string selection,string event, string initiator){
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".navigator.selection",selection);
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".navigator.initiator",initiator);
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".navigator.event",event);
  }
