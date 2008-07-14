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


#uses "navigator.ctl"

global string   TAB_VIEWS_CTRL_NAME          = "tabView";
global string   NAVIGATOR_TAB_FILENAME       = "navigator_viewSelection.pnl";


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
   	LOG_ERROR("navTabCtrl.ctl:navTabCtrl_getViewPanels|panelConfigDP doesn't exist: "+ panelConfigDP);
    }
    // Strip last element and retry
    // check if g_currentDatapoint < DB:LOFAR,
    // if so, reset and leave
    g_currentDatapoint = navFunct_dpStripLastElement(g_currentDatapoint);
    string dp = dpSubStr(g_currentDatapoint,DPSUB_SYS);
    if (dp == "") {
      g_currentDatapoint=MainDBName+"LOFAR";
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
bool navTabCtrl_showView()
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
    
    LOG_DEBUG("navTabCtrl.ctl:navTabCtrl_showView|Trying to load panel: "+viewPanels[1]);
    setValue(tabCtrl,"namedRegisterPanel", ACTIVE_TAB, viewPanels[1], makeDynString(""));
    return true;
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
