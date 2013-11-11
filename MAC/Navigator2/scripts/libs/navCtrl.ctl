// navCtrl.ctl
//
//  Copyright (C) 2002-2004  // TabChanoged: The Tab has changed, so a new panel needs to be initialized and put in place
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
  string selection="";
  
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
    // Check for top tab and change when needed, also set the new active DP panel if available 
  if (anEvent == "ChangeTab") {
    if (ACTIVE_TAB != aSelection) {
      navTabCtrl_setSelectedTab(aSelection);
      ACTIVE_TAB = aSelection;
    }
    if (navTabCtrl_showView()) {
      
      navFunct_waitObjectReady(500,"handleViewBoxEvent:ChangeTab wait navTabCtrl_showView");
      
      //clear old highlights
      dynClear(strHighlight);        
      
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
      
      navFunct_waitObjectReady(500,"handleViewBoxEvent:ChangeTab wait Locator ChangeSelection"+aSelection);
      
      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);

      navFunct_waitObjectReady(500,"handleViewBoxEvent:ChangeTab wait FastJumper ChangeSelection"+aSelection);

      // inform headLines Object
      dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);

      navFunct_waitObjectReady(500,"handleViewBoxEvent:ChangeTab wait HeadLines ChangeInfo");
      
    }
  }
  
  // Panel needs to be changed
  if (anEvent == "ChangePanel") {
    // To be able to handle same panel for different choices we introduce the possiblity to give a fake extra _level in the
    // selection datapoint, in that case the selection will be stripped from the fake point and set to the one b4 that
    // a fake point will be known by the # delim
    
    string var="";
    if (strpos(aSelection[1],"#") >= 0) {
      dyn_string aS = strsplit(aSelection[1],"#");
      selection = aS[1];
      var= aS[2];
      
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent|#selection: "+selection);
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent|#var:       "+var);
      if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".panelParamList")) {
        dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".panelParamList",var);
      } else {
        LOG_WARN("navCtrl.ctl:navCtrl_handleViewBoxEvent| Error: no dp " + DPNAME_NAVIGATOR + g_navigatorID+".panelParamList");
      }
    }
      

    
    if (dpExists(selection)) {
      g_currentDatapoint=selection;
    }
    
    if (navTabCtrl_showView()) {
        
      navFunct_waitObjectReady(500,"handleViewBoxEvent:ChangePanel wait navTabCtrl_showView");
      
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
      
      navFunct_waitObjectReady(500,"handleViewBoxEvent:ChangePanel wait Locator ChangeSelection"+aSelection);

      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);

      navFunct_waitObjectReady(500,"handleViewBoxEvent:ChangePanel wait FastJumper ChangeSelection"+aSelection);
      
      // inform headLines Object
      dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);
   
      navFunct_waitObjectReady(500,"handleViewBoxEvent:ChangePanel wait HeadLines ChangeInfo");
      
    }
    return;
  }
  
  if (anEvent == "Reload") {
    dpSet(VIEWBOXACTIONDP,"Reload");

    navFunct_waitObjectReady(500,"handleViewBoxEvent:Reload wait ViewBox Reload");
      
  }
  
  if (anEvent == "Update") {
   
    // The Viewbox signals that he is ready, so the global lists should be filled also,
    // determine the missing parts and fill those based on the lists filled.
    // so if g_observationsList is filled, fill hardware and processes based on this
    // if g_processesList is filled, fill hardware and observations list based on this
    // and if g_harwareList is filled, fill observation and processes List based on this
    
    if (dynlen(g_observationsList) > 0) {
      LOG_INFO("navCtrl.ctl:navCtrl_handleViewBoxEvent| g_observations entry");
      
      navFunct_fillObservationsTree();
      navFunct_fillHardwareLists();
      navFunct_fillProcessesList();
    } else if (dynlen(g_processesList) > 0) {
      LOG_INFO("navCtrl.ctl:navCtrl_handleViewBoxEvent| g_processes entry");
      navFunct_fillProcessesTree();
      navFunct_fillHardwareLists();
      navFunct_fillObservationsList();
    } else if (dynlen(g_stationList) > 0) {
      LOG_INFO("navCtrl.ctl:navCtrl_handleViewBoxEvent| g_stationList entry");
      navFunct_fillHardwareTree();
      navFunct_fillProcessesList();
      navFunct_fillObservationsList();
    }
    

    dpSet(TOPDETAILSELECTIONACTIONDP,"Update");
    navFunct_waitObjectReady(1500,"handleViewBoxEvent:Update wait TopDetail");
      
    dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Update");
    navFunct_waitObjectReady(500,"handleViewBoxEvent:Update wait BottomDetailSelection");
      
    dpSet(LOCATORACTIONDP,"Update");
    navFunct_waitObjectReady(500,"handleViewBoxEvent:Update wait Locator");
            
    dpSet(FASTJUMPERACTIONDP,"Update");
    navFunct_waitObjectReady(500,"handleViewBoxEvent:Update wait FastJumper");
      
    dpSet(PROGRESSBARACTIONDP,"Update");
    navFunct_waitObjectReady(500,"handleViewBoxEvent:Update wait ProgressBar");
      
    dpSet(HEADLINESACTIONDP,"Update");
    navFunct_waitObjectReady(500,"handleViewBoxEvent:Update wait HeadLines");
      

    return;
  }  
  
  if (anEvent == "EventClick") {
    
    // The viewbox gave an eventclick, this means we want to trigger the hightlight sequence
    // for a piece of hardware, or an observation, or a process
    // in case of a piece of hardware, we want to highlight the hardware itself,
    // the processes involved with this piece of hardware and the observations that
    // use this piece of hardware. In all cases the global lists should have been filled prior to this
    // command, so if needed we can also use these lists to determine the involved parties.
    
    string typeSelector="";
    string observationType="";
    string selection="";
    
    dynClear(highlight);
    
    
        
        
    // ACTIVE_TAB can be used to see if we are looking at Hardware, processes or observations
   
    for (int i=1;i<= dynlen(aSelection);i++){
      dyn_string sel = strsplit(aSelection[i],"|"); 
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent|sel: "+sel); 
      for (int j = 1; j <= dynlen(sel); j++) {
        if (!dynContains(highlight,sel[j])) {
          dynAppend(highlight,sel[j]);
        }
 
          LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent|selection we are looking for: ",sel[j]);
          LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent|g_stationList: ",g_stationList);
          LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent|g_processesList: ",g_processesList);
          LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent|g_observationsList: ",g_observationsList);
   
        // if ACTIVE_TAB is hardware, we also want to look for all involved processes and observations
        if (ACTIVE_TAB == "Hardware") {
          navCtrl_highlightAddObservationsFromHardware(sel[j]);
          navCtrl_highlightAddProcessesFromHardware(sel[j]);
  
        } else if (ACTIVE_TAB == "Observations") {
          // if selection == observation, add involved hardware
           navCtrl_highlightAddHardwareFromObservation(sel[j]);
           navCtrl_highlightAddProcessesFromObservation(sel[j]);

        } else if (ACTIVE_TAB == "Processes") {
          // The selected event was allready added to the selectionList above
          
          // If selection is in the g_stationList
          //   add processes from g_processesList that contain the station
          //   and all Observations that are found from the observation
          // if selection is in g_observationsList
          //  add all MCU001: and all lines that contain the observation from g_processesList
          //  add all Hardware found from Observation
          // if selection is in g_processesList
          //  add station if found in selection
          //  add observation if found in selection
          //
          if (dynContains(g_stationList,sel[j])) {
            navCtrl_highlightAddProcessesFromHardware(sel[j]);
            navCtrl_highlightAddObservationsFromHardware(sel[j]);
          } else if (dynContains(g_observationsList,sel[j])) {
            navCtrl_highlightAddProcessesFromObservation(sel[j]);
            navCtrl_highlightAddHardwareFromObservation(sel[j]);            
          } else if (dynContains(g_processesList,sel[j])) {
            navCtrl_highlightAddHardwareFromProcess(sel[j]);
            navCtrl_highlightAddObservationFromProcess(sel[j]);
          }
        }
      }         
    }      
                         
    // we now have a list of items that need to be highlighted
    LOG_TRACE("navCtrl.ctl:navCtrl_handleViewBoxEvent| highlightList contains now: "+highlight);                          
    if (dynlen(highlight) > 0) {
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent| Kick object highlight trigger");
      dpSet(DPNAME_NAVIGATOR + g_navigatorID+".objectTrigger",true);
    }
    
    
    // inform headLines Object
    dpSet(HEADLINESACTIONDP,"ChangeInfo|"+aSelection);
    navFunct_waitObjectReady(500,"handleViewBoxEvent:EventClick wait HeadLines");
      
    dpSet(TOPDETAILSELECTIONACTIONDP,"Highlight");
    navFunct_waitObjectReady(500,"handleViewBoxEvent:EventClick wait TopDetailSelection");
      
    dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Highlight");
    navFunct_waitObjectReady(500,"handleViewBoxEvent:EventClick wait BottomDetailSelection");

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
  string selection="";
  
  
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
    navFunct_waitObjectReady(2000,"handleViewSelectionEvent:Initialized wait TopDetailSelection");
      
    dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Initialize");
    navFunct_waitObjectReady(500,"handleViewSelectionEvent:Initialized wait BottomDetailSelection");
      
    dpSet(LOCATORACTIONDP,"Initialize");
    navFunct_waitObjectReady(500,"handleViewSelectionEvent:Initialized wait Locator");
      
    dpSet(FASTJUMPERACTIONDP,"Initialize");
    navFunct_waitObjectReady(500,"handleViewSelectionEvent:Initialized wait FastJumper");
      
    dpSet(PROGRESSBARACTIONDP,"Initialize");
    navFunct_waitObjectReady(500,"handleViewSelectionEvent:Initialized wait ProgressBar");
      
    dpSet(HEADLINESACTIONDP,"Initialize");
    navFunct_waitObjectReady(500,"handleViewSelectionEvent:Initialized wait HeadLines");
      
    dpSet(ALERTSACTIONDP,"Initialize");
    navFunct_waitObjectReady(500,"handleViewSelectionEvent:Initialized wait Alerts");
      
    return;
  }
  
  //ChangePanel
  if (anEvent == "ChangePanel") {
 
    // To be able to handle same panel for different choices we introduce the possiblity to give a fake extra _level in the
    // selection datapoint, in that case the selection will be stripped from the fake point and set to the one b4 that
    // a fake point will be known by the # delim
    
    string var="";
    if (strpos(selection,"#") >= 0) {
      dyn_string aS = strsplit(selection,"#");
      selection = aS[1];
      var= aS[2];
      
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewSelectionEvent|#selection: "+selection);
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewSelectionEvent|#var:       "+var);
      if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".panelParamList")) {
        dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".panelParamList",var);
      } else {
        LOG_WARN("navCtrl.ctl:navCtrl_handleViewSelectionEvent| Error: no dp " + DPNAME_NAVIGATOR + g_navigatorID+".panelParamList");
      }
    }
      

    if (dpExists(selection)) {
      g_currentDatapoint=selection;
    }
    
    if (navTabCtrl_showView()) {
      navFunct_waitObjectReady(500,"handleViewSelectionEvent:ChangePanel wait navTabCtrl_showView");
      
        
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+g_currentDatapoint);
      navFunct_waitObjectReady(500,"handleViewSelectionEvent:ChangePanel wait Locator ChangeSelection");
      

      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+g_currentDatapoint);
      navFunct_waitObjectReady(500,"handleViewSelectionEvent:ChangePanel wait FastJumper ChangeSelection");
      
    }
  }  
  
  // TabChanged: The Tab has changed, so a new panel needs to be initialized and put in place
  if (anEvent == "TabChanged") {
    if (aSelection != ACTIVE_TAB) {
      
      navFunct_clearGlobalLists();
      // if on the other Tab a dp was saved restore it, and also
      // keep the current datapoint save in de tabDatapoint
      navTabCtrl_saveAndRestoreCurrentDP(aSelection);
      navTabCtrl_removeView();
      
      ACTIVE_TAB = aSelection;
      if (navTabCtrl_showView()) {
        navFunct_waitObjectReady(750,"handleViewSelectionEvent:TabChanged wait navTabCtrl_showView");
      
        
        // change locator
        dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
        navFunct_waitObjectReady(500,"handleViewSelectionEvent:TabChanged wait Locator ChangeSelection "+ aSelection);
      

        // change fastJumper
        dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);
        navFunct_waitObjectReady(500,"handleViewSelectionEvent:TabChanged wait FastJumper ChangeSelection "+ aSelection);
      

        // inform headLines Object
        dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);
        navFunct_waitObjectReady(500,"handleViewSelectionEvent:TabChanged wait HeadLines ChangeInfo");
      
        
        // update selectors   
        dpSet(TOPDETAILSELECTIONACTIONDP,"Update");
        navFunct_waitObjectReady(500,"handleViewSelectionEvent:TabChanged wait TopDetailSelection Update");
      
        dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Update");      }
        navFunct_waitObjectReady(500,"handleViewSelectionEvent:TabChanged wait BottomDetailSelection Update");
      
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
      dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Update");
        navFunct_waitObjectReady(500,"handleDetailSelectionEvent:ChangeSelectiond wait BottomDetailSelection Update");
      
    } else {
      dpSet(TOPDETAILSELECTIONACTIONDP,"Update");
        navFunct_waitObjectReady(500,"handleDetailSelectionEvent:ChangeSelectiond wait TopDetailSelection Update");
      
    }
  }
  
  // Fill highlight string        
  if (anEvent == "EventClick") { 
    // Empty highlight string
    dynClear(highlight);
    for (int i=1;i<= dynlen(aSelection);i++){
      dyn_string sel = strsplit(aSelection[i],"|"); 
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|sel: "+sel); 
      if (dynlen(sel) > 1) {
          LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|selection we are looking for: ",sel[2]);
          LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|g_stationList: ",g_stationList);
          LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|g_processesList: ",g_processesList);
          LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|g_observationsList: ",g_observationsList);

          
        if (sel[1] == "Observations") {
          typeSelector=sel[1];
          if (dynlen(sel) >= 2) observationType=sel[2];
          if (dynlen(sel) >= 3) { 
            selection=sel[3];
            navProgressCtrl_handleObservation(selection);
            if (!dynContains(highlight,selection)) {
              dynAppend(highlight,selection);
            }
          }
          
          // if selection == observation, add involved hardware && software
          navCtrl_highlightAddHardwareFromObservation(selection);
          navCtrl_highlightAddProcessesFromObservation(selection);
        } else if (sel[1] == "Hardware") {  // Hardware
          typeSelector=sel[1];
          observationType="";
          selection=sel[2];
          if (!dynContains(highlight,selection)) {
            dynAppend(highlight,selection);
          }
          navCtrl_highlightAddObservationsFromHardware(selection);
          navCtrl_highlightAddProcessesFromHardware(selection);
        } else { // processes
          typeSelector=sel[1];
          observationType="";
          selection=sel[2];
          if (!dynContains(highlight,selection)) {
            dynAppend(highlight,selection);
          }
          navCtrl_highlightAddHardwareFromProcess(selection);
          navCtrl_highlightAddObservationsFromProcess(selection);
        }
      }          
    }   
    LOG_TRACE("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| highlight contains now: "+highlight);                          
    if (dynlen(highlight) > 0) {
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Kick trigger");
      dpSet(DPNAME_NAVIGATOR + g_navigatorID+".objectTrigger",true);
          
    }
  }
  
  if (anEvent == "ChangePanel") {
 
    //check if a tab change should be initiated
    if (ACTIVE_TAB != typeSelector && typeSelector != "") {
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|Active tab should be changed to : "+ typeSelector);
      navTabCtrl_saveAndRestoreCurrentDP(typeSelector);
      ACTIVE_TAB = typeSelector;
      navTabCtrl_setSelectedTab(typeSelector);
    }
    
    // To be able to handle same panel for different choices we introduce the possiblity to give a fake extra _level in the
    // selection datapoint, in that case the selection will be stripped from the fake point and set to the one b4 that
    // a fake point will be known by the # delim
    
    string var="";
    if (strpos(selection,"#") >= 0) {
      dyn_string aS = strsplit(selection,"#");
      selection = aS[1];
      var= aS[2];
      
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|#selection: "+selection);
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|#var:       "+var);
      if (dpExists(DPNAME_NAVIGATOR + g_navigatorID + ".panelParamList")) {
        dpSet(DPNAME_NAVIGATOR + g_navigatorID + ".panelParamList",var);
      } else {
        LOG_WARN("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Error: no dp " + DPNAME_NAVIGATOR + g_navigatorID+".panelParamList");
      }
    }
      

    if (dpExists(selection)) {
      g_currentDatapoint=selection;
    }
    
    if (navTabCtrl_showView()) {
        navFunct_waitObjectReady(500,"handleDetailSelectionEvent:ChangePanel wait navTabCtrl_showView");
      
        
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
        navFunct_waitObjectReady(500,"handleDetailSelectionEvent:ChangePanel wait Locator ChangeSelection "+aSelection);
      

      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);
        navFunct_waitObjectReady(500,"handleDetailSelectionEvent:ChangePanel wait FastJumper ChangeSelection "+aSelection);
      
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
        navFunct_waitObjectReady(500,"handleLocatorEvent:ChangePanel wait navTabCtrl_showView");
      
      
      //clear old highlights
      dynClear(strHighlight);        
      
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
        navFunct_waitObjectReady(500,"handleLocatorEvent:ChangePanel wait Locator ChangeSelection " + aSelection);
      
      
      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);
        navFunct_waitObjectReady(500,"handleLocatorEvent:ChangePanel wait FastJumper ChangeSelection " + aSelection);
      

      // inform headLines Object
      dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);
        navFunct_waitObjectReady(500,"handleLocatorEvent:ChangePanel wait HeadLines ChangeInfo ");
      
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
        navFunct_waitObjectReady(500,"handleAlertsEvent:ChangeTab wait navTabCtrl_showView");
      
      
      //clear old highlights
      dynClear(strHighlight);        
      
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
      navFunct_waitObjectReady(500,"handleAlertsEvent:ChangeTab wait Locator ChangeSelection " + aSelection);
     
      
      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);
      navFunct_waitObjectReady(500,"handleAlertsEvent:ChangeTab wait FastJumper ChangeSelection " + aSelection);     

      // inform headLines Object
      dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);
      navFunct_waitObjectReady(500,"handleAlertsEvent:ChangeTab wait HeadLines ChangeInfo " + aSelection);
      
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
      navFunct_waitObjectReady(500,"handleFastJumperEvent:ChangePanel wait navTabCtrl_showView");
      
      
      //clear old highlights
      dynClear(strHighlight);        
      
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);
      navFunct_waitObjectReady(500,"handleFastJumperEvent:ChangePanel wait Locator ChangeSelection "+aSelection);
      
      
      // change fastJumper
      dpSet(FASTJUMPERACTIONDP,"ChangeSelection|"+aSelection);
      navFunct_waitObjectReady(500,"handleFastJumperEvent:ChangePanel wait FastJumper ChangeSelection "+aSelection);
      

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

