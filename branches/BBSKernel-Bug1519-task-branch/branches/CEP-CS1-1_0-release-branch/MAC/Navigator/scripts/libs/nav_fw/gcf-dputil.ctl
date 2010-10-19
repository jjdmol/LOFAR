//# gcf-dputil.ctl
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
//# Common GCF datapoint utils
//# 

#uses "nav_fw/gcf-logging.ctl"
#uses "nav_fw/gcf-common.ctl"
#uses "nav_fw/gcf-util.ctl"


///////////////////////////////////////////////////////////////////////////
//
// Function dpAccessable: Checks whether the given dpName is accessable and
//                        existing (in case of a distributed system.
//
// Input : datapoint name, including systemName
// Output: TRUE, if accessable and existing,
//         FALSE, if not accessable and/or not existing
//
///////////////////////////////////////////////////////////////////////////
bool dpAccessable(string dpName)
{
	LOG_DEBUG("dpAccessable: ",dpName);

	// DP must exist
	if (!dpExists(dpName)) {
		return FALSE;
	}

	// is DP our own systemname?
	string dpSystemName = strrtrim(dpSubStr(dpName, DPSUB_SYS), ":");
	if (getSystemName() == (dpSystemName + ":")) {
		return TRUE;
	}

	// DP is of other system, check if we know this system
	lockDistSystemSemaphore();
	// check if the dist. system with the systemname from the dpName is reachable
	if (dynlen(g_distributedSystems) > 0) {
		// Check whether the first part of the dpName is a valid system name 
		// and the name of a reachable dist. system
		dyn_string splitedDPName = strsplit(dpName, ':');
		for (int i = 1; i <= dynlen(g_distributedSystems); i++) {
			if (getSystemName(g_distributedSystems[i]) == (splitedDPName[1] + ":")) {
				unlockDistSystemSemaphore();
				return TRUE;
			}          
		}
	}
	unlockDistSystemSemaphore();

	LOG_WARN("Not accessible: ",dpName);

	return FALSE;
}




//////////////////////////////////////////////////////////////////////////////////
//
// Function getElementsFromDp(DP, withoutRef) : dyn elements
//
// Fills the dpe selectionlist for a datapoint selection
//
///////////////////////////////////////////////////////////////////////////////////
dyn_string getElementsFromDp(string datapoint, bool withoutRef = FALSE)
{
  LOG_DEBUG("getElementsFromDp: ", datapoint, "FALSE");
  dyn_string output;
  int elementIndex;

  dyn_string elements = getDpTypeStructure(datapoint);
  // skip the first element in the array because it contains the root element  
  for (elementIndex = 2; elementIndex <= dynlen(elements); elementIndex++) 
  {
    if (!withoutRef || (withoutRef && strpos(elements[elementIndex], ".__") < 1)) 
    {
      dynAppend(output, substr(elements[elementIndex], 1)); // cut leading dot (".")
    }
  }

  dynSortAsc(output);
  return output;
}

///////////////////////////////////////////////////////////////////////////
//
// Function dpIsDistributed: Checks if the given dpName is on another system
//
// Input: 1. Datapoint name, including systemName
//
// Output: TRUE,  given dpName is on another system
//         FALSE, given dpName is the same system
//
///////////////////////////////////////////////////////////////////////////
bool dpIsDistributed(string dpName)
{
  return (getSystemName() != dpSubStr(dpName, DPSUB_SYS));
}

///////////////////////////////////////////////////////////////////////////
//
// Function referenceSign()
// 
// return: " ->>" if reference is distributed
//         " ->"  if reference is NOT distributed
// this to increase performance.
///////////////////////////////////////////////////////////////////////////
string referenceSign(string reference)
{


	return (dpIsDistributed(reference) ? " ->>" : " ->");
}


///////////////////////////////////////////////////////////////////////////
//
// Function SplitDatapointPath
// 
// Splits the given datapointname in its lose components (nodes and elements)
// E.g:  System:A_B_C.d --> A, B, C, d
//
///////////////////////////////////////////////////////////////////////////
dyn_string splitDatapointPath(string newDatapoint)
{
	LOG_DEBUG("splitDatapointPath: ", newDatapoint);

	int 	   i;
	dyn_string datapointElement;
	dyn_string datapointPath = strsplit(newDatapoint, "_");

	// cut system name myself. Necessary for datapoint parts that are not datapoints themselves
	string  datapointName = datapointPath[1];
	int 	sepPos = strpos(datapointName, ":");
	if (sepPos >= 0) {
		datapointName = substr(datapointName, sepPos + 1);
	}
	datapointPath[1] = datapointName;

	// if datapointElement present, split last entry of datapointPath
	datapointElement = strsplit(datapointPath[dynlen(datapointPath)], ".");
	if (dynlen(datapointElement) > 1) {
		datapointPath[dynlen(datapointPath)  ] = datapointElement[1];
		datapointPath[dynlen(datapointPath) + 1] = datapointElement[2];
	}

	return datapointPath;
}

