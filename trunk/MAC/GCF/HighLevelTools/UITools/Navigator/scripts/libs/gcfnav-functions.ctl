// global functions. All event handlers are implemented here

global string  ACTIVEX_TREE_CTRL      = "FlyTreeXCtrl.FlyTreeX";
global string  ACTIVEX_TREE_CTRL_NAME = "FlyTreeXCtrl1";
global string  LIST_TREE_CTRL_NAME   = "list";
global string  TAB_VIEWS_CTRL_NAME    = "TabViews";
global bool    ACTIVEX_SUPPORTED      = false;
global int     NR_OF_TABS             = 10;
global mapping g_itemID2datapoint;
global mapping g_datapoint2itemID;
global bool    g_initializing         = true;
global bool    g_treeCtrlInitializing = true;

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
  idispatch activeXctrl = createComObject(ACTIVEX_TREE_CTRL);
  if(activeXctrl==0)
  {
    DebugTN("I cannot create a COM object!? What the ....?? You must be running Linux or something.",activeXctrl);
    ACTIVEX_SUPPORTED = false;
  }
  else
  {
    DebugTN("I can create a COM object! ",activeXctrl);
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
  shape treeCtrl = getTreeCtrl(); 
  if(ActiveXSupported())
  {
    return treeCtrl.Selected; 
  }
  else
  {
/* old tree    
    int selectedPos = treeCtrl.selectedPos;
    if(selectedPos == -1)
      selectedPos = 0;
    return selectedPos;
*/
    dyn_string exceptionInfo;
    unsigned selectedPos;
    fwTreeView_getSelectedPosition(selectedPos,exceptionInfo);
    if(selectedPos == -1)
      selectedPos = 0;
    return selectedPos;
  }
}

///////////////////////////////////////////////////////////////////////////
//Function showTab(string systemName, string datapointPath)
// 
// shows the tab identified by the systemname and datapoint
///////////////////////////////////////////////////////////////////////////
void showTab(string dpTabs, string systemName, string datapointPath)
{
  shape tabCtrl = getTabCtrl();
  string dpSelectedDatapointContainer = "__navigator.selectedDatapoint";
  string dpSelectedDatapoint = systemName + ":" + datapointPath;
  if(dpExists(dpSelectedDatapointContainer))
  {
    if(!dpExists(dpSelectedDatapoint))
    {
      dpSelectedDatapoint="";
    }
    dpSet(dpSelectedDatapointContainer,dpSelectedDatapoint);
  }

  dyn_string panelParameters = makeDynString("$datapoint:" + datapointPath, "$systemName:" + systemName);

  // get tab properties
  int selectedTab;
  dyn_anytype tabs;
  dyn_string arDpTabs;
  string dpTabString;
  int tabId=1;
  dpTabString = dpTabs+".tab"+tabId;
  while(dpExists(dpTabString))
  {
    dynAppend(arDpTabs,dpTabString);
    tabId++;
    dpTabString = dpTabs+".tab"+tabId;
  }
  DebugTN("showTab","arDpTabs: ",arDpTabs);
  // get the selected tab for this resource type
  dpGet(dpTabs+".selectedTab",selectedTab);
  
  // get the tabs references for this resource type
  dpGet(arDpTabs,tabs);

  for(tabId=1;tabId<=dynlen(tabs);tabId++)
  {
    if(dpExists(tabs[tabId]))
    {
      string caption;
      string panel;
      dpGet(tabs[tabId]+".caption",caption,tabs[tabId]+".panel",panel);
      if(strlen(caption)>0 && strlen(panel)>0)
      {
        DebugTN("showTab","making tab visible: ",tabId);
        tabCtrl.namedColumnHeader("tab"+tabId) = caption;
        tabCtrl.registerPanel(tabId-1,panel,panelParameters);
        tabCtrl.registerVisible(tabId-1)=TRUE;
      }
      else
      {
        DebugTN("showTab","empty caption or filename; making tab invisible: ",tabId);
        tabCtrl.registerVisible(tabId-1)=FALSE;
      }
    }
    else
    {
      DebugTN("showTab","tab reference not found; making tab invisible: ",tabId);
      tabCtrl.registerVisible(tabId-1)=FALSE;
    }
  }
  // make the rest of the tabs invisible
  int i;
  for(i=tabId;i<=NR_OF_TABS;i++)
  {
    DebugTN("showTab","tab undefined; making tab invisible: ",i);
    tabCtrl.registerVisible(i-1)=FALSE;
  }
  if(selectedTab < 1 || selectedTab > dynlen(tabs))
    selectedTab=1;
  tabCtrl.activeRegister(selectedTab-1);
}

