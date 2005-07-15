// file: fwTreeView.ctl (old name: tree/libTree.ctl)
//
// contents: 
// companion library of the generic panel tree/treeView.pnl which is used to 
// display items in a tree view form.
//
// author: Ph. Gras CERN/EP - University of Karlsruhe
// laset modification dates: April 13, 00 1st version (v1.0)
//  date           author         version       modification
// April 13, 00    Ph. Gras        v1.0      1st version
// April 14, 00    Ph. Gras        v1.0b     1.change folder mark according if 
//                                           it is collapses or expanded
//                                           2. correction of one error in 
//                                           fwTreeView_defaultCollapse.
// April 16,00     Ph. Gras        v1.0      fix bug in fwTreeView_defaultExpand
//                                           /Collapse():
//                                           problem with last child fixed. 
// April 25,00     Ph. Gras        v1.0b     fixing remaining problem with last child 
// April 9, 01     Ph. Gras        v2.0      1. complete function renaming in
//                                           order to include it in JCOP 
//                                           framework.
//                                           2. making comment compliante with
//                                           doc++.
// May 2, 01       Ph. Gras        v2.0      1. move dynInsertOneItem and 
//                                           dynRemoveEx to fwUtil.ctl library.
//
// Dec 2, 03		mgonzale	  v2.0.3	Added function to get selected node
//											Defined constants for: indexes to access node, state, etc
//											Added possibility to refresh the tree from outside
//
// Jan 13, 05      de Bruijn      v2.04     Added function fwTreeView_Tree2ViewIndex
//
//
//
//
//------------------------------------------------------------------------------

/** @name treeView.ctl library.
 * Companion library of the generic panel tree/treeView.pnl which is used to 
 * display items in a tree view form.
 * @author Ph. Gras CERN/EP - University of Karlsruhe
 * @version 2.0
 */
//@{

/** @name "constants"
 */

// indexes to access node object
const int fwTreeView_HANDLE = 1;
const int fwTreeView_LEVEL 	= 2;
const int fwTreeView_NAME	= 3;
const int fwTreeView_STATE 	= 4;
const int fwTreeView_VALUE 	= 5;

// state of nodes
const unsigned fwTreeView_BRANCH	= 0x1;
const unsigned fwTreeView_EXPANDED	= 0x2;
const unsigned fwTreeView_HIDDEN	= 0x4;

// marks correponding to state of nodes for display
const string fwTreeView_BRANCH_MARK_COLLAPSED	= "+"; 
const string fwTreeView_BRANCH_MARK_EXPANDED	= "-";

// trailing speces for each level
const unsigned fwTreeView_INDENT	= 4;

//@{
/** Flag for hidden tree node.
 * <p>Usage: JCOP framework internal, public
 */
unsigned fwTreeView_HIDDEN() { return fwTreeView_HIDDEN; }

/** Flag for an expanded node.
 * <p>Usage: JCOP framework internal, public
 */
unsigned fwTreeView_EXPANDED() { return fwTreeView_EXPANDED; }

/** Flag for a branch.
 * <p>Usage: JCOP framework internal, public
 */
unsigned fwTreeView_BRANCH() { return fwTreeView_BRANCH; }
//@}   

/** @name public functions
 */
//@{

/** Function to call successively to construct in an easy way the tree.
 * This function takes in charge the update of node states update.
 * The state of the appended node is set to set hidden collapsed leaf.
 * The state of the node preceding the appended node is updated:
 * levels are compared to determine the relationship of the two nodes
 * <p>Usage: JCOP framework internal, public
 *
 * @param name name of the node to append
 * @param value value of the node to append.
 * @param handle user handle of the node to append. If you don't need
 * a such handle, just put 0.
 * @param level (or generation) of the node inside the tree, 0 for root,
 * 1 for root's children, 2 for chidren of a root's chid... 
 */
