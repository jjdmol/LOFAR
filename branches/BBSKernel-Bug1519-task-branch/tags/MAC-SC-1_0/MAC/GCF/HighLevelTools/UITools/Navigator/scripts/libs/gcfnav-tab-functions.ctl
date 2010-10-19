//# gcfnav-tab-functions.ctl
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
//#  You should have received a copy of the GNU General Public Licenlse
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//#
//# global functions for the Tabs of the Navigator. All event handlers are implemented here
//#

#uses "gcf-util.ctl"
#uses "gcfnav-configuration-functions.ctl"
#uses "gcfnav-functions.ctl"

global string  VIEW_COMBOBOX_CTRL      = "ComboBoxViews";
global string  VIEW_TABS_CTRL          = "TabViews";
global string  VIEW_TABS_VIEW_NAME     = "View";
global string  VIEW_TABS_CONFIG_NAME   = "Config";

global mapping g_subViews;
global mapping g_subViewConfigs;
global string  g_datapoint;
global string  g_configPanelFileName;
global int     g_selectedView;
global string  g_selectedViewName;

///////////////////////////////////////////////////////////////////////////
//Function mappingClear
//  
// removes all items from a mapping
//
///////////////////////////////////////////////////////////////////////////
void mappingClear(mapping &map)
{
  LOG_TRACE("mappingClear begin",map);
  while(mappinglen(map)>0)
    mappingRemove(map,mappingGetKey(map,1));
  LOG_TRACE("mappingClear end",map);
}

///////////////////////////////////////////////////////////////////////////
//Function NavTabInitialize
//  
// initializes the panel
// parameters:
//   datapoint        : string representing the selected datapoint, including system name and (if applicable) element name
//
///////////////////////////////////////////////////////////////////////////
void NavTabInitialize(string datapoint)
{
  LOG_DEBUG("NavTabInitialize(datapoint): ",datapoint);
  
  dyn_errClass err;
  shape viewsComboBoxCtrl = getShape(VIEW_COMBOBOX_CTRL);  
  shape viewTabsCtrl      = getShape(VIEW_TABS_CTRL);  
  
  // clear mappings and combobox
  viewsComboBoxCtrl.deleteAllItems();
  mappingClear(g_subViews);
  mappingClear(g_subViewConfigs);
  g_selectedView = 1;
  g_selectedViewName = navConfigGetSelectedView();
  LOG_DEBUG("NavTabInitialize: selView",g_selectedViewName);
  
  g_datapoint = datapoint;
  g_configPanelFileName = "";
  
  string viewsPath = navConfigGetViewsPath();
  
  string dpNameConfig = navConfigGetViewConfig(g_datapoint);
  
  int selectedSubView;
  int selectedViewUnused;
  dyn_string views;
  dyn_int  nrOfSubViews;
  dyn_string subViews;
  dyn_string configs;
  if(navConfigGetViewConfigElements(dpNameConfig,
                                    selectedViewUnused,
                                    selectedSubView,
                                    views,
                                    nrOfSubViews,
                                    subViews,
                                    configs))
  {
    LOG_DEBUG("NavTabInitialize:",g_selectedView,selectedSubView,LOG_DYN(views),LOG_DYN(nrOfSubViews),LOG_DYN(subViews),LOG_DYN(configs));
    for(int i=1;i<=dynlen(views);i++)
    {
      if(navConfigGetViewCaption(views[i]) == g_selectedViewName)
        g_selectedView = i;
    }

    // create the mapping
    int beginSubViews=1;
    if(g_selectedView < 1)
      g_selectedView = 1;
    if(selectedSubView < 1)
      selectedSubView = 1;
    if(selectedSubView > viewsComboBoxCtrl.itemCount())
      selectedSubView=1;
    for(int i=1;i<g_selectedView;i++)
    {
      beginSubViews += nrOfSubViews[i];
    }
    for(int i=beginSubViews;i<beginSubViews+nrOfSubViews[g_selectedView];i++)
    {
      // get subviews config
      string subViewCaption;
      string subViewFileName;
      if(navConfigGetSubViewConfigElements(subViews[i],subViewCaption,subViewFileName))
      {
        LOG_DEBUG("subviewcaption,subviewfilename:",subViewCaption,viewsPath+subViewFileName);
        g_subViews[subViewCaption] = viewsPath+subViewFileName;
        g_subViewConfigs[subViewCaption] = configs[i];
      }
    }
    LOG_TRACE("g_subViews = ",g_subViews);
    LOG_TRACE("g_subViewConfigs = ",g_subViewConfigs);
    
    // fill the combobox
    for(int i=1;i<=mappinglen(g_subViews);i++)
    {
      viewsComboBoxCtrl.appendItem(mappingGetKey(g_subViews,i));
    }

    // get the config panel filename    
    g_configPanelFileName = navConfigGetViewConfigPanel(views[g_selectedView]);
    g_configPanelFileName = viewsPath + g_configPanelFileName;
    
    viewsComboBoxCtrl.selectedPos(selectedSubView);
    ComboBoxViewsSelectionChanged();
  }
}


