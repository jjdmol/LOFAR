//# gcfnav-functions.ctl
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

#uses "gcfnav-pmlinterface.ctl"
#uses "gcf-util.ctl"
#uses "gcf-common.ctl"
#uses "gcfnav-configuration-functions.ctl"

global string   ACTIVEX_TREE_CTRL      = "NOT FlyTreeXCtrl.FlyTreeX";
global string   ACTIVEX_TREE_CTRL_NAME = "FlyTreeXCtrl1";
global string   LIST_TREE_CTRL_NAME    = "list";
global string   TAB_VIEWS_CTRL_NAME    = "TabViews";
global string   NAVIGATOR_TAB_FILENAME = "navigator/navigator_tab.pnl";
global bool     ACTIVEX_SUPPORTED      = false;
global int      NR_OF_VIEWS            = 10;
global dyn_string  g_itemID2datapoint;
global mapping  g_datapoint2itemID;
global bool     g_initializing         = true;
global int      g_curSelNode = 0;
global int      STARTUP_DELAY = 1;
global int      id;                    //needed for changing the selection in the tree (panel navigation, ER 218)
global int      treeAddCount = 0;      //test teller for performance issue
global unsigned g_nodeID               = 0;
global unsigned g_parentIndex          = 0;
global dyn_string  g_referenceList = "";
global string MESSAGE_DPACCESS = "Resource is not accessable.";
global string MESSAGE_ACCESSDENIED = "ACCESS DENIED\n\nYou have no rights to access this resource.";
//Enumaration for the use of userrights.
global int	UR_TREEACCESS1            = 6;
global int	UR_TREEACCESS2            = 7;
global int	UR_TREEACCESS3            = 8;
global int	UR_TREEACCESS4            = 9;
global int	UR_TREEACCESS5            = 10;
global int	UR_TREEACCESS6            = 11;
global int	UR_TREEACCESS7            = 12;
global int	UR_TREEACCESS8            = 13;
global int	UR_TREEACCESS9            = 14;
global int	UR_TREEACCESS10            = 15;
global int	UR_CONFIGSYSTEMSUBVIEW     = 19;
global int	UR_ADDREMOVESYSTEMSUBVIEW  = 20; 
global int	UR_COMMANDSOPERATOR        = 21;
global int	UR_COMMANDSMAINTENANCE     = 22;
global int	UR_COMMANDSASTRONOMY       = 23;
global int	UR_SYSTEMMANAGEMENT        = 24;


//////////////////////////////////////////////////////////////////////////////////
// FunctionName: convertOriginal2ReferenceDP
//
// If the dpName is a reference, conf
///////////////////////////////////////////////////////////////////////////////////
void convertOriginal2ReferenceDP(string datapointOriginal, string &datapointPathRef)
{
  if(mappingHasKey(g_datapoint2itemID,datapointOriginal))
  {
    datapointPathRef = datapointOriginal;
  }
  else
  {
   datapointPathRef = buildPathFromNode(g_curSelNode);
  }
}

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: RefreshWithDP
//
// TEST DOORNAVIGEREN
///////////////////////////////////////////////////////////////////////////////////
void RefreshWithDP(string newDatapoint)
{
  string datapointPath;
  DebugN("Trigger with dpName!!");
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_NEWDATAPOINT, newDatapoint);
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE,0);
}


///////////////////////////////////////////////////////////////////////////
// changeSelectedPostion
// 
// 
///////////////////////////////////////////////////////////////////////////
void changeSelectedPosition(string newDatapoint)
{
  int i;
  long nodeID;
  dyn_string datapointPath = splitDatapointPath(newDatapoint);
  string systemName = substr(getSystemName(getSystemId()), 0, (strlen(getSystemName(getSystemId()))-1));
  if(g_datapoint==systemName)
  {
    TreeView_OnExpand(1);
  }
  string temp = "";
  string temp_dpe = "";

  dyn_string dcurrent = splitDatapointPath(g_datapoint);
  dyn_string dnew     = splitDatapointPath(newDatapoint);
  int Index;
/*  for(int i=1; i<=dynlen(dcurrent); i++)  //get the identical datapointPath
  {
    if(dcurrent==dnew);
      Index++;
  }
  
  for(int p=1 ; p<Index; p++) //build the identical datapointPath
  {
    if(1==p)
      temp = datapointPath[p]; //build datapoint
    else
      temp = temp + "_" + datapointPath[p]; //build datapoint
    
    DebugN("temp"+p+":"+temp);
  }
*/
  //for (i=Index ; i<=dynlen(datapointPath); i++) //navigate from this point on
  for (i=1 ; i<=dynlen(datapointPath); i++)
  {
    if (i==1)
    {
      temp = datapointPath[i];
    }
    else
    {
	  if (i==dynlen(datapointPath)) //last element in datapointPath could be an datapoint element
	  {
	   temp_dpe = temp + "." + datapointPath[i];
	  }
      temp = temp + "_" + datapointPath[i]; //build datapoint
    }

    nodeID = getNodeFromDatapoint(temp);
    ////////////////////////////////////////
//    DebugN("[----------------------]");
//    DebugN("temp  :"+temp);
//    DebugN("nodeID:"+nodeID);
    if(nodeID==0) //temp not found
    {
      nodeID = getNodeFromDatapoint(temp + " ->"); //maybe a local reference
//      DebugN("temp  :"+temp);
//      DebugN("nodeID:"+nodeID);
      if(nodeID!=0)
      {
        temp = temp + " ->";
      }
      else
      {
        nodeID = getNodeFromDatapoint(temp + " ->>"); //temp maybe a remote reference
        if(nodeID!=0)
        {
          temp = temp + " ->>";
        }
      }
    }
    if (i!=dynlen(datapointPath)) // do not expand last node, costs to much time/performance!!!!
    {
      if (nodeID !=0)
      {
        TreeView_OnExpand((nodeID));
      }
      else
      {
        nodeID = getNodeFromDatapoint(temp_dpe); //nodeID not found, try the datapoint element
        if (nodeID !=0)
  	  {
	    TreeView_OnExpand(nodeID);
   	}
    }
    }
  }

  fwTreeView_draw(); 
  g_curSelNode = nodeID; //update global info
  fwTreeView_setSelectedPosition(fwTreeView_Tree2ViewIndex(nodeID));
}

