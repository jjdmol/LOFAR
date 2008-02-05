//# gcfnav-tree.ctl
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
//# tree functions for the Navigator.
//#

#uses "nav_fw/gcf-logging.ctl"
#uses "nav_fw/gcf-dputil.ctl"
#uses "nav_fw/fwTreeView.ctl"

global string      ACTIVEX_TREE_CTRL      = "NOT FlyTreeXCtrl.FlyTreeX";
global string      ACTIVEX_TREE_CTRL_NAME = "FlyTreeXCtrl1";
global string      LIST_TREE_CTRL_NAME    = "list";
global dyn_string  g_itemID2datapoint;
global mapping     g_datapoint2itemID;
global bool        g_showDPE              = false; //Show datapoint elements in the tree. If yes=> do!
global int         g_curSelNode           = 1;
global int         id;                             //needed for changing the selection in the tree (panel navigation, ER 218)
global int         treeAddCount           = 0;     //test teller for performance issue
global unsigned    g_nodeID               = 0;
global unsigned    g_parentIndex          = 0;
global dyn_string  g_referenceList        = "";
//Enumaration for the use of userrights.
global int         UR_TREEACCESS1         = 6;
global int         UR_TREEACCESS2         = 7;
global int         UR_TREEACCESS3         = 8;
global int         UR_TREEACCESS4         = 9;
global int         UR_TREEACCESS5         = 10;
global int         UR_TREEACCESS6         = 11;
global int         UR_TREEACCESS7         = 12;
global int         UR_TREEACCESS8         = 13;
global int         UR_TREEACCESS9         = 14;
global int         UR_TREEACCESS10        = 15;



//============================ tree manipulation routines ========================

///////////////////////////////////////////////////////////////////////////
//  
// Function changeSelectedPostion(newDPname)
//
// Try to set the selection of the tree to the given datapointname.
// By searching the path of the DP part by part and expanding the tree
// when neccesary we hope we can reach the datapoint.
// 
///////////////////////////////////////////////////////////////////////////
void changeSelectedPosition(string newDatapoint)
{
	LOG_DEBUG("changeSelectedPostion> changing selection to :",newDatapoint);

	int  		i;
	long 		nodeID;
	dyn_string 	datapointPath = splitDatapointPath(newDatapoint);
	string 		systemName    = strrtrim(dpSubStr(newDatapoint, DPSUB_SYS), ":");
	string 		temp 		  = "";
	string 		temp_dpe 	  = "";

	//  if (g_datapoint == systemName) {
	//    TreeView_OnExpand(1);
	//  }

	// for every part of the fullpath
	for (i = 0; i <= dynlen(datapointPath); i++) {
		if (i == 0) {
			temp = systemName;
		}
		else if (i == 1) {
			temp = temp + ":" + datapointPath[i];
		}
		else {
			if (i == dynlen(datapointPath)) { //last element in datapointPath could be an datapoint element 
				temp_dpe = temp + "." + datapointPath[i];
			}
			temp = temp + "_" + datapointPath[i]; //build datapoint
		}

		// try to find this (part of the) path in the tree.
		nodeID = getNodeFromDatapoint(temp);
		LOG_DEBUG("changeSelectedPostion> trying to expand '" + temp + "', nodeID=" + nodeID);
		if (nodeID == 0) { //temp not found 
			nodeID = getNodeFromDatapoint(temp + " ->"); //maybe a local reference
			if (nodeID != 0) {
				temp = temp + " ->";
			}
			else {
				nodeID = getNodeFromDatapoint(temp + " ->>"); //temp maybe a remote reference
				if (nodeID != 0) {
					temp = temp + " ->>";
				}
			}
		}
		// NodeID may contain indexNr or 0 when not found

		if (i != dynlen(datapointPath)) { // do not expand last node, costs to much time/performance!!!!
			if (nodeID == 0) {
				nodeID = getNodeFromDatapoint(temp_dpe); //nodeID not found, try the datapoint element
			}
			// REO can't we break out of the for loop when nodeID == 0 ????

			if (nodeID != 0) {  // nodeID in administration?
				TreeView_OnExpand((nodeID));
				LOG_DEBUG("changeSelectedPostion> expanding node " + nodeID);
			}
		}
	} // for all elements in the path

	fwTreeView_draw(); 
	g_curSelNode = nodeID; //update global info		 REO what happens if nodeID == 0 ????
	fwTreeView_setSelectedPosition(fwTreeView_Tree2ViewIndex(nodeID));
}

///////////////////////////////////////////////////////////////////////////
//
// Function getSelectedNode
//  
// returns the selected node in either the activex tree control or the 
// emulated tree control
// 0 = nothing selected. First element in the tree is node nr. 1
//
///////////////////////////////////////////////////////////////////////////
long getSelectedNode()
{
	LOG_DEBUG("getSelectedNode");

	long  selectedNode = 0;
	shape treeCtrl 	   = getTreeCtrl(); 

	if (ActiveXSupported()) {
		selectedNode = treeCtrl.Selected; 
	}
	else {
		int selectedPos;
		fwTreeView_getSelectedPosition(selectedPos);
		LOG_TRACE("selected pos:", selectedPos);
		if (selectedPos >= 1) {
			selectedNode = fwTreeView_view2TreeIndex(selectedPos);
		}
		else {
			selectedNode = selectedPos;
		}
	}
	LOG_TRACE("selected node:", selectedNode);

	return selectedNode;
}

