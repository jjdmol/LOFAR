// this function is called after the user correctly logged in 
// this should start whatever this project wants 
 
afterLogin(string user, string password, string newLocale, int closeModules = 1) 
{
  // WARNING string variable 'panel' below must be set 
  // - if the user would like to have a special beginning panel
  // - or isMotif will be started
  string panel    = "navigator.pnl", s;         // default panelName  to be opened after login
  string module   = "PVSS-II";                  // default moduleName to be used
  string locale   = getLocale(getActiveLang()); // current locale 

  bool           usePT=true;
  unsigned       xResolution, yResolution, templateNumber,
                 x=1000, y=600, i;
  dyn_string     panels;
  dyn_uint       xs=makeDynInt(1600,1280,1024),
                 ys=makeDynInt(1200,1024, 768);

  //**** mhalper: only if demo application ******************************************************
  if (dpExists("ApplicationProperties"))
  { 
    // Get panelName to be opened
    dpGet("ApplicationProperties.panels.basepanel:_online.._value",     panel,
          "ApplicationProperties.common.module_title:_online.._value",  module);

    // Set to defaults if datapoints are empty
    if (module == "")
      module = "PVSS II";              // default moduleName to be used 
    if (panel == "")
      panel  = "basePanel_user.pnl";   // default panel to be used 
  }
  //*********************************************************************************************

  if ( /*isMotif() ||*/ !dpExists("_PanelTopology"))
    usePT = FALSE;

  if (usePT)
  {
    dpGet("_PanelTopology.fileName:_online.._value",panels,
          "_PanelTopology.template.xResolution:_online.._value",xResolution,
          "_PanelTopology.template.yResolution:_online.._value",yResolution,
          "_PanelTopology.template.templateNumber:_online.._value",templateNumber);

    if (dynlen(panels)<1 || panels[1]=="")
      usePT = FALSE;
  }
  
  // panel topology not found
  if (usePT)
  {
    if (templateNumber == 4 )
    {
      s = getPath(PANELS_REL_PATH,"basePanel_user.pnl");
    }
    else
    {
      s = "";
      for (i=1;i<=dynlen(xs);i++)
      {
        if (xResolution==xs[i] && yResolution==ys[i])
        {
          s=getPath(PANELS_REL_PATH,"para/PanelTopology/templates/basePanel_"+i+"_"+templateNumber+".pnl");
          break;
        }
      }
    }
    
    if (access(s,F_OK)==0)
    {
      if ( templateNumber != 4 )
        panel = "para/PanelTopology/templates/basePanel_"+i+"_"+templateNumber+".pnl";
      else
        panel = "basePanel_user.pnl";
    }
    x=xResolution-10; y=yResolution-50;
  }

  //opening new module
  if ( newLocale != locale) 
  {
    startThread("changeLang", newLocale);
      //int    err = switchLang(getLangIdx(newLocale));
  }
  if ((closeModules == 1 || closeModules == 3) &&
       isModuleOpen(module) &&
       myModuleName() != "mainModule" &&
       myModuleName() != "naviModule" &&
       myModuleName() != "infoModule" &&
       myModuleName() != module)
  {
       ModuleOff(module);
       while ( isModuleOpen(module) ) // ... von Milos 
         delay(0,100);
//       delay(0,500);
  }

  if (!isModuleOpen(module))
     ModuleOn(module,0,0,x,y,-1,-1,"Scale");
  playDemoStartUpSound();

  if (usePT)
  {
    if ((closeModules == 1 || closeModules == 3) &&
         myModuleName() != "mainModule" &&
         myModuleName() != "naviModule" &&
         myModuleName() != "infoModule" &&
         myModuleName() != module)
    /*
    if ( myModuleName() != "mainModule" &&
         myModuleName() != "naviModule" &&
         myModuleName() != "infoModule" &&
         myModuleName() != module)
    */
      ModuleOff(myModuleName());
    else
      if (myModuleName() != module )
        PanelOff();
      
    if (isModuleOpen("naviModule"))  //redraw navi panel
    {
      if (dpExists("ApplicationProperties") && panel == "basePanel_user.pnl" )  //mhalper: only for the demo application
        RootPanelOnModule("para/PanelTopology/templates/naviPanel_user.pnl","Navi","naviModule",makeDynString());
      else
        RootPanelOnModule("para/PanelTopology/templates/naviPanel_"+i+"_"+templateNumber+".pnl","Navi","naviModule",makeDynString());
    }
    else	
    {
      RootPanelOnModule(panel,"",module,"");
      playDemoStartUpSound();
    }
  }
  else
  {
    RootPanelOnModule(panel,"",module,makeDynString("$baseDP:LOFAR")); 
    
    if ( module != myModuleName()) 
      ModuleOff(myModuleName());

    playDemoStartUpSound();
  }
  // last action ... close login-Module 
} 

//**************************
changeLang(string newLocale)
{
    int    err = switchLang(getLangIdx(newLocale));
}

//**************************
playDemoStartUpSound()
{
  bool           onStartEnabled;
  string         soundfile; 
  
  //**** mhalper: only if demo application*****************************
  if (dpExists("ApplicationProperties"))
  {
    dpGet("ApplicationProperties.sound.onStart:_online.._value",        soundfile,
          "ApplicationProperties.sound.onStartEnabled:_online.._value", onStartEnabled);

    //On every new login play the startup-sound
    if (onStartEnabled == 1) da_startSound(soundfile,FALSE);
  }
  //*******************************************************************
}
