//# gcfnavigator.ctl
//#
//#  Copyright (C) 2002-2004
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

//#
//# global functions for the Navigator.
//#

#uses "nav_fw/gcf-logging.ctl"
#uses "nav_fw/gcf-util.ctl"
#uses "nav_fw/gcf-common.ctl"
#uses "nav_fw/gcfnav-configuration.ctl"
#uses "nav_fw/gcfnav-tree.ctl"


global string   TAB_VIEWS_CTRL_NAME    = "TabViews";
global string   NAVIGATOR_TAB_FILENAME = "navigator_tab.pnl";
global bool     ACTIVEX_SUPPORTED      = false;
global int      NR_OF_VIEWS            = 10;
global bool     g_initializing         = true;
global int      STARTUP_DELAY = 1;
global string MESSAGE_DPACCESS = "Resource is not accessable.";
global string MESSAGE_ACCESSDENIED = "ACCESS DENIED\n\nYou have no rights to access this resource.";
//Enumaration for the use of userrights.
global int  UR_CONFIGSYSTEMSUBVIEW     = 19;
global int  UR_ADDREMOVESYSTEMSUBVIEW  = 20; 
global int  UR_COMMANDSOPERATOR        = 21;
global int  UR_COMMANDSMAINTENANCE     = 22;
global int  UR_COMMANDSASTRONOMY       = 23;
global int  UR_SYSTEMMANAGEMENT        = 24;
/////////////////////////////////////////////
//globals voor pathNames
global string g_path_temp     = ""; 
global string g_path_gnuplot  = ""; 
global string g_path_pictureconverter  = "";
global int g_path_temp_index     = 1;
global int g_path_gnuplot_index  = 2;
global int g_path_pictureconverter_index  = 3;

//==================== miscellaneous environmental functions =====================