fwTreeView_appendNode(string name, anytype value, anytype handle, int level, string referenceName = "")
{
	int i;
	dyn_anytype node;
	dyn_anytype previousNode;
  
	//DebugTN("in fwTreeView_appendNode:", name, value, handle, level);
		
	// update state of the last node of the tree:
	// branch if the appended node is a child of this node.
	if(i = fwTreeView_getNodeCount(referenceName))
	{ // the tree is not empty
		previousNode = fwTreeView_getNode(i, referenceName);
		if(previousNode[2] == level - 1)
		{
			previousNode[fwTreeView_STATE] |= fwTreeView_BRANCH;
		}
		fwTreeView_replaceNode(previousNode, i, referenceName);
	}
	
	node[fwTreeView_HANDLE] = handle;
	node[fwTreeView_LEVEL] = level;
	node[fwTreeView_NAME] = name;
	
	// set state and handle:
	// only root visible:
	if(level == 0)	// root => visible
	{ 
		// visible collapsed leaf
		node[fwTreeView_STATE] = 0; 
	}
	else	// not root => not visible
	{
		// hidden collapsed leaf
		node[fwTreeView_STATE] = fwTreeView_HIDDEN; 
	}
	
	node[fwTreeView_VALUE] = value;
	
	// append the node to the tree:
	fwTreeView_insertNode(node, fwTreeView_getNodeCount() + 1, referenceName);
}


/** Function to call successively to construct in an easy way the tree.
 * This function inserts a node at a given index in the tree
 * This function takes in charge the update of node states update.
 * The state of the appended node is set to set hidden collapsed leaf.
 * The state of the node preceding the appended node is updated:
 * levels are compared to determine the relationship of the two nodes
 * <p>Usage: JCOP framework internal, public
 *
 * @param index position in the tree where to insert the node
 * @param name name of the node to append
 * @param value value of the node to append.
 * @param handle user handle of the node to append. If you don't need
 * a such handle, just put 0.
 * @param level (or generation) of the node inside the tree, 0 for root,
 * 1 for root's children, 2 for chidren of a root's chid... 
 */
fwTreeView_insertTreeNode(unsigned index, string name, anytype value, anytype handle, int level, string referenceName = "")
{
  dyn_anytype node;
  dyn_anytype previousNode;
  
  //DebugTN("in fwTreeView_insertTreeNode:", index,name, value, handle, level);
  
  // update state of the last node of the tree:
  // branch if the appended node is a child of this node.
  if(index > 1)
  { // the tree is not empty
    previousNode = fwTreeView_getNode(index-1, referenceName);
    if(previousNode[2] == level - 1)
    {
     previousNode[fwTreeView_STATE] |= fwTreeView_BRANCH;
    }
    fwTreeView_replaceNode(previousNode, index-1, referenceName);
  }
 
  node[fwTreeView_HANDLE] = handle;
  node[fwTreeView_LEVEL] = level;
  node[fwTreeView_NAME] = name;
 
  // set state and handle:
  // only root visible:
  if(level == 0)  // root => visible
  { 
    // visible collapsed leaf
    node[fwTreeView_STATE] = 0; 
  }
  else  // not root => not visible
  {
    // hidden collapsed leaf
    node[fwTreeView_STATE] = fwTreeView_HIDDEN; 
  }
 
  node[fwTreeView_VALUE] = value;
 
  // append the node to the tree:
  fwTreeView_insertNode(node, index, referenceName);
}

/** Function to call successively to construct in an easy way the tree.
 * This function appends a node to a parent. The node is appended to the existing 
 * childs of the parent.
 * This function takes in charge the update of node states update.
 * The state of the appended node is set to set hidden collapsed leaf.
 * The state of the node preceding the appended node is updated:
 * levels are compared to determine the relationship of the two nodes
 * <p>Usage: JCOP framework internal, public
 *
 * @param parentIndex position in the tree of the parent to which to append the node
 * @param name name of the node to append
 * @param value value of the node to append.
 * @param handle user handle of the node to append. If you don't need
 * a such handle, just put 0.
 * @param level (or generation) of the node inside the tree, 0 for root,
 * 1 for root's children, 2 for chidren of a root's chid... 
 */
