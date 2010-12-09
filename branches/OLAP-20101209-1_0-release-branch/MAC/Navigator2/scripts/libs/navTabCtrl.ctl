// navTabCtrl.ctl
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
// functions to control the Tabview of the navigator
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// navTabCtrl_initializeTabViews         : Initializes the tabview
// navTabCtrl_getTabCtrlName             : Returns the name of the tab control that contains the views
// navTabCtrl_getTabCtrl                 : Returns the tab control shape
// navTabCtrl_getViewPanels              : Returns  panels of the specified dp_view
// navTabCtrl_showView                   : Sets the panelView(s)
// navTabCtrl_getSelectedView            : Returns the caption of the currently selected view
// navTabCtrl_getNrTabs                  : Returns the nr of available tabs
// navTabCtrl_fillPanelChoice            : fills panelchoice with possible panels on this point and ennablesdisables the choice
// navTabCtrl_saveAndRestoreCurrentDP    : saves the currentDP to a global reflecting the tab, and restores the old to current if the new tab had a saved DP

#uses "navigator.ctl"

global string   TAB_VIEWS_CTRL_NAME          = "tabView";
global string   NAVIGATOR_TAB_FILENAME       = "navigator_viewSelection.pnl";
global bool     tabCtrlHasPanel              = FALSE;


///////////////////////////////////////////////////////////////////////////
//
// Function navTabCtrl_initializeTabViews()
//
// Initializes the tabview by hidding all tab-panes
//
///////////////////////////////////////////////////////////////////////////
void navTabCtrl_initializeTabViews()
{
  LOG_TRACE("navTabCtrl.ctl:navTabCtrl_initializeTabViews|entered");

  // hide all tabs
  dyn_errClass  err;
  int 	        setValueResult;
  string 				tabCtrl = navTabCtrl_getTabCtrlName();
  int 					nrTabs  = navTabCtrl_getNrTabs();
  
  int   	i = 0;
  do {
    setValueResult = setValue(tabCtrl, "registerVisible", i, FALSE);
    err = getLastError();
    i++;
  } while(dynlen(err) == 0 && i < nrTabs && setValueResult == 0);

  LOG_TRACE("navTabCtrl.ctl:navTabCtrl_initializeTabViews|ended");
}

