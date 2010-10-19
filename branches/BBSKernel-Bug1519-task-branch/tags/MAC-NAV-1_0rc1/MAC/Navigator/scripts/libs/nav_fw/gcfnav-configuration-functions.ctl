//# gcfnav-configuration-functions.ctl
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

#uses "nav_fw/gcf-util.ctl"
#uses "nav_fw/gcfnav_view.ctl"

global string   DPNAME_NAVIGATOR                = "__navigator";
global string   ELNAME_RESOURCEROOTS            = "resourceRoots";
global string   ELNAME_IGNOREENABLEDROOTS       = "ignoreEnabledRoots";
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
global string   ELNAME_MESSAGE		              = "message";
global string   DPTYPENAME_NAVIGATOR_INSTANCE   = "GCFNavigatorInstance";
global int      g_navigatorID = 0;


///////////////////////////////////////////////////////////////////////////
// Function: navConfigInitPathNames   
//           Initiliases the global pathNames, according top the operating
//           system;
//
// Input: 1. operating system
//
// Output: 1. g_path_temp is set
//         1. g_path_gnuplot is set
///////////////////////////////////////////////////////////////////////////
void navConfigInitPathNames()
{
  dyn_string pathNames;
  if (_WIN32)
  {
    dpGet("__navigator.pathNamesWindows", pathNames);
    if ("" != pathNames[g_path_temp_index])
      g_path_temp    = pathNames[g_path_temp_index];
    else
      g_path_temp    = "c:/temp";

    g_path_gnuplot = pathNames[g_path_gnuplot_index];
    g_path_pictureconverter = pathNames[g_path_pictureconverter_index];
  }
  else
  {
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
// Function: navConfigGetPathName   
//           converts a given pathName, according to the operating system,
//           so it be used by a PVSS system command.
//            
// Input: 1. user configured pathName
//
// Output: 1. pathName with replaced slashes and backslashes
///////////////////////////////////////////////////////////////////////////
string navConfigGetPathName(string pathName)
{
  string output;
  if (_WIN32) //windows
  {
    strreplace(pathName, "/", "\\");
    output = pathName;
  }
  else      //It must be Linux
  {
    strreplace(pathName, "/", "//");
  }
  return output;
}

string navConfigGetSlashes()
{
  string output;
  if (_WIN32) //windows
  {
    output = "\\";
  }
  else
  {
    output = "//";
  }
  return output;
}



///////////////////////////////////////////////////////////////////////////
//Function navConfigGetNavigatorID
//  
// returns the navigator ID
//
///////////////////////////////////////////////////////////////////////////
int navConfigGetNavigatorID()
{
  return g_navigatorID;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigSetNavigatorID
//  
// sets the navigator ID
//
///////////////////////////////////////////////////////////////////////////
void navConfigSetNavigatorID(int newID)
{
  bool createConfiguration = false;
  if (newID > 256)
  {
    g_navigatorID = newID;
    if (!dpAccessable(DPNAME_NAVIGATOR + g_navigatorID))
    {
      createConfiguration = true;
    }
  }
  else
  {
    g_navigatorID = myManNum();
    if (!dpAccessable(DPNAME_NAVIGATOR + g_navigatorID))
    {
      createConfiguration = true;
    }
  }
  if (createConfiguration)
  {
    dpCreate(DPNAME_NAVIGATOR + g_navigatorID, DPTYPENAME_NAVIGATOR_INSTANCE);
  }
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_USECOUNT, 0);
  LOG_DEBUG("Using Navigator ID:", g_navigatorID);
}
  
///////////////////////////////////////////////////////////////////////////
//Function navConfigIncreaseUseCount
//  
// increases the usecount of the navigator
//
///////////////////////////////////////////////////////////////////////////
void navConfigIncreaseUseCount()
{
  // increase usecount
  int usecount = 0;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_USECOUNT, usecount);
  usecount++;
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_USECOUNT, usecount);
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigDecreaseUseCount
//  
// decreases the usecount of the navigator
//
///////////////////////////////////////////////////////////////////////////
void navConfigDecreaseUseCount()
{
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

///////////////////////////////////////////////////////////////////////////
//Function navConfigCheckEnabled
//  
// returns the true if the datapoint is enabled or in the ignore list
// returns false otherwise
//
///////////////////////////////////////////////////////////////////////////
bool navConfigCheckEnabled(string datapointName)
{
  bool enabled = false;
  if (dpAccessable(datapointName + "__enabled"))
  {
    enabled = true;
    LOG_TRACE("dpName__enabled Exists", datapointName, enabled);
  }
  else
  {
    LOG_TRACE("dpName__enabled NOT Exists", datapointName, enabled);
    // check the ignoreEnabledRoots field
    dyn_string ignoreEnabledDPs;
    dyn_errClass err;
    dpGet(DPNAME_NAVIGATOR + "." + ELNAME_IGNOREENABLEDROOTS, ignoreEnabledDPs);
    // If the datapointName is a reference, use the refernce to check for enabled
    err = getLastError();
    if (dynlen(err) == 0)
    {
      for (int i = 1; i <= dynlen(ignoreEnabledDPs) && !enabled; i++)
      {
        int pos = strpos(datapointName, ignoreEnabledDPs[i]);
        LOG_TRACE("checkEnabled", pos, datapointName, ignoreEnabledDPs[i]);
        if (pos >= 0)
        {
          enabled = true;
        }
      }
    }

  }
  return enabled;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigSetSelectedElement
// 
// write the selected element in the configuration
///////////////////////////////////////////////////////////////////////////
void navConfigSetSelectedElement(string datapointPath)
{
  string dpSelectedElementContainer = DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDELEMENT;
  string dpSelectedElement = datapointPath;
  if (dpAccessable(dpSelectedElementContainer))
  {
    dpSet(dpSelectedElementContainer, dpSelectedElement);
  }
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigGetViewsPath()
// 
// returns the relative path where the navigator views are stored
///////////////////////////////////////////////////////////////////////////
string navConfigGetViewsPath()
{
  string viewsPath = "nav_fw/";
  dpGet(DPNAME_NAVIGATOR+ "." + ELNAME_VIEWSPATH, viewsPath);
  return viewsPath;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigGetSelectedView()
// 
// returns the caption of the currently selected view
///////////////////////////////////////////////////////////////////////////
string navConfigGetSelectedView()
{
  string selectedViewCaption = "List";
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDVIEWCAPTION, selectedViewCaption);
  return selectedViewCaption;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigSetSelectedView()
// 
// sets the caption of the currently selected view
///////////////////////////////////////////////////////////////////////////
void navConfigSetSelectedView(string datapoint, int viewid)
{
  string dpViewConfig = navConfigGetViewConfig(datapoint);
  dyn_string views = navConfigGetViews(dpViewConfig);
  if (viewid <= dynlen(views))
  {
    string caption = navConfigGetViewCaption(views[viewid]);
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDVIEWCAPTION, caption);
  }
  dpSet(dpViewConfig + "." + ELNAME_SELECTEDVIEW, viewid);
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigGetViews()
// 
// returns the views for the specified resource type
///////////////////////////////////////////////////////////////////////////
dyn_string navConfigGetViews(string dpViewConfig)
{
  dyn_string views;
  // get the views references for this resource type
  dpGet(dpViewConfig + "." + ELNAME_VIEWS, views);
  return views;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigGetViewCaption
// 
// returns the caption of the specified view
///////////////////////////////////////////////////////////////////////////
string navConfigGetViewCaption(string dpView)
{
  string caption;
  dpGet(dpView + "." + ELNAME_CAPTION, caption);
  return caption;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigGetViewConfigPanel
// 
// returns the config panel of the specified view
///////////////////////////////////////////////////////////////////////////
string navConfigGetViewConfigPanel(string dpView)
{
  string configPanel;
  dpGet(dpView + "." + ELNAME_CONFIGPANEL, configPanel);
  return configPanel;
}


///////////////////////////////////////////////////////////////////////////
//Function checkForReference
//
// parameters: parentDatapoint - get the children of this datapoint
//             depth           - how many levels of children to get
// 
// returns - original parentDatapoint if it is a reference
//         - bool if it is a reference
//         - dyn_string reference with ref information
///////////////////////////////////////////////////////////////////////////
bool checkForReference(string &parentDatapoint, dyn_string &reference, bool &parentDatapointIsReference)
{
  dyn_string refOut;
  bool stopCheck = FALSE;
  for (int i = 1; (i <= dynlen(g_referenceList) && !stopCheck); i++)
  {
    refOut = strsplit(g_referenceList[i], "=");
    if (dynlen(refOut) >= 1)
    {
      if (strpos(parentDatapoint, refOut[1]) == 0)
      {
        stopCheck = TRUE;
        parentDatapointIsReference = TRUE;
        strreplace(parentDatapoint, refOut[1], refOut[2]);
        reference = refOut;
      }
    }
  }
  return parentDatapointIsReference;
}

///////////////////////////////////////////////////////////////////////////
//Function checkForReferenceReplaceOriginal
//
// parameters: resources  
//             reference  
// 
// returns - reference resources in stead of original resources
///////////////////////////////////////////////////////////////////////////
void checkForReferenceReplaceOriginal(dyn_string &resources, dyn_string reference)
{
  for (int i = 1; i <= dynlen(resources); i++)
  {
    strreplace(resources[i], reference[2], reference[1]);
  }
}


///////////////////////////////////////////////////////////////////////////
//Function navConfigGetResources
//
// parameters: parentDatapoint - get the children of this datapoint
//             depth           - how many levels of children to get
// 
// returns the names of the resources that are added to the tree
///////////////////////////////////////////////////////////////////////////
dyn_string navConfigGetResources(string parentDatapoint, int depth)
{
  dyn_string resources;
  dyn_string allResources;
  dyn_string resourceRoots;
  dyn_string reference;
  dyn_errClass err;
  int maxDepth;
  bool parentDatapointIsReference;
  checkForReference(parentDatapoint, reference, parentDatapointIsReference);
  if (parentDatapoint == "")
  {
    maxDepth = depth;
    // read the roots from the configuration
    dpGet(DPNAME_NAVIGATOR + "." + ELNAME_RESOURCEROOTS, resourceRoots);
    err = getLastError();
    if (dynlen(err) > 0)
    {
      // if nothing specified, take the local PIC and PAC trees
      resourceRoots = makeDynString("PIC", "PAC");
    }  
  }
  else
  {
    dyn_string dpPathElements = strsplit(parentDatapoint, "_");
    maxDepth = depth + dynlen(dpPathElements);
    resourceRoots = makeDynString(parentDatapoint);
  }
  for (int i = 1; i <= dynlen(resourceRoots); i++)
  {
    // query the database for all resources under the given root
    dynAppend(allResources, dpNames(resourceRoots[i] + "*"));
  }
  
  /////////////////////////////////////////////////////////////
  LOG_DEBUG("All resources: ", dynlen(allResources));
  dyn_string newCollection;
  dyn_string checkCollection = allResources;
  dyn_string enabledCollection = dynPatternMatch("*__enabled", allResources);
  int enabledCollectionLength = dynlen(enabledCollection);
  int position;
  for (int j = 1; j<= enabledCollectionLength; j++)
  {
    string enabledCollectionItem = enabledCollection[j];
    strreplace(enabledCollectionItem, "__enabled", "");
    position = dynContains(checkCollection, enabledCollectionItem);
    if (position > 0)
    {
      LOG_DEBUG("Adding: ", checkCollection[position]);
      dynAppend(newCollection, checkCollection[position]);
      dynRemove(checkCollection, position);
    }
    else if (0 == position)
    {
      if (navPMLisTemporary(enabledCollectionItem + "__enabled"))
      {
        LOG_DEBUG("Adding temp: ", enabledCollectionItem);
        dynAppend(newCollection, enabledCollectionItem);
      }
    }
  }
  ///////////////////////////////////////////////////////////////

  if (dynlen(checkCollection) > 0)
  {
    bool enabled = FALSE;
    dyn_string ignoreEnabledDPs;
    dyn_errClass err;
    dpGet(DPNAME_NAVIGATOR + "." + ELNAME_IGNOREENABLEDROOTS, ignoreEnabledDPs);
    
    for (int j = 1; j <= dynlen(checkCollection); j++)
    {
      // check the ignoreEnabledRoots field
      err = getLastError();
      if (dynlen(err) == 0)
      {
        for (int n = 1; n <= dynlen(ignoreEnabledDPs) && !enabled; n++)
        {
          int pos = strpos(checkCollection[j], ignoreEnabledDPs[n]);
          if (pos >= 0)
          {
            dynAppend(newCollection, checkCollection[j]);
            enabled = TRUE;
          }
        }
      }
    }
  }
  ////////////////////////////////////////////////////////////////
  allResources = newCollection;
  /////////////////////////////////////////////////////////////
  // now remove all DP's with double '_' (e.g. __enabled)
  // strip everything below the requested level
  // remove duplicates  
  int i = 1;
  
  while(i <= dynlen(allResources))
  {
    if (strpos(allResources[i], "__") < 0)
    {
      strreplace(allResources[i], "__enabled", "");
      dyn_string dpPathElements = strsplit(allResources[i], "_");
      string addResource;
      int d = 1;
      while(d <= maxDepth && d <= dynlen(dpPathElements))
      {
        if (d > 1)
          addResource += "_";
        addResource += dpPathElements[d];
        d++;
      }
      if (!dynContains(resources, addResource))
      {
        LOG_DEBUG("Adding: ", addResource);
        dynAppend(resources, addResource);
      }
    }
    i++;
  }
  if (parentDatapointIsReference)
  {
    checkForReferenceReplaceOriginal(resources, reference);
  }
  return resources;
}


///////////////////////////////////////////////////////////////////////////
// Functionname: navConfigGetEnvironment
// Function: 
// 
// Input: 1. environmentName:  "" = current environment,
//                            !"" = given environment
//        2. userName, in combination with environment Personal:
//                      "" = current user(name)
//                     !"" = given user(name)
// returns the view config datapoint corresponding to the datapointpath
///////////////////////////////////////////////////////////////////////////
string navConfigGetEnvironment(string environmentName, string userName)
{
  string environment;
  string environmentOutput;
  string environmentType;
  string environmentNumber;
  dyn_string environmentNames;
  dpGet(DPNAME_NAVIGATOR + "." + ELNAME_ENVIRONMENTNAMES, environmentNames);
  
  if (environmentName == "") // work with the current selected environment
  {
    string navInstanceEnvironmentName;
    dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDENVIRONMENT, navInstanceEnvironmentName);
    environment = navInstanceEnvironmentName;
  }
  else                    // work with a given environment
  {
    environment = environmentName;
  }
    
  if (environment == "Personal")
  {
    environmentType = "U";
    if (userName != "") // is userName is given, use it.
    {
      environmentNumber = getUserId(userName);
    }
    else
    {
      environmentNumber = getUserId();
    }
  }
  else
  {
    environmentType = "E" ;
    environmentNumber = dynContains(environmentNames, environment);
  }
  environmentOutput = environmentType + strexpand("\\fill{0}", 4-strlen(environmentNumber)) + environmentNumber;
  
  return environmentOutput;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigfillEnvironmentList
// 
// fills a comb
///////////////////////////////////////////////////////////////////////////
void navConfigfillEnvironmentList(string dp1, dyn_string environmentNames)
{
  setValue("", "deleteAllItems");
  string selectedEnvironment;
  int itemCount = 0;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDENVIRONMENT, selectedEnvironment);
  
  
  for (int i = 1; i<= dynlen(environmentNames); i++)
  {
    if (environmentNames[i] != "")
      setValue("", "appendItem", environmentNames[i]);
    if (environmentNames[i] == selectedEnvironment)
    {
      getValue("", "itemCount", itemCount);
      setValue("", "selectedPos", itemCount);
    }
    
  }
  if (itemCount == 0)
  {
    setValue("", "selectedPos", 1);
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDENVIRONMENT, "Personal");
  }
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigGetViewConfig
// 
// returns the view config datapoint corresponding to the datapointpath
///////////////////////////////////////////////////////////////////////////
string navConfigGetViewConfig(string datapointPath)
{
  string datapointType;
  string dpViewConfig = "";
  string dpNameTemp = datapointPath;
  bool isReference;
  dyn_string reference;
  
  checkForReference(dpNameTemp, reference, isReference);
  if (dpAccessable(dpNameTemp + "__enabled"))
  {
    datapointType = getDpTypeFromEnabled(dpNameTemp + "__enabled.");
    // find __nav_<datapointType>_viewconfig datapoint
    dpViewConfig = "__nav" + navConfigGetEnvironment("", "") + "_" + datapointType + "_viewconfig";
  }
  else if (dpExists(dpNameTemp)) // Explicit use op dpExist!!!
  {
    datapointType = dpTypeName(datapointPath);
    dpViewConfig = "__nav" + navConfigGetEnvironment("", "") + "_" + datapointType + "_viewconfig";
  }
  else
  {
    // a system root node is selected
    // find __nav<environment>_<systemname>_viewconfig datapoint
    dpViewConfig = "__nav" + navConfigGetEnvironment("", "") + "_" + datapointPath + "_viewconfig";
  }
  if (!dpAccessable(dpViewConfig))
  {
    LOG_TRACE("navConfigGetViewConfig", "DP does not exist, using default configuration", dpViewConfig);
    dpViewConfig = "__nav_default_viewconfig";
  }
  LOG_TRACE(dpViewConfig);
  return dpViewConfig;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigGetViewConfigElements
// 
// returns the view config elements corresponding to the viewconfig
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
  bool success = true;
  dyn_errClass err;

  dpGet(dpViewConfig + "." + ELNAME_SELECTEDVIEW, selectedView,
        dpViewConfig + "." + ELNAME_SELECTEDSUBVIEW, selectedSubView,
        dpViewConfig + "." + ELNAME_VIEWS, views,
        dpViewConfig + "." + ELNAME_NROFSUBVIEWS, nrOfSubViews,
        dpViewConfig + "." + ELNAME_SUBVIEWS, subViews,
        dpViewConfig + "." + ELNAME_CONFIGS, configs);
  
  err = getLastError(); //test whether no errors
  if (dynlen(err) > 0)
  {
    throwError(err); // write errors to stderr
    success = false;
  }
  return success;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigSetSelectedSubView
// 
// writes the selected subview into the configuration database
///////////////////////////////////////////////////////////////////////////
void navConfigSetSelectedSubView(
  string      datapoint,
  int         selectedSubView)
{
  string dpViewConfig = navConfigGetViewConfig(datapoint);
  dpSet(dpViewConfig + "." + ELNAME_SELECTEDSUBVIEW, selectedSubView);
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigGetSubViewConfigElements
// 
// returns the subview config elements corresponding to the viewconfig
///////////////////////////////////////////////////////////////////////////
bool navConfigGetSubViewConfigElements(
  string dpSubViewConfig,
  string &subViewCaption,
  string &subViewFileName)
{
  bool success = true;
  dyn_errClass err;

  dpGet(dpSubViewConfig + "." + ELNAME_CAPTION, subViewCaption,
        dpSubViewConfig + "." + ELNAME_FILENAME, subViewFileName);
  
  err = getLastError(); //test whether no errors
  if (dynlen(err) > 0)
  {
    throwError(err); // write errors to stderr
    success = false;
  }
  return success;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigSanityCheck
// 
// returns true if the View configuration items are correct
// returns false and a message otherwise
///////////////////////////////////////////////////////////////////////////
bool navConfigSanityCheck(string &message)
{
  bool sane = true;
  dyn_dyn_anytype tab;
  int i;
  dpQuery("SELECT '_original.._value' FROM '__nav_*' WHERE _DPT=\"GCFNavViewConfiguration\" ", tab);
  
  int viewsIndex    = 4;
  
  while(viewsIndex <= dynlen(tab) && sane)
  {
    int subViewsIndex     = viewsIndex + 1;
    int configsIndex      = viewsIndex + 2;
    int nrOfSubviewsIndex = viewsIndex + 3;
    
    // check number of items in subviews, configs and nrOfSubViews
    int nrOfViews = dynlen(tab[viewsIndex][2]);
    sane = (nrOfViews == dynlen(tab[nrOfSubviewsIndex][2]));
    //DebugTN("check nrOfViews:", sane, nrOfViews, dynlen(tab[nrOfSubviewsIndex][2]));
    if (!sane)
    {
      message = "Invalid view configuration\n#views and nrOfSubViews do not correspond\n(" + tab[viewsIndex][1] + ")";
      DebugTN(message);
    }
    
    if (sane)
    {
      int nrOfSubViews = 0;
      for (i = 1; i <= dynlen(tab[nrOfSubviewsIndex][2]); i++)
        nrOfSubViews += tab[nrOfSubviewsIndex][2][i];
      sane = (nrOfSubViews == dynlen(tab[subViewsIndex][2]) && nrOfSubViews == dynlen(tab[configsIndex][2]));
      //DebugTN("check nrOfSubViews:", sane, nrOfSubViews, dynlen(tab[subViewsIndex][2]), dynlen(tab[configsIndex][2]));
      if (!sane)
      {
        message = "Invalid view configuration\n#subviews, #configs and total nrOfSubViews do not correspond\n(" + tab[viewsIndex][1] + ")";
        DebugTN(message);
      }
    }
    
    if (sane)
    {
      for (i = 1; i <= dynlen(tab[viewsIndex][2]) && sane; i++)
      {
        sane = dpAccessable(tab[viewsIndex][2][i]);
        LOG_TRACE("check view existance:", sane, tab[viewsIndex][2][i]);
      }
      if (!sane)
      {
        message = "Invalid view configuration\nview item (" + (i-1) + ") does not exist\n(" + tab[viewsIndex][1] + ")";
        DebugTN(message);
      }
    }
    
    if (sane)
    {
      for (i = 1; i <= dynlen(tab[subViewsIndex][2]) && sane; i++)
      {
        sane = dpAccessable(tab[subViewsIndex][2][i]);
        DebugTN("check subview existance:", sane, tab[subViewsIndex][2][i]);
      }
      if (!sane)
      {
        message = "Invalid view configuration\nsubview item (" + (i-1) + ") does not exist\n(" + tab[viewsIndex][1] + ")";
        DebugTN(message);
      }
    }
    
    if (sane)
    {
      for (i = 1; i <= dynlen(tab[configsIndex][2]) && sane; i++)
      {
        sane = dpAccessable(tab[configsIndex][2][i]);
        DebugTN("check configs existance:", sane, tab[configsIndex][2][i]);
      }
      if (!sane)
      {
        message = "Invalid view configuration\nconfig item (" + (i-1) + ") does not exist\n(" + tab[viewsIndex][1] + ")";
        DebugTN(message);
      }
    }
    
    viewsIndex += 6;
  }
  
  return sane;
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigSubscribeUpdateTrigger
// 
// subscribes to the update trigger datapointelement of the current naviagtor instance
///////////////////////////////////////////////////////////////////////////
void navConfigSubscribeUpdateTrigger(string callback)
{
  dpConnect(callback, false, DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE);
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigTriggerNavigatorRefresh
// 
// writes a dummy value to the navigator configuration 
// the navigator will refresh its views. This function is used
// to trigger a refresh of the entire navigator from a subview.
///////////////////////////////////////////////////////////////////////////
void navConfigTriggerNavigatorRefresh()
{
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE, 0);
}


///////////////////////////////////////////////////////////////////////////
//Function navConfigTriggerNavigatorRefreshWithDP
// 
// 1. writes a new datapoint name to the navigator configuration with must be
// new displayed dp in the tree view. 
// 2.writes a dummy value to the navigator configuration 
// the navigator will refresh its views, for the new selected datapoint
// This function is used to trigger a refresh of the entire navigator from
// a subview.
///////////////////////////////////////////////////////////////////////////
void navConfigTriggerNavigatorRefreshWithDP(string newDatapoint)
{
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_NEWDATAPOINT, newDatapoint);
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE, 0);
}


///////////////////////////////////////////////////////////////////////////
//Function environmentsAvailableToUser
// 
// returns the environments Available To the current User
///////////////////////////////////////////////////////////////////////////
dyn_string environmentsAvailableToUser()
{
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
  for (int i = 1; i <= dynlen(GroupIdsSplit); i++)
  {
   currentUserGroupNames[i] = Groups_UserName[dynContains(Groups_UserId, GroupIdsSplit[i])];
  }

  dpGet(DPNAME_NAVIGATOR + ".environmentGroups", environmentGroups);
  dpGet(DPNAME_NAVIGATOR + ".environmentNames", environmentNames);
 
  for (int i = 1; i <= dynlen(environmentGroups); i++) //UG assignment to Environment
  {
    dyn_string environmentGroupSplit= strsplit(environmentGroups[i], "|"); //which UG do we have
    for (int j = 1; j <= dynlen(environmentGroupSplit); j++) // UG group root,para,quest for env. X
    {
      for (int k = 1; k <= dynlen(currentUserGroupNames); k++) //user membership in UG guest etc.
      {
        dyn_string currentUserGroupNamesSplit= strsplit(currentUserGroupNames[k], "|"); 
        for (int m = 1; m <= dynlen(currentUserGroupNamesSplit); m++)
        {
          if (currentUserGroupNamesSplit[m] == environmentGroupSplit[j])
          {
            environmentListAvailableToUser[dynlen(environmentListAvailableToUser) + 1] = environmentNames[i];
          }
        }
      }
    }
  }
  return environmentListAvailableToUser;
}  

///////////////////////////////////////////////////////////////////////////
// Function navConfigConfigSubviewPermitted
//  
// if $configDatapoint exits, and not a DPE is selected in the tree, and a
// personal environment can always be configured, but a system environment
// only when you have the user-right.
//
// returns or the configuration of a subview is permitted, nrof Colums, titel etc.
///////////////////////////////////////////////////////////////////////////
bool navConfigConfigSubviewPermitted()
{
  string selectedEnvironment;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDENVIRONMENT, selectedEnvironment);
  
  if (dpAccessable($configDatapoint) &&
     dpGetElementName($datapoint) == "" && 
     ((selectedEnvironment == "Personal") || (selectedEnvironment != "Personal" && getUserPermission(UR_CONFIGSYSTEMSUBVIEW))))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//////////////////////////////////////////////////////////////////////////////////
// FunctionName: navConfigGetElementsFromDp
//
// Fills the dpe selectionlist for a datapoint selection
///////////////////////////////////////////////////////////////////////////////////
dyn_string navConfigGetElementsFromDp(string datapoint)
{
  string selectedDP;
  dyn_dyn_string elementNames;
  dyn_dyn_int elementTypes;
  dyn_string output;
  int elementIndex;

  dpTypeGet(getDpTypeFromEnabled(datapoint + "__enabled."), elementNames, elementTypes);
  for (elementIndex = 2; elementIndex <= dynlen(elementNames); elementIndex++) 
  {
    int elementLevel = dynlen(elementNames[elementIndex])-1; // how deep is the element?
    string elementName = elementNames[elementIndex][elementLevel + 1];
    output[dynlen(output) + 1] = elementName;
  }
  dynSortAsc(output);
  return output;
}



///////////////////////////////////////////////////////////////////////////
//Function arrangeUserGroupMembership
// 
// fills the selectionboxes: member of and not member of
///////////////////////////////////////////////////////////////////////////
void arrangeUserGroupMembership()
{
  UG_selected.deleteAllItems;
  UG_available.deleteAllItems;
  dyn_string environmentGroups, environmentNames, UserGroups;
  dpGet("_Groups.UserName", UserGroups);
  dpGet(DPNAME_NAVIGATOR + ".environmentGroups", environmentGroups);
  dpGet(DPNAME_NAVIGATOR + ".environmentNames", environmentNames);
  string GroupsForEnvironment = environmentGroups[dynContains(environmentNames, ComboBox_environmentMembership.selectedText)];
  dyn_string GroupsForEnvironmentsplit = strsplit(GroupsForEnvironment, "|");
  
  for (int i = 1; i <= dynlen(GroupsForEnvironmentsplit); i++)
  {
    int position = dynContains(UserGroups, GroupsForEnvironmentsplit[i]);
    if (position>0)
    {
      UG_selected.appendItem = GroupsForEnvironmentsplit[i];
      dynRemove(UserGroups, position);
    }
  }
  UG_available.items = UserGroups;
}

navConfigAddRemoveSubView()
{
  dyn_errClass err;
  dyn_float dfReturn;  // return fields: [1] = success/failure, [2] = new nr of subviews
  dyn_string dsReturn; // not used
  dfReturn[1] = 0; // 0 = failure, 1 = success
  dfReturn[2] = 0; // new nr of subviews
  string viewsPath = navConfigGetViewsPath();
  string subViewName;
  if (comboCaption.visible == true) // adding existing subview
    subViewName = comboCaption.selectedText;
  else
    subViewName = textFieldCaption.text;
  
  string cleanedText;  
  for (int i = 0; i<strlen(subViewName); i++)
  {
    if (! ( (subViewName[i] >= 'a' && subViewName[i] <= 'z') || 
           (subViewName[i] >= 'A' && subViewName[i] <= 'Z') || 
           (subViewName[i] >= '0' && subViewName[i] <= '9') ) )
    {
      cleanedText += "-";
    }
    else
    {
      cleanedText += subViewName[i];
    }
  }
  subViewName = $viewName + "_" + cleanedText;

  string subViewDpName = "__nav" + navConfigGetEnvironment("", "") + "_subview_" + subViewName;
  string configDpTypeName = "GCFNavSubViewConfig" + $viewName;
  string configDpName     = "__nav" + navConfigGetEnvironment("", "") + "_" + $selectedElementDpType + "_config_" + subViewName;
  string viewConfigDpName = "__nav" + navConfigGetEnvironment("", "") + "_" + $selectedElementDpType + "_viewconfig";
  if ($addView)
  {
    //########################################################
    if (!dpAccessable(subViewDpName)&& !dpAccessable(configDpName))
    {
      if (comboCaption.visible == false) // adding new subview
      {
        // create new GCFNavSubView instance
        dpCreate(subViewDpName, "GCFNavSubView"); //__nav_subview_Alert_Red-Alert-125
        err = getLastError();
        if (dynlen(err) > 0)
        {
          errorDialog(err);
          // open dialog box with errors
          throwError(err); // write errors to stderr
        }
        else
        {
          dpSet(subViewDpName + ".caption", textFieldCaption.text,
                subViewDpName + ".filename", viewsPath + textFieldFileName.text);
        }
      }      
      err = getLastError();
      if (dynlen(err) > 0)
      {
        errorDialog(err);
        // open dialog box with errors
        throwError(err); // write errors to stderr
      }
      else
      {
        // create new config datapoint
        LOG_DEBUG("creating DP:", configDpName, configDpTypeName);
        dpCreate(configDpName, configDpTypeName); //__nav_TLcuPic_config_Alert_Red-Alert-125
        err = getLastError();
        if (dynlen(err) > 0)
        {
          errorDialog(err);
          // open dialog box with errors
          throwError(err); // write errors to stderr
        }
        else
        {
          dyn_int nrOfSubViews;
          dyn_string subViews, configs;
          if (!dpAccessable(viewConfigDpName))
          {
            // create a new datapoint, based on the default config
            int err;
            dpCopy("__nav_default_viewconfig", viewConfigDpName, err);
    
            // copy the contents of the default config
            int i;
            dyn_string allC;
            dyn_anytype para;
            allC = dpNames("__nav_default_viewconfig" + ".**:_original.._value", "GCFNavViewConfiguration");
            dpGet(allC, para);
            for (i = 1; i <= dynlen(allC); i++)
            {
              strreplace(allC[i], "__nav_default_viewconfig", viewConfigDpName);
            }
            dpSet(allC, para);
          }
          dpGet(viewConfigDpName + ".nrOfSubViews", nrOfSubViews,
                viewConfigDpName + ".subViews", subViews,
                viewConfigDpName + ".configs", configs);
  
          err = getLastError();
          if (dynlen(err) > 0)
          {
            errorDialog(err);
            // open dialog box with errors
            throwError(err); // write errors to stderr
          }
          else
          {
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
            if (dynlen(err) > 0)
            {
              errorDialog(err);
              // open dialog box with errors
              throwError(err); // write errors to stderr
            }
            else
            {
              dfReturn[1] = 1; // 0 = failure, 1 = success
              dfReturn[2] = nrOfSubViews[$selectedView];
            }
          }
        }
      }
    }
    else
    {
      //If the current subview name already exists, show message on screen
      string message = "Entered caption already exists. \nPlease enter a new one.";
      ChildPanelOnCentralModal(viewsPath + "MessageWarning.pnl", "Warning", makeDynString("$1:" + message));
      return;
    }
    //########################################################
  }
  else
  {
    // remove subview config for the selected datapoint type and the subview datapoint

    int selectedSubView;
    dyn_int nrOfSubViews;
    dyn_string subViews, configs;
    if (dpAccessable(viewConfigDpName))
    {
      dpGet(viewConfigDpName + ".selectedSubView", selectedSubView,
            viewConfigDpName + ".nrOfSubViews", nrOfSubViews,
            viewConfigDpName + ".subViews", subViews,
            viewConfigDpName + ".configs", configs);
      err = getLastError();
      if (dynlen(err) > 0)
      {
        errorDialog(err);
        // open dialog box with errors
        throwError(err); // write errors to stderr
      }
      else
      {
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
        if (dynlen(err) > 0)
        {
          errorDialog(err);
          // open dialog box with errors
          throwError(err); // write errors to stderr
        }
        else
        {
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
