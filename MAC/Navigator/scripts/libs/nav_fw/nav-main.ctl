//# nav-main.ctl
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

#uses "nav_fw/gcfnav-pmlinterface.ctl"
#uses "nav_fw/gcf-util.ctl"
#uses "nav_fw/gcf-common.ctl"
#uses "nav_fw/gcfnav-configuration-functions.ctl"

global dyn_string  g_itemID2datapoint;
global mapping  g_datapoint2itemID;
global bool     g_initializing         = true;
global int      STARTUP_DELAY = 1;
global int      id;                    //needed for changing the selection in the tree (panel navigation, ER 218)
global int      treeAddCount = 0;      //test teller for performance issue
global unsigned g_nodeID               = 0;
global unsigned g_parentIndex          = 0;
global dyn_string  g_referenceList = "";
//Enumaration for the use of userrights.
global int  UR_TREEACCESS1            = 6;
global int  UR_TREEACCESS2            = 7;
global int  UR_TREEACCESS3            = 8;
global int  UR_TREEACCESS4            = 9;
global int  UR_TREEACCESS5            = 10;
global int  UR_TREEACCESS6            = 11;
global int  UR_TREEACCESS7            = 12;
global int  UR_TREEACCESS8            = 13;
global int  UR_TREEACCESS9            = 14;
global int  UR_TREEACCESS10            = 15;
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

// Includes
#uses "nav_fw/treeView.ctl"
#used "nav_fw/nav-tabview.ctl"
#used "nav_fw/nav-config.ctl"

// Constants
global string   DPNAME_NAVIGATOR                = "__navigator";
global string   ELNAME_PROPSETFILTERS           = ".resourceRoots";
global string   ELNAME_IGNOREENABLEDROOTS       = ".ignoreEnabledRoots";
global string   ELNAME_ENVIRONMENTNAMES         = ".environmentNames";
global string   ELNAME_ENVIRONMENTGROUPS        = ".environmentGroups";
global string   ELNAME_SELECTEDELEMENT          = ".selectedElement";
global string   ELNAME_SELECTEDVIEWCAPTION      = ".selectedViewCaption";
global string   ELNAME_USECOUNT                 = ".useCount";
global string   ELNAME_VIEWSPATH                = ".viewsPath";
global string   ELNAME_VIEWS                    = ".views";
global string   ELNAME_CAPTION                  = ".caption";
global string   ELNAME_FILENAME                 = ".filename";
global string   ELNAME_CONFIGPANEL              = ".configPanel";
global string   ELNAME_SELECTEDVIEW             = ".selectedView";
global string   ELNAME_SELECTEDSUBVIEW          = ".selectedSubView";
global string   ELNAME_SELECTEDENVIRONMENT      = ".selectedEnvironment";
global string   ELNAME_NROFSUBVIEWS             = ".nrOfSubViews";
global string   ELNAME_SUBVIEWS                 = ".subViews";
global string   ELNAME_CONFIGS                  = ".configs";
global string   ELNAME_TRIGGERUPDATE            = ".triggerUpdate";
global string   ELNAME_NEWDATAPOINT             = ".newDatapoint";
const string  	ELNAME_TREEACCESS								= ".treeAccess";
global string   ELNAME_MESSAGE		              = ".message";
global string   DPTYPENAME_NAVIGATOR_INSTANCE   = "GCFNavigatorInstance";

global string MESSAGE_DPACCESS = "Node is not accessable.";
global string MESSAGE_ACCESSDENIED = "ACCESS DENIED\n\nYou have no rights to access this node.";

global string   ACTIVEX_TREE_CTRL_NAME = "FlyTreeXCtrl1";
global string   LIST_TREE_CTRL_NAME    = "list";
global string   TAB_VIEWS_CTRL_NAME    = "TabViews";
global int      NR_OF_VIEWS            = 10;

// Globals
global int      gNavigatorID 				= 0;
global bool     gShowProperties     = false; //Show datapoint elements in the tree. If yes=> do!
global bool     gActiveXSupported   = false;
global string   gNavigatorTabFile 	= "navigator_tab.pnl";
global int      gCurSelTreeViewIdx 	= 1;


//////////////////////////////////////////////////////////////////////////////////
// FunctionName: 
//
// Params:
// Returns:
//////////////////////////////////////////////////////////////////////////////////

// -------------------------- HANDLERS -------------------------------------------

