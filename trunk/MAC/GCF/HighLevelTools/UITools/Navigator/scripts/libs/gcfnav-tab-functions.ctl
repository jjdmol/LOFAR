// global functions. All event handlers are implemented here
//

global string  VIEW_COMBOBOX_CTRL      = "ComboBoxViews";
global string  VIEW_TABS_CTRL          = "TabViews";
global string  VIEW_TABS_VIEW_NAME     = "View";
global string  VIEW_TABS_CONFIG_NAME   = "Config";
global mapping g_subViews;
global mapping g_subViewConfigs;
global string  g_datapoint;
global string  g_configPanelFileName;
global int     g_selectedView;


///////////////////////////////////////////////////////////////////////////
//Function mappingClear
//  
// removes all items from a mapping
//
///////////////////////////////////////////////////////////////////////////
void mappingClear(mapping &map)
{
  DebugTN("mappingClear begin",map);
  while(mappinglen(map)>0)
    mappingRemove(map,mappingGetKey(map,1));
  DebugTN("mappingClear end",map);
}

///////////////////////////////////////////////////////////////////////////
//Function PanelInitialize
//  
// initializes the panel
// parameters:
//   datapoint        : string representing the selected datapoint, including system name and (if applicable) element name
//
///////////////////////////////////////////////////////////////////////////
void PanelInitialize(string datapoint)
{
  DebugTN("PanelInitialize(datapoint): ",datapoint);
  
  
  dyn_errClass err;
  shape viewsComboBoxCtrl = getShape(VIEW_COMBOBOX_CTRL);  
  shape viewTabsCtrl      = getShape(VIEW_TABS_CTRL);  
  
  // clear mappings and combobox
  viewsComboBoxCtrl.deleteAllItems();
  mappingClear(g_subViews);
  mappingClear(g_subViewConfigs);
  g_selectedView = 1;
  
  g_datapoint = datapoint;
  g_configPanelFileName = "";
  
  string viewsPath = "navigator/views/";
  dpGet("__navigator.viewsPath",viewsPath);
  err = getLastError(); //test whether no errors
  if(dynlen(err) > 0)
  {
    errorDialog(err);
    // open dialog box with errors
    throwError(err); // write errors to stderr
  }
  
  string dpNameConfig = "__nav_default_viewconfig";
  if(dpExists(g_datapoint))
  {
    // get configuration from the database
    string datapointTypeName = dpTypeName(g_datapoint);
    string dpTempNameConfig = "__nav_" + datapointTypeName + "_viewconfig";
    if(dpExists(dpTempNameConfig))
    {
      dpNameConfig = dpTempNameConfig;
    }
  }
  
  int selectedSubView;
  dyn_string views;
  dyn_int  nrOfSubViews;
  dyn_string subViews;
  dyn_string configs;
  dpGet(dpNameConfig+".selectedView",g_selectedView,
        dpNameConfig+".selectedSubView",selectedSubView,
        dpNameConfig+".views",views,
        dpNameConfig+".nrOfSubViews",nrOfSubViews,
        dpNameConfig+".subViews",subViews,
        dpNameConfig+".configs",configs);
  
  err = getLastError(); //test whether no errors
  if(dynlen(err) > 0)
  {
    errorDialog(err);
    // open dialog box with errors
    throwError(err); // write errors to stderr
  }
  else
  {
    DebugTN("dpGet:",g_selectedView,selectedSubView,views,nrOfSubViews,subViews,configs);

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
      dpGet(subViews[i] + ".caption",subViewCaption,
            subViews[i] + ".filename",subViewFileName);
      err = getLastError(); //test whether no errors
      if(dynlen(err) > 0)
      {
        errorDialog(err);
        // open dialog box with errors
        throwError(err); // write errors to stderr
      }
      else
      {
        DebugTN("subviewcaption,subviewfilename:",subViewCaption,subViewFileName);
        g_subViews[subViewCaption] = viewsPath+subViewFileName;
        g_subViewConfigs[subViewCaption] = configs[i];
      }
    }
    DebugTN("g_subViews = ",g_subViews);
    DebugTN("g_subViewConfigs = ",g_subViewConfigs);
    
    // fill the combobox
    for(int i=1;i<=mappinglen(g_subViews);i++)
    {
      viewsComboBoxCtrl.appendItem(mappingGetKey(g_subViews,i));
    }

    // get the config panel filename    
    dpGet(views[g_selectedView]+".configPanel",g_configPanelFileName);
    err = getLastError(); //test whether no errors
    if(dynlen(err) > 0)
    {
      errorDialog(err);
      // open dialog box with errors
      throwError(err); // write errors to stderr
      
      g_configPanelFileName = "";
    }
    else
    {
      g_configPanelFileName = viewsPath + g_configPanelFileName;
    }  
    
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
  DebugTN("ComboBoxViewsSelectionChanged()");
  
  shape viewsComboBoxCtrl = getShape(VIEW_COMBOBOX_CTRL);  
  shape viewTabsCtrl      = getShape(VIEW_TABS_CTRL);  
  
  // store the selected subview:
  int selectedSubViewIndex = viewsComboBoxCtrl.selectedPos();
  string dpNameConfig = "__nav_default_viewconfig";
  if(dpExists(g_datapoint))
  {
    // get configuration from the database
    string datapointTypeName = dpTypeName(g_datapoint);
    string dpTempNameConfig = "__nav_" + datapointTypeName + "_viewconfig";
    if(dpExists(dpTempNameConfig))
    {
      dpNameConfig = dpTempNameConfig;
    }
  }
  dpSet(dpNameConfig+".selectedSubView",selectedSubViewIndex);
  
  // if config tab is selected, some more actions may be required
  
  string selectedSubView = viewsComboBoxCtrl.selectedText();
  string selectedPanel = g_subViews[selectedSubView];
  dyn_string panelParameters = makeDynString(
    "$datapoint:" + g_datapoint,
    "$configDatapoint:" + g_subViewConfigs[selectedSubView]);
  DebugTN("selectedSubView,selectedPanel,panelParameters: ",selectedSubView,selectedPanel,panelParameters);
  viewTabsCtrl.namedRegisterPanel(VIEW_TABS_VIEW_NAME,selectedPanel,panelParameters);
  
  string datapointTypeName = dpTypeName(g_datapoint);
  dyn_string configPanelParameters = makeDynString(
    "$selectedView:" + g_selectedView,
    "$selectedElementDpType:" + datapointTypeName,
    "$configDatapoint:"+g_subViewConfigs[selectedSubView]);
  DebugTN("configPanel,configParameters: ",g_configPanelFileName,configPanelParameters);
  viewTabsCtrl.namedRegisterPanel(VIEW_TABS_CONFIG_NAME,g_configPanelFileName,configPanelParameters);
}