void navCtrl_highlightAddObservationsFromHardware(string selection) {
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddObservationsFromHardware|entered with highlight: "+ highlight + " selection: " + selection);  
  // check all available observations
  for (int i = 1; i <= dynlen(g_observationsList);i++) {
    string longObs="LOFAR_ObsSW_"+g_observationsList[i];
        
    string station = g_stationList[1];
    string choice="";
    string strSearch="";
    int    iSearch=-1;

    // if more stations in lists, we are on a multiple station panel, and no other hardware is selectable
    // if not on the "Hardware" tab we only need to add the station
    if (dynlen(g_stationList) > 1 || (dynlen(g_stationList) == 1 && ACTIVE_TAB != "Hardware")) {
      station = selection;
      choice = "Station";
      strSearch=selection;
    } else {
      // in all other cases we need to determine the station involved in the panel
      // and the hardware that triggered the click
          
          
      if (strpos(selection,"Cabinet") > -1) {
        string c=selection;
        strreplace(c,"Cabinet","");
        iSearch=c;
        choice = "Cabinet";
      } else if (strpos(selection,"Subrack") > -1) {
        string c=selection;
        strreplace(c,"Subrack","");
        iSearch=c;
        choice = "Subrack";  
      } else if (strpos(selection,"RSPBoard") > -1) {
        string c=selection;
        strreplace(c,"RSPBoard","");
        iSearch=c;
        choice = "RSPBoard";
      } else if (strpos(selection,"TBBoard") > -1) {
        string c=selection;
        strreplace(c,"TBBoard","");
        iSearch=c;
        choice = "TBBoard";
      } else if (strpos(selection,"RCU") > -1) {
        string c=selection;
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
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddObservationsFromHardware|leaving with highlight now: ", highlight);
}

void navCtrl_highlightAddHardwareFromObservation(string selection) {
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddHardwareFromObservation|entered with highlight: " + highlight+ "selection: " + selection); 
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddHardwareFromObservation|g_stationList: " + g_stationList); 
   
  // loop through global hardware lists and add all hardware that is used by this observation
          
  // if more then one station available only stations need to be examend. This is also the case when looking at other tabs then hardware,
  // on processes or Observationtabs we only need to know if the station is involved
  string longObs="LOFAR_ObsSW_"+selection;
  string station="";  
  if (dynlen(g_stationList) > 1 || (dynlen(g_stationList) == 1 && ACTIVE_TAB != "Hardware")) {
    for (int k=1; k<= dynlen(g_stationList); k++) {
      if (g_stationList[k] == navFunct_bareDBName(CEPDBName) || navFunct_hardware2Obs(g_stationList[k], longObs,"Station",g_stationList[k],0)) {
        if (!dynContains(highlight,g_stationList[k])) {
          dynAppend(highlight,g_stationList[k]);
        }
      }
    }
  } else if (dynlen(g_stationList) == 1){
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
  
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddHardwareFromObservation|leaving with highlight: " + highlight);
}

// selection is a single hardware item, check for all processes that have that hardware in its line
void navCtrl_highlightAddProcessesFromHardware(string selection) {
  
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddProcessesFromHardware|entered with highlight: "+ highlight + " selection: " + selection);  
  for (int i = 1;i<= dynlen(g_processesList); i++) {
    if (strpos(g_processesList[i],selection) >= 0) {
      dyn_string aS = navCtrl_stripElements(g_processesList[i]);
      for (int j=1; j<= dynlen(aS); j++) {
        if (!dynContains(highlight,aS[j])) {
          dynAppend(highlight,aS[j]);
        }
      }
    }
  }
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddProcessesFromHardware|leaving with highlight: " + highlight);
}

// selection is a single processline, check if it contains hardware
void navCtrl_highlightAddHardwareFromProcess(string selection) {
  
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddHardwareFromProcess|entered with highlight: "+ highlight + " selection: " + selection);  
  // since we can have Observation.. in the processlist, we want to add hardware from observation sometimes.
  if (strpos(selection,"Observation") == 0) {
    navCtrl_highlightAddHardwareFromObservation(selection);
  } else {  
  
    for (int i = 1;i<= dynlen(g_processesList); i++) {
      if (strpos(g_processesList[i],selection) >= 0) {
        dyn_string aS= strsplit(g_processesList[i],":");
        if (dynlen(aS) > 0) {
          if (!dynContains(highlight,aS[1])) {
            dynAppend(highlight,aS[1]);
          }
        }
      }
    }
  }
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddHardwareFromProcess|leaving with highlight: " + highlight);
}

// selection is a single processline, check if it contains an observation
void navCtrl_highlightAddObservationsFromProcess(string selection) {
  
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddObservationsFromProcess|entered with highlight: "+ highlight + " selection: " + selection);  
  for (int i = 1;i<= dynlen(g_processesList); i++) {
    if (strpos(g_processesList[i],selection) >= 0) {

      // check if the string contains an observation
      int start = strpos(g_processesList[i],"LOFAR_ObsSW_TempObs");
      if ( start > 0 ) {
        string realName= substr(g_processesList[i],start,23);
        LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddObservationsFromProcess|Found observation: " + realName);
        string obsName=claimManager_realNameToName(realName);
        string bareObs=substr(obsName,strpos(obsName,"Observation"));
        if (!dynContains(highlight,bareObs)) {
          dynAppend(highlight,bareObs);
        }
      }
    }
  }
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddObservationsFromProcess|leaving with highlight: " + highlight);
}

// selection is a single observationitem, check for all processes that have that hardware in its line
void navCtrl_highlightAddProcessesFromObservation(string selection) {
  
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddProcessesFromObservation|entered with highlight: "+ highlight + " selection: " + selection);  
  string obsName = claimManager_nameToRealName("LOFAR_ObsSW_"+selection);
  for (int i = 1;i<= dynlen(g_processesList); i++) {
    if (strpos(g_processesList[i],obsName) >= 0) {
      dyn_string aS = navCtrl_stripElements(g_processesList[i]);
      for (int j=1; j<= dynlen(aS); j++) {
        if (!dynContains(highlight,aS[j])) {
          dynAppend(highlight,aS[j]);
        }
      }
    }
  }
  LOG_DEBUG("navCtrl.ctl:navCtrl_highlightAddProcessesFromHardware|leaving with highlight: " + highlight);
}

// Strips names like xxxx:yyyy_zzzz_wwww
// into xxxx: yyyy zzzz wwww 
dyn_string navCtrl_stripElements(string aString) {
  
  dyn_string ret;
  string remainder;
  dyn_string aS = strsplit(aString,":");
  if (dynlen(aS) > 1) {
    // found station
    dynAppend(ret,aS[1]);
    remainder=aS[2];
  } else {
    remainder=aS[1];
  }
  
  aS= strsplit(remainder,"_");    
  for (int i=1; i<= dynlen(aS); i++) {
    dynAppend(ret,aS[i]);
  }
  return ret;
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
    
    // if a system came online or went offline the viewbox needs a trigger to reload
    if (event == "DistChanged") {
      // change locator
      dpSet(VIEWBOXACTIONDP,"DistChanged");
        navFunct_waitObjectReady(500,"handleNavigatorEvent:DistChanged wait ViewBox DistChanged");
      
      
    }
  }