///////////////////////////////////////////////////////////////////////////
//
// Function checkDpPermit
//
// Checks if the current user has permission to access the current datapoint
// This information is stored in the __navigator.treeAccess6..15 variables.
//
// Output: TRUE,  permitted to access given dpName 
//         FALSE, denied to given dpName 
//
///////////////////////////////////////////////////////////////////////////
bool checkDpPermit(string datapointPath)
{
	LOG_DEBUG("checkDpPermit: ",datapointPath);

	// construct basename of the treeAccess DPs
	dyn_string treeAccess;
	dpGet(getSystemName() + "__navigator.treeAccess", treeAccess);

	// Derefence the given datapoint
	dyn_string	reference;
	bool 		dpIsReference;
	checkForReference(datapointPath, reference, dpIsReference);

	dyn_string treeAccessPermit;
	bool permit = FALSE;
	int permitLength = 0;

	for (int i = 6; i <= 15; i++) {
		if (treeAccess[i] != "") {
			if (patternMatch(treeAccess[i] + "*", datapointPath)) {
				if (getUserPermission(i)) {
					if (permitLength <= strlen(treeAccess[i])) {
						permit = TRUE;
						permitLength = strlen(treeAccess[i]);
					}
				}
				else {
					if (strlen(datapointPath) >= strlen(treeAccess[i])) {
						return FALSE;
					}
				}
			}
		}
	}

	return permit;
}

/////////////////////////////////////////////////////////////////////
//
// Function dpGetElementName(DP) : elementName
//
// Returns the elementname of a Datapoint
//
/////////////////////////////////////////////////////////////////////
string dpGetElementName(string DPName)
{
  LOG_DEBUG("dpGetElementName: ", DPName);
  return strltrim(dpSubStr(DPName, DPSUB_DP_EL), dpSubStr(DPName, DPSUB_DP));
}

/////////////////////////////////////////////////////////////////////
//
// Function dpGetElementValueInt(DP, ViewType, subElem, &value) : ???
//
// Returns the value of an SubElement
//
/////////////////////////////////////////////////////////////////////
int dpGetElementValueInt(string DPName, string ViewType, string SubElement, int &Value)
{
  LOG_DEBUG("dpGetElementValueInt: ", DPName, ViewType, SubElement, Value);
  return dpGet("__navigator_" +
               dpTypeName(DPName)  +
               "_" +
               ViewType +
               dpGetElementName(DPName) +
               "." +SubElement, Value);
}


/////////////////////////////////////////////////////////////////////
//
// Function dpSetElementValueInt(DP, ViewType, subElem, value)
//
// Sets the value of an SubElement
//
/////////////////////////////////////////////////////////////////////
int dpSetElementValueInt(string DPName, string ViewType, string SubElement, int Value)
{
  LOG_DEBUG("dpSetElementValueInt: ", DPName, ViewType, SubElement, Value);
  return dpSet("__navigator_" +
               dpTypeName(DPName)  +
               "_" +
               ViewType +
               dpGetElementName(DPName) +
               "." +SubElement, Value);
}


/////////////////////////////////////////////////////////////////////
//
// Function dpGetElementValueString(DP, ViewTypw, subElem, &value)
//
// Returns the value of an SubElement
//
/////////////////////////////////////////////////////////////////////
int dpGetElementValueString(string DPName, string ViewType, string SubElement, string &Value)
{
  LOG_DEBUG("dpGetElementValueString: ", DPName, ViewType, SubElement, Value);
  return dpGet("__navigator_" +
               dpTypeName(DPName)  +
               "_" +
               ViewType +
               dpGetElementName(DPName) +
               "." +SubElement, Value);
}

/////////////////////////////////////////////////////////////////////
//
// Function dpGetElementValueFloat(DP, ViewType, subElem, &value)
//
// Returns the value of an SubElement
//
/////////////////////////////////////////////////////////////////////
int dpGetElementValueFloat(string DPName, string ViewType, string SubElement, float &Value)
{
  LOG_DEBUG("dpGetElementValueFloat: ", DPName, ViewType, SubElement, Value);
  return dpGet("__navigator_" +
               dpTypeName(DPName)  +
               "_" +
               ViewType +
               dpGetElementName(DPName) +
               "." +SubElement, Value);
}

/////////////////////////////////////////////////////////////////////
//
// Function getPathComponent
//
// get part at place <index> from a datapointpath <aDP>
//
/////////////////////////////////////////////////////////////////////
string getPathComponent(string aDP,int index) {
  	string result="";
  	LOG_DEBUG("datapoint: ",aDP);
  	LOG_DEBUG("index: " , index);
  	dyn_string dpElements = splitDatapointPath(aDP);
  	if(dynlen(dpElements) >= index) {
    	result = dpElements[3];
  	}
  	return result;
}
