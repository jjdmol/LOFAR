// file: treeView.ctl 
//
// contents: 
// companion library of the generic panel tree/treeView.pnl which is used to 
// display items in a tree view form.
//
// laset modification dates: April 13, 00 1st version (v1.0)
// author: Tilman Mueller - Astron
//  date           author         version       modification
//------------------------------------------------------------------------------

/** @name treeView.ctl library.
 * Companion library of the generic panel nav_fw/treeView.pnl which is used to 
 * display items in a tree view form.
 * @author T. Mueller - Astron
 * @version 0.1
 */
//@{

/** @name "constants"
 */

// indexes to access node table column
const int treeView_VIEWTEXT = 1;
const int treeView_PARENTID = 2;
const int treeView_PATH     = 3;
const int treeView_LEVEL    = 4;
const int treeView_INFO     = 5;

// indexes to access node map table column
const int treeView_V_NODEID = 1;
const int treeView_V_LEVEL  = 2;

// marks correponding to state of nodes for display
const char treeView_BRANCH_MARK_COLLAPSED = '+'; 
const char treeView_BRANCH_MARK_EXPANDED  = '-';

// trailing speces for each level
const unsigned treeView_INDENT  = 4;

/** @name public functions
 */
//@{

void treeView_Init()
{
  if (addGlobal("gNodeTable", DYN_DYN_ANYTYPE_VAR) == 0)
  {
    
  }
  if (addGlobal("gTreeViewItem2NodeTable", DYN_DYN_ANYTYPE_VAR) == 0) // index == listBoxIndex, col. 1 == nodeID, 2 == level 
  {
  
  }
  id = -1;
}