unsigned fwTreeView_appendToParentNode(unsigned parentIndex, string name, anytype value, anytype handle, int level, string referenceName = "")
{
  dyn_anytype node;
  dyn_anytype parentNode;
  dyn_anytype testNode;
  unsigned nodeId;
  
  //DebugTN("in fwTreeView_appendToParentNode:", parentIndex,name, value, handle, level);
  //DebugTN("name:" + name + " |value:" + value + " |handle:" + handle + " |level:"+ level);
  if(parentIndex <= 1 || parentIndex > fwTreeView_getNodeCount(referenceName))
  { // simply append it to the tree, because the parent is the root or undefined
    fwTreeView_appendNode(name,value,handle,level,referenceName);
    nodeId = fwTreeView_getNodeCount(referenceName);
    DebugTN("in fwTreeView_appendToParentNode: node appended, parent is root or illegal");
  }
  else
  {
    // check the requested level against the the level of the parent
    parentNode = fwTreeView_getNode(parentIndex, referenceName);
    if(level != (parentNode[fwTreeView_LEVEL] + 1))
    { // append node because parent is not exactly one level higher than the level of the node.
      fwTreeView_appendNode(name,value,handle,level,referenceName);
      nodeId = fwTreeView_getNodeCount(referenceName);
      DebugTN("in fwTreeView_appendToParentNode: requested level is not parent level + 1");
    }
    else
    {
      // insert the node after the last direct child of the parent
	  //DebugTN("____________________________________");
      int i=1;

	  if (g_parentIndex==parentIndex)
	  {
		  g_nodeID = g_nodeID + 1;
		  i = g_nodeID;
		  //DebugTN("##____skip_while_loop_____##");
	  }
	  else
	  {
        if(parentIndex+i <= fwTreeView_getNodeCount(referenceName))
        {       
          testNode = fwTreeView_getNode(parentIndex+i,referenceName);
		      int nodeCount = fwTreeView_getNodeCount(referenceName); // nodeCount retreived from outside the while loop.
          while(testNode[fwTreeView_LEVEL] > parentNode[fwTreeView_LEVEL] && 
                parentIndex+i <= nodeCount)
          {
            i++;
            if(parentIndex+i <= nodeCount)
            {
              testNode = fwTreeView_getNode(parentIndex+i,referenceName);
            }
          }
        }
      g_nodeID = i;
      }
    fwTreeView_insertTreeNode(parentIndex+i,name,value,handle,level,referenceName);
	  nodeId = parentIndex+i;
	  g_parentIndex = parentIndex;
    }
  }

  return nodeId;
}


/** Function to call before closing a
 * panel referencing one or several tree/treeView.pnl.
 * This function remove the tree from memory.
 */
fwTreeView_close()
{
	int dummy;
	dummy = _fwTreeView_closeCriticalSection();
}


/** Default function to pass as $sOnExpand parameter.
 * This function expand the children of a node by
 * setting their state to unhidden.
 * This function will be called by id = startThread(..)
 * and need to have at its LAST line 'id = -1;'
 * @param index index of the node to expand.
 */
fwTreeView_defaultExpand(unsigned index, string referenceName = "")
{
	unsigned pos = index;
	unsigned len = fwTreeView_getNodeCount(referenceName);
	dyn_anytype parentNode = fwTreeView_getNode(index, referenceName);
	dyn_anytype nextNode;
	
	//DebugN("expanding " + index + " reference name " + referenceName);
	//DebugN("index " + index);
	//DebugN(parentNode);
	
	if((pos < len) && (parentNode[fwTreeView_STATE] & fwTreeView_BRANCH))
  	{
    	// there are some children.
    	// browse tree up to the next brother of parentNode:
    	for(pos = index + 1; pos <= len && (nextNode = fwTreeView_getNode(pos, referenceName))[fwTreeView_LEVEL] > parentNode[fwTreeView_LEVEL]; pos++)
		{
			// nextNode=child of parentNode if level(nextNode)=level(parentNode)+1:
			if(nextNode[fwTreeView_LEVEL] == parentNode[fwTreeView_LEVEL] + 1)
			{
				// nextNode=child of parentNode
				// unhide the child
				nextNode[fwTreeView_STATE] = nextNode[fwTreeView_STATE] & ~fwTreeView_HIDDEN; 
				fwTreeView_replaceNode(nextNode, pos, referenceName);
			}
		}
		// set parent node state to expanded
		parentNode[fwTreeView_STATE] = parentNode[fwTreeView_STATE] | fwTreeView_EXPANDED;
		fwTreeView_replaceNode(parentNode, index, referenceName);
	}
 	// signal the end of the thread (function called by id = startThread(...)).
 	id = -1; 
}

/** Default function to pass as $sOnCollapse parameter
 * This function collapse a node by setting the
 * states of its children to hidden.
 * This function will be called by id = startThread(..)
 * and need to have at its LAST line 'id = -1;'
 * @param index	index of the node to collapse
 */