///////////////////////////////////////////////////////////////////////////
//Function ActiveXSupported
//  
// returns true if the panel contains the ActiveX tree control
///////////////////////////////////////////////////////////////////////////
bool ActiveXSupported() 
{ 
  return ACTIVEX_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////
//Function setActiveXSupported
//  
// sets the global variable that indicates if activeX is supported
//
///////////////////////////////////////////////////////////////////////////
void setActiveXSupported() 
{
  idispatch activeXctrl = 0;
//  if(isFunctionDefined("createComObject"))
//    activeXctrl = createComObject(ACTIVEX_TREE_CTRL);
  if(activeXctrl==0)
  {
    LOG_TRACE("I cannot create a COM object!? What the ....?? You must be running Linux or something.","");
    ACTIVEX_SUPPORTED = false;
  }
  else
  {
    LOG_TRACE("I can create a COM object! ",activeXctrl);
    releaseComObject(activeXctrl);
    ACTIVEX_SUPPORTED = true;
  }
}

///////////////////////////////////////////////////////////////////////////
//Function getTreeCtrlName
//  
// returns the name of the ActiveX tree control if activeX is supported, 
// returns the name of the emulated tree control otherwise
///////////////////////////////////////////////////////////////////////////
string getTreeCtrlName()
{
  if(ActiveXSupported())
  {
    return ACTIVEX_TREE_CTRL_NAME;
  }
  else
  {
    return LIST_TREE_CTRL_NAME;
  }
}

///////////////////////////////////////////////////////////////////////////
//Function getTreeCtrl
//  
// returns the ActiveX tree control shape if activeX is supported, 
// returns the emulated tree control shape otherwise
///////////////////////////////////////////////////////////////////////////
shape getTreeCtrl()
{
  return getShape(getTreeCtrlName());
}

///////////////////////////////////////////////////////////////////////////
//Function getTabCtrlName
//  
// returns the name of the tab control that contains the views
///////////////////////////////////////////////////////////////////////////
string getTabCtrlName()
{
  return TAB_VIEWS_CTRL_NAME;
}

///////////////////////////////////////////////////////////////////////////
//Function getTabCtrl
//  
// returns the tab control shape
///////////////////////////////////////////////////////////////////////////
shape getTabCtrl()
{
  return getShape(getTabCtrlName());
}

///////////////////////////////////////////////////////////////////////////
//Function getSelectedNode
//  
// returns the selected node in either the activex tree control or the 
// emulated tree control
// 0 = nothing selected. First element in the tree is node nr. 1
///////////////////////////////////////////////////////////////////////////
long getSelectedNode()
{
	long selectedNode = 0;
  shape treeCtrl = getTreeCtrl(); 
  if(ActiveXSupported())
  {
    selectedNode = treeCtrl.Selected; 
  }
  else
  {
    int selectedPos;
    fwTreeView_getSelectedPosition(selectedPos);
    LOG_TRACE("selected pos:",selectedPos);
    if(selectedPos >= 1)
    {
    	selectedNode = fwTreeView_view2TreeIndex(selectedPos);
    }
    else
    {
    	selectedNode = selectedPos;
    }
  }
  LOG_TRACE("selected node:",selectedNode);
  return selectedNode;
}

///////////////////////////////////////////////////////////////////////////
//Function refreshNavigator()
// 
// refreshes the views of the navigator
///////////////////////////////////////////////////////////////////////////
void refreshNavigator()
{
  if(!g_initializing)
  {
    LOG_TRACE("refreshNavigator  ",g_curSelNode);
    if(g_curSelNode != 0)
    {
      string datapointPath = buildPathFromNode(g_curSelNode);

      // if the datapointPath is a reference, it will be translated to the
      // the original datapointPath.
      dyn_string reference;
      bool dpIsReference;
      checkForReference(datapointPath, reference, dpIsReference);
      string dpViewConfig = navConfigGetViewConfig(datapointPath);

      showView(dpViewConfig,datapointPath);
    }
  }
  else
  {
    LOG_DEBUG("refreshNavigator suppressed while initializing ");
  }
}

///////////////////////////////////////////////////////////////////////////
//Function showActiveView(string datapointPath)
// 
// shows the active tab identified by the datapoint
///////////////////////////////////////////////////////////////////////////
void showActiveView(string dpViewConfig, string datapointPath)
{
  LOG_DEBUG("showActiveView",dpViewConfig,datapointPath);
  shape tabCtrl = getTabCtrl();
  string viewsPath = navConfigGetViewsPath();
  int selectedViewTabId=tabCtrl.activeRegister+1;
  LOG_DEBUG("showActiveView, active view = ",selectedViewTabId);

  navConfigSetSelectedElement(datapointPath);
  dyn_string panelParameters = makeDynString("$datapoint:" + datapointPath);

  // get tab properties
  dyn_string views = navConfigGetViews(dpViewConfig);
  
  for(int tabId=1;tabId<=dynlen(views);tabId++)
  {
    if(tabId != selectedViewTabId)
    {
      // load empty panel in non-visible tabs to enhance performance
      tabCtrl.registerPanel(tabId-1,"navigator/views/nopanel.pnl",makeDynString(""));
    }
    else
    {
      if(dpExists(views[selectedViewTabId]))
      {
        tabCtrl.registerPanel(selectedViewTabId-1,NAVIGATOR_TAB_FILENAME,panelParameters);
      }
      else
      {
        LOG_TRACE("showActiveView","tab reference not found; making tab invisible: ",selectedViewTabId);
        tabCtrl.registerVisible(selectedViewTabId-1)=FALSE;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//Function showView(string datapointPath)
// 
// shows the tab identified by the datapoint
///////////////////////////////////////////////////////////////////////////
void showView(string dpViewConfig, string datapointPath)
{
  LOG_DEBUG("showView",dpViewConfig,datapointPath);
  shape tabCtrl = getTabCtrl();
  string viewsPath = navConfigGetViewsPath();
  int selectedViewTabId=1;
  int tabId;

  navConfigSetSelectedElement(datapointPath);
  dyn_string panelParameters = makeDynString("$datapoint:" + datapointPath);
  // get the selected tab
  string selectedViewCaption = navConfigGetSelectedView();
  LOG_DEBUG("showView","selected View:",selectedViewCaption);

  // get tab properties
  dyn_string views = navConfigGetViews(dpViewConfig);
  
  for(tabId=1;tabId<=dynlen(views);tabId++)
  {
    if(dpExists(views[tabId]))
    {
      string caption = navConfigGetViewCaption(views[tabId]);
      if(strlen(caption)>0)
      {
        LOG_DEBUG("showView","making tab visible: ",tabId,caption);
        tabCtrl.namedColumnHeader("tab"+tabId) = caption;
        tabCtrl.registerVisible(tabId-1)=TRUE;
        
        // check if this tab is currently selected
        if(caption == selectedViewCaption)
        {
          LOG_DEBUG("showView","caption=selectedViewCaption");
          selectedViewTabId = tabId;
          tabCtrl.registerPanel(tabId-1,NAVIGATOR_TAB_FILENAME,panelParameters);
        }
      }
      else
      {
        LOG_TRACE("showView","empty caption or filename; making tab invisible: ",tabId);
        tabCtrl.registerVisible(tabId-1)=FALSE;
      }
    }
    else
    {
      LOG_TRACE("showView","tab reference not found; making tab invisible: ",tabId);
      tabCtrl.registerVisible(tabId-1)=FALSE;
    }
  }
  // check if this tab is currently selected
  if(selectedViewCaption == "")
  {
    LOG_DEBUG("showView","selectedViewCaption=\"\"");
    selectedViewTabId = 1;
    tabCtrl.registerPanel(0,NAVIGATOR_TAB_FILENAME,panelParameters);
  }
  tabCtrl.activeRegister = selectedViewTabId-1;
  
  // make the rest of the views invisible
  int i;
  for(i=tabId;i<=NR_OF_VIEWS;i++)
  {
    LOG_TRACE("showView","tab undefined; making tab invisible: ",i);
    tabCtrl.registerVisible(i-1)=FALSE;
  }
}

///////////////////////////////////////////////////////////////////////////
//Function treeAddNode
//  
// Adds text to the treeview and returns the id of the added item
// - The parentId parameter is used to add the node to its parent in the 
//   ActiveX control
// - The level parameter is used to indent the node in the tree list control
//
///////////////////////////////////////////////////////////////////////////
long treeAddNode(long parentId,int level,string text) 
{ 
  long nodeId=0;
  shape treeCtrl = getTreeCtrl();

  if(parentId==-1)
  {
    nodeId = fwTreeView_appendToParentNode(0,text,"",0,level);
  }
  else
  {
    nodeId = fwTreeView_appendToParentNode(parentId,text,"",0,level);
	  treeAddCount++;
  }
  return nodeId;
}

///////////////////////////////////////////////////////////////////////////
//Function treeAddDatapoints()
//      
// parameters: names           - names of the datapoints to add
//
// Adds names of datapoints and their elements to the treeview
///////////////////////////////////////////////////////////////////////////
void treeAddDatapoints(dyn_string names)
{
  int namesIndex;
  dyn_dyn_string elementNames;
  dyn_dyn_int elementTypes;
  dyn_string addedDatapoints;
  string systemName;
  long addedNode=0;

  g_parentIndex = 0;           // variable used in function fwTreeView_appendToParentNode
  g_nodeID  = 0;  // to increase performance
  if(dynlen(names)>0)
  {
    systemName = strrtrim(dpSubStr(names[1],DPSUB_SYS),":");

	  // Check if the item already exists
	  if(mappingHasKey(g_datapoint2itemID,systemName))
	  {
	    addedNode = g_datapoint2itemID[systemName];
	  }
	  else
	  {
		  addedNode = treeAddNode(-1,0,systemName);
      LOG_TRACE("Added root node: ",addedNode,systemName);
      insertDatapointNodeMapping(addedNode,systemName);
    }  
	  
    dyn_int internalNodeMapping;
    dyn_string internalFullDPName;

	  // go through the list of datapoint names
	  for(namesIndex=1;namesIndex<=dynlen(names);namesIndex++)
	  {
	    int pathIndex;
	    dyn_string dpPathElements;
	    string datapointName;
	    int parentId;
	    // remove the System part from the datapoint name
	    datapointName = dpSubStr(names[namesIndex],DPSUB_DP);
        if(datapointName == "")
        {
          datapointName = names[namesIndex];
          // cut system name myself. Necessary for datapoint parts that are not datapoints themselves
          int sepPos = strpos(datapointName,":");
          if(sepPos >= 0)
          {
            datapointName = substr(datapointName,sepPos+1);
          }
        }

	    // only add ENABLED datapoints
	    dyn_string reference;
	    string dpName=names[namesIndex];
	    bool dpIsReference=false;
      checkForReference(dpName, reference, dpIsReference);
	    if(dpIsReference) //If the dpName is a reference, use the original datapoint
	    {
	      dpName=reference[2];
	    }
	    else
	    {
	      dpName=names[namesIndex];
	    }
      if(navConfigCheckEnabled(dpName))
	    {
		    // split the datapoint path in elements
		    dpPathElements = strsplit(datapointName,"_");
		    string addingDPpart = systemName;
		    for(pathIndex=0;pathIndex<=dynlen(dpPathElements);pathIndex++)
		    {
		      // Check if the item already exists
		      if(mappingHasKey(g_datapoint2itemID,addingDPpart))
		      {
		        addedNode = g_datapoint2itemID[addingDPpart];
		      }
		      else
		      {
		        // item does not exist
		        dynAppend(addedDatapoints,addingDPpart);
		        if(addingDPpart != systemName)
		        {
		          addedNode = treeAddNode(parentId,pathIndex,dpPathElements[pathIndex]); 
				      if (addedNode!=0)
			        {
			          internalNodeMapping[dynlen(internalNodeMapping)+1]=addedNode;
			          internalFullDPName[dynlen(internalFullDPName)+1] = addingDPpart;
			        }
              LOG_TRACE("Added node: ",addedNode,parentId,pathIndex,dpPathElements[pathIndex]);
		        }
          }
		      parentId = addedNode;
          if(pathIndex<dynlen(dpPathElements))
          {
  		      if(pathIndex==0)
  		      {
  		        addingDPpart = addingDPpart + ":" + dpPathElements[pathIndex+1];
  		      }
  		      else if(pathIndex<dynlen(dpPathElements))
  		      {
  		       addingDPpart = addingDPpart + "_" + dpPathElements[pathIndex+1];
  		      }
          }
		    }
			  
        // get the datapoint structure
        dynClear(elementNames);
        dynClear(elementTypes);
        dpIsReference=false;
        checkForReference(names[namesIndex], reference, dpIsReference);

        dpTypeGet(dpTypeName(names[namesIndex]),elementNames,elementTypes);

        // add elements of this datapoint, if any, skip system stuff
        if(addedNode != 0 && addingDPpart != systemName && dynlen(elementNames) > 1)
        {
          int         elementIndex;
          dyn_int     parentIds;
          dyn_string  parentNodes;
          parentIds[1]   = addedNode;
          parentNodes[1] = "";
          // skip the first element in the array because it contains the datapoint type name
          
          //for(elementIndex=2;elementIndex<=dynlen(elementNames);elementIndex++) 
          int max;
          if (namesIndex == 1)  max = dynlen(elementNames);
          else max = 2;
          string fullDPname;
          for(elementIndex=2;elementIndex<=max;elementIndex++) 
          {
            // every last item of each array contains an element name (see help on dpTypeGet())
            // file:///opt/pvss/pvss2_v3.0/help/en_US.iso88591/WebHelp/ControlA_D/dpTypeGet.htm
            int elementLevel = dynlen(elementNames[elementIndex])-1; // how deep is the element?
            string elementName = elementNames[elementIndex][elementLevel+1];
            if("__"==substr(elementName, 0,2)) //Check if the elementName contains reference info
            {
              dyn_string referenceContent;
              dpGet(addingDPpart + "."+elementName, referenceContent);
              for(int k=1; k<=dynlen(referenceContent); k++)
              {
                dyn_string referenceSplit = strsplit(referenceContent[k], "=");
                g_referenceList[dynlen(g_referenceList)+1]= addingDPpart + "_" + referenceSplit[1] + referenceSign(referenceSplit[2])+"=" + referenceSplit[2];
                names[dynlen(names)+1]= addingDPpart + "_" + referenceSplit[1] + referenceSign(referenceSplit[2]);
              }
            }
            else
            {
              fullDPname = addingDPpart+parentNodes[elementLevel]+"."+elementName;
            }
            //a reference element must never appear in the treebrowser
            if(!("__"==substr(elementName, 0,2)))
            {            
              if(mappingHasKey(g_datapoint2itemID,fullDPname))
              {
                addedNode = g_datapoint2itemID[fullDPname];
              }
              else
              {
                addedNode = treeAddNode(parentIds[elementLevel],pathIndex-1+elementLevel,elementName); 
                LOG_TRACE("Added element node: ",addedNode,parentIds[elementLevel],pathIndex-1+elementLevel,fullDPname);
			          if (addedNode!=0)
                {
    			        internalNodeMapping[dynlen(internalNodeMapping)+1]=addedNode;
        			    internalFullDPName[dynlen(internalFullDPName)+1] = fullDPname;
			          }
              }
              parentIds[elementLevel+1] = addedNode; // remember this node as parent at its level in case there are elements below this one
              parentNodes[elementLevel+1] = parentNodes[elementLevel]+"."+elementName;
            }
          }
         }
	    }
	  }
	  if(dynlen(internalNodeMapping)!=0)
	  { 
	    insertInternalNodeMapping(internalNodeMapping, internalFullDPName);
    }
  }
}


///////////////////////////////////////////////////////////////////////////
//Function referenceSign()
// 
// return: " ->>" if reference is distributed
//         " ->"  if reference is NOT distributed
// this to increase performance.
///////////////////////////////////////////////////////////////////////////
string referenceSign(string reference)
{
 string refSign;
 if(dpIsDistributed(reference))
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
//Function insertInternalNodeMapping()
// 
// gathers all insertions for a datapoint and then handles is in one time,
// this to increase performance.
///////////////////////////////////////////////////////////////////////////
insertInternalNodeMapping(dyn_int internalNodeMapping, dyn_string fullDPname)
{
  int length=mappinglen(g_datapoint2itemID);
  int teller=0;
  int fullDPNameLength = dynlen(internalNodeMapping);

  //# If the first node number is equal or smaller than the last node number id
  //# the global list, this function must be called;
  if (internalNodeMapping[1]<=mappinglen(g_datapoint2itemID)) 
  {
    for(int i= (length+ dynlen(internalNodeMapping)); i>=internalNodeMapping[1]; i--)
    {
	    if (i > internalNodeMapping[dynlen(internalNodeMapping)])
	    {
	      g_itemID2datapoint[i] = g_itemID2datapoint[(i-dynlen(internalNodeMapping))];
        g_datapoint2itemID[g_itemID2datapoint[(i-dynlen(internalNodeMapping))]] = i;
	    }
	    else if (i>=internalNodeMapping[1] && i<=internalNodeMapping[dynlen(internalNodeMapping)])
	    {
	      g_itemID2datapoint[i] = fullDPname[dynlen(fullDPname) + teller];
		    g_datapoint2itemID[fullDPname[dynlen(fullDPname) + teller]] = i;
		    teller--;
	    }
    }
  }
  else
  {
    for(int j=1; j<=dynlen(internalNodeMapping); j++)
    {
      g_itemID2datapoint[internalNodeMapping[j]] = fullDPname[j];
	  g_datapoint2itemID[fullDPname[j]] = internalNodeMapping[j];
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//Function buildPathFromNode()
// 
// builds a datapoint path from a node in the treeview
///////////////////////////////////////////////////////////////////////////
string buildPathFromNode(long Node)
{
	string datapointPath = "";
	if(Node >= 1)
	{
  	datapointPath = g_itemID2datapoint[Node];
  }
  LOG_TRACE("buildPathFromNode(",Node,") returns ",datapointPath);
  return datapointPath;
}

///////////////////////////////////////////////////////////////////////////
//Function getNodeFromDatapoint
// 
// returns the nodeid in the treeview of the corresponding datapoint
///////////////////////////////////////////////////////////////////////////
long getNodeFromDatapoint(string dpe)
{
  long nodeId=0;
  
  //string datapointName = dpSubStr(dpe,DPSUB_SYS_DP_EL); Origiganel 
  string datapointName = getSystemName(getSystemId()) + dpe;  //New AdB 18-3-2005
  LOG_TRACE("getNodeFromDatapoint: searching for: ",dpe,datapointName);
  if(mappingHasKey(g_datapoint2itemID,datapointName))
  {
    nodeId = g_datapoint2itemID[datapointName];
  }
  LOG_TRACE("found??? nodeId= ",nodeId);
  return nodeId;
}

///////////////////////////////////////////////////////////////////////////
//Function insertDatapointNodeMapping
// 
// inserts the node and dp in their mappings. Because it is an insert,
// existing mappings with the same and higher node-id's must be updated.
///////////////////////////////////////////////////////////////////////////
void insertDatapointNodeMapping(int node, string dp)
{
  for(int i=dynlen(g_itemID2datapoint); i>=node && i>=1; i--)
  {
    g_itemID2datapoint[i+1] = g_itemID2datapoint[i];
  }
  g_itemID2datapoint[node] = dp;
  
  for(int i=1;i<=mappinglen(g_datapoint2itemID);i++)
  {
    int value = mappingGetValue(g_datapoint2itemID,i);
    if(value >= node)
    {
      g_datapoint2itemID[mappingGetKey(g_datapoint2itemID,i)] = value+1;
    }
  }
  g_datapoint2itemID[dp] = node;
}

///////////////////////////////////////////////////////////////////////////
//Function AlertStatusHandler()
// 
// called when an alert status changes
///////////////////////////////////////////////////////////////////////////
void AlertStatusHandler(string dpe, long status)
{
  if(!g_initializing)
  {
    LOG_TRACE("AlertStatusHandler()",dpe,status);
    // search dpe in tree
    long nodeId = getNodeFromDatapoint(dpe);
    if(nodeId>=0)
    {
      shape treeCtrl = getTreeCtrl(); 
      if(ActiveXSupported())
      {
        idispatch node;
        node = treeCtrl.ExtractNode(nodeId);
        node.Text = stripColorTags(node.Cells(1));
        if(status == 0)
        {
          node.Data = "";
        }
        else
        {
          node.Data = "A";
          string nodeText = addColorTags(node.Cells(1),"0x0000ff");
          LOG_TRACE(nodeText);
          node.Text = nodeText;
          LOG_TRACE(node.Text);
        }
        treeCtrl.InvalidateRow(node.GetRow());
      }
      else
      {
  // TODO
      }
    }
    LOG_TRACE("~AlertStatusHandler()",dpe,status);
  }
}

///////////////////////////////////////////////////////////////////////////
//Function addColorTags
//
// adds HTML color tags to the specified string
///////////////////////////////////////////////////////////////////////////
string addColorTags(string text, string fgcolor="" ,string bgcolor = "")
{
  LOG_TRACE("COLOR TAGS DISABLED");
  string taggedText = text;

/*
  if(strlen(fgcolor)>0)
  {
    taggedText = "<color=" + fgcolor + ">" + taggedText + "</color>";
  }
  if(strlen(bgcolor)>0)
  {
    taggedText = "<bgcolor=" + bgcolor + ">" + taggedText + "</bgcolor>";
  }
  LOG_TRACE(taggedText);
*/
  return taggedText;
}

///////////////////////////////////////////////////////////////////////////
//Function stripColorTags
//
// strips HTML color and bgcolor tags from the specified string
///////////////////////////////////////////////////////////////////////////
string stripColorTags(string text)
{
  LOG_TRACE("COLOR TAGS DISABLED");
  string untaggedText = text;
/*
  dyn_string tags=makeDynString("<color=","</color","<bgcolor=","</bgcolor");
  int tagBegin, tagEnd, i;
  for(i=1;i<=dynlen(tags);i++)
  {
    tagBegin = strpos(untaggedText,tags[i]);
    if(tagBegin>=0)
    {
      int tagEnd = strpos(untaggedText,">");
      if(tagEnd>tagBegin)
      {
        string beginPart,endPart;
        beginPart="";
        endPart="";
        if(tagBegin>0)
        {
          beginPart = substr(untaggedText,0,tagBegin);
        }
        endPart = substr(untaggedText,tagEnd+1);
        untaggedText = beginPart + endPart;
      }
    }
  }
*/
 return untaggedText;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
//  Action handlers
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//Function Navigator_HandleEventInitialize()
//
// initializes the navigator
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleEventInitialize()
{
  LOG_TRACE("Navigator_HandleEventInitialize()","");
  
  // before the first thing, we check the sanity of the configuration
  string sanityMessage;
  if(!navConfigSanityCheck(sanityMessage))
  {
    gcfUtilMessageWarning("Sanity check failed",sanityMessage);
  }
  
  // first thing to do: get a new navigator ID
  // check the commandline parameter:
  int navID=0;
  if(isDollarDefined("$ID"))
    navID=$ID;
  navConfigSetNavigatorID(navID);
  navConfigIncreaseUseCount();
  navConfigSubscribeUpdateTrigger("Navigator_HandleUpdateTrigger");
  
  navPMLinitialize();
  
  g_initializing=true;
  
  setActiveXSupported();
  LOG_TRACE("ActiveXSupported global variable set to ",ActiveXSupported());
  
  // show the ActiveX tree control if it can be created
  shape treeActiveX = getShape(ACTIVEX_TREE_CTRL_NAME);
  shape treeList    = getShape(LIST_TREE_CTRL_NAME);
  if(ActiveXSupported())
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
//  InitializeTree(); cannot do it here because tree will not be visible initially, only after double click. Strange but true
  
  delay(STARTUP_DELAY); // wait for the tree control to complete initialization
  
  g_initializing = false;

  // configure the tabs
  long selectedNode = getSelectedNode();
  if(selectedNode != 0)
  {
    string datapointPath = buildPathFromNode(selectedNode);
    string dpViewConfig = navConfigGetViewConfig(datapointPath);
    if(selectedNode!=0 && dpExists(dpViewConfig))
    {
      showView(dpViewConfig,datapointPath);
    }
  }    
  
  LOG_DEBUG("~Navigator_HandleEventInitialize()");
}

///////////////////////////////////////////////////////////////////////////
//Function Navigator_HandleEventTerminate()
//
// NOTE: it is NOT possible to call dpGet in the terminate handler!
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleEventTerminate()
{
  LOG_DEBUG("Navigator_HandleEventTerminate()");
}

///////////////////////////////////////////////////////////////////////////
//Function Navigator_HandleEventClose()
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
//Function Navigator_HandleUpdateTrigger()
//
// refreshes the navigator
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleUpdateTrigger(string dpe,int trigger)
{
  string newDatapoint;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_NEWDATAPOINT, newDatapoint);
  //if ((newDatapoint !="") && dpExists(newDatapoint)) Orginal
  if (newDatapoint !="")
  {
    changeSelectedPosition(newDatapoint);
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_NEWDATAPOINT, "");
    refreshNavigator();
  }
  else
  {
    refreshNavigator();
  }
  
}

///////////////////////////////////////////////////////////////////////////
//Function TabViews_HandleEventInitialize()
//
// initializes the tabview
///////////////////////////////////////////////////////////////////////////
void TabViews_HandleEventInitialize()
{
  // the initialization of the main panel initializes the tabviews
  // nothing should be done here
}

///////////////////////////////////////////////////////////////////////////
//Function InitializeTabViews()
//
// initializes the tabview
///////////////////////////////////////////////////////////////////////////
void InitializeTabViews()
{
  LOG_DEBUG("InitializeTabViews()");
  shape tabCtrl = getTabCtrl();
  // hide all tabs
  int i=0;
  dyn_errClass err;
  int setValueResult;
  do
  {
    setValueResult = setValue(getTabCtrlName(),"registerVisible",i,FALSE);
    err = getLastError();
    LOG_TRACE("registerVisible",i,setValueResult,err);
    i++;
  } while(dynlen(err)==0 && i<NR_OF_VIEWS && setValueResult==0);
  LOG_DEBUG("~InitializeTabViews()");
}

///////////////////////////////////////////////////////////////////////////
//Function TabViews_HandleEventSelectionChanged
// 
// stores the selected tab number in the database
///////////////////////////////////////////////////////////////////////////
void TabViews_HandleEventSelectionChanged()
{
  if(!g_initializing)
  {
    LOG_DEBUG("TabViews_HandleEventSelectionChanged");

    shape tabCtrl = getTabCtrl();
    long selectedNode = getSelectedNode();
    if(selectedNode != 0)
    {
      string datapointPath = buildPathFromNode(selectedNode);
      dyn_string reference;
      bool dpIsReference;
      checkForReference(datapointPath, reference, dpIsReference);
      navConfigSetSelectedView(datapointPath,tabCtrl.activeRegister+1);
    
      string dpViewConfig = navConfigGetViewConfig(datapointPath);
      if(selectedNode!=0 && dpExists(dpViewConfig))
      {
        showActiveView(dpViewConfig,datapointPath);
      }
    }
  }
  else
  {
    LOG_DEBUG("TabViews_HandleEventSelectionChanged suppressed while initializing");
  }
}

///////////////////////////////////////////////////////////////////////////
//Function TreeCtrl_HandleEventInitialize()
//
// initializes the Resources treeview
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventInitialize()
{
  InitializeTree();
}

///////////////////////////////////////////////////////////////////////////
//Function InitializeTree()
//
// initializes the Resources treeview
///////////////////////////////////////////////////////////////////////////
void InitializeTree()
{
  LOG_DEBUG("InitializeTree()");
  dyn_errClass err;

  mapping empty;
  g_datapoint2itemID = empty;
  dynClear(g_itemID2datapoint);
  dynClear(g_referenceList);
  LOG_TRACE("global stuff lengths:",dynlen(g_itemID2datapoint),mappinglen(g_datapoint2itemID));
  
  shape treeCtrl = getTreeCtrl();
  idispatch items;
  if(ActiveXSupported())
  {
    items = treeCtrl.Items;
    items.BeginUpdate();
    items.Clear();
    treeCtrl.SortType = 0;
  }
  else
  {
    fwTreeView_watchDog(); // prevent memory leak when closing controlling window
  }
  
  // get top level resources. "" means no parent, 1 means: 1 level deep
  dyn_string resources = navConfigGetResources("",1);
  LOG_DEBUG("adding resources: ",LOG_DYN(resources));
  treeAddDatapoints(resources);
  if(ActiveXSupported())
  {
    items.EndUpdate();
  }
  else
  {
  }
  
  LOG_DEBUG("~InitializeTree()");
}

///////////////////////////////////////////////////////////////////////////
//Function TreeCtrl_EventOnSelChange(long Node)
// 
// initializes the Resources treeview
// TODO: optimize for selection change to the same resource type. Then don't reconfigure the tabs
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnSelChange(long Node)
{
  if(g_curSelNode != Node)
  {
    g_curSelNode = Node;
    refreshNavigator();   
  }
}

///////////////////////////////////////////////////////////////////////////
//Function TreeCtrl_EventOnExpand(long Node)
// 
// expands a node in the Resources treeview
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnExpand(long Node)
{
  if(!g_initializing)
  {
    LOG_DEBUG("TreeCtrl_HandleEventOnExpand ",Node);
    if(Node != 0)
    {
      string datapointPath = buildPathFromNode(Node);
      if(Node==1)
      {
        datapointPath="";
      }
        // get top level resources. "" means no parent, 1 means: 1 level deep
        dyn_string resources = navConfigGetResources(datapointPath,1);
        LOG_DEBUG("adding resources: ",LOG_DYN(resources));
        treeAddDatapoints(resources);
    }
  }
  else
  {
    LOG_DEBUG("TreeCtrl_HandleEventOnExpand suppressed while initializing ");
  }
}

///////////////////////////////////////////////////////////////////////////
// TreeView_OnCollapse
// 
// called when an item is expanded
///////////////////////////////////////////////////////////////////////////
TreeView_OnCollapse(unsigned pos)
{
  LOG_DEBUG("TreeView_OnCollapse",pos);
  TreeCtrl_HandleEventOnCollapse(pos);

  // the last line of code of each fwTreeView event handler MUST be the following:
  id = -1; 
}


///////////////////////////////////////////////////////////////////////////
//Function TreeCtrl_HandleEventOnCollapse(long Node)
// 
// expands a node in the Resources treeview
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnCollapse(unsigned Node)
{
  int collapsedNodes=1;
  dyn_string datapoint ;
  int k=1;
  string temp;

  fwTreeView_pruneChildren(Node, collapsedNodes, ""); //get all nodes which will be collapsed and
                                                      //clear these nodes from the tree
  //retrieve all dpnames for these nodes
  for(int j=Node+1; j<=(Node+collapsedNodes);j++)
  {
    datapoint[k]= g_itemID2datapoint[j];
    k++;
  }

  //delete the collapse nodes from g_itemID2datapoint and g_datapoint2itemID
  for(int i=1; i<=dynlen(datapoint); i++)
  {
    long nodeID=getNodeFromDatapoint(datapoint[i]);
    dynRemove(g_itemID2datapoint,dynContains(g_itemID2datapoint,datapoint[i]));      
    mappingRemove(g_datapoint2itemID,datapoint[i]);
  }

  //renumber the mapping of the dp's
  if(mappinglen(g_datapoint2itemID)>1)
  {
    for(int i=1 ; i<=mappinglen(g_datapoint2itemID); i++)
    {
      temp = g_itemID2datapoint[i];
      if(temp!="" && temp !=0)
      {
        g_datapoint2itemID[temp]=i;
      }
    }
  }
  //Delete references in g_referenceList for the related node
  string datapointPath = buildPathFromNode(Node);
  checkForReferenceRemoveFromList(datapointPath);
  
  //mark the node as COLLAPSED
  dyn_anytype parentNode;
  parentNode = fwTreeView_getNode(Node, "");
  parentNode[fwTreeView_STATE] = parentNode[fwTreeView_STATE] & ~fwTreeView_EXPANDED;
  fwTreeView_replaceNode(parentNode, Node, "");
  fwTreeView_draw();
}

///////////////////////////////////////////////////////////////////////////
//Function checkForReferenceReplaceOriginal
//
// parameters: datapointPath - deletes al related references form the
//             g_referenceList
// 
// returns - none
///////////////////////////////////////////////////////////////////////////
void checkForReferenceRemoveFromList(string datapointPath)
{
  dyn_string refOut;
  
  for(int i=dynlen(g_referenceList); i>=1; i--)
  {
    refOut = strsplit(g_referenceList[i],"=");
    if (patternMatch(datapointPath+ "*",refOut[1]) && !(datapointPath==refOut[1]))
    {
      dynRemove(g_referenceList, i);
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//Function TreeCtrl_HandleEventOnDrawCell
// 
// draws icons in columns if needed
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnDrawCell(long Col, long Row, float Left, float Top, float Right, float Bottom)
{
  if(!g_initializing)
  {
    if(ActiveXSupported())
    {
      shape treeCtrl = getTreeCtrl();
      if(Row >= treeCtrl.FixedRows)
      {
        if(Col==0)
        {
          idispatch aNode;
          aNode = treeCtrl.GetNodeAt(Left,Top);
          if(aNode != 0)
          {
            if(aNode.Data != 0)
            {
              if(aNode.Data == "A" && (treeCtrl.ImagesWidth < (Right - Left)))
              {
                LOG_TRACE("data is A and image fits",aNode.Data,treeCtrl.ImagesWidth,Right,Left);
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
  if(Node != 0)
  {
    string datapointPath = buildPathFromNode(Node);
    string dpViewConfig = navConfigGetViewConfig(datapointPath);
    LOG_TRACE("ButtonMaximize_HandleEventClick",Node,dpViewConfig);
    if(Node!=0 && dpExists(dpViewConfig))
    {
      int selectedView;
      int selectedSubView;
      dyn_string views;
      dyn_int  nrOfSubViews;
      dyn_string subViews;
      dyn_string configs;
      
      if(navConfigGetViewConfigElements(dpViewConfig,
            selectedView,
            selectedSubView,
            views,
            nrOfSubViews,
            subViews,
            configs))
      {
        LOG_TRACE("viewConfig:",selectedView,selectedSubView,views,nrOfSubViews,subViews,configs);
    
        // create the mapping
        int beginSubViews=1;
        if(selectedView < 1)
          selectedView = 1;
        if(selectedSubView < 1)
          selectedSubView = 1;
        for(int i=1;i<selectedView;i++)
        {
          beginSubViews += nrOfSubViews[i];
        }
        // get subviews config
        string subViewCaption;
        string subViewFileName;
        if(navConfigGetSubViewConfigElements(subViews[beginSubViews+selectedSubView-1],subViewCaption,subViewFileName))
        {
          string viewsPath = navConfigGetViewsPath();
          LOG_DEBUG("subviewcaption,subviewfilename:",subViewCaption,viewsPath+subViewFileName);

          dyn_string panelParameters = makeDynString(
            "$datapoint:" + datapointPath,
            "$configDatapoint:" + configs[beginSubViews+selectedSubView-1]);
          ModuleOnWithPanel(datapointPath,-1,-1,0,0,1,1,"",viewsPath+subViewFileName, subViewCaption, panelParameters);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////
// TreeView_OnInit
// 
// called when the list is initialized
///////////////////////////////////////////////////////////////////////////
TreeView_OnInit()
{
  LOG_DEBUG("TreeView_OnInit");

  TreeCtrl_HandleEventInitialize();  

  // the last line of code of each fwTreeView event handler MUST be the following:
  id = -1; 
}

///////////////////////////////////////////////////////////////////////////
// TreeView_OnSelect
// 
// called when an item is selected
///////////////////////////////////////////////////////////////////////////
TreeView_OnSelect(unsigned pos)
{
  LOG_DEBUG("TreeView_OnSelect",pos);
  string datapointPath = buildPathFromNode(pos);
  dyn_string reference;
  bool parentDatapointIsReference;
  checkForReference(datapointPath, reference, parentDatapointIsReference);

  if(checkDpPermit(datapointPath) || pos==1)
  {
    if(!parentDatapointIsReference || (parentDatapointIsReference && dpAccessable(datapointPath)))
    {
      TreeCtrl_HandleEventOnSelChange(pos);
    }
    else
    {
      navConfigMessageWarning(MESSAGE_DPACCESS); //dp not accessable
    }
  }
  else
  {
    navConfigMessageWarning(MESSAGE_ACCESSDENIED);  //access to resource denied
  }

  // the last line of code of each fwTreeView event handler MUST be the following:
  id = -1; 
}


///////////////////////////////////////////////////////////////////////////
// TreeView_OnExpand
// 
// called when an item is expanded
///////////////////////////////////////////////////////////////////////////
TreeView_OnExpand(unsigned pos)
{
  LOG_DEBUG("TreeView_OnExpand",pos);
  string datapointPath = buildPathFromNode(pos);
  if(pos==1)
  {
    datapointPath="";
  }      
  if(checkDpPermit(datapointPath) || pos==1)
  {
    dyn_string reference;
    bool parentDatapointIsReference;
    checkForReference(datapointPath, reference, parentDatapointIsReference);
    if(!parentDatapointIsReference || (parentDatapointIsReference && dpAccessable(datapointPath)))
    {
      TreeCtrl_HandleEventOnExpand(pos);
      // also call the default OnExpand implementation to expand the node
      fwTreeView_defaultExpand(pos);  
    }
    else
    {
      navConfigMessageWarning(MESSAGE_DPACCESS); //dp not accessable
    }
  }
  else
  {
    navConfigMessageWarning(MESSAGE_ACCESSDENIED);  //access to resource denied
  }

  // the last line of code of each fwTreeView event handler MUST be the following:
  id = -1; 
}




///////////////////////////////////////////////////////////////////////////
//Function SplitDatapointPath
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
  int sepPos = strpos(datapointName,":");
  if(sepPos >= 0)
  {
    datapointName = substr(datapointName,sepPos+1);
  }
  datapointPath[1] = datapointName;
  
  // if datapointElement present, split last entry of datapointPath
  datapointElement = strsplit(datapointPath[dynlen(datapointPath)], ".");
  if (dynlen(datapointElement)>1)
  {
    datapointPath[dynlen(datapointPath)  ] = datapointElement[1];
    datapointPath[dynlen(datapointPath)+1] = datapointElement[2];
  }
  return datapointPath;
}


//////////////////////////////////////////////////////////////////////////////////
//
// FunctionName: ProgressBar
// Function    : display
// 
///////////////////////////////////////////////////////////////////////////////////
void progressBar(float Maximum, float value)
{
  float Minimum = 0;
  int waarde;
  float positie;
	if (value>Minimum)
	{
    setValue("progressBar", "visible", TRUE);
    setValue("progressBar_border", "visible", TRUE);
  }
	
  setValue("progressBar", "scale", value/Maximum, 1.0);
  
  if (Maximum==value)
  {
    delay(0,200);
    setValue("progressBar", "visible", FALSE);
    setValue("progressBar_border", "visible", FALSE);
  }
}

///////////////////////////////////////////////////////////////////////////
// Function dpIsDistributed: Checks or the given dpName is on another system
//
// Input: 1. Datapoint name, including systemName
//
// Output: TRUE,  given dpName is on another system
//         FALSE, given dpName is the same system
///////////////////////////////////////////////////////////////////////////
bool dpIsDistributed(string dpName)
{
  string dpSystemName = strrtrim(dpSubStr(dpName,DPSUB_SYS),":");
  if(getSystemName()==(dpSystemName+":"))
    return FALSE;
  else
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// Function checkDpPermit: Checks or the current user has permission to
//                         access the current datapoint
//
// Output: TRUE,  permitted to access given dpName 
//         FALSE, denied to given dpName 
///////////////////////////////////////////////////////////////////////////
bool checkDpPermit(string datapointPath)
{
  dyn_string treeAccess;
  dyn_string treeAccessPermit;
  bool permit=FALSE, dpIsReference;
  int permitLength;
  dpGet(getSystemName() + "__navigator.treeAccess", treeAccess);
  dyn_string reference;
  checkForReference(datapointPath, reference, dpIsReference);

  for(int i=6;i<=15;i++)
  {
    if(treeAccess[i]!="")
    {
      if(patternMatch(treeAccess[i]+"*", datapointPath))
      {
        if(getUserPermission(i))
        {
          if(permitLength<=strlen(treeAccess[i]))
          {
            permit=TRUE;
            permitLength=strlen(treeAccess[i]);
          }
        }
        else if(!getUserPermission(i))
        {
          if(strlen(datapointPath)>=strlen(treeAccess[i]))
          {
            return FALSE;
          }
        }
      }
    }
  }
  return permit;
}



///////////////////////////////////////////////////////////////////////////
// Function queryDatabaseForDP: Query's the (distributed)database according the
//                         given options
// [NEW!!]
// Input: 1. Datapoint name, including systemName
//        2. Attribute (E.g. "_online.._value")
//        3. Use function progressBar to display a progressBar
//
// Output: dyn_string with the resultsm exlusief the current datapointPath
///////////////////////////////////////////////////////////////////////////
dyn_string queryDatabaseForDP(string attribute, string datapointPath, bool useProgressBar)
{
  string tempDP;
  int j=1;
  dyn_string output;
  dyn_dyn_anytype tab;
  string temp;
  dyn_dyn_string elementNames;
  dyn_dyn_int elementTypes;
  string datapointPathOriginal = datapointPath;
  int elementIndex;
  bool dpIsReference=false;
  dyn_string reference;
  string REMOTESYSTEM="";
  string firstXResults = "";

  checkForReference(datapointPath, reference, dpIsReference);
  if(dpIsReference && !dpIsDistributed(datapointPath))
  {
    strreplace(datapointPath, datapointPathOriginal, reference[2]);
  }  
  else if(dpIsReference && dpIsDistributed(datapointPath))
  {
    strreplace(datapointPath, datapointPathOriginal, reference[2]);
    REMOTESYSTEM = " REMOTE '" + strrtrim(dpSubStr(reference[2],DPSUB_SYS),":") + "'";
  }
  dpQuery("SELECT '"+attribute+"' FROM '"+datapointPath + "*__enabled' "+REMOTESYSTEM, tab);
  int maximumCount = dynlen(tab);
  for(int i=2 ; i<=dynlen(tab) ; i++)
  {
    tempDP = tab[i][1];
    if(tempDP!="")
    {
      strreplace(tempDP, "__enabled.", "");
      if(checkDpPermit(tempDP))
      {
        strreplace(tempDP, datapointPath, "");
        if(strpos(tempDP, "_")==0)
        {
          tempDP = strltrim(tempDP, "_");
        }
        if(tempDP!="")
        {
          output[j]=tempDP;
          j++;
        }
      }
    }
    if(useProgressBar)
      progressBar(maximumCount, j);
  }
  if(useProgressBar)
    progressBar(maximumCount, maximumCount);
  dynSortAsc(output); //sort the dyn_string output (alphanumeric)
  return output;
}





///////////////////////////////////////////////////////////////////////////
// Function queryDatabase: Query's the (distributed)database according the
//                         given options
//
// Input: 1. Datapoint name, including systemName
//        2. Attribute (E.g. "_online.._value")
//        3. Search depth (relative, from current position)
//        4. How many first items must be retrieve via the query!!!
//        5. Give the number of allowed DP-Types  ### UNDER CONSTRUCTION  ###
//        6. Use function progressBar to display a progressBar
//
// Output: dyn_string with the resultsm exlusief the current datapointPath
//         
///////////////////////////////////////////////////////////////////////////
dyn_string queryDatabase(string attribute, string datapointPath, int first, int searchDepth, bool useProgressBar, bool singleSearch)
{
  dyn_string output;
  dyn_dyn_anytype tab;
  string temp, asterisk="*";
  int currentDepth = dynlen(strsplit(datapointPath,"_"));
  dyn_dyn_string elementNames;
  dyn_dyn_int elementTypes;
  string datapointPathOriginal = datapointPath;
  int elementIndex;
  int outputCounter=1;
  bool dpIsReference=false;
  dyn_string reference;
  string REMOTESYSTEM="";
  string firstXResults = "";
  checkForReference(datapointPath, reference, dpIsReference);
  if(dpIsReference && !dpIsDistributed(datapointPath))
  {
    strreplace(datapointPath, datapointPathOriginal, reference[2]);
  }  
  else if(dpIsReference && dpIsDistributed(datapointPath))
  {
    strreplace(datapointPath, datapointPathOriginal, reference[2]);
    REMOTESYSTEM = " REMOTE '" + strrtrim(dpSubStr(reference[2],DPSUB_SYS),":") + "'";
  }
  //How many items must be retrieved (this __enabled and this is a DP-Type)
  if(first>0)
  {
    firstXResults = " FIRST " + first;
  }
  if(singleSearch)
  {
    asterisk="";
  }
  dpQuery("SELECT '"+attribute+"' FROM '"+datapointPath + "__enabled' "+REMOTESYSTEM + firstXResults, tab);
  int maximumCount = dynlen(tab);
  int maximumCounter = 0;
  int i=2;
  string systemName;
  if(dynlen(tab)>=2)
  {
    systemName = strrtrim(dpSubStr(tab[i][1], DPSUB_SYS),":");
  for(i=2 ; i<=dynlen(tab) ; i++)
  {
    int functionOk;
    
    temp = systemName+":"+substr(dpSubStr(tab[i][1], DPSUB_DP), 0, (strlen(dpSubStr(tab[i][1], DPSUB_DP))-strlen("__enabled")));
    if((dynlen(strsplit(temp, "_"))<= (currentDepth+searchDepth)) || (searchDepth==0))
    {
      dynClear(elementNames);  //clear the content
      dynClear(elementTypes);
      functionOk = dpTypeGet(getDpTypeFromEnabled(tab[i][1]),elementNames,elementTypes);
      if(functionOk==-1)
         DebugN("Error in __enabled entry["+i+"]: " +tab[i][1]);
      for(elementIndex=2;elementIndex<=dynlen(elementNames);elementIndex++) 
      {
        int elementLevel = dynlen(elementNames[elementIndex])-1; // how deep is the element?
        string elementName = elementNames[elementIndex][elementLevel+1];
        output[outputCounter] = temp + "." + elementName;
        outputCounter++;
      }
    }
    maximumCounter++;
    //if the progressBar must be used
    if(useProgressBar)
    {
      progressBar(maximumCount, maximumCounter);
    }
  }//end of for loop
  }//end of if
  //Hide the progress bar
  if(useProgressBar)
  {
    progressBar(maximumCount, maximumCount);
  }
  dynSortAsc(output); //sort the dyn_string output (alphanumeric)
  return output;
}

string getLoadTypeFromEnabled(string datapointPath)
{
  string content;
  dyn_string contentSplit;
  dpGet(datapointPath,content);
  contentSplit = strsplit(content, "=");
  return contentSplit[2];
}

string getDpTypeFromEnabled(string datapointPath)
{
  string content;
  dyn_string contentSplit;
  dpGet(datapointPath,content);
  contentSplit = strsplit(content, "=");
    if(dynlen(contentSplit)<2)
    DebugN(contentSplit);

  return contentSplit[2];
}


//////////////////////////////////////////////////////////////////////////////////
// FunctionName: navConfigMessageWarning, used to display a message to the end-user
//
// 
///////////////////////////////////////////////////////////////////////////////////
void navConfigMessageWarning(string message)
{
  ChildPanelOnCentralModal("navigator/MessageWarning", "Warning", makeDynString("$1:"+message));
}