///////////////////////////////////////////////////////////////////////////
//Function ComboBoxViewsSelectionChanged
//  
// called when the combobox changes
//
///////////////////////////////////////////////////////////////////////////
void ComboBoxViewsSelectionChanged()
{
  LOG_TRACE("ComboBoxViewsSelectionChanged()");
  
  shape viewsComboBoxCtrl = getShape(VIEW_COMBOBOX_CTRL);  
  shape viewTabsCtrl      = getShape(VIEW_TABS_CTRL);  
  
  // store the selected subview:
  int selectedSubViewIndex = viewsComboBoxCtrl.selectedPos();
  navConfigSetSelectedSubView(g_datapoint,selectedSubViewIndex);
  
  // if config tab is selected, some more actions may be required
  
  string selectedSubView = viewsComboBoxCtrl.selectedText();
  string selectedPanel = g_subViews[selectedSubView];

  //if for selectedSubView a mapping has been found, use this name,
  //otherwise parse an empty string to prevent WARNINGS
  string insertSubViewConfigs="";
  if(mappingHasKey(g_subViewConfigs, selectedSubView))
  {
    insertSubViewConfigs = g_subViewConfigs[selectedSubView];   
  }
  else
  {
    insertSubViewConfigs = "";
  }

  dyn_string panelParameters = makeDynString(
    "$datapoint:" + g_datapoint,
    "$configDatapoint:" + insertSubViewConfigs);
  LOG_TRACE("selectedSubView,selectedPanel,panelParameters: ",selectedSubView,selectedPanel,panelParameters);
  
  //---------------------------------------------------------------
  //viewTabsCtrl.namedRegisterPanel divided in three possibilities.
  //1. panelfile present and accessable
  //2. There is no subview configured for this DP-Type
  //1. panelfile present and accessable
  if(viewsComboBoxCtrl.itemCount==0)
  {
    viewTabsCtrl.namedRegisterPanel(VIEW_TABS_VIEW_NAME,"navigator/views/nosubview.pnl",panelParameters);
    LOG_DEBUG("## No subview configured for this datapoint type");
  }
  else if(access(getPath(PANELS_REL_PATH)+selectedPanel,F_OK) == 0 && selectedPanel!="")
  {
    viewTabsCtrl.namedRegisterPanel(VIEW_TABS_VIEW_NAME,selectedPanel,panelParameters);
	  LOG_TRACE("## selectedPanel:", selectedPanel);
  }
  else if (selectedPanel=="0")
  {
    selectedPanel = "navigator/views/nopanel.pnl";
    viewTabsCtrl.namedRegisterPanel(VIEW_TABS_VIEW_NAME,"navigator/views/nopanel.pnl",panelParameters);
    LOG_DEBUG("## No panel configured for this subview");
  }
  else  //3. The configured panel file for this subview has not been found
  {
	  string oldSelectedPanel = selectedPanel;
	  viewTabsCtrl.namedRegisterPanel(VIEW_TABS_VIEW_NAME,"navigator/views/nopanelfound.pnl",panelParameters);
	  LOG_DEBUG("## VIEW_TABS_VIEW_NAME: ", VIEW_TABS_VIEW_NAME);
    LOG_DEBUG("## File does not exist:",oldSelectedPanel);
  }            
 
  string datapointTypeName = "";
  //DebugTN("g_datapoint",g_datapoint);
  if(dpExists(g_datapoint))
    datapointTypeName = dpTypeName(g_datapoint);
  else
    datapointTypeName = g_datapoint;
  //////////////////////////////////////////////////////////////

  
//  DebugN("test125:"+(g_subViewConfigs[selectedSubView]));
//  if(g_subViewConfigs[selectedSubView]==0)
  
  dyn_string configPanelParameters = makeDynString(
    "$selectedView:" + g_selectedView,
    "$viewName:" + g_selectedViewName,
    "$selectedElementDpType:" + datapointTypeName,
    "$datapoint:" + g_datapoint,
    "$configDatapoint:"+insertSubViewConfigs);
  
  //////////////////////////////////////////////////////////////  
//  This is the original one
/*  dyn_string configPanelParameters = makeDynString(
    "$selectedView:" + g_selectedView,
    "$viewName:" + g_selectedViewName,
    "$selectedElementDpType:" + datapointTypeName,
    "$datapoint:" + g_datapoint,
    "$configDatapoint:"+g_subViewConfigs[selectedSubView]); */
  LOG_TRACE("configPanel,configParameters: ",g_configPanelFileName,configPanelParameters);
  
  // check if the file exists:
  if(access(getPath(PANELS_REL_PATH)+g_configPanelFileName,F_OK) == 0 && g_configPanelFileName!="")
  {
    viewTabsCtrl.namedRegisterPanel(VIEW_TABS_CONFIG_NAME,g_configPanelFileName,configPanelParameters);
  }
  else
  {
    LOG_DEBUG("File does not exist:",g_configPanelFileName);
  }
}

