// this function is called after the user correctly logged in 
// this should start whatever this project wants 
 
afterLogin(string user, string password, string newLocale, int closeModules = 1, int iConfNum = 1) 
{
  // WARNING string variable 'panel' below must be set 
  // - if the user would like to have a special beginning panel
  // - or isMotif will be started
  string panel    = "navigator.pnl", s;           // default panelName  to be opened after login
  string module   = "PVSS-II";                  // default moduleName to be used
  string locale   = getLocale(getActiveLang()); // current locale 

  string myModule ;

  if (closeModules > 0)
     myModule = myModuleName();
  
  bool           usePT=true;
  bool           useNewPT=false; //cstoeg: new PT with preview and Multiscreening
   
  unsigned       xResolution, yResolution, templateNumber,
                 x=1000, y=600, i;
  dyn_string     panels;
  dyn_uint       xs=makeDynInt(1600,1280,1024),
                 ys=makeDynInt(1200,1024, 768);
    switchToSplitWithActiveDrivers();
 
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
  
  if (dpExists("_"+user+"_UiConfiguration"))
  { 
    useNewPT = true;
  }
 
  if ( /*isMotif() ||*/ !dpExists("_PanelTopology"))
    usePT = FALSE;
   else
     if(!useNewPT && dpExists("_Default_UiConfiguration")) //IM 88664 -> PVSS woman should be shown if PT but now Template is configured for user
       usePT = false;

  if (usePT)
  {
    dpGet("_PanelTopology.fileName:_online.._value",panels,
          "_PanelTopology.template.xResolution:_online.._value",xResolution,
          "_PanelTopology.template.yResolution:_online.._value",yResolution,
          "_PanelTopology.template.templateNumber:_online.._value",templateNumber);

    if (dynlen(panels)<1 || panels[1]=="")
      usePT = FALSE;
  }
  
  if (useNewPT)
  {
    if(!g_iNumberOfScreens || dynlen(g_diXRes)<=0 || dynlen(g_dsTemplates)<=0 )
    {
      useNewPT = false; //Error: Nothing parameterized on DP
    }
  }
    
  
  // panel topology not found
  if (usePT && !useNewPT)
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
  if ( newLocale != locale && newLocale != "") 
  {
    startThread("changeLang", newLocale);
      //int    err = switchLang(getLangIdx(newLocale));
  }
  if ((closeModules == 1 || closeModules == 3) &&
       isModuleOpen(module) &&
       myModule != "mainModule" &&
       myModule != "naviModule" &&
       myModule != "infoModule" &&
       myModule != module)
  {
       ModuleOff(module);
       while ( isModuleOpen(module) ) // ... von Milos 
         delay(0,100);
//       delay(0,500);
  }
  
  if(!useNewPT)  //cstoeg: if new Panel Topology is used than do not open a module
  {
  if (closeModules==0 || !isModuleOpen(module))
  {
     ModuleOn(module,0,0,x,y,-1,-1,"Scale");

     while (!isModuleOpen(module))
        delay(0, 100);
  }
  }

  playDemoStartUpSound();
  
  if (useNewPT) // Initialise the different libarys and open basePanels
  {
    ptms_InitializeScreens(iConfNum);
    ptms_InitializeSystems();
    
    // HOOK >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> HOOK
    // When a function 'HOOK_ScreenConfiguration()' is defined
    // then call this one to modify Screen Configuration. 
    // The Configration can be modified by changing the values of the global Variables:
    // g_dbUsedScreens = dyn String of Used Screens
    // g_dsTemplates = dyn String of loaded templates
    // g_dsPanels = dyn String of loaded panels
    // g_dsCharacters = dyn String of Screen Characters

    if( isFunctionDefined( "HOOK_ScreenConfiguration" ) )
  {
      string strEval = "bool main(){ return HOOK_ScreenConfiguration(); }";
      bool bResult; //Result of the function
     
      evalScript( bResult, strEval, makeDynString() );
                    
    
    } 
    // HOOK <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< HOOK

    ptnavi_InitializeAfterLogin();
    ptms_LoadBasePanels();
  }
  else if (usePT)
  {
     
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
    
    if ((closeModules == 1 || closeModules == 3) &&
         myModule != "mainModule" &&
         myModule != "naviModule" &&
         myModule != "infoModule" &&
         myModule != module)
      ModuleOff(myModule);
    else 
      if (myModule != module && closeModules != 0 )
        PanelOff();
   }
  else
  {
    RootPanelOnModule(panel,"",module,""); 
    
    if ( module != myModule && closeModules != 0) 
      ModuleOff(myModule);

    playDemoStartUpSound();
  }
  //on logOut the lastScreenView will be saved
  msc_deleteFav(user, "lastScreenView");

  delay(0,50); //IM 98250: otherwise the panel can not be cloesed because module can not be found
  //removeGlobals which are created in login.pnl
  if(globalExists("g_iNumberOfScreens"))
  {
    removeGlobal("g_iNumberOfScreens");
    removeGlobal("g_sConfigName");
    removeGlobal("g_dbUsedScreens"); // Used Screens 
    removeGlobal("g_diXRes"); // Resolution of Screen
    removeGlobal("g_diYRes");
    removeGlobal("g_dsTemplates");  // Templates of the specific Screens
    removeGlobal("g_dsPanels");     // Panels to Load
    removeGlobal("g_dsCharacters"); // Characters of Screen
    removeGlobal("g_dsLCharacters");
    removeGlobal("g_dsRCharacters");
    removeGlobal("g_iActNumberOfScreens");
    removeGlobal("g_iDefaultResX");
    removeGlobal("g_iDefaultResY");
    removeGlobal("g_sDefaultTemplate");
  }
  
  // last action ... close login-Module 
  PanelOff();
  
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
