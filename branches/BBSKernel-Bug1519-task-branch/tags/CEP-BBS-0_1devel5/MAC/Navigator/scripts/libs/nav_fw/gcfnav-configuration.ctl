//# gcfnav-configuration.ctl
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
//# configuration storage functions for the Navigator.
//#

#uses "nav_fw/gcf-logging.ctl"
#uses "nav_fw/gcfnav-view.ctl"
#uses "nav_fw/gcfnavigator.ctl"

global string   DPNAME_NAVIGATOR                = "__navigator";
global string   ELNAME_RESOURCEROOTS            = "resourceRoots";

global string   ELNAME_ENVIRONMENTNAMES         = "environmentNames";
global string   ELNAME_ENVIRONMENTGROUPS        = "environmentGroups";
global string   ELNAME_SELECTEDELEMENT          = "selectedElement";
global string   ELNAME_SELECTEDVIEWCAPTION      = "selectedViewCaption";
global string   ELNAME_USECOUNT                 = "useCount";
global string   ELNAME_VIEWSPATH                = "viewsPath";
global string   ELNAME_VIEWS                    = "views";
global string   ELNAME_CAPTION                  = "caption";
global string   ELNAME_FILENAME                 = "filename";
global string   ELNAME_CONFIGPANEL              = "configPanel";
global string   ELNAME_SELECTEDVIEW             = "selectedView";
global string   ELNAME_SELECTEDSUBVIEW          = "selectedSubView";
global string   ELNAME_SELECTEDENVIRONMENT      = "selectedEnvironment";
global string   ELNAME_NROFSUBVIEWS             = "nrOfSubViews";
global string   ELNAME_SUBVIEWS                 = "subViews";
global string   ELNAME_CONFIGS                  = "configs";
global string   ELNAME_TRIGGERUPDATE            = "triggerUpdate";
global string   ELNAME_NEWDATAPOINT             = "newDatapoint";
global string   ELNAME_MESSAGE                  = "message";
global string   DPTYPENAME_NAVIGATOR_INSTANCE   = "GCFNavigatorInstance";
global int      g_navigatorID = 0;

//==================== path related functions ====================================

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigInitPathNames   
//
// Initiliases the global pathNames, according top the operating system;
//
// Output: 1. g_path_temp is set
//         1. g_path_gnuplot is set
//
///////////////////////////////////////////////////////////////////////////
void navConfigInitPathNames()
{
  LOG_DEBUG("navConfigInitPathNames");
  dyn_string pathNames;
  if (_WIN32) {
    dpGet("__navigator.pathNamesWindows", pathNames);
    if ("" != pathNames[g_path_temp_index])
      g_path_temp    = pathNames[g_path_temp_index];
    else
      g_path_temp    = "c:/temp";

    g_path_gnuplot = pathNames[g_path_gnuplot_index];
    g_path_pictureconverter = pathNames[g_path_pictureconverter_index];
  }
  else {
    dpGet("__navigator.pathNamesLinux", pathNames);
    if ("" != pathNames[g_path_temp_index])
      g_path_temp    = pathNames[g_path_temp_index];
    else
      g_path_temp    = "/tmp";
      
    g_path_gnuplot = pathNames[g_path_gnuplot_index];
    g_path_pictureconverter = pathNames[g_path_pictureconverter_index];
  }
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetPathName (pathname) : pathname
//
// Converts a given pathName, according to the operating system,
// so it be used by a PVSS system command.
//            
// Input: 1. user configured pathName
// Output: 1. pathName with replaced slashes and backslashes
//
///////////////////////////////////////////////////////////////////////////
string navConfigGetPathName(string pathName)
{
  LOG_DEBUG("navConfigGetPathName: ",pathName);
  string output;
  if (_WIN32) { //windows
    strreplace(pathName, "/", "\\");
    output = pathName;
  }
  else {    //It must be Linux
    strreplace(pathName, "/", "//");
  }
  return output;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetSlashes(): pathSeparator
//
// Returns the path separator of the OS.
//
////////////////////////////////////////////////////////////////////////////////
string navConfigGetSlashes()
{
  string output;
  if (_WIN32) { //windows
    output = "\\";
  }
  else {
    output = "//";
  }
  return output;
}

//========================== Navigator ID and useCount ===========================

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetNavigatorID() : ID
//  
// returns the navigator ID
//
///////////////////////////////////////////////////////////////////////////
int navConfigGetNavigatorID()
{
  return g_navigatorID;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigSetNavigatorID(ID)
//  
// sets the navigator ID
//
///////////////////////////////////////////////////////////////////////////
void navConfigSetNavigatorID(int newID)
{
  LOG_DEBUG("navConfigSetNavigatorID: ",newID);

  if (newID > 256) { // forcing an ID?
    g_navigatorID = newID;
  }
  else {	// no, take my manager number as navigatorID
    g_navigatorID = myManNum();
  }

  // if there is no DP yet available, create one.
  if (!dpAccessable(DPNAME_NAVIGATOR + g_navigatorID)) {
    LOG_DEBUG("Creating new navigator configuration");
    dpCreate(DPNAME_NAVIGATOR + g_navigatorID, DPTYPENAME_NAVIGATOR_INSTANCE);
  }

  // set usecount to 0
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_USECOUNT, 0);

  LOG_DEBUG("Using Navigator ID:", g_navigatorID);
}
  
///////////////////////////////////////////////////////////////////////////
//
// Function navConfigIncreaseUseCount ()
//  
// increases the usecount of the navigator
// GCFNavigatorInstance : __navigator<x>.useCount
//
///////////////////////////////////////////////////////////////////////////
void navConfigIncreaseUseCount()
{
  LOG_DEBUG("navConfigIncreaseUseCount");
  // increase usecount
  int usecount = 0;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_USECOUNT, usecount);
  usecount++;
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_USECOUNT, usecount);
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigDecreaseUseCount ()
//  
// decreases the usecount of the navigator
//
///////////////////////////////////////////////////////////////////////////
void navConfigDecreaseUseCount()
{
  LOG_DEBUG("navConfigDecreaseUseCount");
  // lower usecount of this navigator
  int usecount = 0;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_USECOUNT, usecount);
  usecount--;
  if (usecount > 0)
  {
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_USECOUNT, usecount);
  }
  else
  {
    // if usecount == 0, remove datapoint  
    dpDelete(DPNAME_NAVIGATOR + g_navigatorID);
  }
}