///////////////////////////////////////////////////////////////////////////
//Function setSelectedTab(string strTabCtrl, string dpTabs)
// 
// sets the selected tab property in the database
///////////////////////////////////////////////////////////////////////////
void setSelectedTab(string dpTabs)
{
  shape tabCtrl = getTabCtrl();
  // get tab properties
  int selectedTab = tabCtrl.activeRegister + 1;
  // get the selected tab for this resource type
  DebugTN("setSelectedTab",dpTabs,selectedTab);
  dpSet(dpTabs+".selectedTab",selectedTab);
}

///////////////////////////////////////////////////////////////////////////
//Function getTabsDatapoint
// 
// returns the tabs datapoint corresponding to the systemname and datapointpath
///////////////////////////////////////////////////////////////////////////
string getTabsDatapoint(string systemName, string datapointPath)
{
  string datapointType;
  string fullPath = systemName + ":" + datapointPath;
  string dpTabs = "";
  
  if(dpExists(fullPath))
  {
    datapointType = dpTypeName(fullPath);
    // find __navigator_<datapointType>_tabs datapoint
    dpTabs = "__navigator_"+datapointType+"_tabs";
  }
  if(!dpExists(dpTabs))
  {
    DebugTN("getTabsDatapoint","DP does not exist, using default configuration",dpTabs);
    dpTabs = "__navigator_default_tabs";
  }

  DebugTN(dpTabs);
  return dpTabs;
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
/* old tree
    string tempText = getLevelledString(text,level);
    treeCtrl.appendItem(tempText);  
    nodeId = treeCtrl.itemCount();
*/
  
    fwTreeView_appendNode(text,"",0,level);
    nodeId = fwTreeView_getNodeCount();

  }
  return nodeId;
}

///////////////////////////////////////////////////////////////////////////
//Function treeAddDatapoints()
//      
// Adds names of datapoints and their elements to the treeview
///////////////////////////////////////////////////////////////////////////
void treeAddDatapoints(dyn_string names)
{
  shape treeCtrl = getTreeCtrl();
  int namesIndex;
  dyn_string addedDatapoints;
  
  // go through the list of datapoint names
  for(namesIndex=1;namesIndex<=dynlen(names);namesIndex++)
  {
    int pathIndex;
    dyn_string dpPathElements;
    string systemName;
    string datapointName;
    dyn_dyn_string elementNames;
    dyn_dyn_int elementTypes;
    int parentId;
    long addedNode=0;
    // get the System part of the datapoint name
    systemName = strrtrim(dpSubStr(names[namesIndex],DPSUB_SYS),":");
    datapointName = dpSubStr(names[namesIndex],DPSUB_DP);
    
    // get the datapoint structure
    dpTypeGet(dpTypeName(names[namesIndex]),elementNames,elementTypes);
    
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
        if(addingDPpart == systemName)
        {
          addedNode = treeAddNode(-1,0,addingDPpart);
//          DebugTN("Added root node: ",addedNode,addingDPpart);
          g_itemID2datapoint[addedNode] = addingDPpart;
          g_datapoint2itemID[addingDPpart] = addedNode;
        }
        else
        {
          addedNode = treeAddNode(parentId,pathIndex,dpPathElements[pathIndex]); 
//          DebugTN("Added node: ",addedNode,parentId,pathIndex,dpPathElements[pathIndex]);
          g_itemID2datapoint[addedNode] = addingDPpart;
          g_datapoint2itemID[addingDPpart] = addedNode;
          if(dpPathElements[pathIndex] == "Alert")
          {
            // subscribe to Alert status
            DebugTN("subscribing to: ",names[namesIndex]);
            dpConnect("AlertStatusHandler",TRUE,names[namesIndex] + ".status:_online.._value");
          }
        }
        
        // add elements of this datapoint, if any, skip system stuff
        if(addedNode != 0 && addingDPpart != systemName)
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
            
            addedNode = treeAddNode(parentIds[elementLevel],pathIndex+elementLevel,elementName); 
//            DebugTN("Added node: ",addedNode,parentIds[elementLevel],pathIndex+elementLevel,elementName);
            string fullDPname = addingDPpart+parentNodes[elementLevel]+"."+elementName;
            g_itemID2datapoint[addedNode] = fullDPname;
            g_datapoint2itemID[fullDPname] = addedNode;
            
            parentIds[elementLevel+1] = addedNode; // remember this node as parent at its level in case there are elements below this one
            parentNodes[elementLevel+1] = parentNodes[elementLevel]+"."+elementName;
          }
        }
      }
      parentId = addedNode;
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
}