fwTreeView_defaultCollapse(unsigned index, string referenceName = "")
{  
	unsigned pos;
	unsigned subFolderDepth;
	dyn_anytype node;
	dyn_anytype parentNode;
	dyn_string path;
	dyn_int position;

	if(index >  fwTreeView_getNodeCount(referenceName))
	{
    	DebugN(	"collapseFolder(dyn_anytype& tree,"
	   			" unsigned index): index out of bounds)");
	}
	else
	{
		// parent node:
		parentNode = fwTreeView_getNode(index, referenceName);
		pos = index + 1; //index of first node to collapse
		int nodeCount = fwTreeView_getNodeCount(referenceName);
		int i=1;
		while(pos <=  nodeCount && (node = fwTreeView_getNode(pos, referenceName))[fwTreeView_LEVEL] > parentNode[fwTreeView_LEVEL])
		{
			// collapses and hides the folders and leaves
			// (collapse bit of a leaf is meaningless => we don't care if we change it)
      node[fwTreeView_STATE] = (node[fwTreeView_STATE] | fwTreeView_HIDDEN) & ~fwTreeView_EXPANDED;
			fwTreeView_replaceNode(node, pos, referenceName);
      i++;
			pos++; // next node
		}
		// update the state of the folder that was collapsed:
		parentNode[fwTreeView_STATE] = parentNode[fwTreeView_STATE] & ~fwTreeView_EXPANDED;
		fwTreeView_replaceNode(parentNode, index, referenceName);
	}
	// signal the end of the thread (function called by id = startThread(...)).
	id = -1; 
}

/** Default function to pass as $sOnSelect parameter.
 * This function do nothing.
 * This function will be called by id = startThread(..)
 * and need to have at its LAST line 'id = -1;'
 * @param pos index of the selecting node, which can be passed to
 * fwTreeView_getNode(), fwTreeView_replace(),.. functions
 */
fwTreeView_defaultSelect(unsigned pos)
{
	// signal the end of the thread (function called by id = startThread(...)).
	id = -1; 
}

/** Updates the list panel where the tree is drawn.
 */
/* Updated by blaakmeer. The old code deleted all items first and then began to build the tree from the ground up
 * This version overwrites existing items, adds items if there are not enough existing items and removes items if there are too many.
 * Pretty cool stuff!
 */
fwTreeView_draw(string referenceName = "")
{
  unsigned branchMarkLen = strlen(fwTreeView_BRANCH_MARK_COLLAPSED); //==strlen(expandeBranchMark)
  int spaceCnt, i, imax;
  string item;
  int existingNodes;
  int visibleNodes=0;

  // erase list:
//	setValue(referenceName + "list", "deleteAllItems");

  getValue(referenceName + "list","itemCount",existingNodes);

  imax = fwTreeView_getNodeCount(referenceName);
  for(i = 1; i <= imax ; i++)
  {
    if(!(fwTreeView_getNode(i, referenceName)[fwTreeView_STATE] & fwTreeView_HIDDEN))		
    {
      visibleNodes++;
      // the node is visible
      spaceCnt = fwTreeView_INDENT * fwTreeView_getNode(i, referenceName)[fwTreeView_LEVEL];
      item = strexpand("\\fill{ }", spaceCnt);
      if((fwTreeView_getNode(i, referenceName)[fwTreeView_STATE] & fwTreeView_BRANCH) && (fwTreeView_getNode(i, referenceName)[fwTreeView_STATE] & fwTreeView_EXPANDED))
      {
        // this is an expanded branch
        item += fwTreeView_BRANCH_MARK_EXPANDED;
        //	DebugN("expanded",fwTreeView_getNode(i)[fwTreeView_STATE]);
      }
      else 
      {
        if(fwTreeView_getNode(i, referenceName)[fwTreeView_STATE] & fwTreeView_BRANCH)
        {
          // this is a collapsed branch
          item += fwTreeView_BRANCH_MARK_COLLAPSED;
          //	DebugN("collapsed", fwTreeView_getNode(i)[fwTreeView_STATE]);
        }
        else
        {
          // this is a leaf
          item += strexpand("\\fill{ }", branchMarkLen);
        }
      }
      item += fwTreeView_getNode(i, referenceName)[fwTreeView_NAME];
      if(visibleNodes<=existingNodes)
        setValue(referenceName + "list", "replacePos", visibleNodes, item);
      else
        setValue(referenceName + "list", "appendItem", item);
    }
  }
  // remove surplus items
  int deletePos = visibleNodes+1;
  while(deletePos <= existingNodes)
  {
    setValue(referenceName + "list","deletePos",deletePos);
    getValue(referenceName + "list","itemCount",existingNodes);
  }
}