///////////////////////////////////////////////////////////////////////////
// FunctionName: TreeView_OnInit
// 
// called when the list is initialized
///////////////////////////////////////////////////////////////////////////
TreeView_OnInit()
{
  LOG_DEBUG("TreeView_OnInit");

  TreeCtrl_HandleEventInitialize();  

  // the last line of code of each treeView event handler MUST be the following:
  id = -1; 
}

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: TreeCtrl_HandleEventInitialize()
//
// initializes the Resources treeview
//////////////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventInitialize()
{
  initTree();
}

///////////////////////////////////////////////////////////////////////////
// TreeView_OnExpand
// 
// called when an item is expanded
///////////////////////////////////////////////////////////////////////////
TreeView_OnExpand(unsigned treeViewIdx)
{
  LOG_DEBUG("TreeView_OnExpand", selNodeIdx);

	TreeCtrl_HandleEventOnExpand(selNodeIdx)
  // the last line of code of each treeView event handler MUST be the following:
  id = -1; 
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: TreeCtrl_EventOnExpand(long Node)
// 
// expands a node in the Resources treeview
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnExpand(long treeViewIdx)
{
  dyn_anytype node = navConvTreeIdx2Node(treeViewIdx);
  
  if (dynlen(node) != treeView_INFO)
  {
    return;
  }      
  if (checkDpPermit(node[treeView_INFO]) || node[treeView_PARENTID] == 0)
  {
    dyn_string propSets = getPropertySets(node[treeView_INFO], 2);
	  LOG_DEBUG("adding property sets: ", LOG_DYN(propSets));
	  addPropSetsToTree(node[treeView_PATH], propSets);
	  // also call the default OnExpand implementation to expand the node
	  treeView_defaultOnExpandNode(treeViewIdx);  
  }
  else
  {
    navMessageWarning(MESSAGE_ACCESSDENIED);  //access to resource denied
  }
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: TreeView_OnSelect
// 
// called when an item is selected
///////////////////////////////////////////////////////////////////////////
TreeView_OnSelect(unsigned pos)
{
  LOG_DEBUG("TreeView_OnSelect", pos);
	TreeCtrl_HandleEventOnSelChange(pos);

  // the last line of code of each treeView event handler MUST be the following:
  id = -1; 
}
///////////////////////////////////////////////////////////////////////////
// FunctionName: TreeCtrl_EventOnSelChange(long Node)
// 
// initializes the Resources treeview
// TODO: optimize for selection change to the same resource type. Then don't reconfigure the tabs
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnSelChange(long treeViewIdx)
{
  dyn_anytype node = navConvTreeIdx2Node(treeViewIdx);
  
  if (dynlen(node) != treeView_INFO)
  {
    return;
  }      
  if (checkDpPermit(node[treeView_INFO]) || node[treeView_PARENTID] == 0)
  {
  	string dpViewConfig = navConfigGetViewConfig(node[treeView_INFO]);
    showView(dpViewConfig, datapointPath);
  }
  else
  {
    navMessageWarning(MESSAGE_ACCESSDENIED);  //access to resource denied
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
  treeView_deleteChildren(Node); 
  // TODO delete only childrens of childrens
  treeView_defaultOnCollapseNode(Node);
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: Navigator_HandleUpdateTrigger()
//
// refreshes the navigator
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleUpdateTrigger(string dpe, int trigger)
{
  string newDatapoint;
  dpGet(DPNAME_NAVIGATOR + gNavigatorID + ELNAME_NEWDATAPOINT, newDatapoint);
  if (newDatapoint != "")
  {
    changeSelectedPosition(newDatapoint);
    dpSet(DPNAME_NAVIGATOR + gNavigatorID + ELNAME_NEWDATAPOINT, "");
    refreshNavigator();
  }
  else
  {
    refreshNavigator();
  }
  
}
// -------------------------- PUBLIC METHODS -------------------------------------

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: navConvTreeIdx2Node
//
// Params: treeViewIdx - index of an item of the treeView
// Returns: node object, empty array if treeViewIdx not valid, otherwise it contains 'treeView_INFO' items
///////////////////////////////////////////////////////////////////////////////////
dyn_anytype navConvTreeIdx2Node(unsigned treeViewIdx)
{
	unsigned nodeId = treeView_convTreeIdx2NodeId(treeViewIdx);
	return treeView_getNode(nodeId);
}

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: navConvTreeIdx2NodePath
//
// Params: treeViewIdx - index of an item of the treeView
// Returns: the complete node path including the node name on the index, "" if treeViewIdx not valid
//////////////////////////////////////////////////////////////////////////////////
string navConvTreeIdx2NodePath(unsigned treeViewIdx)
{
	string nodePath = "";
	dyn_anytype node = navConvTreeIdx2Node(treeViewIdx);
	if (dynlen(node) == treeView_INFO)
	{
		nodePath = node[treeView_PATH];
	}
	return nodePath;
}

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: navConvTreeIdx2DP
//
// Params: treeViewIdx - index of an item of the treeView
// Returns: the node info field of the node on the index, "" if treeViewIdx not valid
//////////////////////////////////////////////////////////////////////////////////
string navConvTreeIdx2DP(unsigned treeViewIdx)
{
	string dp = "";
	dyn_anytype node = navConvTreeIdx2Node(treeViewIdx);
	if (dynlen(node) == treeView_INFO)
	{
		dp = node[treeView_INFO];
	}
	return dp;
}

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: navMessageWarning
//
// used to display a message to the end-user
// 
///////////////////////////////////////////////////////////////////////////////////
void navMessageWarning(string message)
{
  ChildPanelOnCentralModal(navGetViewsPath() + "MessageWarning.pnl", "Warning", makeDynString("$1:" + message));
}

///////////////////////////////////////////////////////////////////////////
//Function navGetViewsPath()
// 
// returns the relative path where the navigator views are stored
///////////////////////////////////////////////////////////////////////////
string navGetViewsPath()
{
  dpGet(DPNAME_NAVIGATOR + ELNAME_VIEWSPATH, viewsPath);
  if (viewsPath == "") viewsPath = "nav_fw/";
  return viewsPath;
}

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: navCorrectResourceRoots,  used to check the current systemname
// and fill GCFNavigator resourceroots with the present Systemname
// 
///////////////////////////////////////////////////////////////////////////////////

void navCorrectResourceRoots()
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

// -------------------------- PRIVATE METHODS ------------------------------------

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: initTree()
//
// initializes the treeview with propertysets
//////////////////////////////////////////////////////////////////////////////////
void initTree()
{
  LOG_DEBUG("initTree()");
 
  shape treeCtrl = getTreeCtrl();
  idispatch items;
  if (ActiveXSupported())
  {
    items = treeCtrl.Items;
    items.BeginUpdate();
    items.Clear();
    treeCtrl.SortType = 0;
  }
  
  // get top level property sets. "" means root, 2 means: 2 level depth
  dyn_string propSets = getPropertySets("", 2);
  LOG_DEBUG("adding property sets: ", LOG_DYN(propSets));
  addPropSetsToTree("", propSets);
  treeView_defaultOnExpandNode(0); // shows the root nodes
  if (ActiveXSupported())
  {
    items.EndUpdate();
  }
  LOG_DEBUG("~initTree()");
  delay(2, 0);
  shape treeList    = getShape(LIST_TREE_CTRL_NAME);
  gCurSelTreeViewIdx = 1;
  LOG_TRACE("Init,  set selected node[1]", gCurSelTreeViewIdx);
  treeList.selectedPos = 1;
}

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: getPropertySets
//
// Params: parentDatapoint - get the children of this datapoint
//         depth           - how many levels of children to get
// 
// Returns: the names of the property sets, which have to be added to the tree
//////////////////////////////////////////////////////////////////////////////////
dyn_string getPropertySets(string parent, int depth)
{
  dyn_string foundPropSets;
  dyn_string allFoundDPs;
  dyn_string propSetFilters;
  dyn_errClass err;
  int maxDepth;

  /////////////////////////////////////////////////////////////
	// 1) Determine whether the parent is the root or not
	//    and determine a list of property set filters
  if (parent == "")
  {
  	// parent is root
    maxDepth = depth;
    // read the filters from the navigator configuration
    dpGet(DPNAME_NAVIGATOR + ELNAME_PROPSETFILTERS, propSetFilters);
    err = getLastError();
    if (dynlen(err) > 0 || dynlen(propSetFilters) == 0)
    {
      // if nothing specified, take the local PIC, PAC, VIC or GSO trees
      propSetFilters = makeDynString("PIC", "PAC", "VIC", "GSO");
    }  
  }
  else
  {
    dyn_string dpPathElements = strsplit(parent, ":_");
    maxDepth = depth + dynlen(dpPathElements);
    // if the parent does not contain a system name than the maxDepth must be increased with 1
    // because the dpNames method always returns the systemname in the DP name
    // and the system name is always a separate level too
    if (strpos(parent, ":") < 0) maxDepth++;
    propSetFilters = makeDynString(parent);
  }

  /////////////////////////////////////////////////////////////
  // 2) Get all property sets (DP's), which matches with the filter
  for (int i = 1; i <= dynlen(propSetFilters); i++)
  {
    // query the database for all propSets under the given root (filter)
    // NOTE: This query also includes the "__enabled" DP's
    dynAppend(allFoundDPs, dpNames(propSetFilters[i] + "*"));
  }
  
  /////////////////////////////////////////////////////////////
  // 3) Make a list of enabled property sets (enabledPropSets)
  //    and a list of existing property sets, which are not enabled (checkPropSets)
  //		These property sets (checkPropSets) maybe will never be enabled (for instance SNMP related stuff).
  //	  They are marked by the ignoredEnabledRoots list in the navigator configuration and 
  //		have therefor to be checked against this configuration (see step 4).
  LOG_DEBUG("All found DP's: ", dynlen(allFoundDPs));
  dyn_string allFoundPropSets;
  dyn_string checkPropSets = allFoundDPs; // collection with DP's, which have no "__enabled" equivalent
  dyn_string enabledPropSets = dynPatternMatch("*__enabled", allFoundDPs);
  int propSetIdx, enablePropSetIdx;
  for (int j = 1; j <= dynlen(enabledPropSets); j++)
  {
    string enabledPropSetsItem = enabledPropSets[j];
    strreplace(enabledPropSetsItem, "__enabled", "");
    propSetIdx = dynContains(checkPropSets, enabledPropSetsItem);
    if (propSetIdx > 0)
    {
      LOG_DEBUG("Adding: ", enabledPropSetsItem);
      dynAppend(allFoundPropSets, enabledPropSetsItem);
      dynRemove(checkPropSets, propSetIdx);
	    enablePropSetIdx = dynContains(checkPropSets, enabledPropSetsItem + "__enabled");
      if (enablePropSetIdx > 0)
      {
	      dynRemove(checkPropSets, enablePropSetIdx));
	    }
    }
    else if (0 == propSetIdx)
    {
    	// normally the property set should always be of category "temporary" in this case
      if (navPMLisTemporary(enabledPropSetsItem))
      {
        LOG_DEBUG("Adding temp: ", enabledPropSetsItem);
        dynAppend(allFoundPropSets, enabledPropSetsItem);
      }
    }
  }
  
  ///////////////////////////////////////////////////////////////
	// 4) If there are property sets (DP's), which are not enabled, they have to be checked against the
	//    ignoreEnableRoots list.
  if (dynlen(checkPropSets) > 0)
  {
    dyn_string ignoreEnabledFilters;
    dpGet(DPNAME_NAVIGATOR + ELNAME_IGNOREENABLEDROOTS, ignoreEnabledFilters);
		// check the ignoreEnabledFilters field
    err = getLastError();
    if (dynlen(err) == 0)
    { 
    	dyn_string foundIgnoreItems;   
	    for (int n = 1; n <= dynlen(ignoreEnabledFilters); n++)
	    {
	    	foundIgnoreItems = dynPatternMatch(ignoreEnabledFilters[n] + "*", checkPropSets);
	      dynAppend(allFoundPropSets, foundIgnoreItems);
	    }
    }
  }

  ///////////////////////////////////////////////////////////////
  // 5) strip everything below the requested level
  // 		remove duplicates  
 	int d;
  dyn_string psPathElements;
  string selectedPropSet;
  for (int i = 1; i <= dynlen(allFoundPropSets); i++)
  {
	  psPathElements = strsplit(allFoundPropSets[i], "_");
	  // the first element always contains the system name and is also a separate level 
	  // this level therefore can be default skipped 
	  for (d = 2; d <= maxDepth && d <= dynlen(psPathElements); d++)
	  {
	    if (d > 2)
	      selectedPropSet += "_";
	    selectedPropSet += psPathElements[d - 1];
	  }
	  if (!dynContains(foundPropSets, selectedPropSet))
	  {
	    LOG_DEBUG("Adding: ", selectedPropSet);
	    dynAppend(foundPropSets, selectedPropSet);
	  }
  }
  dynSortAsc(foundPropSets);
  return foundPropSets;
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: addPropSetsToTree()
//      
// Adds names of property sets and their elements to the treeview
//
// Params: parent -
//				 names  - names of the property sets to add
//
///////////////////////////////////////////////////////////////////////////
void addPropSetsToTree(string parent, dyn_string names)
{
  if (dynlen(names) > 0)
  {
	  string systemName;

  	/////////////////////////////////////////////////////////////////////
  	// 1) Determine parentId and add systemname as root node if necessary
  	unsigned parentId = 0;
    systemName = strrtrim(dpSubStr(names[1], DPSUB_SYS), ":");
  	if (getSystemName() == (systemName + ":") && parent == "")
  	{
  		treeView_addNode(systemName, parentId, getSystemName());
  	}
  	else
  	{
  		dyn_unsigned nodeIds = treeView_findNodes(parent);
	  	if (dynlen(nodeIds) == 1)
	  	{
	  		parentId = nodeIds[1];
	  	}
	  	else
	  	{
	  		LOG_FATAL("parent '" + parent + "' was not known in the treeView!!! No items added!!!");
	  		return;
	  	}
  	}
		dyn_anytype parentNode = treeView_getNode(parentId);
		string parentDp = "";
		if (dynlen(parentNode) == treeView_INFO)
		{
			parentDp = parentNode[treeView_INFO];
		}
		string curName;
		string splittedName;
		int j, k, l, m;
		unsigned curNewParentId, curNewPropParentId;
		dyn_string propertyStructure;
		string curPropPath, fullPropPath;
		string splittedProp;
		string elementName;
		dyn_string references, referenceSplit, refPropSets;
    for (int i = 1; i <= dynlen(names); i++)
    {
    	curName = names[i];
			// cut the parentDp part from the current dp name to get only the (right) part 
			// to be added to the tree
			strreplace(curName, parentDp, "");
			splittedName = strsplit(curName, "_");
			curNewParentId = parentId;
			for (j = 1; j <= dynlen(splittedName); j++)
			{
				curNewParentId = treeView_addNode(splittedName[j], curNewParentId, names[i]);
				if (curNewParentId < 1) break;
			}
			curNewPropParentId = curNewParentId;
			if (j > dynlen(splittedName))
			{
   			propertyStructure = getPSTypeStructure(names[i]);
 				for (k = 1; k <= dynlen(propertyStructure); k++)
   			{
   				curNewParentId = curNewPropParentId;
   				curPropPath = propertyStructure[k];
   				splittedProp = strsplit(curPropPath, ".");
   				fullPropPath = names[i];
   				for (l = 1; l <= dynlen(splittedProp); l++)
   				{
   					fullPropPath += "." + splittedProp[l];
   					if ("__" == substr(splittedProp[l], 0, 2))
   					{
   						if (dpAccessable(fullPropPath))
   						{    							
   							dpGet(fullPropPath, references);
   							for (m = 1; m <= dynlen(references); m++)
   							{
	                referenceSplit = strsplit(references[m], "=");
	                string referenceViewText = referenceSplit[1] + referenceSign(referenceSplit[2]);
				    			curNewParentId = treeView_addNode(referenceViewText, curNewParentId, referenceSplit[2]);					    			
				    			if (curNewParentId < 1) break;
				    			refNode = treeView_getNode(curNewParentId);
	                // Because this is a reference, the PS's of the branch must be retrieved and
	                // added to the tree.
				    			if (dynlen(refNode) > 0)
				    			{
				    				refNodePath = refNodePath[treeView_PATH];
		                LOG_TRACE("Add reference: ", referenceViewText);
		                dyn_string refPropSets = getPropertySets(referenceSplit[2], 1);
		                addPropSetsToTree(refNodePath, refPropSets);
				    			}
				    			// else -> refnode was not added succesfully
   							} // end m loop
   						} 
   						// it is sure that this property can no has childs
   						break;
   					}
   					else if (gShowProperties)
   					{
   						curNewParentId = treeView_addNode(splittedProp[l], curNewParentId, fullPropPath);
   					} // fi "__"
   				} // end l loop
   			} // end k loop
   		} // fi j > dynlen(..)
    } // end i loop
	} // fi dynlen(names)
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: getPSTypeStructure()
//      
// Gets the type structure of a property set (PS) located on a local system or remote system
// even if PS does not exists as DP, if not exists than the __enabled DP will be used
// if DPtype in the __enabled DP has no DP instance than a dummy DP will be created
//
// Params: ps - property set from which the typestructure is requested
//						  NOTE: The ps param may not contains element, config or attribute tags
// Returns: a list with the type structure including the root element of the PS
///////////////////////////////////////////////////////////////////////////
dyn_string getPSTypeStructure(string ps)
{
	dyn_string propertyStructure = makeDynString();
	if (dpAccessable(ps))
	{
    propertyStructure = dpNames(ps + ".**");
	}
	else
	{
		string systemName;
		string psType = getPropSetType(ps);
	  if (psType != "")
	  {	  	
	  	systemName = dpSubStr(ps + "__enabled.", DPSUB_SYS);
			dyn_string dps = dpNames(systemName + "*", psType);			
			if (dynlen(dps) == 0)
			{
				ps = "__dummy_" + psType; // system name may not be included in name of the DP to be created
				dpCreate(ps, psType, getSystenId(systemName));
				ps = systemName + ps;
			}
			else
			{
				ps = dps[1];
			}
			propertyStructure = dpNames(ps + ".**");
		}
	}
	for (int i = 1; i <= dynlen(propertyStructure); i++)
	{
		propertyStructure[i] = substr(propertyStructure[i], strlen(ps));
	}
	return propertyStructure;
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: getPropSetType()
//
// retrieves the Type information from DP == property set, otherwise from <DP>__enabled
//
// Params: psName - property set name, including systemName
//
// Returns: Property set type
//         
///////////////////////////////////////////////////////////////////////////
string getPropSetType(string psName)
{
  string propSetType = dpTypeName(psName);
  if (strlen(propSetType) == 0)
  {
	  if (dpAccessable(psName + "__enabled."))
	  {
		  string enabledValue;
		  dyn_string enabledValueSplit;
		  dpGet(psName + "__enabled.", enabledValue);
		  enabledValueSplit = strsplit(enabledValue, "|");
		  if (dynlen(enabledValueSplit) == 2)
		  {
		    propSetType = enabledValueSplit[2];
		  }
		  else
		  {  
		    LOG_WARN("Erronous '" + psName + "__enabled' value: " + enabledValueSplit);
		  }
	  }
  }
  return propSetType;
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: referenceSign()
// 
// Params: reference - reference property set
// Returns: " ->>" if reference is distributed
//          " ->"  if reference is NOT distributed
///////////////////////////////////////////////////////////////////////////
string referenceSign(string reference)
{
 string refSign;
 if (dpIsDistributed(reference))
 {
   refSign = " ->>";
 }
 else
 {
   refSign = " ->";
 }

  return refSign;
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: dpIsDistributed()
//
// Checks whether the given dpName is on another system or not
//
// Params: dpName - Datapoint name, including systemName
//
// Returns: TRUE,  given dpName is on another system
//          FALSE, given dpName is the same system
///////////////////////////////////////////////////////////////////////////
bool dpIsDistributed(string dpName)
{
  string dpSystemName = strrtrim(dpSubStr(dpName, DPSUB_SYS), ":");
  return (getSystemName() != (dpSystemName + ":"));
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: ActiveXSupported
//  
// Returns: true if the panel contains the ActiveX tree control
///////////////////////////////////////////////////////////////////////////
bool ActiveXSupported() 
{ 
  return gActiveXSupported;
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: setActiveXSupported
//  
// sets the global variable that indicates if activeX is supported
//
///////////////////////////////////////////////////////////////////////////
void setActiveXSupported() 
{
  idispatch activeXctrl = 0;
  if (activeXctrl == 0)
  {
    LOG_TRACE("I cannot create a COM object!? What the ....?? You must be running Linux or something.", "");
    gActiveXSupported = false;
  }
  else
  {
    LOG_TRACE("I can create a COM object! ", activeXctrl);
    releaseComObject(activeXctrl);
    gActiveXSupported = true;
  }
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: getTreeCtrlName
//  
// returns the name of the ActiveX tree control if activeX is supported, 
// returns the name of the emulated tree control otherwise
///////////////////////////////////////////////////////////////////////////
string getTreeCtrlName()
{
  if (ActiveXSupported())
  {
    return ACTIVEX_TREE_CTRL_NAME;
  }
  else
  {
    return LIST_TREE_CTRL_NAME;
  }
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: getTreeCtrl
//  
// returns the ActiveX tree control shape if activeX is supported, 
// returns the emulated tree control shape otherwise
///////////////////////////////////////////////////////////////////////////
shape getTreeCtrl()
{
  return getShape(getTreeCtrlName());
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: getTabCtrlName
//  
// returns the name of the tab control that contains the views
///////////////////////////////////////////////////////////////////////////
string getTabCtrlName()
{
  return TAB_VIEWS_CTRL_NAME;
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: getTabCtrl
//  
// returns the tab control shape
///////////////////////////////////////////////////////////////////////////
shape getTabCtrl()
{
  return getShape(getTabCtrlName());
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: checkDpPermit
//
// Checks or the current user has permission to access the current datapoint
//
// Params: datapointPath
// Returns: TRUE,  permitted to access given dpName 
//          FALSE, denied to given dpName 
///////////////////////////////////////////////////////////////////////////
bool checkDpPermit(string datapointPath)
{
  dyn_string treeAccess;
  bool permit = FALSE;
  int permitLength = 0;
  dpGet(DPNAME_NAVIGATOR + ELNAME_TREEACCESS, treeAccess);

  for (int i = 6; i <= 15; i++)
  {
    if (treeAccess[i] != "")
    {
      if (patternMatch(treeAccess[i] + "*", datapointPath))
      {
        if (getUserPermission(i))
        {
          if (permitLength <= strlen(treeAccess[i]))
          {
            permit = TRUE;
            permitLength = strlen(treeAccess[i]);
          }
        }
        else
        {
          if (strlen(datapointPath) >= strlen(treeAccess[i]))
          {
            permit = FALSE;
            break;
          }
        }
      }
    }
  }
  return permit;
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: changeSelectedPostion
// 
// Params: newDatapoint - 
///////////////////////////////////////////////////////////////////////////
void changeSelectedPosition(string newDatapoint)
{
  int i;
  long nodeID;
  dyn_string datapointPath = splitDatapointPath(newDatapoint);
  string systemName = substr(getSystemName(getSystemId()), 0, (strlen(getSystemName(getSystemId())) - 1));
  if (g_datapoint == systemName)
  {
    TreeView_OnExpand(1);
  }
  string temp = "";
  string temp_dpe = "";

  dyn_string dcurrent = splitDatapointPath(g_datapoint);
  dyn_string dnew     = splitDatapointPath(newDatapoint);
  int Index;
  for (i = 1; i <= dynlen(datapointPath); i++)
  {
    if (i == 1)
    {
      temp = datapointPath[i];
    }
    else
    {
    if (i == dynlen(datapointPath)) //last element in datapointPath could be an datapoint element
    {
     temp_dpe = temp + "." + datapointPath[i];
    }
      temp = temp + "_" + datapointPath[i]; //build datapoint
    }

    nodeID = getNodeFromDatapoint(temp);
    if (nodeID == 0) //temp not found
    {
      nodeID = getNodeFromDatapoint(temp + " ->"); //maybe a local reference
      if (nodeID != 0)
      {
        temp = temp + " ->";
      }
      else
      {
        nodeID = getNodeFromDatapoint(temp + " ->>"); //temp maybe a remote reference
        if (nodeID != 0)
        {
          temp = temp + " ->>";
        }
      }
    }
    if (i != dynlen(datapointPath)) // do not expand last node, costs to much time/performance!!!!
    {
      if (nodeID != 0)
      {
        TreeView_OnExpand((nodeID));
      }
      else
      {
        nodeID = getNodeFromDatapoint(temp_dpe); //nodeID not found, try the datapoint element
        if (nodeID != 0)
      {
      TreeView_OnExpand(nodeID);
      
     }
    }
    }
  }

  treeView_draw(); 
  gCurSelTreeViewIdx = nodeID; //update global info
  treeView_setSelectedPosition(treeView_Tree2ViewIndex(nodeID));
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: getSelectedNode
//  
// returns the selected node in either the activex tree control or the 
// emulated tree control
// 0 = nothing selected. First element in the tree is node nr. 1
///////////////////////////////////////////////////////////////////////////
long getSelectedNode()
{
  long selectedNode = 0;
  shape treeCtrl = getTreeCtrl(); 
  if (ActiveXSupported())
  {
    selectedNode = treeCtrl.Selected; 
  }
  else
  {
    int selectedPos;
    treeView_getSelectedPosition(selectedPos);
    LOG_TRACE("selected pos:", selectedPos);
    if (selectedPos >= 1)
    {
      selectedNode = treeView_view2TreeIndex(selectedPos);
    }
    else
    {
      selectedNode = selectedPos;
    }
  }
  LOG_TRACE("selected node:", selectedNode);
  return selectedNode;
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: refreshNavigator()
// 
// refreshes the views of the navigator
///////////////////////////////////////////////////////////////////////////
void refreshNavigator()
{
  if (!g_initializing)
  {
    LOG_TRACE("refreshNavigator  ", gCurSelTreeViewIdx);
    if (gCurSelTreeViewIdx != 0)
    {
      string datapointPath = buildPathFromNode(gCurSelTreeViewIdx);

      // if the datapointPath is a reference, it will be translated to the
      // the original datapointPath.
      dyn_string reference;
      bool dpIsReference;
      checkForReference(datapointPath, reference, dpIsReference);
      string dpViewConfig = navConfigGetViewConfig(datapointPath);

      showView(dpViewConfig, datapointPath);
    }
  }
  else
  {
    LOG_DEBUG("refreshNavigator suppressed while initializing ");
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
  string viewsPath = navGetViewsPath();
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
// FunctionName: getNodeFromDatapoint
// 
// returns the nodeid in the treeview of the corresponding datapoint
///////////////////////////////////////////////////////////////////////////
long getNodeFromDatapoint(string dpe)
{
  long nodeId = 0;
  
  //string datapointName = dpSubStr(dpe, DPSUB_SYS_DP_EL); Origiganel 
  string datapointName = getSystemName(getSystemId()) + dpe;  //New AdB 18-3-2005
  LOG_TRACE("getNodeFromDatapoint: searching for: ", dpe, datapointName);
  if (mappingHasKey(g_datapoint2itemID, datapointName))
  {
    nodeId = g_datapoint2itemID[datapointName];
  }
  LOG_TRACE("found??? nodeId= ", nodeId);
  return nodeId;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  Action handlers
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// FunctionName: Navigator_HandleEventInitialize()
//
// initializes the navigator
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleEventInitialize()
{
  LOG_TRACE("Navigator_HandleEventInitialize()", "");
  
  // before the first thing, we check the sanity of the configuration
  string sanityMessage;
  
  // Set the global variable: gNavigatorTabFile
  gNavigatorTabFile = navGetViewsPath() + "navigator_tab.pnl";
  
  if (!navConfigSanityCheck(sanityMessage))
  {
    gcfUtilMessageWarning("Sanity check failed", sanityMessage);
  }
  
  // first thing to do: get a new navigator ID
  // check the commandline parameter:
  int navID = 0;
  if (isDollarDefined("$ID"))
    navID = $ID;
  navConfigSetNavigatorID(navID);
  navConfigIncreaseUseCount();
  navConfigSubscribeUpdateTrigger("Navigator_HandleUpdateTrigger");
  
  navPMLinitialize();
  
  g_initializing = true;
  
  setActiveXSupported();
  LOG_TRACE("ActiveXSupported global variable set to ", ActiveXSupported());
  
  // show the ActiveX tree control if it can be created
  shape treeActiveX = getShape(ACTIVEX_TREE_CTRL_NAME);
  shape treeList    = getShape(LIST_TREE_CTRL_NAME);
  if (ActiveXSupported())
  {
    treeActiveX.visible = TRUE;
    treeList.visible    = FALSE;
  }
  else
  {
    treeActiveX.visible = FALSE;
    treeList.visible    = TRUE;
  }
  
  // manually control the initialization of the tree and tabviews
  InitializeTabViews();
  //initTree(); //cannot do it here because tree will not be visible initially, only after double click. Strange but true
  
  delay(STARTUP_DELAY); // wait for the tree control to complete initialization
  g_initializing = false;

  
  // configure the tabs
  long selectedNode = getSelectedNode();
  if (selectedNode != 0)
  {
    string datapointPath = buildPathFromNode(selectedNode);
    string dpViewConfig = navConfigGetViewConfig(datapointPath);
    if (selectedNode != 0 && dpAccessable(dpViewConfig))
    {
      showView(dpViewConfig, datapointPath);
    }
  }    

  LOG_DEBUG("~Navigator_HandleEventInitialize()");
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: Navigator_HandleEventTerminate()
//
// NOTE: it is NOT possible to call dpGet in the terminate handler!
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleEventTerminate()
{
  LOG_DEBUG("Navigator_HandleEventTerminate()");
}

///////////////////////////////////////////////////////////////////////////
// FunctionName: Navigator_HandleEventClose()
//
// de-initializes the navigator
// NOTE: it is NOT possible to call dpGet in the terminate handler!
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleEventClose()
{
  LOG_DEBUG("Navigator_HandleEventClose()");

  navPMLterminate();

  navConfigDecreaseUseCount();
    
  PanelOff();
}


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
            string viewsPath = navGetViewsPath();
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