///////////////////////////////////////////////////////////////////////////
//Function ConfigTabAddSubViewClicked
//  
// called when the "add subview" button is clicked
//
///////////////////////////////////////////////////////////////////////////
bool ConfigTabAddSubViewClicked(string viewName, int selectedView, string selectedElementDpType, string configDatapoint, int &nrOfSubViews)
{
  bool success;
  dyn_float resultFloat;
  dyn_string resultString;
  ChildPanelOnCentralModalReturn(
    "navigator/navigator_newsubview.pnl",
    "Add Sub-view",
    makeDynString("$addView:TRUE", 
                  "$viewName:" + viewName,
                  "$selectedView:" + selectedView, 
                  "$selectedElementDpType:" + selectedElementDpType, 
                  "$configDatapoint:" + configDatapoint),
    resultFloat,
    resultString);
    
  success = resultFloat[1];
  nrOfSubViews = resultFloat[2];
  if(success)
  {
    navConfigTriggerNavigatorRefresh();
  }
  return success;
}

///////////////////////////////////////////////////////////////////////////
//Function ConfigTabRemoveSubViewClicked
//  
// called when the "remove subview" button is clicked
//
///////////////////////////////////////////////////////////////////////////
bool ConfigTabRemoveSubViewClicked(string viewName, int selectedView, string selectedElementDpType, string configDatapoint, int &nrOfSubViews)
{
  bool success;
  dyn_float resultFloat;
  dyn_string resultString;
  ChildPanelOnCentralModalReturn(
    "navigator/navigator_newsubview.pnl",
    "Remove Sub-view",
    makeDynString("$addView:FALSE", 
                  "$viewName:" + viewName,
                  "$selectedView:" + selectedView, 
                  "$selectedElementDpType:" + selectedElementDpType, 
                  "$configDatapoint:" + configDatapoint),
    resultFloat,
    resultString);

  success = resultFloat[1];
  nrOfSubViews = resultFloat[2];
  if(success)
  {
    navConfigTriggerNavigatorRefresh();
  }
  return success;
}