///////////////////////////////////////////////////////////////////////////
//
// Function ActiveXSupported
//  
// returns true if the panel contains the ActiveX tree control
//  
///////////////////////////////////////////////////////////////////////////
bool ActiveXSupported() 
{ 
  return ACTIVEX_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////
//
// Function setActiveXSupported
//  
// sets the global variable that indicates if activeX is supported
//
///////////////////////////////////////////////////////////////////////////
void setActiveXSupported() 
{
  idispatch activeXctrl = 0;
  if (activeXctrl == 0) {
    LOG_TRACE("I cannot create a COM object!? What the ....?? You must be running Linux or something.", "");
    ACTIVEX_SUPPORTED = false;
  }
  else {
    LOG_TRACE("I can create a COM object! ", activeXctrl);
    releaseComObject(activeXctrl);
    ACTIVEX_SUPPORTED = true;
  }
}

///////////////////////////////////////////////////////////////////////////
//
// Function getTreeCtrlName
//  
// returns the name of the ActiveX tree control if activeX is supported, 
// returns the name of the emulated tree control otherwise
//  
///////////////////////////////////////////////////////////////////////////
string getTreeCtrlName()
{
  if (ActiveXSupported()) {
    return ACTIVEX_TREE_CTRL_NAME;
  }
  else {
    return LIST_TREE_CTRL_NAME;
  }
}

///////////////////////////////////////////////////////////////////////////
//
// Function getTreeCtrl
//  
// returns the ActiveX tree control shape if activeX is supported, 
// returns the emulated tree control shape otherwise
//  
///////////////////////////////////////////////////////////////////////////
shape getTreeCtrl()
{
  return getShape(getTreeCtrlName());
}

///////////////////////////////////////////////////////////////////////////
//
// Function getTabCtrlName
//  
// returns the name of the tab control that contains the views
//  
///////////////////////////////////////////////////////////////////////////
string getTabCtrlName()
{
  return TAB_VIEWS_CTRL_NAME;
}

///////////////////////////////////////////////////////////////////////////
//
// Function getTabCtrl
//  
// returns the tab control shape
//  
///////////////////////////////////////////////////////////////////////////
shape getTabCtrl()
{
  return getShape(getTabCtrlName());
}





//========================== draw and redraw functions ===========================

///////////////////////////////////////////////////////////////////////////
//
// Function refreshNavigator()
// 
// Refreshes the views of the navigator using the current selected DP
// as DP that show be redrawn!
// 
///////////////////////////////////////////////////////////////////////////
void refreshNavigator()
{
	LOG_DEBUG("refreshNavigator");

	if (g_initializing) {
		LOG_DEBUG("refreshNavigator suppressed while initializing ");
		return;
	}

	if (g_curSelNode == 0) {
		return;
	}

	LOG_TRACE("refreshNavigator  ", g_curSelNode);
	string datapointPath = buildPathFromNode(g_curSelNode);

	// if the datapointPath is a reference, it will be translated to the
	// the original datapointPath.
	dyn_string reference;
	bool 	   dpIsReference;
	checkForReference(datapointPath, reference, dpIsReference);
	string dpViewConfig = navConfigGetViewConfig(datapointPath);

	showView(dpViewConfig, datapointPath);
}

///////////////////////////////////////////////////////////////////////////
//
// Function showActiveView(string datapointPath)
// 
// Shows the active tab identified by the datapoint
//
///////////////////////////////////////////////////////////////////////////
void showActiveView(string dpViewConfig, string datapointPath)
{
  LOG_DEBUG("showActiveView", dpViewConfig, datapointPath);
  shape		tabCtrl 		  = getTabCtrl();
  string	viewsPath 		  = navConfigGetViewsPath();
  int		selectedViewTabId = tabCtrl.activeRegister + 1;
  LOG_DEBUG("showActiveView, active view = ", selectedViewTabId);

  navConfigSetSelectedElement(datapointPath);
  dyn_string panelParameters = makeDynString("$datapoint:" + datapointPath);

  // get tab properties
  dyn_string views = navConfigGetViews(dpViewConfig);
  
  for (int tabId = 1; tabId <= dynlen(views); tabId++) {
    if (tabId != selectedViewTabId) {
      // load empty panel in non-visible tabs to enhance performance
      tabCtrl.registerPanel(tabId - 1, viewsPath + "nopanel.pnl", makeDynString(""));
    }
    else {
      if (dpAccessable(views[selectedViewTabId])) {
        tabCtrl.registerPanel(selectedViewTabId - 1, NAVIGATOR_TAB_FILENAME, panelParameters);
      }
      else {
        LOG_TRACE("showActiveView", "tab reference not found; making tab invisible: ", selectedViewTabId);
        tabCtrl.registerVisible(selectedViewTabId - 1) = FALSE;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//
// Function showView(string datapointPath)
// 
// Shows the tab identified by the datapoint
//
///////////////////////////////////////////////////////////////////////////
void showView(string dpViewConfig, string datapointPath)
{
	LOG_DEBUG("showView", dpViewConfig, datapointPath);

	shape 	tabCtrl 		  = getTabCtrl();
	string	viewsPath 		  = navConfigGetViewsPath();
	int 	selectedViewTabId = 1;
	int 	tabId;

	navConfigSetSelectedElement(datapointPath);
	dyn_string panelParameters = makeDynString("$datapoint:" + datapointPath);
	// get the selected tab
	string selectedViewCaption = navConfigGetSelectedView();
	LOG_DEBUG("showView", "selected View:", selectedViewCaption);

	// get tab properties
	dyn_string views = navConfigGetViews(dpViewConfig);

	// At first find the selected tabview, and replace the current registerPanel
	// by an empty panel. So the panel doesn't blink any more.
	for (tabId = 1; tabId <= dynlen(views); tabId++) {
		if (dpAccessable(views[tabId])) {
			string caption = navConfigGetViewCaption(views[tabId]);
			if (strlen(caption) > 0) {
				if (caption == selectedViewCaption) {
					tabCtrl.registerPanel(tabId - 1, viewsPath + "nopanel.pnl", makeDynString(""));
				}
			}
		}
	}

	//Now arrange the other tabs
	for (tabId = 1; tabId <= dynlen(views); tabId++) {
		if (dpAccessable(views[tabId])) {
			string caption = navConfigGetViewCaption(views[tabId]);
			if (strlen(caption) > 0) {
				LOG_DEBUG("showView", "making tab visible: ", tabId, caption);
				tabCtrl.namedColumnHeader("tab" + tabId) = caption;
				tabCtrl.registerVisible(tabId - 1) = TRUE;

				// check if this tab is currently selected
				if (caption == selectedViewCaption) {
					LOG_DEBUG("showView", "caption=selectedViewCaption");
					selectedViewTabId = tabId;
					//bring this option outside this loop next line is new
					//tabCtrl.registerPanel(tabId - 1, NAVIGATOR_TAB_FILENAME, panelParameters);
				}
			}
			else {
				LOG_TRACE("showView", "empty caption or filename; making tab invisible: ", tabId);
				tabCtrl.registerVisible(tabId - 1) = FALSE;
			}
		}
		else {
			LOG_TRACE("showView", "tab reference not found; making tab invisible: ", tabId);
			tabCtrl.registerVisible(tabId - 1) = FALSE;
		}
		//delay(1, 0);
	}

	//NOW display the selected tabview, after the tabs are filled and ready
	tabCtrl.registerPanel(selectedViewTabId - 1, NAVIGATOR_TAB_FILENAME, panelParameters);

	// check if this tab is currently selected
	if (selectedViewCaption == "") {
		LOG_DEBUG("showView", "selectedViewCaption=\"\"");
		selectedViewTabId = 1;
		tabCtrl.registerPanel(0, NAVIGATOR_TAB_FILENAME, panelParameters);
	}
	tabCtrl.activeRegister = selectedViewTabId - 1;

	// make the rest of the views invisible
	int i;
	for (i = tabId; i <= NR_OF_VIEWS; i++) {
		LOG_TRACE("showView", "tab undefined; making tab invisible: ", i);
		tabCtrl.registerVisible(i - 1) = FALSE;
	}

}




//================================================================================

///////////////////////////////////////////////////////////////////////////
//
// EventHandler AlertStatusHandler()
// 
// called when an alert status changes
//
///////////////////////////////////////////////////////////////////////////
void AlertStatusHandler(string dpe, long status)
{
	LOG_DEBUG("AlertStatusHandler: ", dpe, status);

	if (!g_initializing) {
		LOG_TRACE("AlertStatusHandler()", dpe, status);
		// search dpe in tree
		long nodeId = getNodeFromDatapoint(dpe);
		if (nodeId >= 0) {
			shape treeCtrl = getTreeCtrl(); 
			if (ActiveXSupported()) {
				idispatch node;
				node = treeCtrl.ExtractNode(nodeId);
				node.Text = stripColorTags(node.Cells(1));
				if (status == 0) {
					node.Data = "";
				}
				else {
					node.Data = "A";
					string nodeText = addColorTags(node.Cells(1), "0x0000ff");
					LOG_TRACE(nodeText);
					node.Text = nodeText;
					LOG_TRACE(node.Text);
				}
				treeCtrl.InvalidateRow(node.GetRow());
			}
			else {
				// TODO
			}
		}
		LOG_TRACE("~AlertStatusHandler()", dpe, status);
	}
}



//======================== Action Handlers =======================================
//
// The following functions are used the handle the 'actions' that are 
// generated by the user by clicking on the tree object.
//


///////////////////////////////////////////////////////////////////////////
//
// Function Navigator_HandleEventInitialize()
//
// initializes the navigator
//
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleEventInitialize()
{
	LOG_DEBUG("Navigator_HandleEventInitialize()");

	// before the first thing, we check the sanity of the configuration
	string sanityMessage;

	// Set the global variable: NAVIGATOR_TAB_FILENAME
	NAVIGATOR_TAB_FILENAME = navConfigGetViewsPath() + "navigator_tab.pnl";

	// Set the global statecolors/colornames.
	initLofarColors();

	// Check if all datapoints of the type GCFNavViewConfiguration contain
	// valid data. These datapoint contain the administration of all panels
	// and subviews.
	if (!navConfigSanityCheck(sanityMessage)) {
		showPopupMessage("Sanity check failed", sanityMessage);
	}

	// first thing to do: get a new navigator ID
	// check the commandline parameter:
	int navID = 0;
	if (isDollarDefined("$ID")) {
		navID = $ID;
	}
	// make sure there is a __navigator<id> datapoint of the type GCFNavigatorInstance.
	navConfigSetNavigatorID(navID); 
	navConfigIncreaseUseCount();
	navConfigSubscribeUpdateTrigger("Navigator_HandleUpdateTrigger"); // __navigator<x>.triggerUpdate 
	
	g_initializing = true;

	// if ActiveX is supported use that treeObject otherwise use the 'normal' one.
	setActiveXSupported();
	LOG_TRACE("ActiveXSupported global variable set to ", ActiveXSupported());
	// show the ActiveX tree control if it can be created
	shape treeActiveX = getShape(ACTIVEX_TREE_CTRL_NAME);
	shape treeList    = getShape(LIST_TREE_CTRL_NAME);
	if (ActiveXSupported()) {
		treeActiveX.visible = TRUE;
		treeList.visible    = FALSE;
	}
	else {
		treeActiveX.visible = FALSE;
		treeList.visible    = TRUE;
	}

	// manually control the initialization of the tree and tabviews
	InitializeTabViews();

	//InitializeTree(); 
	//cannot do it here because tree will not be visible initially, only after double click. 
	//Strange but true

	delay(STARTUP_DELAY); // wait for the tree control to complete initialization
	g_initializing = false;

	// configure the tabs
	long selectedNode = getSelectedNode();
	if (selectedNode != 0) {
		string datapointPath = buildPathFromNode(selectedNode);
		string dpViewConfig  = navConfigGetViewConfig(datapointPath);
		if (dpAccessable(dpViewConfig)) {
			showView(dpViewConfig, datapointPath);
		}
		else {
			LOG_WARN("Unable to read view configuration",datapointPath,dpViewConfig);
		}
	}    

	LOG_DEBUG("~Navigator_HandleEventInitialize()");
}

///////////////////////////////////////////////////////////////////////////
//
// Function Navigator_HandleEventTerminate()
//
// NOTE: it is NOT possible to call dpGet in the terminate handler!
//
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleEventTerminate()
{
  LOG_DEBUG("Navigator_HandleEventTerminate()");
}

///////////////////////////////////////////////////////////////////////////
//
// Function Navigator_HandleEventClose()
//
// de-initializes the navigator
// NOTE: it is NOT possible to call dpGet in the terminate handler!
//
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleEventClose()
{
  LOG_DEBUG("Navigator_HandleEventClose()");

  navConfigDecreaseUseCount();
    
  PanelOff();
}

///////////////////////////////////////////////////////////////////////////
//
// Function Navigator_HandleUpdateTrigger()
//
// Refreshes the navigator and optionally change selection to 
// '__navigator<x>.newDataPoint'
//
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleUpdateTrigger(string dpe, int trigger)
{
	LOG_DEBUG("Navigator_HandleUpdateTrigger: ", dpe, trigger);

	string newDatapoint;
	dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_NEWDATAPOINT, newDatapoint);

	if (newDatapoint != "") {
		changeSelectedPosition(newDatapoint);
		dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_NEWDATAPOINT, "");
		refreshNavigator();
	}
	else {
		refreshNavigator();
	}
}

//========================= TabView routines =====================================

///////////////////////////////////////////////////////////////////////////
//
// Function InitializeTabViews()
//
// Initializes the tabview by hidding all tab-panes
//
///////////////////////////////////////////////////////////////////////////
void InitializeTabViews()
{
	LOG_DEBUG("InitializeTabViews()");

	// hide all tabs
	dyn_errClass err;
	int 		 setValueResult;
	shape 		 tabCtrl = getTabCtrl();
	int   		 i = 0;
	do {
		setValueResult = setValue(getTabCtrlName(), "registerVisible", i, FALSE);
		err = getLastError();
		LOG_TRACE("registerVisible", i, setValueResult, err);
		i++;
	} while(dynlen(err) == 0 && i < NR_OF_VIEWS && setValueResult == 0);

	LOG_DEBUG("~InitializeTabViews()");
}

///////////////////////////////////////////////////////////////////////////
//
// Function TabViews_HandleEventInitialize()
//
// initializes the tabview
//
///////////////////////////////////////////////////////////////////////////
void TabViews_HandleEventInitialize()
{
  // the initialization of the main panel initializes the tabviews
  // nothing should be done here
}

///////////////////////////////////////////////////////////////////////////
//
// Function TabViews_HandleEventSelectionChanged
// 
// stores the selected tab number in the database
//
///////////////////////////////////////////////////////////////////////////
void TabViews_HandleEventSelectionChanged()
{
	// don't handle changes during initialisation
	if (g_initializing) {
		LOG_DEBUG("TabViews_HandleEventSelectionChanged suppressed while initializing");
		return;
	}

	// If nothing is selected we are done.
	long  selectedNode = getSelectedNode();
	if (selectedNode == 0) {
		return;
	}

	LOG_DEBUG("TabViews_HandleEventSelectionChanged");

	shape		tabCtrl = getTabCtrl();
	string		datapointPath = buildPathFromNode(selectedNode);
	dyn_string	reference;
	bool 		dpIsReference;
	checkForReference(datapointPath, reference, dpIsReference);
	navConfigSetSelectedView(datapointPath, tabCtrl.activeRegister + 1);

	string dpViewConfig = navConfigGetViewConfig(datapointPath);
	if (dpAccessable(dpViewConfig)) {
		showActiveView(dpViewConfig, datapointPath);
	}
	else {
		LOG_WARN("Unable to read view configuration",datapointPath,dpViewConfig);
	}
}



///////////////////////////////////////////////////////////////////////////
// 
// Function ButtonMaximize_HandleEventClick()
// 
// Maximizes the current subview
// 
///////////////////////////////////////////////////////////////////////////
void ButtonMaximize_HandleEventClick()
{
	long Node = getSelectedNode();
	if (Node == 0) {
		return;
	}

	LOG_DEBUG("ButtonMaximize_HandleEventClick");

	dyn_errClass err;
	shape 		 tabCtrl = getTabCtrl();
	string		 datapointPath = buildPathFromNode(Node);
	string		 dpViewConfig = navConfigGetViewConfig(datapointPath);
	LOG_TRACE("ButtonMaximize_HandleEventClick", Node, dpViewConfig);

	if (dpAccessable(dpViewConfig)) {
		LOG_WARN("Unable to read view configuration",datapointPath,dpViewConfig);
		return;
	}

	int 		selectedView;
	int 		selectedSubView;
	dyn_string	views;
	dyn_int  	nrOfSubViews;
	dyn_string	subViews;
	dyn_string	configs;

	if (!navConfigGetViewConfigElements(dpViewConfig, selectedView,
							selectedSubView, views, nrOfSubViews, subViews, configs)) {
		LOG_WARN("Unable to read configElements:", selectedView, selectedSubView, 
					views, nrOfSubViews, subViews, configs);
		return;
	}

	LOG_TRACE("viewConfig:", selectedView, selectedSubView, views, nrOfSubViews, subViews, configs);

	if (selectedView == 0 || selectedSubView <= 0) {
		return;
	}

	// create the mapping
	int beginSubViews = 1;
	for (int i = 1; i < selectedView; i++) {
		beginSubViews += nrOfSubViews[i];
	}

	// get subviews config
	string subViewCaption;
	string subViewFileName;
	if (navConfigGetSubViewConfigElements(subViews[beginSubViews + selectedSubView - 1], 
											subViewCaption, subViewFileName)) {
		LOG_DEBUG("subviewcaption, subviewfilename:", subViewCaption, subViewFileName);

		string dpNameTemp = datapointPath;
		bool isReference;
		dyn_string reference;
		string referenceDatapoint = "";
		checkForReference(dpNameTemp, reference, isReference);

		if (isReference) {
			referenceDatapoint = datapointPath;
		}
		dyn_string panelParameters = makeDynString(
					"$datapoint:" + dpNameTemp,
					"$configDatapoint:" + configs[beginSubViews + selectedSubView - 1],
					"$referenceDatapoint:" + referenceDatapoint);
		ModuleOnWithPanel(dpNameTemp, -1, -1, 0, 0, 1, 1, "", subViewFileName, subViewCaption, panelParameters);
	}
}



