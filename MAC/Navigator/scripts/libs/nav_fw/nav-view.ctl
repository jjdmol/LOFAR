//# nav-com.ctl
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
//# global functions for the Navigator. All event handlers are implemented here
//#

global string   TAB_VIEWS_CTRL_NAME    = "TabViews";
global int      NR_OF_VIEWS            = 10;

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: 
//
// Params:
// Returns:
//////////////////////////////////////////////////////////////////////////////////

// -------------------------- HANDLERS -------------------------------------------

// -------------------------- PUBLIC METHODS -------------------------------------



// -------------------------- PRIVATE METHODS ------------------------------------
///////////////////////////////////////////////////////////////////////////
// FunctionName: showView(string datapointPath)
// 
// shows the tab identified by the datapoint
///////////////////////////////////////////////////////////////////////////
void showView(string dpViewConfig, string datapointPath)
{
  LOG_DEBUG("showView", dpViewConfig, datapointPath);
  shape tabCtrl = getTabCtrl();
  string viewsPath = navGetViewsPath();
  int selectedViewTabId = 1;
  int tabId;

  navConfigSetSelectedElement(datapointPath);
  dyn_string panelParameters = makeDynString("$datapoint:" + datapointPath);
  // get the selected tab
  string selectedViewCaption = navConfigGetSelectedView();
  LOG_DEBUG("showView", "selected View:", selectedViewCaption);

  // get tab properties
  dyn_string views = navConfigGetViews(dpViewConfig);

  // At first find the selected tabview, and replace the current registerPanel
  // by an empty panel. So the panel doesn't blink any more.
  for (tabId = 1; tabId <= dynlen(views); tabId++)
  {
    if (dpAccessable(views[tabId]))
    {
      string caption = navConfigGetViewCaption(views[tabId]);
      if (strlen(caption) > 0)
      {
        if (caption == selectedViewCaption)
        {
          tabCtrl.registerPanel(tabId - 1, viewsPath + "nopanel.pnl", makeDynString(""));
        }
      }
    }
  }
  
  //Now arrange the other tabs
  for (tabId = 1; tabId <= dynlen(views); tabId++)
  {
    if (dpAccessable(views[tabId]))
    {
      string caption = navConfigGetViewCaption(views[tabId]);
      if (strlen(caption) > 0)
      {
        LOG_DEBUG("showView", "making tab visible: ", tabId, caption);
        tabCtrl.namedColumnHeader("tab" + tabId) = caption;
        tabCtrl.registerVisible(tabId - 1) = TRUE;
        
        // check if this tab is currently selected
        if (caption == selectedViewCaption)
        {
          LOG_DEBUG("showView", "caption=selectedViewCaption");
          
          selectedViewTabId = tabId;
          //bring this option outside this loop next line is new
          //tabCtrl.registerPanel(tabId - 1, gNavigatorTabFile, panelParameters);
        }
      }
      else
      {
        LOG_TRACE("showView", "empty caption or filename; making tab invisible: ", tabId);
        tabCtrl.registerVisible(tabId - 1) = FALSE;
      }
    }
    else
    {
      LOG_TRACE("showView", "tab reference not found; making tab invisible: ", tabId);
      tabCtrl.registerVisible(tabId - 1) = FALSE;
    }
    //delay(1, 0);
  }

  //NOW display the selected tabview, after the tabs are filled and ready
  tabCtrl.registerPanel(selectedViewTabId - 1, gNavigatorTabFile, panelParameters);

  // check if this tab is currently selected
  if (selectedViewCaption == "")
  {
    LOG_DEBUG("showView", "selectedViewCaption=\"\"");
    selectedViewTabId = 1;
    tabCtrl.registerPanel(0, gNavigatorTabFile, panelParameters);
  }
  tabCtrl.activeRegister = selectedViewTabId - 1;
  
  // make the rest of the views invisible
  int i;
  
  for (i = tabId; i <= NR_OF_VIEWS; i++)
  {
    LOG_TRACE("showView", "tab undefined; making tab invisible: ", i);
    tabCtrl.registerVisible(i - 1) = FALSE;
  }

}