///////////////////////////////////////////////////////////////////////////
//
// Function treeAddNode
//  
// Adds text to the treeview and returns the id of the added item
// - The parentId parameter is used to add the node to its parent in the 
//   ActiveX control
// - The level parameter is used to indent the node in the tree list control
//
///////////////////////////////////////////////////////////////////////////
long treeAddNode(long parentId, int level, string text) 
{ 
  long nodeId = 0;

  if (parentId == -1) {
    nodeId = fwTreeView_appendToParentNode(0, text, "", 0, level);
  }
  else {
    nodeId = fwTreeView_appendToParentNode(parentId, text, "", 0, level);
    treeAddCount++;
  }
  return nodeId;
}

///////////////////////////////////////////////////////////////////////////
//
// Function treeAddDatapoints()
//      
// parameters: names           - names of the datapoints to add
//
// Adds names of datapoints and their elements to the treeview
///////////////////////////////////////////////////////////////////////////
void treeAddDatapoints(dyn_string names)
{
	LOG_DEBUG("treeAddDatapoints: ",names);

	int 		namesIndex;
	dyn_string	addedDatapoints;
	string 		systemName;
	long 		addedNode = 0;

	if (dynlen(names) <= 0) {	// if no names passed, return.
		return;
	}

	dynSortAsc(names); 		// Sort the dyn_string names to avoid problems with references

	g_parentIndex = 0;  		// variable used in function fwTreeView_appendToParentNode
	g_nodeID      = 0; 		// to increase performance

	// go through the list of datapoint names and:
	// [1] make the g_datapoint2itemID and g_itemID2datapoint lists up to date
	// [2] make sure that all DPs in the path to the real DPs are in the tree.
	// [3] add all the elements of the DPs to the tree, dereferencing any references.
	string aName;
	string currentSystemName = "";
	for (namesIndex = 1; namesIndex <= dynlen(names); namesIndex++) {
		dyn_int 	internalNodeMapping;		// \ for more efficient updating g_itemID2datapoint
		dyn_string	internalFullDPName;		// / and g_datapoint2itemID.
		string 		datapointName;

		// [1] check if systemname of this DP is already in the dp2itemid list.
		// addedNode contains itemnumber of item in list afterwards
		systemName = strrtrim(dpSubStr(names[namesIndex], DPSUB_SYS), ":");
		if (currentSystemName != systemName) {	// different system as previous DP?
			currentSystemName = systemName;
			// Check if the item already exists
			if (mappingHasKey(g_datapoint2itemID, systemName)) {
				addedNode = g_datapoint2itemID[systemName];
			}
			else {
				// add to TreeObject
				addedNode = treeAddNode(-1, 0, systemName);
				LOG_TRACE("Added root node: ", addedNode, systemName);
				// update g_datapoint2itemID and g_itemID2datapoint
				insertDatapointNodeMapping(addedNode, systemName);
			}  
		}

		// remove the System part from the datapoint name
		aName = names[namesIndex];
		datapointName = dpSubStr(aName, DPSUB_DP);
		if (datapointName == "") {
			datapointName = aName;
			// cut system name myself. Necessary for datapoint parts that are not datapoints themselves
			int sepPos = strpos(datapointName, ":");
			if (sepPos >= 0) {
				datapointName = substr(datapointName, sepPos + 1);
			}
		}

		dyn_string 	reference;
		string 		dpName 		  = aName;
		bool 		dpIsReference = false;
		checkForReference(dpName, reference, dpIsReference);
		if (dpIsReference) { //If the dpName is a reference, use the original datapoint
			LOG_DEBUG("DP is reference", dpName, reference[2]);
		}

		// [2] split the datapoint path in elements and make sure that every part
		// of the full datapointname is in the tree or add it to the tree.
		int 		pathIndex;
		dyn_string	dpPathElements;
		int 		parentId;
		string addingDPpart = systemName;
		dpPathElements      = strsplit(datapointName, "_");
		for (pathIndex = 0; pathIndex <= dynlen(dpPathElements); pathIndex++) {
			// Check if the item already exists as normal dpe or as a reference 
			if (mappingHasKey(g_datapoint2itemID, addingDPpart)) {
				addedNode = g_datapoint2itemID[addingDPpart];
			}
			else if (mappingHasKey(g_datapoint2itemID, addingDPpart+" ->")) {
				addedNode = g_datapoint2itemID[addingDPpart+" ->"];        
			}
			else if (mappingHasKey(g_datapoint2itemID, addingDPpart+" ->>")) {
				addedNode = g_datapoint2itemID[addingDPpart+" ->>"];        
			}
			else {
				// item does not exist
				dynAppend(addedDatapoints, addingDPpart);	// add to local addedDPsList
				if (addingDPpart != systemName) {
					// add to TreeObject
					addedNode = treeAddNode(parentId, pathIndex, dpPathElements[pathIndex]); 
					if (addedNode != 0) {
						internalNodeMapping[dynlen(internalNodeMapping) + 1] = addedNode;
						internalFullDPName[dynlen(internalFullDPName) + 1] = addingDPpart;
					}
					LOG_TRACE("Added node: ", addedNode, parentId, pathIndex, dpPathElements[pathIndex]);
				}
			}
			parentId = addedNode;

			// update the addinDPpart variable that contains that subpart of the DPname that 
			// must be checked/added.
			if (pathIndex < dynlen(dpPathElements)) {
				if (pathIndex == 0) {
					addingDPpart = addingDPpart + ":" + dpPathElements[pathIndex + 1];
				}
				else if (pathIndex < dynlen(dpPathElements)) {
					addingDPpart = addingDPpart + "_" + dpPathElements[pathIndex + 1];
				}
			}
		} // for all parts of the DPname

		// [3] ...
		// get the datapoint structure
		dyn_string elements = getDpTypeStructure(dpName);
		dyn_string splittedElement;
		// is DP a real DP with more than 1 element?
		if ((addedNode != 0) && (addingDPpart != systemName) && (dynlen(elements) > 1)) {          
			int         elementIndex;
			dyn_int     parentIds;
			dyn_string  parentNodes;
			parentIds[1]   = addedNode;
			parentNodes[1] = "";

			string fullDPname;
			// skip the first element in the array because it contains the root element
			for (elementIndex = 2; elementIndex <= dynlen(elements); elementIndex++) {
				splittedElement     = strsplit(elements[elementIndex], ".");
				int    elementLevel = dynlen(splittedElement) - 1; 		// how deep is the element?
				string elementName  = splittedElement[elementLevel + 1];

				// first handle the reference fields (those start with __)
				if ("__" == substr(elementName, 0, 2)) { //Check if the elementName contains reference info 
					if (elementName == "__childDp" )  {		 	// special case: __childDp ?
							
						// __childDP contains e.g. CS001=CS001:LOFAR_PIC
						if (strpos(addingDPpart, " ->") > -1) {
							LOG_WARN("TreeAddDataPoints: This should not happen !!!!");
						}
						else {			
							string referenceContent;
							// get reference definition: CS001=CS001:LOFAR_PIC
							if (dpAccessable(addingDPpart + "." + elementName)) {
								dpGet(addingDPpart + "." + elementName, referenceContent);
							}
							else {
								LOG_WARN("Unable to get reference info for datapoint",addingDPpart + "." + elementName);
							}
							
							dyn_string referenceSplit = strsplit(referenceContent, "=");
							if (dynlen(referenceSplit) >= 2) {
								string referenceText = addingDPpart + referenceSign(referenceSplit[2]);
								// e.g. MCU001:LOFAR_PIC_Core_CS001->>
								//only add reference if not allready present...
								if (! dynContains(g_referenceList,referenceText + "=" + referenceSplit[2])) {
									dynAppend(g_referenceList, referenceText + "=" + referenceSplit[2]);
								}

								// Change internalNodeMapping Name to reference name
								int index = dynContains(internalFullDPName,addingDPpart);
								dynRemove(internalFullDPName,index);
								dynInsertAt(internalFullDPName, referenceText,index);

								// Remove the old datapoint
								int index = dynContains(names,addingDPpart);
								dynRemove(names,index);
								dynInsertAt(names, referenceText,index);
								LOG_TRACE("Add reference: ", referenceText);

								// Because this is a reference, the DP's of the branche must be retrieved and
								// add to the dyn_string names, for correct build-up of the tree.
								dyn_string refResources = navConfigGetResources(referenceText, 2);
								dynAppend(names, refResources);
							} // legal reference contents
							else { // add by REO
								LOG_WARN("Invalid reference: ", referenceContent);
							}

						} // addingPart contains ->
					} // element = __childDP
					else { 
						// element is not __childDP: add a reference to the tree
						if (strpos(addingDPpart, " ->") < 0) {
							string referenceContent;
							if (dpAccessable(addingDPpart + "." + elementName)) {
								dpGet(addingDPpart + "." + elementName, referenceContent);
							}
							else {
								LOG_WARN("Unable to get reference info for datapoint",addingDPpart + "." + elementName);
							}

							LOG_TRACE("ref. content to split: " + referenceContent);
							dyn_string referenceSplit = strsplit(referenceContent, "=");
							if (dynlen(referenceSplit) >=2) {
								string referenceText = addingDPpart + "_" + referenceSplit[1] + referenceSign(referenceSplit[2]);

								//only add reference if not allready present...
								if (! dynContains(g_referenceList,referenceText + "=" + referenceSplit[2])) {
									dynAppend(g_referenceList, referenceText + "=" + referenceSplit[2]);
								} 
								dynAppend(names, referenceText);
								// Because this is a reference, the DP's of the branche must be retrieved and
								// add to the dyn_string names, for correct build-up of the tree.
								dyn_string refResources = navConfigGetResources(referenceText, 2);
								dynAppend(names, refResources);
							} 
							else {
								LOG_DEBUG("No reference info found");
							}

						} // addingPart contains ->
					} // __childDP or not
				} // element started with __
				else if (g_showDPE) { //show elements?
					//a reference element must never appear in the treebrowser
					fullDPname = addingDPpart + parentNodes[elementLevel] + "." + elementName;
					if (mappingHasKey(g_datapoint2itemID, fullDPname)) {
						addedNode = g_datapoint2itemID[fullDPname];
					}
					else {
						addedNode = treeAddNode(parentIds[elementLevel], pathIndex - 1 + elementLevel, elementName); 
						LOG_TRACE("Added element node: ", addedNode, parentIds[elementLevel], pathIndex - 1 + elementLevel, fullDPname);
						if (addedNode != 0) {
							internalNodeMapping[dynlen(internalNodeMapping) + 1] = addedNode;
							internalFullDPName[dynlen(internalFullDPName) + 1] = fullDPname;
						}
					}

					// remember this node as parent at its level in case there are elements below this one
					parentIds[elementLevel + 1] = addedNode; 
					parentNodes[elementLevel + 1] = parentNodes[elementLevel] + "." + elementName;
				} // element did not start with __

			} // for all elements of the DP
		} // DP is not a systemname and has >1 elements

		if (dynlen(internalNodeMapping) != 0) { 
			// update g_datapoint2itemID and g_itemID2datapoint with all parts of the DP.
			insertInternalNodeMapping(internalNodeMapping, internalFullDPName);
		}

	} // for all names

	LOG_DEBUG("g_itemID2datapoint : "+g_itemID2datapoint);
	LOG_DEBUG("g_datapoint2itemID : "+g_datapoint2itemID);
}