/** Retrieves a node of the tree
 * @param index index of the node to retrieve
 */
dyn_anytype fwTreeView_getNode(unsigned index, string referenceName = "")
{
	int treeIndex;
	string treeIndexString;
	
	getValue(referenceName + "TreeArrayIndex", "text", treeIndexString);
	treeIndex = treeIndexString;
	//_trees_AWqOe1L6[1] is the offset
	return _trees_AWqOe1L6[treeIndex + _trees_AWqOe1L6[1]][index + 1];
}


/** Returns the number of nodes of the tree
 */
unsigned fwTreeView_getNodeCount(string referenceName = "")
{
	int treeIndex;
	string treeIndexString;
	 
	if(dynlen(_trees_AWqOe1L6) == 0)
	{// no tree at all
		return 0;
	} 
	
	//'trees' offset: the tree is stored in _trees_AWqOe1L6[offest+treeIndex] 
	// where offset =  _trees_AWqOe1L6[1]
	getValue(referenceName + "TreeArrayIndex", "text", treeIndexString);
	treeIndex = treeIndexString;
	//  DebugN(treeIndex,offset); 
	if((treeIndex + _trees_AWqOe1L6[1]) <= 0)
	{	// index not correct 
		return 0;
	} 
	// 0 if _trees_AWqOe1L6 is itself empty 
	return dynlen(_trees_AWqOe1L6) >= treeIndex + _trees_AWqOe1L6[1] ? dynlen(_trees_AWqOe1L6[treeIndex + _trees_AWqOe1L6[1]]) - 1: 0; 
    return 0;
}


/** Inserts a node in the tree.
 * @param node node to insert
 * @param index index of the inserted node
 */
fwTreeView_insertNode(dyn_anytype node, unsigned index, string referenceName = "")
{
	int treeIndex;
	string treeIndexString;
	//anytype node_ = node;

	getValue(referenceName + "TreeArrayIndex", "text", treeIndexString);
	treeIndex = treeIndexString;
	//dynInsertAt(_trees_AWqOe1L6[treeIndex+offset], node_, index);
	//_trees_AWqOe1L6[1] is the offset
	fwUtil_dynInsertOneItem(_trees_AWqOe1L6[treeIndex + _trees_AWqOe1L6[1]], node, index + 1); 
}


/** Remove a node from the tree.
 * @param index index of the node to remove
 */
fwTreeView_removeNode(unsigned index, string referenceName = "")
{
	int treeIndex;
	string treeIndexString;
	
	getValue(referenceName + "TreeArrayIndex", "text", treeIndexString);
	treeIndex = treeIndexString;
  
	//_trees_AWqOe1L6[1] is the offset
	fwUtil_dynRemoveEx(_trees_AWqOe1L6[treeIndex+_trees_AWqOe1L6[1]], index+1);
}

/** Remove a node and all of its children (if it has any).
 * @param index index of the node to remove
 */
fwTreeView_prune(unsigned index, string referenceName = "")
{
	unsigned len;
	unsigned pos;
	unsigned subFolderDepth;
	dyn_anytype node;
	dyn_anytype parentNode;

	if(index >  fwTreeView_getNodeCount(referenceName))
	{
		DebugN(	"collapseFolder(dyn_anytype& tree,"
				" unsigned index): index out of bounds)");
	}
	else
	{
		// parent node:
		parentNode = fwTreeView_getNode(index, referenceName);
		
		// index of first child node to remove
		pos = index + 1; 
		
		len = fwTreeView_getNodeCount(referenceName);
		while((pos <= len ) && ((node = fwTreeView_getNode(pos, referenceName))[fwTreeView_LEVEL] > parentNode[fwTreeView_LEVEL]))
		{
			// remove child node:
			fwTreeView_removeNode(pos, referenceName);
			len--;
		}
		// remove the parent node:
		fwTreeView_removeNode(index, referenceName);
	}
}

/** Remove for a node all of its children (if it has any).
 * @param index index of the node to remove and return the number of removed childs
 */
