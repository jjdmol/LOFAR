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

#uses "gcf-util.ctl"

global string   DPNAME_NAVIGATOR                = "__navigator";
global string   ELNAME_RESOURCEROOTS            = "resourceRoots";
global string   ELNAME_IGNOREENABLEDROOTS       = "ignoreEnabledRoots";
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
global string   ELNAME_NROFSUBVIEWS             = "nrOfSubViews";
global string   ELNAME_SUBVIEWS                 = "subViews";
global string   ELNAME_CONFIGS                  = "configs";
global string   ELNAME_TRIGGERUPDATE            = "triggerUpdate";
global string   ELNAME_NEWDATAPOINT             = "newDatapoint";
global string   DPTYPENAME_NAVIGATOR_INSTANCE   = "GCFNavigatorInstance";
global int      g_navigatorID = 0;

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
  if(newID != 0)
  {
    g_navigatorID = newID;
    if(!dpExists(DPNAME_NAVIGATOR + g_navigatorID))
    {
      createConfiguration = true;
    }
  }
  else
  {
    int id=1;
    while(id<20 && !createConfiguration) // if no free config found, use config #20
    {
      if(!dpExists(DPNAME_NAVIGATOR + id))
      {
        g_navigatorID = id;
        createConfiguration=true;
      }
      else
      {
        id++;
      }
    }
  }
  if(createConfiguration)
  {
    dpCreate(DPNAME_NAVIGATOR + g_navigatorID, DPTYPENAME_NAVIGATOR_INSTANCE);
  }
  else if(!createConfiguration) // if no free config found, use config #20
  {
    g_navigatorID = 20;
    dpCreate(DPNAME_NAVIGATOR + g_navigatorID, DPTYPENAME_NAVIGATOR_INSTANCE);
  }
  LOG_DEBUG("Using Navigator ID:",g_navigatorID);
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
  int usecount=0;
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
  int usecount=0;
  dpGet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_USECOUNT, usecount);
  usecount--;
  if(usecount > 0)
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
  if(dpExists(datapointName + "__enabled"))
  {
    enabled = true;
  }
  else
  {
    // check the ignoreEnabledRoots field
    dyn_string ignoreEnabledDPs;
    dyn_errClass err;
    dpGet(DPNAME_NAVIGATOR + "." + ELNAME_IGNOREENABLEDROOTS,ignoreEnabledDPs);
    err = getLastError();
    if(dynlen(err) == 0)
    {
      for(int i=1;i<=dynlen(ignoreEnabledDPs) && !enabled;i++)
      {
        int pos = strpos(datapointName,ignoreEnabledDPs[i]);
        LOG_TRACE("checkEnabled",pos,datapointName,ignoreEnabledDPs[i]);
        if(pos >= 0)
        {
          enabled=true;
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
  if(dpExists(dpSelectedElementContainer))
  {
    dpSet(dpSelectedElementContainer,dpSelectedElement);
  }
}

///////////////////////////////////////////////////////////////////////////
//Function navConfigGetViewsPath()
// 
// returns the relative path where the navigator views are stored
///////////////////////////////////////////////////////////////////////////
string navConfigGetViewsPath()
{
  string viewsPath = "navigator/views/";
  dpGet(DPNAME_NAVIGATOR+ "." + ELNAME_VIEWSPATH,viewsPath);
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
  if(viewid <= dynlen(views))
  {
    string caption = navConfigGetViewCaption(views[viewid]);
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_SELECTEDVIEWCAPTION, caption);
  }
  dpSet(dpViewConfig + "." + ELNAME_SELECTEDVIEW,viewid);
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
  dyn_errClass err;
  int maxDepth;
  
  if(parentDatapoint == "")
  {
    maxDepth = depth;
    // read the roots from the configuration
    dpGet(DPNAME_NAVIGATOR+"." + ELNAME_RESOURCEROOTS,resourceRoots);
    err = getLastError();
    if(dynlen(err)>0)
    {
      // if nothing specified, take the local PIC and PAC trees
      resourceRoots = makeDynString("PIC","PAC");
    }  
  }
  else
  {
    dyn_string dpPathElements = strsplit(parentDatapoint,"_");
    maxDepth = depth + dynlen(dpPathElements);
    resourceRoots = makeDynString(parentDatapoint);
  }
  for(int i=1;i<=dynlen(resourceRoots);i++)
  {
    // query the database for all resources under the given root
    dynAppend(allResources,dpNames(resourceRoots[i]+"*"));
  }
  
  // now remove all DP's with double '_' (e.g. __enabled)
  // strip everything below the requested level
  // remove duplicates  
  int i=1;
  while(i<=dynlen(allResources))
  {
    if(strpos(allResources[i],"__") < 0)
    {
      dyn_string dpPathElements = strsplit(allResources[i],"_");
      string addResource;
      int d=1;
      while(d<=maxDepth && d<=dynlen(dpPathElements))
      {
        if(d>1)
          addResource += "_";
        addResource += dpPathElements[d];
        d++;
      }
      if(!dynContains(resources,addResource))
      {
        LOG_DEBUG("Adding: ",addResource);
        dynAppend(resources,addResource);
      }
    }
    i++;
  }
  return resources;
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
  
  if(dpExists(datapointPath))
  {
    datapointType = dpTypeName(datapointPath);
    // find __nav_<datapointType>_viewconfig datapoint
    dpViewConfig = "__nav_"+datapointType+"_viewconfig";
  }
  else
  {
    // a system root node is selected
    // find __nav_<systemname>_viewconfig datapoint
    dpViewConfig = "__nav_"+datapointPath+"_viewconfig";
  }
  if(!dpExists(dpViewConfig))
  {
    LOG_TRACE("navConfigGetViewConfig","DP does not exist, using default configuration",dpViewConfig);
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

  dpGet(dpViewConfig + "." + ELNAME_SELECTEDVIEW,selectedView,
        dpViewConfig + "." + ELNAME_SELECTEDSUBVIEW,selectedSubView,
        dpViewConfig + "." + ELNAME_VIEWS,views,
        dpViewConfig + "." + ELNAME_NROFSUBVIEWS,nrOfSubViews,
        dpViewConfig + "." + ELNAME_SUBVIEWS,subViews,
        dpViewConfig + "." + ELNAME_CONFIGS,configs);
  
  err = getLastError(); //test whether no errors
  if(dynlen(err) > 0)
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
  dpSet(dpViewConfig + "." + ELNAME_SELECTEDSUBVIEW,selectedSubView);
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

  dpGet(dpSubViewConfig + "." + ELNAME_CAPTION,subViewCaption,
        dpSubViewConfig + "." + ELNAME_FILENAME,subViewFileName);
  
  err = getLastError(); //test whether no errors
  if(dynlen(err) > 0)
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
  dpQuery("SELECT '_original.._value' FROM '__nav_*' WHERE _DPT=\"GCFNavViewConfiguration\" ",tab);
  
  int viewsIndex    = 4;
  
  while(viewsIndex <= dynlen(tab) && sane)
  {
    int subViewsIndex     = viewsIndex + 1;
    int configsIndex      = viewsIndex + 2;
    int nrOfSubviewsIndex = viewsIndex + 3;
    
    // check number of items in subviews, configs and nrOfSubViews
    int nrOfViews = dynlen(tab[viewsIndex][2]);
    sane = (nrOfViews == dynlen(tab[nrOfSubviewsIndex][2]));
    //DebugTN("check nrOfViews:",sane,nrOfViews,dynlen(tab[nrOfSubviewsIndex][2]));
    if(!sane)
    {
      message = "Invalid view configuration\n#views and nrOfSubViews do not correspond\n(" + tab[viewsIndex][1] + ")";
      DebugTN(message);
    }
    
    if(sane)
    {
      int nrOfSubViews = 0;
      for(i=1;i<=dynlen(tab[nrOfSubviewsIndex][2]);i++)
        nrOfSubViews += tab[nrOfSubviewsIndex][2][i];
      sane = (nrOfSubViews == dynlen(tab[subViewsIndex][2]) && nrOfSubViews == dynlen(tab[configsIndex][2]));
      //DebugTN("check nrOfSubViews:",sane,nrOfSubViews,dynlen(tab[subViewsIndex][2]),dynlen(tab[configsIndex][2]));
      if(!sane)
      {
        message = "Invalid view configuration\n#subviews, #configs and total nrOfSubViews do not correspond\n(" + tab[viewsIndex][1] + ")";
        DebugTN(message);
      }
    }
    
    if(sane)
    {
      for(i=1;i<=dynlen(tab[viewsIndex][2]) && sane;i++)
      {
        sane = dpExists(tab[viewsIndex][2][i]);
        DebugTN("check view existance:",sane,tab[viewsIndex][2][i]);
      }
      if(!sane)
      {
        message = "Invalid view configuration\nview item (" + (i-1) + ") does not exist\n(" + tab[viewsIndex][1] + ")";
        DebugTN(message);
      }
    }
    
    if(sane)
    {
      for(i=1;i<=dynlen(tab[subViewsIndex][2]) && sane;i++)
      {
        sane = dpExists(tab[subViewsIndex][2][i]);
        DebugTN("check subview existance:",sane,tab[subViewsIndex][2][i]);
      }
      if(!sane)
      {
        message = "Invalid view configuration\nsubview item (" + (i-1) + ") does not exist\n(" + tab[viewsIndex][1] + ")";
        DebugTN(message);
      }
    }
    
    if(sane)
    {
      for(i=1;i<=dynlen(tab[configsIndex][2]) && sane;i++)
      {
        sane = dpExists(tab[configsIndex][2][i]);
        DebugTN("check configs existance:",sane,tab[configsIndex][2][i]);
      }
      if(!sane)
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
// subscribes to the update trigger datapoint
///////////////////////////////////////////////////////////////////////////
void navConfigSubscribeUpdateTrigger(string callback)
{
  dpConnect(callback,false,DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE);
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
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE,0);
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
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_NEWDATAPOINT,newDatapoint);
  dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_TRIGGERUPDATE,0);
}