// ====================== g_itemId2datapoint & g_datapoint2itemID =================
//
// Administration functions to maintain the g_itemID2datapoint and
// g_datapoint2ItemID arrays.
// 
// insertDatapointNodeMapping(nodenr, dpname)			insert 1 point
// insertInternalNodeMapping (dyn nodenrm dyn dpname)	insert complete path to DP
// getNodeFromDatapoint		 (dpname) : nodenr
// buildPathFromNode		 (nodenr) : dpname
// convertOriginal2ReferenceDP(name, &refname)

///////////////////////////////////////////////////////////////////////////
//
// Function insertDatapointNodeMapping(nodenr, dpname)
// 
// inserts the node and dp in their mappings. Because it is an insert,
// existing mappings with the same and higher node-id's must be updated.
//
///////////////////////////////////////////////////////////////////////////
void insertDatapointNodeMapping(int node, string dp)
{
	LOG_DEBUG("insertDatapointNodeMapping: ", node, dp);

	// make place by moving higher elements one up
	for (int i = dynlen(g_itemID2datapoint); i >= node && i >= 1; i--) {
		g_itemID2datapoint[i + 1] = g_itemID2datapoint[i];
	}
	g_itemID2datapoint[node] = dp;	// insert new

	// increment all points with a higher itemID
	for (int i = 1; i <= mappinglen(g_datapoint2itemID); i++) {
		int value = mappingGetValue(g_datapoint2itemID, i);
		if (value >= node) {
			g_datapoint2itemID[mappingGetKey(g_datapoint2itemID, i)] = value + 1;
		}
	}
	g_datapoint2itemID[dp] = node;	// add new datapointname
}