fwTreeView_pruneChildren(unsigned index, int &collapsedNodes, string referenceName = "")
{
	unsigned len;
	unsigned pos;
	unsigned subFolderDepth;
	dyn_anytype node;
	dyn_anytype parentNode;
	dyn_anytype allPos;
	int posCount=1;

	if(index >  fwTreeView_getNodeCount(referenceName))
	{
		DebugN(	"collapseFolder(dyn_anytype& tree,"
				" unsigned index): index out of bounds)");
	}
	else
	{
		//DebugN("index="+index);
		// parent node:
		parentNode = fwTreeView_getNode(index, referenceName);
		
		// index of first child node to remove
		pos = index + 1; 
		len = fwTreeView_getNodeCount(referenceName);
		while((pos <= len ) && ((node = fwTreeView_getNode(pos, referenceName))[fwTreeView_LEVEL] > parentNode[fwTreeView_LEVEL]))
		{
			// remove child node:
			fwTreeView_removeNode(pos, referenceName);
			allPos[posCount]=pos;
			posCount++;
			len--;
		}
		collapsedNodes=dynlen(allPos);
	}
}














/** Replaces a node of the tree by a new node.
 * @param node new node
 * @param index of the node to replace
 */
fwTreeView_replaceNode(dyn_anytype node, unsigned index, string referenceName = "")
{
	int treeIndex;
	string treeIndexString;
	
	getValue(referenceName + "TreeArrayIndex", "text", treeIndexString);
	treeIndex = treeIndexString;
	
	//  DebugN("treeIndex+offset=" + (treeIndex+_trees_AWqOe1L6[1]) + "\tindex=" + index);
	
	//_trees_AWqOe1L6[1] is the offset
	_trees_AWqOe1L6[treeIndex + _trees_AWqOe1L6[1]][index + 1] = node; 
}


/** Gets the index of the tree array element
 * corresponding to the viewIndex-th item
 * in the tree view
 * @param viewIndex number of the line where the node appears in the
 * tree view (which is a list widget).
 * @return node index 
 */
unsigned fwTreeView_view2TreeIndex(unsigned viewIndex, string referenceName = "")
{
	unsigned iview;
	unsigned treeIndex = 0; //result
  
	// the following for loop scan the tree from 1 to viewIndex
	// skipping the "hidden" nodes; the iview loop index is not
	// incremented for these nodes.
	for(iview = 1 ; iview <= viewIndex ; )
		if(!(fwTreeView_getNode(++treeIndex, referenceName)[fwTreeView_STATE]& fwTreeView_HIDDEN)) 
			iview++;
  
	return treeIndex;
}

///////////////////////////////////////////////////////////////////////////
//Function fwTreeView_Tree2ViewIndex
// 
// returns of the selected position in the view of the given node
///////////////////////////////////////////////////////////////////////////
unsigned fwTreeView_Tree2ViewIndex(unsigned treeIndex, string referenceName = "")
{
	unsigned itree;
	unsigned viewIndex = 0; //result
  
	// the following for loop scan the tree from 1 to viewIndex
	// skipping the "hidden" nodes; the iview loop index is not
	// incremented for these nodes.
	for(itree = 1 ; itree <= treeIndex ; itree++)
		if(!(fwTreeView_getNode(itree, referenceName)[fwTreeView_STATE]& fwTreeView_HIDDEN)) 
			viewIndex++;
  
	return viewIndex;
}



/** This function start a garbage collector. This garbage collector
 * looks for memory (in _trees_... array) not released
 * after a panel close. This happens for instance if the 
 * panel is closed by the Windows upper top close botton ([X]). 
 */
fwTreeView_watchDog()
{
	startThread("_fwTreeView_watchDogThread");
}

/** This function returns the selected position of the given tree view
 * @param position: gives the selected position
 * @param referenceName: name given to the tree when it was inserted as a reference panel
 */
fwTreeView_getSelectedPosition(unsigned &position, string referenceName = "")
{
	getValue(referenceName + "list", "selectedPos", position);
}

/** This function changes the selected position of the given tree view
 * @param position: gives the selected position
 * @param referenceName: name given to the tree when it was inserted as a reference panel
 */
fwTreeView_setSelectedPosition(unsigned position, string referenceName = "")
{
	setValue(referenceName + "list", "selectedPos", position);
}

/** This function returns the selected node of the given tree view
 * @param node: selected node
 * @param exceptionInfo: returns details of any errors
 * @param referenceName: name given to the tree when it was inserted as a reference panel
 */