int treeView_addNode(string nodeName, unsigned parentId, string nodeInfo)
{
  int newNodeID = 0;
  dyn_anytype parentNode = makeDynAnytype();
  if (parentId > 0 && parentId <= dynlen(gNodeTable))
  {
	  parentNode = gNodeTable[parentId];
	}
  if (parentId == 0 || dynlen(parentNode) == 5) // root node or node with id == parentId exists?
  {
    dyn_string allNodeNames = getDynUInt(gNodeTable, treeView_VIEWTEXT);        
    string rawViewText;
    for (int i = 1; i <= dynlen(gNodeTable); i++)
    {
      if (dynlen(gNodeTable[i] != treeView_INFO) continue;
      rawViewText = strltrim(gNodeTable[i][treeView_VIEWTEXT], " +-");
      if ((rawViewText == nodeName) && (gNodeTable[i][treeView_PARENTID] == parentId))
      {
        if (parentId == 0)
        {
          LOG_WARN("Node '" + nodeName + "' already exists as root!");
        }
        else
        {
          LOG_WARN("Node '" + nodeName + "' already exists in this parent node '" + parentNode[treeView_VIEWTEXT] + "' (" + parentId + ")!");
        }
        newNodeID = i;
        break;
      }
    }
    if (newNodeID == 0)
    {
      // find a free place in the table, if no free place is available a new table item will be added
      int i;
      for (i = 1; i <= dynlen(gNodeTable); i++)
      {
        // a table item is free if the dyn array is empty
        if (dynlen(gNodeTable[i]) == 0) 
        {
          newNodeID = i;
          break;
        }
      }
      if (newNodeID == 0) newNodeID = i; // if id is not found yet use a new ID
      
      // now we have a new unique node
      unsigned nrOfSpaces = treeView_INDENT * (parentId == 0 ? 0 : parentNode[treeView_LEVEL] + 1) + 1;
      string spaces = strexpand("\\fill{ }", nrOfSpaces);
      
      dyn_anytype node = makeDynAnytype(spaces + nodeName, parentId, 
                                        (parentId == 0 ? nodeName : parentNode[treeView_PATH] + "." + nodeName), 
                                        (parentId == 0 ? 0 : parentNode[treeView_LEVEL] + 1), 
                                        nodeInfo);      
      gNodeTable[newNodeID] = node;
    }
  }
  else
  {
    LOG_ERROR("There is no node with id == parentId '" + parentId + "'!");
    newNodeID = -1;    
  }
  
  return newNodeID;
}

bool treeView_deleteNode(unsigned nodeId)
{
  dyn_unsigned allParentIds;
  unsigned nodeTableIdxOfChildToRemove;
  // first delete all childs too
  do 
  {
    allParentIds = getDynUInt(gNodeTable, treeView_PARENTID);
    childNodeTableIdToRemove = dynContains(allParentIds, nodeId);
    if (childNodeTableIdToRemove > 0)
    {
      treeView_deleteNode(childNodeTableIdToRemove);
    }
  }
  while (childNodeTableIdToRemove > 0);
  // no child exists (anymore)
  if (dynlen(gNodeTable[nodeId]) > 0)
  {
    gNodeTable[nodeId] = makeDynAnytype(); // free this table item
    return true;
  }
  else
  {
    return false;
  }
}

bool treeView_updateNode(unsigned nodeId, string nodeName, anytype nodeInfo)
{
  
}

dyn_anytype treeView_getNode(unsigned nodeId)
{
  dyn_anytype node = makeDynAnytype();
  if (dynlen(gNodeTable) >= nodeId && nodeId > 0)
  {
    node = gNodeTable[nodeId];
  }
  return node;
}

unsigned treeView_convTreeIdx2NodeId(unsigned treeViewIdx)
{
  unsigned nodeId = 0;
  if (dynlen(gTreeViewItem2NodeTable) >= treeViewIdx && treeViewIdx > 0)
  {
    nodeId = gTreeViewItem2NodeTable[treeViewIdx][treeView_V_NODEID];    
  }
  return nodeId;
}

dyn_unsigned treeView_findNodes(string nodePattern)
{
  dyn_unsigned foundNodeIds;
  strreplace(nodePattern, "_", ".");
  strreplace(nodePattern, ":", ".");
  
  dyn_string allNodePaths = getDynString(gNodeTable, treeView_PATH);
  for (int i = 1; i <= dynlen(allNodePaths); i++)
  {
    if (patternMatch(nodePattern, allNodePaths[i]))
    {
      dynAppend(foundNodeIds, i);
    }
  }
  
  return foundNodeIds;
}

dyn_unsigned treeView_findNodesByInfo(string nodeInfoPattern)
{
  dyn_unsigned foundNodeIds;
  
  dyn_string allNodeInfos = getDynString(gNodeTable, treeView_INFO);
  for (int i = 1; i <= dynlen(allNodeInfos); i++)
  {
    if (patternMatch(nodeInfoPattern, allNodeInfos[i]))
    {
      dynAppend(foundNodeIds, i);
    }
  }
  
  return foundNodeIds;
}

treeView_defaultOnCollapseNode(unsigned selNodeIdx)
{
  dyn_string listItems = list.items;
  
  if (selNodeIdx > 0 && selNodeIdx <= dynlen(gTreeViewItem2NodeTable))
  {
     unsigned collapseLevel = gTreeViewItem2NodeTable[selNodeIdx][treeView_V_LEVEL];
    while ((selNodeIdx + 1) <= dynlen(gTreeViewItem2NodeTable))
    {
      if (gTreeViewItem2NodeTable[selNodeIdx + 1][treeView_V_LEVEL] == collapseLevel)
      {
        // end of sub tree is reached, so no items should be deleted anymore
        break;
      }
      else
      {
        dynRemove(gTreeViewItem2NodeTable, selNodeIdx + 1);
        dynRemove(listItems, selNodeIdx + 1);
      }
    }
    string collapseItemText = listItems[selNodeIdx];
    unsigned branchMarkPlace = treeView_INDENT * collapseLevel + 1;
    collapseItemText[branchMarkPlace] = treeView_BRANCH_MARK_COLLAPSED;
    listItems[selNodeIdx] = collapseItemText;
    list.items = listItems;
  }
  id = -1;
}

treeView_defaultOnExpandNode(unsigned selNodeIdx)
{
  dyn_string listItems = list.items;
  dyn_anytype expandNode;
  dyn_unsigned allParentIds = getDynUInt(gNodeTable, treeView_PARENTID);
  dyn_string childNodesToAdd;
  string expandItemText;
  if (selNodeIdx > 0 && selNodeIdx <= dynlen(gTreeViewItem2NodeTable))
  {
    expandNode = gTreeViewItem2NodeTable[selNodeIdx];
    expandItemText = listItems[selNodeIdx];
  }
  else if (selNodeIdx == 0) // show rootnodes
  {
  	expandItemText = "";
    expandNode = makeDynAnytype(0, -1);
  }
  else
  {
    id = -1;
    return;
  }
  // 1: find all childs (1 level depth)
  for (int i = 1; i <= dynlen(allParentIds); i++)
  {
    if (allParentIds[i] == expandNode[treeView_V_NODEID])
    {
      dynAppend(childNodesToAdd, gNodeTable[i][treeView_VIEWTEXT] + "," + i);
    }
  }
  
  unsigned branchMarkPlace = treeView_INDENT * expandNode[treeView_V_LEVEL];
  if (dynlen(childNodesToAdd) > 0)
  {
    // 2: sort childs by name
    dynSortAsc(childNodesToAdd);
    // 3: add childs to list item and node map table
    dyn_string splittedChild;
    unsigned childNodeId;
    unsigned branchMarkPlaceOfChilds = treeView_INDENT * (expandNode[treeView_V_LEVEL] + 1);
    string nodeName;
    for (int i = 1; i <= dynlen(childNodesToAdd); i++)
    {
      splittedChild = strsplit(childNodesToAdd[i], ',');
      childNodeId = (unsigned) splittedChild[2];
      nodeName = splittedChild[1];
      if (dynContains(allParentIds, childNodeId) > 0)
      {
        nodeName[branchMarkPlaceOfChilds] = treeView_BRANCH_MARK_COLLAPSED;
      }
      dynInsertAt(listItems, nodeName, selNodeIdx + i);
      dynInsertAt(gTreeViewItem2NodeTable, 
                  makeDynAnytype(childNodeId, (expandNode[treeView_V_LEVEL] + 1)), 
                  selNodeIdx + i);
    }
    if (branchMarkPlace >= 0)
	    expandItemText[branchMarkPlace] = treeView_BRANCH_MARK_EXPANDED;
  }
  else
  {
    if (branchMarkPlace >= 0)
	    expandItemText[branchMarkPlace] = ' ';
  }
  if (selNodeIdx > 0)
  {
	  listItems[selNodeIdx] = expandItemText;
	}
  list.items = listItems;
  id = -1;
}


// end of private functions
//------------------------------------------------------------------------------

//@} end of treeView.ctl library