///////////////////////////////////////////////////////////////////////////
//Function getHierarchyLevel
// 
// returns     the level in the hierarchy of the supplied string
///////////////////////////////////////////////////////////////////////////
int getHierarchyLevel(string text)
{
  int spaces=0;
  while(spaces<strlen(text)&&text[spaces]==' ')
  {
    spaces++;
  }
  return spaces;
}

///////////////////////////////////////////////////////////////////////////
//Function getLevelledString
// 
// returns the supplied string prepended with spaces that indicate its level
///////////////////////////////////////////////////////////////////////////
string getLevelledString(string node, int level)
{
  string tempNode = "";
  int i;
  for(i=2;i<=level;i++)
  {
    tempNode = tempNode + " ";
  }
  tempNode = tempNode + node;
  return tempNode;
}

///////////////////////////////////////////////////////////////////////////
//Function buildPathFromNode()
// 
// builds a datapoint path from a node in the treeview
///////////////////////////////////////////////////////////////////////////
string buildPathFromNode(long Node, string& systemName, string& datapointPath)
{
  DebugTN("buildPathFromNode()",Node, systemName, datapointPath);
  
  string datapoint = g_itemID2datapoint[Node];
  systemName    = strrtrim(dpSubStr(datapoint,DPSUB_SYS),":");
  datapointPath = dpSubStr(datapoint,DPSUB_DP_EL);
  DebugTN("~buildPathFromNode()",Node, systemName, datapointPath);
}

///////////////////////////////////////////////////////////////////////////
//Function getNodeFromDatapoint
// 
// returns the nodeid in the treeview of the corresponding datapoint
///////////////////////////////////////////////////////////////////////////
long getNodeFromDatapoint(string dpe)
{
  long nodeId;
  
  string datapointName = dpSubStr(dpe,DPSUB_SYS_DP_EL);
  DebugTN("getNodeFromDatapoint: searching for: ",dpe,datapointName);
  nodeId = g_datapoint2itemID[datapointName];

  DebugTN("found??? nodeId= ",nodeId);
  return nodeId;
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
    DebugTN("AlertStatusHandler()",dpe,status);
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
          DebugTN(nodeText);
          node.Text = nodeText;
          DebugTN(node.Text);
        }
        treeCtrl.InvalidateRow(node.GetRow());
      }
      else
      {
  // TODO
      }
    }
    DebugTN("~AlertStatusHandler()",dpe,status);
  }
}