fwTreeView_getSelectedNode(dyn_anytype &node, dyn_string &exceptionInfo, string referenceName = "")
{
	unsigned viewIndex, treeIndex;

	fwTreeView_getSelectedPosition(viewIndex, referenceName);
	if(viewIndex >= 1)
	{
		treeIndex = fwTreeView_view2TreeIndex(viewIndex, referenceName);
		node = fwTreeView_getNode(treeIndex, referenceName);
	}
	else
	{
		node = "";
		fwException_raise(	exceptionInfo, 
							"WARNING",
							"fwTreeView_getSelectedNode: no node is selected.",
		 					"");
	}
}

/** Remove a node and all of its children (if it has any).
 * @param index index of the node to remove
 */
fwTreeView_getParentNodePosition(unsigned index, unsigned &parentPos, string referenceName = "")
{
	unsigned len;
	dyn_anytype node, parentNode;

	len = fwTreeView_getNodeCount(referenceName);
	//DebugN("fwTreeView_getParentNodePosition" + index + " " + len);
	if(index > len)
	{
		DebugN(	"fwTreeView_getParentNodePosition(): "
				" the node index " + index + " exceeds the number of nodes in the tree.");
	}
	else
	{
		node = fwTreeView_getNode(index, referenceName);
		
		// index of first child node to remove
		parentPos = index - 1; 
		
		while((parentPos >= 1 ) && ((parentNode = fwTreeView_getNode(parentPos, referenceName))[fwTreeView_LEVEL] >= node[fwTreeView_LEVEL]))
		{
			//DebugN(parentNode, node, parentPos);
			parentPos--;
		}
	}
}
//@} end of public functions

//------------------------------------------------------------------------------
// Functions private to the library and the tree/treeView.pnl
// panel. 
//------------------------------------------------------------------------------

/* internal use for critical section.
 */
int _fwTreeView_closeCriticalSection()
{
	int i;
	int imax = dynlen(_trees_AWqOe1L6);
	
	for(i = imax; i > 2 ; i--)
	{
		if(dynlen(_trees_AWqOe1L6[i]) > 0 && _trees_AWqOe1L6[i][1] == myPanelName())
		{
			// _trees_AWqOe1L6 contains an orphaned tree (i.e. whose panel has been closed)
			// DebugN("delete tree " + _trees_AWqOe1L6[i][1] +  " with index=" + (i-2) );
			dynRemove(_trees_AWqOe1L6, i);
			_trees_AWqOe1L6[1]--; // decrement _trees... offset
		}
	}
	return 0;
}

/* Looks for element in _trees_... whose tree panel
 * has been closed and delete them. See fwTreeView_watchDog.
 * this function must be atomic=> return a dummy value.
 */
int fwTreeView_collectGarbage()
{
	int i;
	int imax = dynlen(_trees_AWqOe1L6);
	bool garbage = FALSE;
  
	for(i = imax; i > 2 ; i--)
	{
		if(dynlen(_trees_AWqOe1L6[i]) > 0 && !isPanelOpen(_trees_AWqOe1L6[i][1]))
		{
			//_trees_AWqOe1L6 contains an orphaned tree (i.e. whose panel has been closed)
			DebugN("Tree Watch Dog: delete tree " + _trees_AWqOe1L6[i][1] +  " with index=" + (i - 2) );
			dynRemove(_trees_AWqOe1L6,i);
			_trees_AWqOe1L6[1]--; // decrement _trees... offset
			garbage = TRUE; // flag that garbage has been found
		}
	}
	return 0; //dummy value
}

/* Initializes a new tree in the _trees_.. array
 * and sets the panel name the tree belongs to.
 * @param treeIndex index of _trees_... where to insert 
 * the tree.
 */
fwTreeView_newTree(int treeIndex)
{
	// int treeIndex;
	dyn_anytype emptyTree;
	//getValue("TreeArrayIndex","text", treeIndex);
	emptyTree[1] = myPanelName();
	_trees_AWqOe1L6[treeIndex+_trees_AWqOe1L6[1]] = emptyTree;
}

/* watch dog thread (see fwTreeView_watchDog())
 */
_fwTreeView_watchDogThread()
{
	int stop = FALSE;
	// run garbage collector until garbage has been found
	while(!stop)
	{
		delay(5);
		stop = fwTreeView_collectGarbage();
	}
}

// end of private functions
//------------------------------------------------------------------------------

//@} end of treeView.ctl library