///////////////////////////////////////////////////////////////////////////
//
// Function navTabCtrl_getTabCtrlName
//  
// returns the name of the tab control that contains the views
//  
///////////////////////////////////////////////////////////////////////////
string navTabCtrl_getTabCtrlName()
{
  return TAB_VIEWS_CTRL_NAME;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navTabCtrl_getTabCtrl
//  
// returns the tab control shape
//  
///////////////////////////////////////////////////////////////////////////
shape navTabCtrl_getTabCtrl()
{
  return getShape(navTabCtrl_getTabCtrlName());
}

///////////////////////////////////////////////////////////////////////////
//
// Function navTabCtrl_setSelectedTab
//  
// set the active tab
//  
///////////////////////////////////////////////////////////////////////////
void navTabCtrl_setSelectedTab(string tab)
{
  string tabCtrl      = navTabCtrl_getTabCtrlName();
  setValue (tabCtrl, "namedActiveRegister", tab);
}

///////////////////////////////////////////////////////////////////////////
//
// Function navTabCtrl_getNrTabs
//  
// returns the nr of Tabs
//  
///////////////////////////////////////////////////////////////////////////
int navTabCtrl_getNrTabs()
{  
  int 					nrTabs=0;
  string 				tabCtrl = navTabCtrl_getTabCtrlName();

  getValue(tabCtrl,"registerCount",nrTabs);
  
  return nrTabs;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navTabCtrl_getViewPanels() : configPanel
// 
// Returns  panels of the specified view
// when no panels available for the g_currentDatapoint
// we take g_currentDatapoint - last level and retry
//
///////////////////////////////////////////////////////////////////////////
dyn_string navTabCtrl_getViewPanels()
{
  dyn_string dpViews;
  LOG_TRACE("navTabCtrl.ctl:navTabCtrl_getViewPanels|entered ");
  
  LOG_DEBUG("navTabCtrl.ctl:navTabCtrl_getViewPanels|Active DP: " + g_currentDatapoint);
  
  
  // get DPT from current DP
  string panelConfigDP="";
  bool lowestLevel=false; // used to skip when allready looked at lowest level
  bool found=false;
  while (!found)  {
    if (dpExists(g_currentDatapoint)) {
      panelConfigDP=ACTIVE_USER+"."+dpTypeName(g_currentDatapoint)+"_"+ACTIVE_TAB;
    }
  
    if (dpExists(panelConfigDP)) {
      if (dpGet(panelConfigDP,dpViews)==-1) {
	LOG_ERROR("navTabCtrl.ctl:navTabCtrl_getViewPanels|Error obtaining panelViews: " + getLastError());
      } else {
        found=true;
        return dpViews;
      }
    } else {
   	LOG_DEBUG("navTabCtrl.ctl:navTabCtrl_getViewPanels|panelConfigDP doesn't exist: "+ panelConfigDP);
    }
    // Strip last element and retry
    // check if g_currentDatapoint < DB:LOFAR,
    // if so, reset and leave
    g_currentDatapoint = navFunct_dpStripLastElement(g_currentDatapoint);
    string dp = dpSubStr(g_currentDatapoint,DPSUB_SYS);
    if (dp == "") {
      if (ACTIVE_TAB == "Hardware") {
        g_currentDatapoint=MainDBName+"LOFAR_PIC_Europe";
      } else if (ACTIVE_TAB == "Observations") {
        g_currentDatapoint=MainDBName+"LOFAR_ObsSW";
      } else if (ACTIVE_TAB == "Processes") {
        g_currentDatapoint=MainDBName+"LOFAR_PermSW";
      }        
      if (lowestLevel) {
        return dpViews;
      } else {
        lowestLevel=true;
      }
    }
        
    //dpViews[1]="nopanel.pnl";
  }
  return dpViews;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navTabCtrl_getSelectedView() : CaptionOfSelectedTabPane
// 
// Returns the caption of the currently selected view
//
///////////////////////////////////////////////////////////////////////////
string navTabCtrl_getSelectedView()
{
	LOG_TRACE("navTabCtrl.ctl:navTabCtrl_getSelectedView|entered");

	string selectedView = "Observations";
  string tabCtrl      = navTabCtrl_getTabCtrlName();

  getValue(tabCtrl,"namedActiveRegister", selectedView);

	return selectedView;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navTabCtrl_showView()
// 
// Shows the tab 
//
///////////////////////////////////////////////////////////////////////////
bool navTabCtrl_showView(int panelNr=1)
{
  LOG_TRACE("navTabCtrl.ctl:navTabCtrl_showView|entered");

  // get Object name 
  string tabCtrl = navTabCtrl_getTabCtrlName();
          
  // get nr of available tabs
  int nrTabs = navTabCtrl_getNrTabs();
      
  // get all panels belonging to this view
  dyn_string viewPanels = navTabCtrl_getViewPanels();
   

  // empty all highlights
  dynClear(highlight);
  dynClear(strHighlight);
  
  // load the view
  if (dynlen(viewPanels) > 0) {
    // remove old view
    navTabCtrl_removeView();
    
    if (panelNr > dynlen(viewPanels)) {
      panelNr=1;
    }
    
    // check if currentDP is available in list and if so change panelNr to the one belonging to this
    // DP.
    
    
    for (int i=1; i<= dynlen(viewPanels); i++) {
      dyn_string splitName = strsplit(viewPanels[i],":");
      if (dynlen(splitName) > 1) {
        if (splitName[2] == dpSubStr(g_currentDatapoint,DPSUB_DP)) {
          panelNr=i;
          break;
        }
      }
    }      
    
    // check if chosen panel exists in the list
    if (panelSelection != "") {
      int nr=0;
      for (int i=1;i<=dynlen(viewPanels);i++) {

        if (strpos(viewPanels[i],panelSelection)>-1) {
          nr=i;
          break;
        }
      }
    
      if (nr > 0){
        panelNr=nr;
      } else {
        panelSelection = "";
      }
    }
    
    
    // check if the panel also has a DPName if available set g_currentDatapoint to that dpName
    dyn_string splitName = strsplit(viewPanels[panelNr],":");
    if (dynlen(splitName) > 1) {
      string syst=dpSubStr(g_currentDatapoint,DPSUB_SYS);
      LOG_DEBUG("navTabCtrl.ctl:navTabCtrl_showView|found new datapoint: "+splitName[2]+" for sys:" + syst);      
      g_currentDatapoint=syst+splitName[2];
    }
    LOG_DEBUG("navTabCtrl.ctl:navTabCtrl_showView|Trying to load panel: "+splitName[1]);
    setValue(tabCtrl,"namedRegisterPanel", ACTIVE_TAB, splitName[1], makeDynString(""));
    tabCtrlHasPanel=true;
    
    // fill and disable/enable the panelChoice combobox
    navTabCtrl_fillPanelChoice(viewPanels,panelNr);
    
    return true;
  } else{
    shape cb=getShape("panelChoice");
    cb.visible(false);
    g_currentDatapoint=""; 
  }   
  return false;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navTabCtrl_removeView()
// 
// removes the panel from the current tab. 
//
///////////////////////////////////////////////////////////////////////////
void navTabCtrl_removeView()
{
  LOG_TRACE("navTabCtrl.ctl:navTabCtrl_removeView|entered");

  // get Object name 
  string tabCtrl = navTabCtrl_getTabCtrlName();
  
  setValue(tabCtrl,"namedRegisterPanel", ACTIVE_TAB, "" , makeDynString(""));
  
}


///////////////////////////////////////////////////////////////////////////
//
// navTabCtrl_fillPanelChoice
// 
// fills panelchoice with possible panels on this point and enables or 
// disables the choice
//
///////////////////////////////////////////////////////////////////////////
void navTabCtrl_fillPanelChoice(dyn_string panels,int panelNr) {
  shape cb=getShape("panelChoice");

  cb.deleteAllItems();
  
  for ( int i = 1; i <= dynlen(panels); i++) {
    dyn_string splitted = strsplit(panels[i],"/\\.");
    string panel=splitted[dynlen(splitted)-1]; 
    cb.appendItem(panel);
  }
  dyn_string data;
  getValue(cb,"items",data);

  cb.selectedPos(1);
  if (cb.itemCount > 1) {
    cb.visible(true);
  } else {
    cb.visible(false);
  }
  cb.selectedPos(panelNr);
        
}

///////////////////////////////////////////////////////////////////////////
//
// navTabCtrl_saveAndRestoreCurrentDP
// 
// saves the g_currentDatapoint to a g_last"+ACTIVE_TAB+"Datapoint
// and if g_last"+newtab+"Datapoint != empty restore that point to g_currentDatapoint
//
///////////////////////////////////////////////////////////////////////////
void navTabCtrl_saveAndRestoreCurrentDP(string newtab) {

  if (ACTIVE_TAB == "Hardware" ) {
    g_lastHardwareDatapoint = g_currentDatapoint;
  } else  if (ACTIVE_TAB == "Processes" ) {
    g_lastProcessesDatapoint = g_currentDatapoint;
  } else  if (ACTIVE_TAB == "Observations" ) {
    g_lastObservationsDatapoint = g_currentDatapoint;
  }

  if (newtab == "Hardware" ) {
    // if systems have changed in dp, do not set to old datapoint.  
    if (dpSubStr(g_currentDatapoint,DPSUB_SYS) == dpSubStr(g_lastHardwareDatapoint,DPSUB_SYS)) {
      g_currentDatapoint = g_lastHardwareDatapoint;
    }
  } else  if (newtab == "Processes" ) {
    // if systems have changed in dp, do not set to old datapoint.  
    if (dpSubStr(g_currentDatapoint,DPSUB_SYS) == dpSubStr(g_lastProcessesDatapoint,DPSUB_SYS)) {
      g_currentDatapoint = g_lastProcessesDatapoint;
    }
  } else  if (newtab == "Observations" ) {
    // if systems have changed in dp, do not set to old datapoint.  
    if (dpSubStr(g_currentDatapoint,DPSUB_SYS) == dpSubStr(g_lastObservationsDatapoint,DPSUB_SYS)) {
      g_currentDatapoint = g_lastObservationsDatapoint;
    }
  } else {
    g_currentDatapoint = MainDBName+"LOFAR";
  }     
}