//============================= tree administration ==============================

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigSetSelectedElement (DPname)
// 
// Write the selected element of the tree in the configuration
//
///////////////////////////////////////////////////////////////////////////
void navConfigSetSelectedElement(string datapointPath)
{
	LOG_DEBUG("navConfigSetSelectedElement: ",datapointPath);

	// construct configDP name
	string dpSelectedElementContainer = DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDELEMENT;
	string dpSelectedElement = datapointPath;

	if (dpAccessable(dpSelectedElementContainer)) {
		dpSet(dpSelectedElementContainer, dpSelectedElement);
	}
	else {
		LOG_WARN("Configuration element does not exist:", dpSelectedElementContainer);
	}
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetSelectedView() : CaptionOfSelectedTabPane
// 
// Returns the caption of the currently selected view
//
///////////////////////////////////////////////////////////////////////////
string navConfigGetSelectedView()
{
	LOG_DEBUG("navConfigGetSelectedView");

	string selectedViewCaption = "List";
	dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDVIEWCAPTION, 
			selectedViewCaption);

	return selectedViewCaption;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigSetSelectedView (DPname, viewID))
// 
// Sets the caption of the currently selected view
//
///////////////////////////////////////////////////////////////////////////
void navConfigSetSelectedView(string datapoint, int viewid)
{
	LOG_DEBUG("navConfigSetSelectedView: ", datapoint, viewid);

	string     dpViewConfig = navConfigGetViewConfig(datapoint);
	dyn_string views 		= navConfigGetViews(dpViewConfig);

	if (viewid <= dynlen(views)) {
		string caption = navConfigGetViewCaption(views[viewid]);
		dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDVIEWCAPTION, caption);
	}
	dpSet(dpViewConfig + "." + ELNAME_SELECTEDVIEW, viewid);
}


///////////////////////////////////////////////////////////////////////////
//
// Function navConfigSetSelectedSubView(DP, selectedDPView)
// 
// writes the selected subview into the configuration database
//
///////////////////////////////////////////////////////////////////////////
void navConfigSetSelectedSubView(
  string      datapoint,
  int         selectedSubView)
{
  LOG_DEBUG("navConfigSetSelectedSubView: ", datapoint, selectedSubView);
  string dpViewConfig = navConfigGetViewConfig(datapoint);
  dpSet(dpViewConfig + "." + ELNAME_SELECTEDSUBVIEW, selectedSubView);
}