///////////////////////////////////////////////////////////////////////////
//Function addColorTags
//
// adds HTML color tags to the specified string
///////////////////////////////////////////////////////////////////////////
string addColorTags(string text, string fgcolor="" ,string bgcolor = "")
{
  DebugTN("COLOR TAGS DISABLED");
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
  DebugTN(taggedText);
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
  DebugTN("COLOR TAGS DISABLED");
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
  DebugTN("Navigator_HandleEventInitialize()");
  
  mapping empty;
  g_itemID2datapoint = empty;
  g_datapoint2itemID = empty;
  DebugTN("global stuff lengths:",mappinglen(g_itemID2datapoint),mappinglen(g_datapoint2itemID));
  
  g_initializing=true;
  
  setActiveXSupported();
  DebugTN("ActiveXSupported global variable set to ",ActiveXSupported());
  
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
  TabViews_HandleEventInitialize();
  
  // wait until tree control has initialized
  while(g_treeCtrlInitializing) delay(0,200);
  
  // select the last selected datapoint
  string dpSelectedDatapoint="";
  string dpSelectedDatapointContainer = "__navigator.selectedDatapoint";
  if(dpExists(dpSelectedDatapointContainer))
  {
    dpGet(dpSelectedDatapointContainer,dpSelectedDatapoint);
  }
  DebugTN("Navigator_HandleEventInitialize: selectedDP=",dpSelectedDatapoint);
  
  g_initializing = false;
  
  if(strlen(dpSelectedDatapoint)>0)
  {
    shape treeCtrl = getTreeCtrl();
    if(ActiveXSupported())
    {
      treeCtrl.Selected = getNodeFromDatapoint(dpSelectedDatapoint);
    }
    else
    {
/* old tree      
      treeCtrl.selectedPos(getNodeFromDatapoint(dpSelectedDatapoint)); 
      TreeCtrl_HandleEventOnSelChange(treeCtrl.selectedPos());
*/

// todo      treeCtrl.selectedPos(getNodeFromDatapoint(dpSelectedDatapoint)); 
// todo      TreeCtrl_HandleEventOnSelChange(treeCtrl.selectedPos());
      
    }
  }

  DebugTN("~Navigator_HandleEventInitialize()");
}

///////////////////////////////////////////////////////////////////////////
//Function Navigator_HandleEventTerminate()
//
// de-initializes the navigator
///////////////////////////////////////////////////////////////////////////
void Navigator_HandleEventTerminate()
{
  DebugTN("Navigator_HandleEventTerminate()");
}

