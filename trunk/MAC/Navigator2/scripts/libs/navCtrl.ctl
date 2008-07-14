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
  
 
  // TabChanged: The Tab has changed, so a new panle needs to be initialized and put in place
  if (anEvent == "ChangePanel") {
    if (navTabCtrl_showView()) {
        
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);

      // inform headLines Object
      dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);
   
      // update selectors   
//      dpSet(TOPDETAILSELECTIONACTIONDP,"Update");
//      dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Update");
    }
    return;
  }
  
  if (anEvent == "Update") {
    dpSet(TOPDETAILSELECTIONACTIONDP,"Update");
    dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Update");
    dpSet(LOCATORACTIONDP,"Update");
    dpSet(PROGRESSBARACTIONDP,"Update");
    dpSet(HEADLINESACTIONDP,"Update");

    return;
  }  
  
  if (anEvent == "EventClick") {
    
    // Empty highlight string
    dynClear(strHighlight);
    dynClear(highlight);
    
    for (int i=1;i<= dynlen(aSelection);i++){
      dyn_string sel = strsplit(aSelection[i],"|"); 
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleViewBoxEvent|sel: "+sel); 
      if (dynlen(sel) > 0) {
        dynAppend(strHighlight,sel[1]);
      }
    }   
    LOG_TRACE("navCtrl.ctl:navCtrl_handleViewBoxEvent| strHighlight contains now: "+strHighlight);                          
    if (dynlen(strHighlight) > 0) {
      LOG_TRACE("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Kick trigger");
      dpSet(DPNAME_NAVIGATOR + g_navigatorID+".trigger",true);
    }
    
    // inform headLines Object
    dpSet(HEADLINESACTIONDP,"ChangeInfo|"+aSelection);
    // also fire highlight mechanism for detail selectors
    dpSet(TOPDETAILSELECTIONACTIONDP,"Highlight|"+aSelection);
    dpSet(BOTTOMDETAILSELECTIONACTIONDP,"Highlight|"+aSelection);
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
    dpSet(PROGRESSBARACTIONDP,"Initialize");
    dpSet(HEADLINESACTIONDP,"Initialize");
    dpSet(ALERTSACTIONDP,"Initialize");
    return;
  }
  
  // TabChanged: The Tab has changed, so a new panle needs to be initialized and put in place
  if (anEvent == "TabChanged") {
    if (aSelection != ACTIVE_TAB) {
      navTabCtrl_removeView();
      
      ACTIVE_TAB = aSelection;
      if (navTabCtrl_showView()) {
        
        // change locator
        dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);

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
        observationType=sel[2];
        selection=sel[3];
      } else {
        typeSelector=sel[1];
        observationType="";
        selection=sel[2];      
      }
    }    
  }
  
  
  // Fill highlight string        
  string action = "Highlight";
  if (anEvent == "EventClick") { 
    // Empty highlight string
    dynClear(highlight);
    dynClear(strHighlight);
    for (int i=1;i<= dynlen(aSelection);i++){
      dyn_string sel = strsplit(aSelection[i],"|"); 
      LOG_DEBUG("navCtrl.ctl:navCtrl_handleDetailSelectionEvent|sel: "+sel); 
      if (dynlen(sel) > 1) {
        if (sel[1] == "Observations") {
          typeSelector=sel[1];
          observationType=sel[2];
          selection=sel[3];
          dynAppend(strHighlight,sel[3]);
          action+="|"+sel[1]+"|"+sel[2]+"|"+sel[3];
        } else {  // Hardware or processes
          typeSelector=sel[1];
          observationType="";
          selection=sel[2];
          dynAppend(strHighlight,sel[2]);
          action+="|"+sel[1]+"|"+sel[2];
        }
      }
    }   
    LOG_TRACE("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| strHighlight contains now: "+strHighlight);                          
    if (dynlen(strHighlight) > 0) {
      LOG_TRACE("navCtrl.ctl:navCtrl_handleDetailSelectionEvent| Kick trigger");
      dpSet(DPNAME_NAVIGATOR + g_navigatorID+".trigger",true);
         
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
        
      // change locator
      dpSet(LOCATORACTIONDP,"ChangeSelection|"+aSelection);

      // inform headLines Object
      dpSet(HEADLINESACTIONDP,"ChangeInfo|"+g_currentDatapoint);
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