//=========================== get config items ===================================

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetViewsPath() : relative_path
// 
// Returns the relative path where the navigator views are stored
//
///////////////////////////////////////////////////////////////////////////
string navConfigGetViewsPath()
{
	LOG_DEBUG("navConfigGetViewsPath");

	string viewsPath = "nav_fw/";
	dpGet(DPNAME_NAVIGATOR+ "." + ELNAME_VIEWSPATH, viewsPath);

	return viewsPath;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetViews(DPviewConfig) : views
// 
// Returns the views for the specified resource type
//
///////////////////////////////////////////////////////////////////////////
dyn_string navConfigGetViews(string dpViewConfig)
{
	LOG_DEBUG("navConfigGetViews: ", dpViewConfig);

	// get the views references for this resource type
	dyn_string views;
	dpGet(dpViewConfig + "." + ELNAME_VIEWS, views);

	return views;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetViewCaption(DPview) : caption
// 
// Returns the caption of the specified view
//
///////////////////////////////////////////////////////////////////////////
string navConfigGetViewCaption(string dpView)
{
	LOG_DEBUG("navConfigGetViewCaption: ", LOG_DYN(dpView));

	string caption;
	dpGet(dpView + "." + ELNAME_CAPTION, caption);

	return caption;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetViewConfigPanel(DPview) : configPanel
// 
// Returns the config panel of the specified view
//
///////////////////////////////////////////////////////////////////////////
string navConfigGetViewConfigPanel(string dpView)
{
	LOG_DEBUG("navConfigGetViewConfigPanel: ", dpView);

	string configPanel;
	dpGet(dpView + "." + ELNAME_CONFIGPANEL, configPanel);

	return configPanel;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navConfigQueryResourceRoots() : ???
//
// Returns - list of expanded resource root names
//
///////////////////////////////////////////////////////////////////////////
dyn_string navConfigQueryResourceRoots()
{
  LOG_DEBUG("navConfigQueryResourceRoots");
  dyn_string resourceRootsForQuery;
  dyn_string resourceRoots;
  int i;
  
  dpGet(DPNAME_NAVIGATOR + "." + ELNAME_RESOURCEROOTS, resourceRootsForQuery);
  for (i = 1; i<=dynlen(resourceRootsForQuery); i++) {
    dyn_string resourceNames = dpNames(resourceRootsForQuery[i]);
    LOG_DEBUG("resourceRoots query results for " + resourceRootsForQuery[i] + ":",LOG_DYN(resourceNames));
    dynAppend(resourceRoots,resourceNames);
  }
  return resourceRoots;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetResources (parentDP, depth) : dyn ChildNames
//
// parameters: parentDatapoint - get the children of this datapoint
//             depth           - how many levels of children to get
// 
// Returns the names of the resources that are added to the tree
//
///////////////////////////////////////////////////////////////////////////
dyn_string navConfigGetResources(string parentDatapoint, int depth)
{
  LOG_DEBUG("navConfigGetResources: ", parentDatapoint, depth);
  dyn_string resources;
  dyn_string allResources;
  dyn_string resourceRoots;
  dyn_string reference;
  dyn_errClass err;
  int maxDepth;
  bool parentDatapointIsReference;
  checkForReference(parentDatapoint, reference, parentDatapointIsReference);
  
  if (parentDatapoint == "") {
    // An empty parentdatapoint indicates that the navigator is starting up
    // The roots are read from the Navigator configuration
    maxDepth = depth;
    // read the roots from the configuration
    resourceRoots = navConfigQueryResourceRoots();
    err = getLastError();
    if (dynlen(err) > 0) {
      // if nothing specified, take the local LOFAR tree
      resourceRoots = makeDynString("LOFAR");
    }  
  }
  else if(strpos(parentDatapoint, ":") < 0) {
    // if the parent datapoint does not contain the system separator ':' then
    // it must be a root datapoint
    LOG_DEBUG("parent is root",parentDatapoint);
    
    maxDepth = depth;
    // read the roots from the configuration
    resourceRoots = navConfigQueryResourceRoots();
    // now only use the resource roots of the requested parent
    if (dynlen(resourceRoots) > 0) {
      int i=1;
      while(i <= dynlen(resourceRoots)) {
        if(strpos(resourceRoots[i],parentDatapoint) == 0) {
          // yup, this resource must stay
          i++;
        }
        else {
          // nope, we don't need this resource at this moment
          dynRemove(resourceRoots,i);
        }
      }
    }
  }
  else {
    dyn_string dpPathElements = strsplit(parentDatapoint, "_");
    maxDepth = depth + dynlen(dpPathElements);
    resourceRoots = makeDynString(parentDatapoint);
  }

  for (int i = 1; i <= dynlen(resourceRoots); i++) {
    // query the database for all resources under the given root
    dynAppend(allResources, dpNames(resourceRoots[i] + "*"));
  }
  
  /////////////////////////////////////////////////////////////
  LOG_DEBUG("All resources: ", dynlen(allResources));



  /////////////////////////////////////////////////////////////
  // strip everything below the requested level
  // remove duplicates  
  int i = 1;
  
  while (i <= dynlen(allResources)) {
    dyn_string dpPathElements = strsplit(allResources[i], "_");
    string addResource;
    int d = 1;
    while (d <= maxDepth && d <= dynlen(dpPathElements)) {
      if (d > 1)
        addResource += "_";
      addResource += dpPathElements[d];
      d++;
    }
    if (!dynContains(resources, addResource)) {
      LOG_DEBUG("Adding: ", addResource);
      dynAppend(resources, addResource);
    }
    i++;
  }
  if (parentDatapointIsReference) {
    checkForReferenceReplaceOriginal(resources, reference);
  }
  return resources;
}


///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetEnvironment(envName, userName) : envCode
// 
// Input: 1. environmentName:  "" = current environment,
//                            !"" = given environment
//        2. userName, in combination with environment Personal:
//                      "" = current user(name)
//                     !"" = given user(name)
// Returns the view config datapoint corresponding to the datapointpath
// E.g. U0001
//
///////////////////////////////////////////////////////////////////////////
string navConfigGetEnvironment(string environmentName, string userName)
{
  LOG_DEBUG("navConfigGetEnvironment: ", environmentName, userName);
  string environment;
  string environmentOutput;
  string environmentType;
  string environmentNumber;
  dyn_string environmentNames;
  dpGet(DPNAME_NAVIGATOR + "." + ELNAME_ENVIRONMENTNAMES, environmentNames);
  
  if (environmentName == "") { // work with the current selected environment
    string navInstanceEnvironmentName;
    dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDENVIRONMENT, navInstanceEnvironmentName);
    environment = navInstanceEnvironmentName;
  }
  else {                    // work with a given environment
    environment = environmentName;
  }
    
  if (environment == "Personal") {
    environmentType = "U";
    if (userName != "")  { // is userName is given, use it.
      environmentNumber = getUserId(userName);
    }
    else {
      environmentNumber = getUserId();
    }
  }
  else {
    environmentType = "E" ;
    environmentNumber = dynContains(environmentNames, environment);
  }
  environmentOutput = environmentType + strexpand("\\fill{0}", 4-strlen(environmentNumber)) + environmentNumber;
  
  return environmentOutput;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetViewConfig(DP) : nameOf_viewconfig_DP
// 
// Returns the view config datapoint corresponding to the datapointpath
//
///////////////////////////////////////////////////////////////////////////
string navConfigGetViewConfig(string datapointPath)
{
  LOG_DEBUG("navConfigGetViewConfig: ", datapointPath);
  string datapointType;
  string dpViewConfig = "";
  string dpNameTemp = datapointPath;
  bool isReference;
  dyn_string reference;
  
  checkForReference(dpNameTemp, reference, isReference);

  if (dpExists(dpNameTemp)) { // Explicit use op dpExist!!!
    datapointType = dpTypeName(datapointPath);
    dpViewConfig = "__nav" + navConfigGetEnvironment("", "") + "_" + datapointType + "_viewconfig";
  }
  else {
    // a system root node is selected
    // find __nav<environment>_<systemname>_viewconfig datapoint
    dpViewConfig = "__nav" + navConfigGetEnvironment("", "") + "_" + datapointPath + "_viewconfig";
  }

  if (!dpAccessable(dpViewConfig)) {
    LOG_INFO("navConfigGetViewConfig", "DP does not exist, using default configuration", dpViewConfig);
    dpViewConfig = "__nav_default_viewconfig";
  }
  LOG_TRACE(dpViewConfig);

  return dpViewConfig;
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetViewConfigElements(...)
// 
// returns the view config elements corresponding to the viewconfig
//
///////////////////////////////////////////////////////////////////////////
bool navConfigGetViewConfigElements(
  string      dpViewConfig,
  int         &selectedView,
  int         &selectedSubView,
  dyn_string  &views,
  dyn_int     &nrOfSubViews,
  dyn_string  &subViews,
  dyn_string  &configs)
{
  LOG_DEBUG("navConfigGetViewConfigElements: ", dpViewConfig, selectedView, selectedSubView, views, nrOfSubViews, subViews, configs);
  bool success = true;
  dyn_errClass err;

  dpGet(dpViewConfig + "." + ELNAME_SELECTEDVIEW, selectedView,
        dpViewConfig + "." + ELNAME_SELECTEDSUBVIEW, selectedSubView,
        dpViewConfig + "." + ELNAME_VIEWS, views,
        dpViewConfig + "." + ELNAME_NROFSUBVIEWS, nrOfSubViews,
        dpViewConfig + "." + ELNAME_SUBVIEWS, subViews,
        dpViewConfig + "." + ELNAME_CONFIGS, configs);
  
  err = getLastError(); //test whether no errors
  if (dynlen(err) > 0) {
    throwError(err); // write errors to stderr
    success = false;
  }
  return success;
}
///////////////////////////////////////////////////////////////////////////
//
// Function navConfigGetSubViewConfigElements(...)
// 
// returns the subview config elements corresponding to the viewconfig
//
///////////////////////////////////////////////////////////////////////////
bool navConfigGetSubViewConfigElements(
  string dpSubViewConfig,
  string &subViewCaption,
  string &subViewFileName)
{
  LOG_DEBUG("navConfigGetSubViewConfigElements: ", dpSubViewConfig, subViewCaption, subViewFileName);
  bool success = true;
  dyn_errClass err;

  dpGet(dpSubViewConfig + "." + ELNAME_CAPTION, subViewCaption,
        dpSubViewConfig + "." + ELNAME_FILENAME, subViewFileName);
  
  err = getLastError(); //test whether no errors
  if (dynlen(err) > 0) {
    throwError(err); // write errors to stderr
    success = false;
  }
  return success;
}

//========================== miscellaneous user functions ========================

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigSanityCheck(&message) : bool
// 
// returns true if the View configuration items are correct
// returns false and a message otherwise
//
///////////////////////////////////////////////////////////////////////////
bool navConfigSanityCheck(string &message)
{
  LOG_DEBUG("navConfigSanityCheck: ", message);
  bool sane = true;
  dyn_dyn_anytype tab;
  int i;
  dpQuery("SELECT '_original.._value' FROM '__nav_*' WHERE _DPT=\"GCFNavViewConfiguration\" ", tab);
  
  int viewsIndex    = 4;
  
  while(viewsIndex <= dynlen(tab) && sane) {
    int subViewsIndex     = viewsIndex + 1;
    int configsIndex      = viewsIndex + 2;
    int nrOfSubviewsIndex = viewsIndex + 3;
    
    // check number of items in subviews, configs and nrOfSubViews
    int nrOfViews = dynlen(tab[viewsIndex][2]);
    sane = (nrOfViews == dynlen(tab[nrOfSubviewsIndex][2]));
    //DebugTN("check nrOfViews:", sane, nrOfViews, dynlen(tab[nrOfSubviewsIndex][2]));
    if (!sane) {
      message = "Invalid view configuration\n#views and nrOfSubViews do not correspond\n(" + tab[viewsIndex][1] + ")";
      LOG_WARN(message);
    }
    
    if (sane) {
      int nrOfSubViews = 0;
      for (i = 1; i <= dynlen(tab[nrOfSubviewsIndex][2]); i++)
        nrOfSubViews += tab[nrOfSubviewsIndex][2][i];
      sane = (nrOfSubViews == dynlen(tab[subViewsIndex][2]) && nrOfSubViews == dynlen(tab[configsIndex][2]));
      //DebugTN("check nrOfSubViews:", sane, nrOfSubViews, dynlen(tab[subViewsIndex][2]), dynlen(tab[configsIndex][2]));
      if (!sane) {
        message = "Invalid view configuration\n#subviews, #configs and total nrOfSubViews do not correspond\n(" + tab[viewsIndex][1] + ")";
        LOG_WARN(message);
      }
    }
    
    if (sane) {
      for (i = 1; i <= dynlen(tab[viewsIndex][2]) && sane; i++) {
        sane = dpAccessable(tab[viewsIndex][2][i]);
      }
      if (!sane) {
        message = "Invalid view configuration\nview item (" + (i-1) + ") does not exist\n(" + tab[viewsIndex][1] + ")";
        LOG_WARN(message);
      }
    }
    
    if (sane) {
      for (i = 1; i <= dynlen(tab[subViewsIndex][2]) && sane; i++) {
        sane = dpAccessable(tab[subViewsIndex][2][i]);
      }
      if (!sane) {
        message = "Invalid view configuration\nsubview item (" + (i-1) + ") does not exist\n(" + tab[viewsIndex][1] + ")";
        LOG_WARN(message);
      }
    }
    
    if (sane) {
      for (i = 1; i <= dynlen(tab[configsIndex][2]) && sane; i++) {
        sane = dpAccessable(tab[configsIndex][2][i]);
      }
      if (!sane) {
        message = "Invalid view configuration\nconfig item (" + (i-1) + ") does not exist\n(" + tab[viewsIndex][1] + ")";
        LOG_WARN(message);
      }
    }
    
    viewsIndex += 6;
  }
  
  return sane;
}

//======================== trigger and update navigator ==========================

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigSubscribeUpdateTrigger(callbackFunction)
// 
// subscribes to the update trigger datapointelement of the current naviagtor instance
//
///////////////////////////////////////////////////////////////////////////
void navConfigSubscribeUpdateTrigger(string callback)
{
  LOG_DEBUG("navConfigSubscribeUpdateTrigger: ", callback);
  dpConnect(callback, false, DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE);
}

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigTriggerNavigatorRefresh()
// 
// Writes a dummy value to the navigator configuration 
// the navigator will refresh its views. This function is used
// to trigger a refresh of the entire navigator from a subview.
//
///////////////////////////////////////////////////////////////////////////
void navConfigTriggerNavigatorRefresh()
{
  LOG_DEBUG("navConfigTriggerNavigatorRefresh");
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE, 0);
}


///////////////////////////////////////////////////////////////////////////
//
// Function navConfigTriggerNavigatorRefreshWithDP(DP)
// 
// 1. writes a new datapoint name to the navigator configuration with must be
// new displayed dp in the tree view. 
// 2.writes a dummy value to the navigator configuration 
// the navigator will refresh its views, for the new selected datapoint
// This function is used to trigger a refresh of the entire navigator from
// a subview.
//
///////////////////////////////////////////////////////////////////////////
void navConfigTriggerNavigatorRefreshWithDP(string newDatapoint)
{
  LOG_DEBUG("navConfigTriggerNavigatorRefreshWithDP: ", newDatapoint);
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_NEWDATAPOINT, newDatapoint);
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE, 0);
}

//========================== environmental routines ==============================

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigfillEnvironmentList(DP, envNames)
// 
// fills a comb  ?????
//
///////////////////////////////////////////////////////////////////////////
void navConfigfillEnvironmentList(string dp1, dyn_string environmentNames)
{
  LOG_DEBUG("navConfigfillEnvironmentList: ", dp1, environmentNames);
  setValue("", "deleteAllItems");
  string selectedEnvironment;
  int itemCount = 0;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDENVIRONMENT, selectedEnvironment);
  
  for (int i = 1; i<= dynlen(environmentNames); i++) {
    if (environmentNames[i] != "")
      setValue("", "appendItem", environmentNames[i]);
    if (environmentNames[i] == selectedEnvironment) {
      getValue("", "itemCount", itemCount);
      setValue("", "selectedPos", itemCount);
    }
  }
  if (itemCount == 0) {
    setValue("", "selectedPos", 1);
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDENVIRONMENT, "Personal");
  }
}

///////////////////////////////////////////////////////////////////////////
//
// Function environmentsAvailableToUser() : dyn environments
// 
// Returns the environments Available To the current User
//
///////////////////////////////////////////////////////////////////////////
dyn_string environmentsAvailableToUser()
{
  LOG_DEBUG("environmentsAvailableToUser");
  dyn_string environmentGroups, environmentNames, currentUserGroupNames;
  dyn_string Users_UserName, Users_GroupIds, Groups_UserName, Groups_UserId;
  dyn_string environmentListAvailableToUser;
  string currentGroupIds, currentUser = getUserName();
  environmentListAvailableToUser[1] = "Personal";
  dpGet("_Users.UserName",  Users_UserName);
  dpGet("_Users.GroupIds",  Users_GroupIds);
  dpGet("_Groups.UserName", Groups_UserName);
  dpGet("_Groups.UserId",  Groups_UserId);
  
  currentGroupIds = Users_GroupIds[dynContains(Users_UserName, currentUser)];
  dyn_string GroupIdsSplit= strsplit(currentGroupIds, ";");
  for (int i = 1; i <= dynlen(GroupIdsSplit); i++) {
   currentUserGroupNames[i] = Groups_UserName[dynContains(Groups_UserId, GroupIdsSplit[i])];
  }

  dpGet(DPNAME_NAVIGATOR + ".environmentGroups", environmentGroups);
  dpGet(DPNAME_NAVIGATOR + ".environmentNames", environmentNames);
 
  for (int i = 1; i <= dynlen(environmentGroups); i++)  { 
	//UG assignment to Environment
    dyn_string environmentGroupSplit= strsplit(environmentGroups[i], "|"); //which UG do we have
    for (int j = 1; j <= dynlen(environmentGroupSplit); j++)  {
      // UG group root,para,quest for env. X
      for (int k = 1; k <= dynlen(currentUserGroupNames); k++) { 
		//user membership in UG guest etc.
        dyn_string currentUserGroupNamesSplit= strsplit(currentUserGroupNames[k], "|"); 
        for (int m = 1; m <= dynlen(currentUserGroupNamesSplit); m++) {
          if (currentUserGroupNamesSplit[m] == environmentGroupSplit[j]) {
            environmentListAvailableToUser[dynlen(environmentListAvailableToUser) + 1] = environmentNames[i];
          }
        }
      }
    }
  }
  return environmentListAvailableToUser;
}  

//////////////////////////////////////////////////////////////////////////////////
//
// Function navConfigCheckResourceRoots
//
// Used to check the current systemname and fill GCFNavigator resourceroots with 
// the present Systemname
// 
///////////////////////////////////////////////////////////////////////////////////
void navConfigCheckResourceRoots() {
  	LOG_DEBUG("navConfigCheckResourceRoots");
  	dyn_string roots;
  	dpGet("__navigator.resourceRoots", roots);
  	string aSystemName = getSystemName();
  	strreplace(aSystemName, ':', "");
  	bool replaced = false;
  
  	for (int i = 1; i <= dynlen(roots); i++) {
  	  	dyn_string aS = strsplit(roots[i], ':');
    	if (aS[1] != aSystemName) {
      		strreplace(roots[i], aS[1], aSystemName);
      		replaced = true;
    	}
  	}
    
  	if (replaced) {
    	dpSet("__navigator.resourceRoots", roots);
  	}
}

//============================= permissions ======================================

///////////////////////////////////////////////////////////////////////////
//
// Function navConfigConfigSubviewPermitted() : bool
//  
// if $configDatapoint exits, and not a DPE is selected in the tree, and a
// personal environment can always be configured, but a system environment
// only when you have the user-right.
//
// returns or the configuration of a subview is permitted, nrof Colums, titel etc.
//
///////////////////////////////////////////////////////////////////////////
bool navConfigConfigSubviewPermitted()
{
  LOG_DEBUG("navConfigConfigSubviewPermitted");
  string selectedEnvironment;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDENVIRONMENT, selectedEnvironment);
  
  if (dpAccessable($configDatapoint) &&
     dpGetElementName($datapoint) == "" && 
     ((selectedEnvironment == "Personal") || (selectedEnvironment != "Personal" && getUserPermission(UR_CONFIGSYSTEMSUBVIEW))))
  {
    return TRUE;
  }
  else {
    LOG_INFO("Subview configuration not permitted for this user");
    return FALSE;
  }
}