///////////////////////////////////////////////////////////////////////////
//
// Function insertInternalNodeMapping(dyn nodenrs, dyn dpnames)
// 
// gathers all insertions for a datapoint and then handles it in one time,
// this to increase performance.
//
///////////////////////////////////////////////////////////////////////////
insertInternalNodeMapping(dyn_int internalNodeMapping, dyn_string fullDPname)
{
  LOG_DEBUG("insertInternalNodeMapping: ", internalNodeMapping, fullDPname);
  int length = mappinglen(g_datapoint2itemID);
  int teller = 0;
  int fullDPNameLength = dynlen(internalNodeMapping);

  //# If the first node number is equal or smaller than the last node number id
  //# the global list, this function must be called;
  if (internalNodeMapping[1] <= mappinglen(g_datapoint2itemID)) {
    for (int i= (length + dynlen(internalNodeMapping)); i >= internalNodeMapping[1]; i--) {
      if (i > internalNodeMapping[dynlen(internalNodeMapping)]) {
        g_itemID2datapoint[i] = g_itemID2datapoint[(i - dynlen(internalNodeMapping))];
        g_datapoint2itemID[g_itemID2datapoint[(i - dynlen(internalNodeMapping))]] = i;
      }
      else if (i >= internalNodeMapping[1] && i <= internalNodeMapping[dynlen(internalNodeMapping)]) {
        g_itemID2datapoint[i] = fullDPname[dynlen(fullDPname) + teller];
        g_datapoint2itemID[fullDPname[dynlen(fullDPname) + teller]] = i;
        teller--;
      }
    }
  }
  else {
    for (int j = 1; j <= dynlen(internalNodeMapping); j++) {
      g_itemID2datapoint[internalNodeMapping[j]] = fullDPname[j];
      g_datapoint2itemID[fullDPname[j]] = internalNodeMapping[j];
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//
// Function buildPathFromNode(nodenr) : dpname
// 
// builds a datapoint path from a node in the treeview
//
///////////////////////////////////////////////////////////////////////////
string buildPathFromNode(long Node)
{
  LOG_DEBUG("buildPathFromNode: ", Node);
  string datapointPath = "";
  if (Node >= 1 && dynlen(g_itemID2datapoint) >= Node) {
    datapointPath = g_itemID2datapoint[Node];
  }
  LOG_TRACE("buildPathFromNode(",Node,") returns ", datapointPath);
  return datapointPath;
}

///////////////////////////////////////////////////////////////////////////
//
// Function getNodeFromDatapoint(dpname) : nodenr
// 
// returns the nodeid in the treeview of the corresponding datapoint
//
///////////////////////////////////////////////////////////////////////////
long getNodeFromDatapoint(string dpe)
{
  LOG_DEBUG("getNodeFromDatapoint: ", dpe);
  long nodeId = 0;
  
  string datapointName = dpSubStr(dpe, DPSUB_SYS_DP_EL); // Original 
  //string datapointName = getSystemName(getSystemId()) + dpe;  //New AdB 18-3-2005 maar wel ...
  LOG_TRACE("getNodeFromDatapoint: searching for: ", dpe, datapointName);
  if (mappingHasKey(g_datapoint2itemID, datapointName)) {
    nodeId = g_datapoint2itemID[datapointName];
  }
  else {
    // maybe it was a systemname?
    if (mappingHasKey(g_datapoint2itemID, dpe)) {
      nodeId = g_datapoint2itemID[dpe];
    }
  }
  LOG_TRACE("found??? nodeId= ", nodeId);
  return nodeId;
}

//////////////////////////////////////////////////////////////////////////////////
// Function convertOriginal2ReferenceDP
//
// If the dpName is known in the tree admin return its own name otherwise return
// the name of the current selected datapoint.
//
// REO: Where the hack do you need such a function for???
//
///////////////////////////////////////////////////////////////////////////////////
void convertOriginal2ReferenceDP(string datapointOriginal, string &datapointPathRef)
{
	LOG_DEBUG("convertOriginal2ReferenceDP: ", datapointOriginal, datapointPathRef);

	// DP exists in tree-admin?
	if (mappingHasKey(g_datapoint2itemID, datapointOriginal)) {
		datapointPathRef = datapointOriginal;	// return original name
	}
	else { // DP not in treeadmin, return DPname of current selection
		datapointPathRef = buildPathFromNode(g_curSelNode);	// REO ???
	}
}

// ============================== g_referenceList =================================
//
// Functions for maintaining the g_referenceList variable
//

///////////////////////////////////////////////////////////////////////////
// Function checkForReference
//
// parameters: parentDatapoint - get the children of this datapoint
//             depth           - how many levels of children to get
// 
// returns - original parentDatapoint if it is a reference
//         - bool if it is a reference
//         - dyn_string reference with ref information
//
///////////////////////////////////////////////////////////////////////////
bool checkForReference(string &parentDatapoint, dyn_string &reference, bool &parentDatapointIsReference)
{
  LOG_DEBUG("checkForReference: ", parentDatapoint);
  dyn_string refOut;
  parentDatapointIsReference = FALSE;
  for (int i = 1; i <= dynlen(g_referenceList); i++) {
    refOut = strsplit(g_referenceList[i], "=");
    if (dynlen(refOut) == 2) {
      if (strpos(parentDatapoint, refOut[1]) == 0) {
        parentDatapointIsReference = TRUE;
        strreplace(parentDatapoint, refOut[1], refOut[2]);
        break;
      }
    }
  }
  reference = refOut;
  return parentDatapointIsReference;
}

///////////////////////////////////////////////////////////////////////////
//
// Function checkForReferenceReplaceOriginal
//
// parameters: resources  
//             reference  
// 
// returns - reference resources in stead of original resources
//
///////////////////////////////////////////////////////////////////////////
void checkForReferenceReplaceOriginal(dyn_string &resources, dyn_string reference)
{
  LOG_DEBUG("checkForReferenceReplaceOriginal: ", resources, reference);
  for (int i = 1; i <= dynlen(resources); i++) {
    strreplace(resources[i], reference[2], reference[1]);
  }
}


///////////////////////////////////////////////////////////////////////////
//
// Function checkForReferenceRemoveFromList
//
// parameters: datapointPath - deletes al related references form the
//             g_referenceList
// 
// returns - none
//
///////////////////////////////////////////////////////////////////////////
void checkForReferenceRemoveFromList(string datapointPath)
{
  LOG_DEBUG("checkForReferenceRemoveFromList: ", datapointPath);
  dyn_string refOut;
  
  for (int i = dynlen(g_referenceList); i >= 1; i--) {
    refOut = strsplit(g_referenceList[i], "=");
    if (patternMatch(datapointPath + "*", refOut[1]) && !(datapointPath == refOut[1])) {
      dynRemove(g_referenceList, i);
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//
// Function inRefList(dpname, dyn reference&, isRef&) : bool
//
// Checks if the given datapoint is a reference to another DP.
// If so, the refered datapoint is returned in the second parameter.
//
///////////////////////////////////////////////////////////////////////////
bool inRefList(string &datapoint, dyn_string &reference, bool &isReference) 
{
  dyn_string refOut;
  isReference=FALSE;
  LOG_DEBUG("Lookup : ", datapoint);

  for (int i= 1; i<= dynlen(g_referenceList); i++) {
    refOut = strsplit(g_referenceList[i],"=");
    LOG_DEBUG("refOut[2] : ",refOut[2]);
    if (strpos(datapoint,refOut[2]) > -1) {
      isReference=TRUE;  
      strreplace(datapoint,refOut[2],refOut[1]);
      break;
    }
  }
  reference = refOut;
  return isReference;  
}

//======================= Tree functions and userEvent handlers ==================

///////////////////////////////////////////////////////////////////////////
// Function InitializeTree()
//
// Initializes the Resources treeview:
//  - clear g_datapoint2itemID, g_itemID2datapoint, g_referenceList
//  - load first 2 levels of datapoints from database
//  - set selection to first item
//
///////////////////////////////////////////////////////////////////////////
void InitializeTree()
{
	LOG_DEBUG("InitializeTree()");
	dyn_errClass err;

	mapping empty;
	g_datapoint2itemID = empty;
	dynClear(g_itemID2datapoint);
	dynClear(g_referenceList);
	LOG_TRACE("global stuff lengths:", dynlen(g_itemID2datapoint), mappinglen(g_datapoint2itemID));

	shape treeCtrl = getTreeCtrl();
	idispatch items;
	if (ActiveXSupported()) {
		items = treeCtrl.Items;
		items.BeginUpdate();
		items.Clear();
		treeCtrl.SortType = 0;
	}
	else {
		fwTreeView_watchDog(); // prevent memory leak when closing controlling window
	}

	// get top level resources. "" means root, 1 means: 1 level deep
	dyn_string resources = navConfigGetResources("", 2);
	LOG_DEBUG("adding ",dynlen(resources)," resources: ", LOG_DYN(resources));
	treeAddDatapoints(resources);

	if (ActiveXSupported()) {
		items.EndUpdate();
	}

	// select first element of the tree.
	LOG_DEBUG("~InitializeTree()");
	delay(2, 0);
	shape treeList    = getShape(LIST_TREE_CTRL_NAME);
	LOG_TRACE("Init,  set selected node[1]", g_curSelNode);
	g_curSelNode = 1;
	treeList.selectedPos = 1;
}

///////////////////////////////////////////////////////////////////////////
//
// Function TreeCtrl_HandleEventInitialize()
//
// initializes the Resources treeview
//
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventInitialize()
{
  InitializeTree();
}

///////////////////////////////////////////////////////////////////////////
//
// Function TreeCtrl_EventOnSelChange(long Node)
// 
// Initializes the Resources treeview
// TODO: optimize for selection change to the same resource type. Then don't reconfigure the tabs
//
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnSelChange(long Node)
{
	LOG_DEBUG("TreeCtrl_HandleEventOnSelChange: ", Node);

	if (g_curSelNode != Node) {
		g_curSelNode = Node;
		refreshNavigator();   
	}
}

///////////////////////////////////////////////////////////////////////////
//
// Function TreeCtrl_EventOnExpand(long Node)
// 
// Expands a node in the Resources treeview
//
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnExpand(long Node)
{
	if (g_initializing) {
		LOG_DEBUG("TreeCtrl_HandleEventOnExpand suppressed while initializing ");
		return;
	}

	LOG_DEBUG("TreeCtrl_HandleEventOnExpand ", Node);

	if (Node != 0) {
		string datapointPath = buildPathFromNode(Node);
		// get top level resources. "" means no parent, 1 means: 1 level deep
		dyn_string resources = navConfigGetResources(datapointPath, 2);
		LOG_DEBUG("adding resources: ", LOG_DYN(resources));
		treeAddDatapoints(resources);
	}
}

///////////////////////////////////////////////////////////////////////////
//
// Function TreeCtrl_HandleEventOnCollapse(long Node)
// 
// expands a node in the Resources treeview
//
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnCollapse(unsigned Node)
{
	LOG_DEBUG("reeCtrl_HandleEventOnCollapse: ", Node);

	int 		collapsedNodes = 1;
	dyn_string	datapoint;
	int 		k = 1;
	string 		temp;

	//get all nodes which will be collapsed and clear these nodes from the tree
	fwTreeView_pruneChildren(Node, collapsedNodes, ""); 

	//retrieve all dpnames for these nodes
	for (int j = Node + 1; j <= (Node + collapsedNodes); j++) {
		datapoint[k]= g_itemID2datapoint[j];
		k++;
	}

	LOG_TRACE("TreeCtrl_HandleEventOnCollapse("+Node+"): removing dp's from tree:",LOG_DYN(datapoint));

	//delete the collapse nodes from g_itemID2datapoint and g_datapoint2itemID
	for (int i = 1; i <= dynlen(datapoint); i++) {
		long nodeID = getNodeFromDatapoint(datapoint[i]);
		dynRemove(g_itemID2datapoint, dynContains(g_itemID2datapoint, datapoint[i]));      
		mappingRemove(g_datapoint2itemID, datapoint[i]);
	}

	//renumber the mapping of the dp's
	if (mappinglen(g_datapoint2itemID) > 1) {
		for (int i = 1; i <= mappinglen(g_datapoint2itemID); i++) {
			temp = g_itemID2datapoint[i];
			if (temp != "" && temp != 0) {
				g_datapoint2itemID[temp] = i;
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

	// finally redraw the tree
	fwTreeView_draw();
}


///////////////////////////////////////////////////////////////////////////
//
// Function TreeCtrl_HandleEventOnDrawCell
// 
// Draws icons in columns if needed (ActiveX only)
//
///////////////////////////////////////////////////////////////////////////
void TreeCtrl_HandleEventOnDrawCell(long Col, long Row, float Left, float Top, float Right, float Bottom)
{
	if (g_initializing || !ActiveXSupported()) {
		return;
	}

    LOG_DEBUG("TreeCtrl_HandleEventOnDrawCell: ", Col, Row, Left, Top, Right, Bottom);

	shape treeCtrl = getTreeCtrl();
	if (Row >= treeCtrl.FixedRows) {
		if (Col == 0) {
			idispatch aNode;
			aNode = treeCtrl.GetNodeAt(Left, Top);
			if (aNode != 0) {
				if (aNode.Data != 0) {
					if (aNode.Data == "A" && (treeCtrl.ImagesWidth < (Right - Left))) {
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


///////////////////////////////////////////////////////////////////////////
//
// Function TreeView_OnCollapse (thread)
// 
// called when an item is expanded
//
///////////////////////////////////////////////////////////////////////////
TreeView_OnCollapse(unsigned pos)
{
  LOG_DEBUG("TreeView_OnCollapse", pos);
  TreeCtrl_HandleEventOnCollapse(pos);

  // the last line of code of each fwTreeView event handler MUST be the following:
  id = -1; 
}

///////////////////////////////////////////////////////////////////////////
//
// Function TreeView_OnExpand (thread)
// 
// called when an item is expanded
//
///////////////////////////////////////////////////////////////////////////
TreeView_OnExpand(unsigned pos)
{
  LOG_DEBUG("TreeView_OnExpand", pos);
  string datapointPath = buildPathFromNode(pos);
  if (checkDpPermit(datapointPath) || pos == 1) {
    dyn_string reference;
    bool parentDatapointIsReference;
    checkForReference(datapointPath, reference, parentDatapointIsReference);
    LOG_DEBUG("check for expand", datapointPath, reference,parentDatapointIsReference, dpAccessable(datapointPath));
    if (!parentDatapointIsReference || (parentDatapointIsReference && dpAccessable(datapointPath))) {
      TreeCtrl_HandleEventOnExpand(pos);
      // also call the default OnExpand implementation to expand the node
      fwTreeView_defaultExpand(pos);  
    }
    else {
      showPopupMessage("Warning",MESSAGE_DPACCESS+" "+datapointPath); //dp not accessable
    }
  }
  else {
    showPopupMessage("Warning",MESSAGE_ACCESSDENIED);  //access to resource denied
  }

  // the last line of code of each fwTreeView event handler MUST be the following:
  id = -1; 
}

///////////////////////////////////////////////////////////////////////////
//
// Function TreeView_OnInit (thread)
// 
// called when the list is initialized
//
///////////////////////////////////////////////////////////////////////////
TreeView_OnInit()
{
  LOG_DEBUG("TreeView_OnInit");

  TreeCtrl_HandleEventInitialize();  

  // the last line of code of each fwTreeView event handler MUST be the following:
  id = -1; 
}

///////////////////////////////////////////////////////////////////////////
//
// Function TreeView_OnSelect (thread)
// 
// called when an item is selected
//
///////////////////////////////////////////////////////////////////////////
TreeView_OnSelect(unsigned pos)
{
  LOG_DEBUG("TreeView_OnSelect", pos);
  string datapointPath = buildPathFromNode(pos);
  dyn_string reference;
  bool parentDatapointIsReference;
  checkForReference(datapointPath, reference, parentDatapointIsReference);
  
  LOG_DEBUG("check for select", datapointPath,reference, parentDatapointIsReference);
  //Check if the access is permitted to this point in the tree
  if (checkDpPermit(datapointPath) || pos == 1) {
    //check if the selected item in the tree is an dpe. If yes, use the dp name to check the existence
    if (strpos(datapointPath, ".") > 0) {
     dyn_string datapointPathSplit = strsplit(datapointPath, ".");
     datapointPath = datapointPathSplit[1];
     //DebugN("datapointPath after split:" + datapointPath);
    }
    if (!parentDatapointIsReference || (parentDatapointIsReference && dpAccessable(datapointPath))) {
      TreeCtrl_HandleEventOnSelChange(pos);
    }
    else {
      LOG_INFO((parentDatapointIsReference ? "Is reference: " : "No reference: ") + datapointPath);
      showPopupMessage("Warning",MESSAGE_DPACCESS+" "+datapointPath); //dp not accessable
    }
  }
  else {
    showPopupMessage("Warning",MESSAGE_ACCESSDENIED);  //access to resource denied
  }

  // the last line of code of each fwTreeView event handler MUST be the following:
  id = -1; 
}

//======================== query related datapoint functions =====================

////////////////////////////////////////////////////////////////////////////////
//
// Function queryDatabaseForDpElements(DPname) : dyn elements
//
// Retrieve all elementnames from the given DP from the database,
//
////////////////////////////////////////////////////////////////////////////////
dyn_string queryDatabaseForDpElements(string datapointPath)
{
	LOG_DEBUG("queryDatabaseForDpElements: ", datapointPath);

	dyn_string	output;
	int 		outputCounter = 1;
	dyn_string  dpes = dpNames(dpSubStr(datapointPath, DPSUB_SYS_DP) + ".**;");
	int 		dpesLen = dynlen(dpes);
	for (int j = 1; j <= dpesLen; j++) {
		if (dpElementType(dpes[j]) != DPEL_TYPEREF && dpElementType(dpes[j]) != DPEL_STRUCT) {
			output[outputCounter] = dpes[j];
			outputCounter++;
		}
	}

	return output;
}

///////////////////////////////////////////////////////////////////////////
//
// Function queryDatabaseForDP
//
// Query's the (distributed)database according the given options
//
// Input: 1. Datapoint name, including systemName
//        2. Attribute (E.g. "_online.._value")
//        3. Use function progressBar to display a progressBar
//
// Output: dyn_string with the result including the current datapointPath
//
///////////////////////////////////////////////////////////////////////////
dyn_string queryDatabaseForDP(string attribute, string datapointPath, bool useProgressBar)
{
	LOG_DEBUG("queryDatabaseForDP: ", attribute, datapointPath, useProgressBar);

	string 			tempDP;
	dyn_string 		output;
	dyn_dyn_anytype tab;
	string 			datapointPathOriginal = datapointPath;
	bool 			dpIsReference = false;
	dyn_string 		reference;
	string 			REMOTESYSTEM = "";

	// derefence datapoint
	checkForReference(datapointPath, reference, dpIsReference);

	// construct query part if DP is on remote system.
	if (dpIsDistributed(datapointPath)) {
		if (dpIsReference) {
			REMOTESYSTEM = " REMOTE '" + strrtrim(dpSubStr(reference[2], DPSUB_SYS), ":") + "'";
		}
		else {
			REMOTESYSTEM = " REMOTE '" + strrtrim(dpSubStr(datapointPath, DPSUB_SYS), ":") + "'";
		}
	}

	// do the query
	dpQuery("SELECT '" + attribute + "' FROM '" + datapointPath  + REMOTESYSTEM, tab);

	int maximumCount = dynlen(tab);
	for (int i = 2; i <= dynlen(tab); i++) {
		tempDP = tab[i][1];
		if (tempDP != "") {
			if (checkDpPermit(tempDP)) {
				if (tempDP != "") {
					dynAppend(output, tempDP);
				}
			}
		}
		if (useProgressBar) {
			progressBar(maximumCount, i);
		}
	} // for all DPs found in database

	if (useProgressBar) {
		progressBar(maximumCount, maximumCount);	// hide bar
	}

	dynSortAsc(output); //sort the dyn_string output (alphanumeric)
	return output;
}


///////////////////////////////////////////////////////////////////////////
//
// Function getDpTypeStructure(datapoint)
//      
// Gets the type structure of a DP located on a local system or remote system
// even if DP does not exists a dummy DP will be created
//
// Params: dp - DP from which the typestructure is requested
//              NOTE: The dp param may not contains element, config or attribute tags
// Returns: a list with the type structure including the root element of the DP
//
///////////////////////////////////////////////////////////////////////////
dyn_string getDpTypeStructure(string dp)
{

  LOG_DEBUG("getDpTypeStructure: ", dp);

  dyn_string typeStructure = makeDynString();
  bool createdDummy = FALSE;

  if (dpAccessable(dp)) {
    typeStructure = dpNames(dp + ".**");
  }
  else {
    LOG_WARN("Error getDpTypeStructure: ", dp);
  }

  // the elements of the structure includes the dp name, so the dp has to be cut off here
  for (int i = 1; i <= dynlen(typeStructure); i++) {
    typeStructure[i] = substr(typeStructure[i], strlen(dp));
  }

  return typeStructure;
}

