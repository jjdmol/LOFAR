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
global int 			STARTUP_DELAY = 1;

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
    unsigned selectedPos;
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
        tabCtrl.registerPanel(tabId-1,NAVIGATOR_TAB_FILENAME,panelParameters);
        tabCtrl.registerVisible(tabId-1)=TRUE;
        
        // check if this tab is currently selected
        if(caption == selectedViewCaption)
        {
          selectedViewTabId = tabId;
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
  if(ActiveXSupported())
  {
    if(parentId == -1)
    {
      nodeId = treeCtrl.Items.Add(text); // adds a root item
    }
    else
    {
      nodeId = treeCtrl.ExtractNode(parentId).Add(text); 
    }
  }
  else
  {
    if(parentId==-1)
    {
      nodeId = fwTreeView_appendToParentNode(0,text,"",0,level);
    }
    else
    {
      nodeId = fwTreeView_appendToParentNode(parentId,text,"",0,level);
    }
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
      if(navConfigCheckEnabled(datapointName))
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
              LOG_TRACE("Added node: ",addedNode,parentId,pathIndex,dpPathElements[pathIndex]);
              insertDatapointNodeMapping(addedNode,addingDPpart);
		          if(dpPathElements[pathIndex] == "Alert")
		          {
		            // subscribe to Alert status
		            LOG_TRACE("subscribing to: ",names[namesIndex]);
		            dpConnect("AlertStatusHandler",TRUE,names[namesIndex] + ".status:_online.._value");
		          }
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
          for(elementIndex=2;elementIndex<=dynlen(elementNames);elementIndex++) 
          {
            // every last item of each array contains an element name (see help on dpTypeGet())
            // file:///opt/pvss/pvss2_v3.0/help/en_US.iso88591/WebHelp/ControlA_D/dpTypeGet.htm
            int elementLevel = dynlen(elementNames[elementIndex])-1; // how deep is the element?
            string elementName = elementNames[elementIndex][elementLevel+1];
            
            string fullDPname = addingDPpart+parentNodes[elementLevel]+"."+elementName;
            if(mappingHasKey(g_datapoint2itemID,fullDPname))
            {
              addedNode = g_datapoint2itemID[fullDPname];
            }
            else
            {
              addedNode = treeAddNode(parentIds[elementLevel],pathIndex-1+elementLevel,elementName); 
              LOG_TRACE("Added element node: ",addedNode,parentIds[elementLevel],pathIndex-1+elementLevel,fullDPname);
              insertDatapointNodeMapping(addedNode,fullDPname);
            }
            
            parentIds[elementLevel+1] = addedNode; // remember this node as parent at its level in case there are elements below this one
            parentNodes[elementLevel+1] = parentNodes[elementLevel]+"."+elementName;
          }
        }
	    }
	  }
  }
/*
  // dump mapping contents:
  DebugTN("g_itemID2datapoint:",g_itemID2datapoint);
  DebugTN("g_datapoint2itemID:",g_datapoint2itemID);
*/
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
  
  string datapointName = dpSubStr(dpe,DPSUB_SYS_DP_EL);
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
  
  // first thing to do: get a new navigator ID
  // check the commandline parameter:
  int navID=0;
  if(isDollarDefined("$ID"))
    navID=$ID;
  navConfigSetNavigatorID(navID);
  navConfigIncreaseUseCount();
  
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

  TabViews_HandleEventSelectionChanged();
  
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
    string selectedViewCaption = tabCtrl.namedActiveRegister;
    long selectedNode = getSelectedNode();
    if(selectedNode != 0)
    {
      string datapointPath = buildPathFromNode(selectedNode);
      navConfigSetSelectedView(selectedViewCaption,datapointPath,tabCtrl.activeRegister+1);
    
      string dpViewConfig = navConfigGetViewConfig(datapointPath);
      if(selectedNode!=0 && dpExists(dpViewConfig))
      {
        showView(dpViewConfig,datapointPath);
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
   
    if(!g_initializing)
    {
      LOG_TRACE("TreeCtrl_HandleEventOnSelChange  ",Node);
      if(Node != 0)
      {
        string datapointPath = buildPathFromNode(Node);
        string dpViewConfig = navConfigGetViewConfig(datapointPath);
 
        showView(dpViewConfig,datapointPath);
      }
    }
    else
    {
      LOG_DEBUG("TreeCtrl_HandleEventOnSelChange suppressed while initializing ");
    }
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
  TreeCtrl_HandleEventOnSelChange(pos);

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
  TreeCtrl_HandleEventOnExpand(pos);

  // also call the default OnExpand implementation to expand the node
  fwTreeView_defaultExpand(pos);

  // the last line of code of each fwTreeView event handler MUST be the following:
  id = -1; 
}