///////////////////////////////////////////////////////////////////////////
// FunctionName: showActiveView(string datapointPath)
// 
// shows the active tab identified by the datapoint
///////////////////////////////////////////////////////////////////////////
void showActiveView(string dpViewConfig, string datapointPath)
{
  LOG_DEBUG("showActiveView", dpViewConfig, datapointPath);
  shape tabCtrl = getTabCtrl();
  string viewsPath = navComGetViewsPath();
  int selectedViewTabId = tabCtrl.activeRegister + 1;
  LOG_DEBUG("showActiveView, active view = ", selectedViewTabId);

  navConfigSetSelectedElement(datapointPath);
  dyn_string panelParameters = makeDynString("$datapoint:" + datapointPath);

  // get tab properties
  dyn_string views = navConfigGetViews(dpViewConfig);
  
  for (int tabId = 1; tabId <= dynlen(views); tabId++)
  {
    if (tabId != selectedViewTabId)
    {
      // load empty panel in non-visible tabs to enhance performance
      tabCtrl.registerPanel(tabId - 1, viewsPath + "nopanel.pnl", makeDynString(""));
    }
    else
    {
      if (dpAccessable(views[selectedViewTabId]))
      {
        tabCtrl.registerPanel(selectedViewTabId - 1, gNavigatorTabFile, panelParameters);
      }
      else
      {
        LOG_TRACE("showActiveView", "tab reference not found; making tab invisible: ", selectedViewTabId);
        tabCtrl.registerVisible(selectedViewTabId - 1) = FALSE;
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  Action handlers
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
// FunctionName: TabViews_HandleEventInitialize()
//
// initializes the tabview
///////////////////////////////////////////////////////////////////////////
void TabViews_HandleEventInitialize()
{
  // the initialization of the main panel initializes the tabviews
  // nothing should be done here
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: InitializeTabViews()
//
// initializes the tabview
///////////////////////////////////////////////////////////////////////////
void InitializeTabViews()
{
  LOG_DEBUG("InitializeTabViews()");
  shape tabCtrl = getTabCtrl();
  // hide all tabs
  int i = 0;
  dyn_errClass err;
  int setValueResult;
  do
  {
    setValueResult = setValue(getTabCtrlName(), "registerVisible", i, FALSE);
    err = getLastError();
    LOG_TRACE("registerVisible", i, setValueResult, err);
    i++;
  } while(dynlen(err) == 0 && i < NR_OF_VIEWS && setValueResult == 0);
  LOG_DEBUG("~InitializeTabViews()");
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: TabViews_HandleEventSelectionChanged
// 
// stores the selected tab number in the database
///////////////////////////////////////////////////////////////////////////
void TabViews_HandleEventSelectionChanged()
{
  if (!g_initializing)
  {
    LOG_DEBUG("TabViews_HandleEventSelectionChanged");

    shape tabCtrl = getTabCtrl();
    long selectedNode = getSelectedNode();
    if (selectedNode != 0)
    {
      string datapointPath = buildPathFromNode(selectedNode);
      dyn_string reference;
      bool dpIsReference;
      checkForReference(datapointPath, reference, dpIsReference);
      navConfigSetSelectedView(datapointPath, tabCtrl.activeRegister + 1);
    
      string dpViewConfig = navConfigGetViewConfig(datapointPath);
      if (selectedNode != 0 && dpAccessable(dpViewConfig))
      {
        showActiveView(dpViewConfig, datapointPath);
      }
    }
  }
  else
  {
    LOG_DEBUG("TabViews_HandleEventSelectionChanged suppressed while initializing");
  }
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: TreeCtrl_EventOnSelChange(long Node)
// 
// initializes the Resources treeview
// TODO: optimize for selection change to the same resource type. Then don't reconfigure the tabs
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnSelChange(long Node)
{
  if (gCurSelTreeViewIdx != Node)
  {
    gCurSelTreeViewIdx = Node;
    refreshNavigator();   
  }
}


///////////////////////////////////////////////////////////////////////////
// TreeView_OnCollapse
// 
// called when an item is expanded
///////////////////////////////////////////////////////////////////////////
TreeView_OnCollapse(unsigned pos)
{
  LOG_DEBUG("TreeView_OnCollapse", pos);
  TreeCtrl_HandleEventOnCollapse(pos);

  // the last line of code of each treeView event handler MUST be the following:
  id = -1; 
}


///////////////////////////////////////////////////////////////////////////
// FunctionName: TreeCtrl_HandleEventOnCollapse(long Node)
// 
// expands a node in the Resources treeview
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnCollapse(unsigned Node)
{
  int collapsedNodes = 1;
  dyn_string datapoint;
  int k = 1;
  string temp;

  treeView_pruneChildren(Node, collapsedNodes, ""); //get all nodes which will be collapsed and
                                                      //clear these nodes from the tree
  //retrieve all dpnames for these nodes
  for (int j = Node + 1; j <= (Node + collapsedNodes); j++)
  {
    datapoint[k]= g_itemID2datapoint[j];
    k++;
  }

  //delete the collapse nodes from g_itemID2datapoint and g_datapoint2itemID
  for (int i = 1; i <= dynlen(datapoint); i++)
  {
    long nodeID = getNodeFromDatapoint(datapoint[i]);
    dynRemove(g_itemID2datapoint, dynContains(g_itemID2datapoint, datapoint[i]));      
    mappingRemove(g_datapoint2itemID, datapoint[i]);
  }

  //renumber the mapping of the dp's
  if (mappinglen(g_datapoint2itemID) > 1)
  {
    for (int i = 1; i <= mappinglen(g_datapoint2itemID); i++)
    {
      temp = g_itemID2datapoint[i];
      if (temp != "" && temp != 0)
      {
        g_datapoint2itemID[temp] = i;
      }
    }
  }
  //Delete references in g_referenceList for the related node
  string datapointPath = buildPathFromNode(Node);
  checkForReferenceRemoveFromList(datapointPath);
  
  //mark the node as COLLAPSED
  dyn_anytype parentNode;
  parentNode = treeView_getNode(Node, "");
  parentNode[treeView_STATE] = parentNode[treeView_STATE] & ~treeView_EXPANDED;
  treeView_replaceNode(parentNode, Node, "");
  treeView_draw();
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: checkForReferenceRemoveFromList
//
// Params: datapointPath - deletes al related references form the
//             g_referenceList
// 
// returns - none
///////////////////////////////////////////////////////////////////////////
void checkForReferenceRemoveFromList(string datapointPath)
{
  dyn_string refOut;
  
  for (int i = dynlen(g_referenceList); i >= 1; i--)
  {
    refOut = strsplit(g_referenceList[i], "=");
    if (patternMatch(datapointPath + "*", refOut[1]) && !(datapointPath == refOut[1]))
    {
      dynRemove(g_referenceList, i);
    }
  }
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: TreeCtrl_HandleEventOnDrawCell
// 
// draws icons in columns if needed
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnDrawCell(long Col, long Row, float Left, float Top, float Right, float Bottom)
{
  if (!g_initializing)
  {
    if (ActiveXSupported())
    {
      shape treeCtrl = getTreeCtrl();
      if (Row >= treeCtrl.FixedRows)
      {
        if (Col == 0)
        {
          idispatch aNode;
          aNode = treeCtrl.GetNodeAt(Left, Top);
          if (aNode != 0)
          {
            if (aNode.Data != 0)
            {
              if (aNode.Data == "A" && (treeCtrl.ImagesWidth < (Right - Left)))
              {
                LOG_TRACE("data is A and image fits", aNode.Data, treeCtrl.ImagesWidth, Right, Left);
                int aLeft, aRight;
                aLeft = Left + (Right - Left - treeCtrl.ImagesWidth) / 2;
                aRight = aLeft + treeCtrl.ImagesWidth;
                treeCtrl.DrawImage(1, aLeft, Top, aRight, Bottom);
              }
            }
          }
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//ButtonMaximize_HandleEventClick()
// 
// maximizes the current subview
///////////////////////////////////////////////////////////////////////////
void ButtonMaximize_HandleEventClick()
{
  LOG_DEBUG("ButtonMaximize_HandleEventClick");
  dyn_errClass err;
  shape tabCtrl = getTabCtrl();
  long Node = getSelectedNode();
  if (Node != 0)
  {
    string datapointPath = buildPathFromNode(Node);
    string dpViewConfig = navConfigGetViewConfig(datapointPath);
    LOG_TRACE("ButtonMaximize_HandleEventClick", Node, dpViewConfig);
    if (Node != 0 && dpAccessable(dpViewConfig))
    {
      int selectedView;
      int selectedSubView;
      dyn_string views;
      dyn_int  nrOfSubViews;
      dyn_string subViews;
      dyn_string configs;
      
      if (navConfigGetViewConfigElements(dpViewConfig,
            selectedView,
            selectedSubView,
            views,
            nrOfSubViews,
            subViews,
            configs))
      {
        LOG_TRACE("viewConfig:", selectedView, selectedSubView, views, nrOfSubViews, subViews, configs);
    
        if (selectedView > 0 && selectedSubView > 0)
        {
          // create the mapping
          int beginSubViews = 1;
          for (int i = 1; i < selectedView; i++)
          {
            beginSubViews += nrOfSubViews[i];
          }
          // get subviews config
          string subViewCaption;
          string subViewFileName;
          if (navConfigGetSubViewConfigElements(subViews[beginSubViews + selectedSubView - 1], subViewCaption, subViewFileName))
          {
            string viewsPath = navComGetViewsPath();
            LOG_DEBUG("subviewcaption, subviewfilename:", subViewCaption, viewsPath + subViewFileName);
  
            string dpNameTemp = datapointPath;
            bool isReference;
            dyn_string reference;
            string referenceDatapoint = "";
            checkForReference(dpNameTemp, reference, isReference);
            
            if (isReference)
            {
              referenceDatapoint = datapointPath;
            }
            dyn_string panelParameters = makeDynString(
              "$datapoint:" + dpNameTemp,
              "$configDatapoint:" + configs[beginSubViews + selectedSubView - 1],
              "$referenceDatapoint:" + referenceDatapoint);
            ModuleOnWithPanel(dpNameTemp, -1, -1, 0, 0, 1, 1, "", viewsPath + subViewFileName, subViewCaption, panelParameters);
          }
        }
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// TreeView_OnSelect
// 
// called when an item is selected
///////////////////////////////////////////////////////////////////////////
TreeView_OnSelect(unsigned pos)
{
  LOG_DEBUG("TreeView_OnSelect", pos);
  string datapointPath = buildPathFromNode(pos);
  dyn_string reference;
  bool parentDatapointIsReference;
  checkForReference(datapointPath, reference, parentDatapointIsReference);
  
  LOG_TRACE("check for expand", parentDatapointIsReference, datapointPath, dpAccessable(datapointPath));
  //Check or the access is permitted to this point in the tree
  if (checkDpPermit(datapointPath) || pos == 1)
  {
    //check or the selected item in the tree is an dpe. If yes, use the dp name to check the existance
    if (strpos(datapointPath, ".") > 0)
    {
     dyn_string datapointPathSplit = strsplit(datapointPath, ".");
     datapointPath = datapointPathSplit[1];
     //DebugN("datapointPath after split:" + datapointPath);
    }
    if (!parentDatapointIsReference || (parentDatapointIsReference && dpAccessable(datapointPath + "__enabled")))
    {
      TreeCtrl_HandleEventOnSelChange(pos);
    }
    else
    {
      navComMessageWarning(MESSAGE_DPACCESS); //dp not accessable
    }
  }
  else
  {
    navComMessageWarning(MESSAGE_ACCESSDENIED);  //access to resource denied
  }

  // the last line of code of each treeView event handler MUST be the following:
  id = -1; 
}






///////////////////////////////////////////////////////////////////////////
// FunctionName: SplitDatapointPath
// 
// returns the datapointPath in a dyn_string
///////////////////////////////////////////////////////////////////////////
dyn_string splitDatapointPath(string newDatapoint)
{
  int i;
  dyn_string datapointElement;
  dyn_string datapointPath= strsplit(newDatapoint, "_");
  string datapointName = datapointPath[1];
  // cut system name myself. Necessary for datapoint parts that are not datapoints themselves
  int sepPos = strpos(datapointName, ":");
  if (sepPos >= 0)
  {
    datapointName = substr(datapointName, sepPos + 1);
  }
  datapointPath[1] = datapointName;
  
  // if datapointElement present, split last entry of datapointPath
  datapointElement = strsplit(datapointPath[dynlen(datapointPath)], ".");
  if (dynlen(datapointElement) > 1)
  {
    datapointPath[dynlen(datapointPath)  ] = datapointElement[1];
    datapointPath[dynlen(datapointPath) + 1] = datapointElement[2];
  }
  return datapointPath;
}


//////////////////////////////////////////////////////////////////////////////////
// Function: progressBar
//           With this function a progress bar can be made. The progress is shown
//           horizontal and is going from the left to the right.
//           [0                                  100%]
// 
//Input: 1. maximum value (range) == 100%
//       2. current value to show in the progress bar.
///////////////////////////////////////////////////////////////////////////////////
void progressBar(float Maximum, float value)
{
  float Minimum = 0;
  int waarde;
  float positie;
  if (value > Minimum)
  {
    setValue("progressBar", "visible", TRUE);
    setValue("progressBar_border", "visible", TRUE);
  }
  
  setValue("progressBar", "scale", value/Maximum, 1.0);
  
  if (Maximum >= value)
  {
    delay(0, 200);
    setValue("progressBar", "visible", FALSE);
    setValue("progressBar_border", "visible", FALSE);
  }
}


dyn_string queryDatabaseForDpElements(string datapointPath)
{
  dyn_string output;
  int outputCounter = 1;
	dyn_string dpes = dpNames(dpSubStr(datapointPath, DPSUB_SYS_DP) + ".**;");
  int dpesLen = dynlen(dpes);
  for (int j = 1; j <= dpesLen; j++)
  {
  	if (dpElementType(dpes[j]) != DPEL_TYPEREF && dpElementType(dpes[j]) != DPEL_STRUCT)
  	{
	  	output[outputCounter] = dpes[j];
      outputCounter++;
    }
  }
  return output;
}

///////////////////////////////////////////////////////////////////////////
// Function queryDatabaseForDP: Query's the (distributed)database according the
//                         given options
// Input: 1. Datapoint name, including systemName
//        2. Attribute (E.g. "_online.._value")
//        3. Use function progressBar to display a progressBar
//
// Output: dyn_string with the resultsm exlusief the current datapointPath
///////////////////////////////////////////////////////////////////////////
dyn_string queryDatabaseForDP(string attribute, string datapointPath, bool useProgressBar)
{
  string tempDP;
  dyn_string output;
  dyn_dyn_anytype tab;
  string datapointPathOriginal = datapointPath;
  bool dpIsReference = false;
  dyn_string reference;
  string REMOTESYSTEM = "";

  checkForReference(datapointPath, reference, dpIsReference);
  if (dpIsReference)
  {
    strreplace(datapointPath, datapointPathOriginal, reference[2]);
    if (dpIsDistributed(datapointPath))
    {
	    REMOTESYSTEM = " REMOTE '" + strrtrim(dpSubStr(reference[2], DPSUB_SYS), ":") + "'";
	  }
  }
  else if (dpIsDistributed(datapointPath))
  {
    REMOTESYSTEM = " REMOTE '" + strrtrim(dpSubStr(datapointPath, DPSUB_SYS), ":") + "'";
  }
  //DebugN("SELECT '" + attribute + "' FROM '" + datapointPath + "__enabled' " + REMOTESYSTEM);
  dpQuery("SELECT '" + attribute + "' FROM '" + datapointPath + "__enabled' " + REMOTESYSTEM, tab);
  int maximumCount = dynlen(tab);
  for (int i = 2; i <= dynlen(tab); i++)
  {
    tempDP = tab[i][1];
    if (tempDP != "")
    {
      strreplace(tempDP, "__enabled.", "");
      if (checkDpPermit(tempDP))
      {
        if (tempDP != "")
        {
          dynAppend(output, tempDP);
        }
      }
    }
    if (useProgressBar)
      progressBar(maximumCount, i);
  }
  if (useProgressBar)
    progressBar(maximumCount, maximumCount);
  dynSortAsc(output); //sort the dyn_string output (alphanumeric)
  return output;
}

///////////////////////////////////////////////////////////////////////////
// Function queryDatabase: Query's the (distributed)database according the
//                         given options
//
// Input: 1. Attribute (E.g. "_online.._value")
//        2. Datapoint name, including systemName
//        3. How many first items must be retrieved via the query!!!
//        4. Search depth (relative, from current position)
//        5. Use function progressBar to display a progressBar
//
// Output: dyn_string with the resultsm exlusief the current datapointPath
//         
///////////////////////////////////////////////////////////////////////////
dyn_string queryDatabase(string attribute, string datapointPath, int first, int searchDepth, bool useProgressBar)
{
  dyn_string output;
  dyn_dyn_anytype tab;
  string fullDpName;
  int currentDepth = dynlen(strsplit(datapointPath, "_"));
  dyn_dyn_string elementNames;
  dyn_dyn_int elementTypes;
  string datapointPathOriginal = datapointPath;
  int elementIndex;
  int outputCounter = 1;
  bool dpIsReference = false;
  dyn_string reference;
  string REMOTESYSTEM = "";
  string firstXResults = "";
  
  checkForReference(datapointPath, reference, dpIsReference);
  if (dpIsReference)
  {
    //strreplace(datapointPath, datapointPathOriginal, reference[2]);
    if (dpIsDistributed(datapointPath))
    {
	    REMOTESYSTEM = " REMOTE '" + strrtrim(dpSubStr(reference[2], DPSUB_SYS), ":") + "'";
	  }
  }
  //How many items must be retrieved (this __enabled and this is a DP-Type)
  if (first > 0)
  {
    firstXResults = " FIRST " + first;
  }
  dpQuery("SELECT '" + attribute + "' FROM '" + datapointPath + "__enabled' " + firstXResults + REMOTESYSTEM, tab);
  int maximumCount = dynlen(tab);
  int maximumCounter = 0;
  int i = 2;
  if (dynlen(tab) >= 2)
  {
  	dyn_dyn_string dds;
    string fullDpName;
    int curDPTElevel = 2;
    int elType, elNTypesItemLen;
    dyn_string levels;
	  for (i = 2; i <= dynlen(tab); i++)
	  {
	    int functionOk;
	    fullDpName = tab[i][1];
	    strreplace(fullDpName, "__enabled.", "");

	    if ((dynlen(strsplit(fullDpName, "_")) <= (currentDepth + searchDepth)) || (searchDepth == 0))
	    {
	      dyn_string dpes = dpNames(fullDpName + ".**;");
	      int dpesLen = dynlen(dpes);
	      for (int j = 1; j <= dpesLen; j++)
	      {
	      	if (dpElementType(dpes[j]) != DPEL_TYPEREF && dpElementType(dpes[j]) != DPEL_STRUCT)
	      	{
		      	output[outputCounter] = dpes[j];	      	
			      outputCounter++;
			    }
		    }
	    }
	    maximumCounter++;
	    //if the progressBar must be used
	    if (useProgressBar)
	    {
	      progressBar(maximumCount, maximumCounter);
	    }
	  }//end of for loop
  }//end of if
  //Hide the progress bar
  if (useProgressBar)
  {
    progressBar(maximumCount, maximumCount);
  }
  dynSortAsc(output); //sort the dyn_string output (alphanumeric)
  return output;
}


//////////////////////////////////////////////////////////////////////////////////
// FunctionName: navConfigCheckResourceRoots,  used to check the current systemname
// and fill GCFNavigator resourceroots with the present Systemname
// 
///////////////////////////////////////////////////////////////////////////////////

void navConfigCheckResourceRoots()
{
  dyn_string roots;
  dpGet(DPNAME_NAVIGATOR + "." + ELNAME_PROPSETFILTERS, roots);
  string aSystemName = getSystemName();
  strreplace(aSystemName, ':', "");
  bool replaced = false;
  
  for (int i = 1; i <= dynlen(roots); i++)
  {
    dyn_string aS = strsplit(roots[i], ':');
    if (aS[1] != aSystemName) 
    {
      strreplace(roots[i], aS[1], aSystemName);
      replaced = true;
    }
  }
    
  if (replaced)
  {
    dpSet(DPNAME_NAVIGATOR + ELNAME_PROPSETFILTERS, roots);
  }
}