///////////////////////////////////////////////////////////////////////////
//Function TabViews_HandleEventInitialize()
//
// initializes the tabview
///////////////////////////////////////////////////////////////////////////
void TabViews_HandleEventInitialize()
{
  DebugTN("TabViews_HandleEventInitialize()");
  shape tabCtrl = getTabCtrl();
  tabCtrl.visible=FALSE;
  // hide all tabs
  int i=0;
  dyn_errClass err;
  int setValueResult;
  do
  {
    setValueResult = setValue(getTabCtrlName(),"registerVisible",i,FALSE);
    err = getLastError();
    DebugTN("registerVisible",i,setValueResult,err);
    i++;
  } while(dynlen(err)==0 && i<NR_OF_TABS && setValueResult==0);
  DebugTN("~TabViews_HandleEventInitialize()");
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
    DebugTN("TabViews_HandleEventSelectionChanged");
    long selectedNode = getSelectedNode();
    if(selectedNode != 0)
    {
      string datapointPath,systemName;
      buildPathFromNode(selectedNode, systemName, datapointPath);
      string dpTabs = getTabsDatapoint(systemName, datapointPath);
      if(selectedNode!=0 && dpExists(dpTabs))
      {
        setSelectedTab(dpTabs);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//Function TreeCtrl_HandleEventInitialize()
//
// initializes the Resources treeview
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventInitialize()
{
  DebugTN("TreeCtrl_HandleEventInitialize()");
  
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
/* old tree
    treeCtrl.visible = false;
    treeCtrl.deleteAllItems();
*/

    treeCtrl.visible = false;
    fwTreeView_watchDog(); // prevent memory leak when closing controlling window
    
  }
  
  dyn_string names;
  names = dpNames("PIC*");
  dynAppend(names,dpNames("PAC*"));
  treeAddDatapoints(names);

  if(ActiveXSupported())
  {
    items.EndUpdate();
  }
  else
  {
/* old tree
    treeCtrl.visible = true;
*/
    treeCtrl.visible = true;
  }
  
  TabViews.visible=TRUE;
  DebugTN("~TreeCtrl_HandleEventInitialize()");
}

///////////////////////////////////////////////////////////////////////////
//Function TreeCtrl_EventOnSelChange(long Node)
// 
// initializes the Resources treeview
// TODO: optimize for selection change to the same resource type. Then don't reconfigure the tabs
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnSelChange(long Node)
{
  if(!g_initializing)
  {
    DebugTN("TreeCtrl_HandleEventOnSelChange  ",Node);
    if(Node != 0)
    {
      string datapointPath,systemName;
      buildPathFromNode(Node, systemName, datapointPath);
      string dpTabs = getTabsDatapoint(systemName, datapointPath);

      showTab(dpTabs,systemName,datapointPath);
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
                DebugTN("data is A and image fits",aNode.Data,treeCtrl.ImagesWidth,Right,Left);
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
// maximizes the current view
///////////////////////////////////////////////////////////////////////////
void ButtonMaximize_HandleEventClick()
{
  DebugTN("ButtonMaximize_HandleEventClick");
  shape tabCtrl = getTabCtrl();
  long Node = getSelectedNode();
  if(Node != 0)
  {
    string datapointPath,systemName;
    buildPathFromNode(Node, systemName, datapointPath);
    string dpTabs = getTabsDatapoint(systemName, datapointPath);
    DebugTN("ButtonMaximize_HandleEventClick",Node,dpTabs);
    if(Node!=0 && dpExists(dpTabs))
    {
      // get tab properties
      int selectedTab = tabCtrl.activeRegister + 1;
      // get the selected tab for this resource type
      string propTab = ".tab" + selectedTab;
      string dpActiveTab,caption,panel;
      if(dpGet(dpTabs+propTab,dpActiveTab) == 0)
      {
        // get panel information from database
        if(dpGet(dpActiveTab+".caption",caption)==0 && dpGet(dpActiveTab+".panel",panel)==0)
        {
          dyn_string panelParameters = makeDynString("$datapoint:" + datapointPath, "$systemName:" + systemName);
          ModuleOnWithPanel(caption+": "+datapointPath,-1,-1,0,0,1,1,"",panel, caption, panelParameters);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////
// TreeView_OnCollapse
// 
// called when an item is collapsed
///////////////////////////////////////////////////////////////////////////
TreeView_OnCollapse(unsigned pos)
{
  DebugTN("TreeView_OnCollapse",pos);

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
  DebugTN("TreeView_OnExpand",pos);

  // the last line of code of each fwTreeView event handler MUST be the following:
	id = -1; 
}

///////////////////////////////////////////////////////////////////////////
// TreeView_OnInit
// 
// called when the list is initialized
///////////////////////////////////////////////////////////////////////////
TreeView_OnInit()
{
  DebugTN("TreeView_OnInit");

  TreeCtrl_HandleEventInitialize();  
  g_treeCtrlInitializing = false;

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
  DebugTN("TreeView_OnSelect",pos);
  TreeCtrl_HandleEventOnSelChange(pos);

  // the last line of code of each fwTreeView event handler MUST be the following:
	id = -1; 
}

///////////////////////////////////////////////////////////////////////////
// TreeView_OnRightClick
// 
// called when the right mouse button is clicked on an item 
///////////////////////////////////////////////////////////////////////////
TreeView_OnRightClick(unsigned pos)
{
  DebugTN("TreeView_OnRightClick",pos);

  // the last line of code of each fwTreeView event handler MUST be the following:
	id = -1; 
}