//===================== routines for changing the configuration ==================

///////////////////////////////////////////////////////////////////////////
//
// Function arrangeUserGroupMembership()
// 
// fills the selectionboxes: member of and not member of
//
///////////////////////////////////////////////////////////////////////////
void arrangeUserGroupMembership()
{
  LOG_DEBUG("arrangeUserGroupMembership");
  UG_selected.deleteAllItems;
  UG_available.deleteAllItems;
  dyn_string environmentGroups, environmentNames, UserGroups;
  dpGet("_Groups.UserName", UserGroups);
  dpGet(DPNAME_NAVIGATOR + ".environmentGroups", environmentGroups);
  dpGet(DPNAME_NAVIGATOR + ".environmentNames", environmentNames);
  string GroupsForEnvironment = environmentGroups[dynContains(environmentNames, ComboBox_environmentMembership.selectedText)];
  dyn_string GroupsForEnvironmentsplit = strsplit(GroupsForEnvironment, "|");
  
  for (int i = 1; i <= dynlen(GroupsForEnvironmentsplit); i++) {
    int position = dynContains(UserGroups, GroupsForEnvironmentsplit[i]);
    if (position>0) {
      UG_selected.appendItem = GroupsForEnvironmentsplit[i];
      dynRemove(UserGroups, position);
    }
  }
  UG_available.items = UserGroups;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function navConfigAddRemoveSubView()
//
//
//
////////////////////////////////////////////////////////////////////////////////
void navConfigAddRemoveSubView()
{
  LOG_DEBUG("navConfigAddRemoveSubView");
  dyn_errClass err;
  dyn_float dfReturn;  // return fields: [1] = success/failure, [2] = new nr of subviews
  dyn_string dsReturn; // not used
  dfReturn[1] = 0; // 0 = failure, 1 = success
  dfReturn[2] = 0; // new nr of subviews
  string viewsPath = navConfigGetViewsPath();
  string subViewName;
  if (comboCaption.visible == true)  // adding existing subview
    subViewName = comboCaption.selectedText;
  else
    subViewName = textFieldCaption.text;
  
  string cleanedText;  
  for (int i = 0; i<strlen(subViewName); i++) {
    if (! ( (subViewName[i] >= 'a' && subViewName[i] <= 'z') || 
           (subViewName[i] >= 'A' && subViewName[i] <= 'Z') || 
           (subViewName[i] >= '0' && subViewName[i] <= '9') ) )
    {
      cleanedText += "-";
    }
    else {
      cleanedText += subViewName[i];
    }
  }
  subViewName = $viewName + "_" + cleanedText;

  string subViewDpName = "__nav" + navConfigGetEnvironment("", "") + "_subview_" + subViewName;
  string configDpTypeName = "GCFNavSubViewConfig" + $viewName;
  string configDpName     = "__nav" + navConfigGetEnvironment("", "") + "_" + $selectedElementDpType + "_config_" + subViewName;
  string viewConfigDpName = "__nav" + navConfigGetEnvironment("", "") + "_" + $selectedElementDpType + "_viewconfig";
  if ($addView) {
    //########################################################
    if (!dpAccessable(subViewDpName) && !dpAccessable(configDpName)) {
      if (comboCaption.visible == false)  { // adding new subview
        // create new GCFNavSubView instance
        dpCreate(subViewDpName, "GCFNavSubView"); //__nav_subview_Alert_Red-Alert-125
        err = getLastError();
        if (dynlen(err) > 0) {
          errorDialog(err);
          // open dialog box with errors
          throwError(err); // write errors to stderr
        }
        else {
          dpSet(subViewDpName + ".caption", textFieldCaption.text,
                subViewDpName + ".filename", viewsPath + textFieldFileName.text);
        }
      }      
      err = getLastError();
      if (dynlen(err) > 0) {
        errorDialog(err);
        // open dialog box with errors
        throwError(err); // write errors to stderr
      }
      else {
        // create new config datapoint
        LOG_DEBUG("creating DP:", configDpName, configDpTypeName);
        dpCreate(configDpName, configDpTypeName); //__nav_TLcuPic_config_Alert_Red-Alert-125
        err = getLastError();
        if (dynlen(err) > 0) {
          errorDialog(err);
          // open dialog box with errors
          throwError(err); // write errors to stderr
        }
        else {
          dyn_int nrOfSubViews;
          dyn_string subViews, configs;
          if (!dpAccessable(viewConfigDpName)) {
            // create a new datapoint, based on the default config
            int err;
            dpCopy("__nav_default_viewconfig", viewConfigDpName, err);
    
            // copy the contents of the default config
            int i;
            dyn_string allC;
            dyn_anytype para;
            allC = dpNames("__nav_default_viewconfig" + ".**:_original.._value", "GCFNavViewConfiguration");
            dpGet(allC, para);
            for (i = 1; i <= dynlen(allC); i++) {
              strreplace(allC[i], "__nav_default_viewconfig", viewConfigDpName);
            }
            dpSet(allC, para);
          }
          dpGet(viewConfigDpName + ".nrOfSubViews", nrOfSubViews,
                viewConfigDpName + ".subViews", subViews,
                viewConfigDpName + ".configs", configs);
  
          err = getLastError();
          if (dynlen(err) > 0) {
            errorDialog(err);
            // open dialog box with errors
            throwError(err); // write errors to stderr
          }
          else {
            int insertIndex = 1;
            for (int i = 1; i <= $selectedView; i++)
              insertIndex += nrOfSubViews[i];
            dfReturn[2] = nrOfSubViews[$selectedView];
            nrOfSubViews[$selectedView] = nrOfSubViews[$selectedView] + 1;
            dynInsertAt(subViews, subViewDpName, insertIndex);
            dynInsertAt(configs, configDpName, insertIndex);
            dpSet(viewConfigDpName + ".nrOfSubViews", nrOfSubViews,
                  viewConfigDpName + ".subViews", subViews,
                  viewConfigDpName + ".configs", configs);
  
            err = getLastError();
            if (dynlen(err) > 0) {
              errorDialog(err);
              // open dialog box with errors
              throwError(err); // write errors to stderr
            }
            else {
              dfReturn[1] = 1; // 0 = failure, 1 = success
              dfReturn[2] = nrOfSubViews[$selectedView];
            }
          }
        }
      }
    }
    else {
      //If the current subview name already exists, show message on screen
      string message = "Entered caption already exists. \nPlease enter a new one.";
      ChildPanelOnCentralModal(viewsPath + "MessageWarning.pnl", "Warning", makeDynString("$1:" + message));
      return;
    }
    //########################################################
  }
  else {
    // remove subview config for the selected datapoint type and the subview datapoint

    int selectedSubView;
    dyn_int nrOfSubViews;
    dyn_string subViews, configs;
    if (dpAccessable(viewConfigDpName)) {
      dpGet(viewConfigDpName + ".selectedSubView", selectedSubView,
            viewConfigDpName + ".nrOfSubViews", nrOfSubViews,
            viewConfigDpName + ".subViews", subViews,
            viewConfigDpName + ".configs", configs);
      err = getLastError();
      if (dynlen(err) > 0) {
        errorDialog(err);
        // open dialog box with errors
        throwError(err); // write errors to stderr
      }
      else {
        int removeIndex = 0;
        for (int i = 1; i<$selectedView; i++)
          removeIndex += nrOfSubViews[i];
        removeIndex += selectedSubView;

        dfReturn[2] = nrOfSubViews[$selectedView];
        nrOfSubViews[$selectedView] = nrOfSubViews[$selectedView]-1;
        dynRemove(subViews, removeIndex);
        dynRemove(configs, removeIndex);

        dpSet(viewConfigDpName + ".nrOfSubViews", nrOfSubViews,
              viewConfigDpName + ".subViews", subViews,
              viewConfigDpName + ".configs", configs);

        err = getLastError();
        if (dynlen(err) > 0) {
          errorDialog(err);
          // open dialog box with errors
          throwError(err); // write errors to stderr
        }
        else {
          dfReturn[1] = 1; // 0 = failure, 1 = success
          dfReturn[2] = nrOfSubViews[$selectedView];
        }
      }
    }
    
   dpDelete(subViewDpName);
   dpDelete(configDpName); 
  }

  // write return parameters to the database
  dpSet("_Ui_" + myManNum() + ".ReturnValue.Float:_original.._value", dfReturn);
  dpSet("_Ui_" + myManNum() + ".ReturnValue.Text:_original.._value", dsReturn);

  PanelOff();
}



