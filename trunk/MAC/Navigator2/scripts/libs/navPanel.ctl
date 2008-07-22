// navPanel.ctl
//
//  Copyright (C) 2002-2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
///////////////////////////////////////////////////////////////////
// Ctrl scripts that are generic for all panels
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// navPanel_initPanel                : sets up initial stuff for each panel
// navPanel_setEvent                 : fires an event from a Panel
// navPanel_showVersion              : shows version info in txt_version object
// navPanel_showTemp                 : shows temperature in txt_temperature object
// navPanel_checkDataPath            : check the dataflow to set dataFlowElement in the RCU panels on/off 
// navPanel_showLogging              : show logMsg and historical data if any
// navPanel_updateLogging            : callback for showLogging
// navPanel_addLogMsg                : adds a new log Msg to the logTable

#uses "navigator.ctl"

global string 		itsEventDp         = "";
global string           itsActionDp        = "";
global dyn_string 	itsSelectionDp     = "";
global string           itsNavigatorObject = "";
global string           sysName            = "";

// Define pathchoices to be able to check datapaths in RCU screen
global bool InputEnable=FALSE;
global bool LBLEnable=FALSE;
global bool LBHEnable=FALSE;
global bool HBAEnable=FALSE;
global bool bandSelLblLbh=FALSE;
global bool VlEnable=FALSE;
global bool VhEnable=FALSE;
global bool bandSelLbaHba=FALSE;
global bool VddVccEnable=FALSE;

// ****************************************
// Name : navPanel_initPanel
// ****************************************
// Description:  
//   Set up the common panel stuff
//
// Params:
//   objectName    :  The navigator object where this panel belongs to. (eg. fw_viewBox)
//
// ***************************************
void navPanel_initPanel(string objectName) {
  itsNavigatorObject = objectName;
  itsActionDp    = DPNAME_NAVIGATOR + g_navigatorID + "." + objectName + ".action";  
  itsEventDp     = DPNAME_NAVIGATOR + g_navigatorID + "." + objectName + ".event";
  itsSelectionDp = DPNAME_NAVIGATOR + g_navigatorID + "." + objectName + ".selection";
  sysName = dpSubStr(g_currentDatapoint,DPSUB_SYS);
  
  // empty global listings
  dynClear(g_stationList);
  dynClear(g_cabinetList);
  dynClear(g_subrackList);
  dynClear(g_RSPList);
  dynClear(g_TBBList);
  dynClear(g_RCUList);
  dynClear(g_observationsList);
  dynClear(g_processesList);
  
  
}

// ****************************************
// Name : navPanel_setEvent
// ****************************************
// Description:  
//   Set the selection and eventpoint on a selected sender object
//
// Params:
//   aSelection  :  The selection that might be of importance for the event handling
//   anEvent     :  The event we want to be triggered
//
// ***************************************
void navPanel_setEvent(dyn_string aSelection,string anEvent) {
  if (navigator_initializing()) {
    return;
  }
  string event=itsNavigatorObject+"|"+anEvent;
  if (dpExists(itsEventDp)) {
     dpSet(itsEventDp,event);
  } else {
    LOG_ERROR("navPanel.ctl:navPanel_setEvent| "+itsEventDp + " Does not exist yet");     
  }
  if (dpExists(itsSelectionDp)) {
    dpSet(itsSelectionDp,aSelection);
  } else {
    LOG_ERROR("navPanel.ctl:navPanel_setEvent| "+itsSelectionDp + " Does not exist yet");     
  }
} 

// ****************************************
// Name: navPanel_showVersion   
// ****************************************
//   Displays the version of the object in txt_version
//
// ****************************************
navPanel_showVersion(string dp1, string version)
{
    if (version !="")
    {
      setValue("txt_version", "text", "ver: " +version);
    }
    else
    {
      setValue("txt_version", "text", "ver: x.x");
    }
} 

// ****************************************
// Name: navPanel_showTemp   
// ****************************************
//   Displays the temperature of the object in txt_temperature
//
// ****************************************
navPanel_showTemp(string dp1, float temp)
{
    if (temp !="")
    {
      setValue("txt_temperature", "text", +temp);
      setValue("txt_temperature", "visible", TRUE);      
    }
    else
    {
      setValue("txt_temperature", "text", "xxxx");
      setValue("txt_temperature", "visible", FALSE);
    }
}

// ****************************************
// Name: navPanel_checkDataPath   
// ****************************************
//    Check the dataflow to set dataFlowElement in the RCU panels on/off 
//           
// ****************************************
navPanel_checkDataPath() {

  bool lbaPath = TRUE;
  bool hbaPath = TRUE;
  bool path    = TRUE;


  // check on LBL/LBH/HBA Enable level
  if (!LBLEnable && !LBHEnable) {
    lbaPath = FALSE;
  }

  if (!HBAEnable) {
    hbaPath = FALSE;
  }


  // check on bandSelLblLbh level
  if(lbaPath) {
    if ((bandSelLblLbh && !LBHEnable) ||
         (!bandSelLblLbh && !LBLEnable))  {
      lbaPath = FALSE;
    }
  }

  // Check on VlEnable and VhEnable Level
  if (lbaPath) {
    lbaPath = VlEnable;
  }
  if (hbaPath) {
    hbaPath = VhEnable;
  }

  // set lba/hba line colors
  if (lbaPath) {
    setValue("line1_lba","foreCol","Lofar_operational");
    setValue("line2_lba","foreCol","Lofar_operational");
  } else {
    setValue("line1_lba","foreCol","Lofar_off");
    setValue("line2_lba","foreCol","Lofar_off");
  }

  if (hbaPath) {
    setValue("line1_hba","foreCol","Lofar_operational");
    setValue("line2_hba","foreCol","Lofar_operational");
  } else {
    setValue("line1_hba","foreCol","Lofar_off");
    setValue("line2_hba","foreCol","Lofar_off");
  }
  

  // determine remainder path based on the above


  if (!lbaPath && !hbaPath) {
    path = FALSE;
  }

  // check based on bandSelLbaHba
  if (path) {
    if ((bandSelLbaHba && hbaPath) ||
         (!bandSelLblLbh && !lbaPath))  {
      path = FALSE;
    }
  }

  if (path) {
    setValue("line1_path","foreCol","Lofar_operational");
  } else {
    setValue("line1_path","foreCol","Lofar_off");
  }

  // check based on VddVccEnable
  if (path) {
    path = VddVccEnable;
  }
 
  if (path) {
    setValue("line2_path","foreCol","Lofar_operational");
  } else {
    setValue("line2_path","foreCol","Lofar_off");
  }

  // check based on InputEnable
  if (path) {
    path = InputEnable;
  }

  if (path) {
    setValue("line3_path","foreCol","Lofar_operational");
  } else {
    setValue("line3_path","foreCol","Lofar_off");
  }
}

// ****************************************
// Name: navPanel_showLogging   
// ****************************************
//    Connects to the given datapoint .logMsg and it's historical data (if any)
//    and logs all into the logWindow
//           
// ****************************************
navPanel_showLogging(string aDP)
{
  LOG_TRACE("navPanel.ctl:navPanel_showLogging| Datapoint: ",aDP);
	
  string dpLog = aDP+".process.logMsg";
  string systemName = dpSubStr(dpLog,DPSUB_SYS);
  string bareDP     = dpSubStr(dpLog,DPSUB_DP_EL_CONF_DET_ATT);

  myLogTable.deleteAllLines();
 
  if(dpExists(dpLog)) {
    // initialize the logging table with historical data:
    dyn_dyn_anytype tab;
    int z;
    time tStart;
    time tStop;
    tStop = getCurrentTime();
    tStart = tStop - 3*3600; // three hours of history
	
    string query="";
            
    if (systemName != MainDBName) {     
      query = "SELECT ALL '_original.._value' FROM '" + dpLog + "' REMOTE'"+systemName +"' TIMERANGE(\"" +
              formatTime("%Y.%m.%d %H:%M:%S",tStart) + "\",\"" +
              formatTime("%Y.%m.%d %H:%M:%S",tStop) + "\",1,0) LAST 100";
    } else {
      query = "SELECT ALL '_original.._value' FROM '" + dpLog + "' TIMERANGE(\"" +
              formatTime("%Y.%m.%d %H:%M:%S",tStart) + "\",\"" +
              formatTime("%Y.%m.%d %H:%M:%S",tStop) + "\",1,0) LAST 100";
    }
      
    LOG_DEBUG("navPanel.ctl:navPanel_showLogging|Query: " + query);
    dpQuery(query, tab);
    LOG_DEBUG("navPanel.ctl:navPanel_showLogging|Found: " + tab + " length: " + dynlen(tab));
	 	 
	 	
    for(z=2;z<dynlen(tab);z++) {
      navPanel_addLogMessage(tab[z][2]);
    }
	
    // connect to logging
    dpConnect("navPanel_updateLogging",dpLog);
  } else {
    LOG_DEBUG("navPanel.ctl:navPanel_showLogging|error connecting to: "+ dpLog);
  }    
}

// ****************************************
// Name: navPanel_updateLogging   
// ****************************************
//    callback for navPanel_showLogging
//           
// ****************************************
navPanel_updateLogging(string dpe, string logMsg)
{
  LOG_TRACE("navPanel.ctl:navPanel_showLogging|LogMsg: "+logMsg);
  if (logMsg != "") {
    navPanel_addLogMessage(logMsg);
  }
}

// ****************************************
// Name: navPanel_addLogMsg   
// ****************************************
//    places a log Msg in the appropiate table
//    split lines like:
//    13-11-06 10:06:00.519|INFO|MAC.GCF.PAL.SAL|Set value of property 'MCU001:LOFAR_PermSW_MACScheduler.OTDB.lastPoll'|GSA_Service.cc:661
//    into relevant pieces
//           
// ****************************************
navPanel_addLogMessage(string aMsg)
{


  LOG_TRACE("navPanel.ctl:navPanel_addLogMsg|msg: " + aMsg);
  if (aMsg != "") {


    dyn_string msgParts;
    string dateTime="";
    string level="";
    string source="";
    string logMsg="";
    string codeLine="";
    bool error=false;

    
    // we need to cut out all \r and \n from the string
    strreplace(aMsg,"\r","");
    strreplace(aMsg,"\n","");
    
    msgParts = strsplit(aMsg,"|");
    
    if (dynlen(msgParts) >=1) {
      dateTime = msgParts[1];
    } else {
      error=true;
    }
    if (dynlen(msgParts) >=2) {
      level    = msgParts[2];
    } else {
      error=true;
    }
    if (dynlen(msgParts) >=3) {
      source   = msgParts[3];
    } else {
      error=true;
    }

    if (dynlen(msgParts) >=4) {
      logMsg = msgParts[4];
    } else {
      error=true;
    }

    if (dynlen(msgParts) >=5) {
      codeLine = msgParts[5];
    } else {
      error=true;
    }
	
	
    LOG_DEBUG("navPanel.ctl:navPanel_showLogging|dateTime: " + dateTime);
    LOG_DEBUG("navPanel.ctl:navPanel_showLogging|level: " + level);
    LOG_DEBUG("navPanel.ctl:navPanel_showLogging|source: " + source);
    LOG_DEBUG("navPanel.ctl:navPanel_showLogging|logMsg: " + logMsg);
    LOG_DEBUG("navPanel.ctl:navPanel_showLogging|codeLine: " + codeLine);

    if (!error) {
      myLogTable.appendLine("time",dateTime,"level",level,"source",source,"message",logMsg,"code",codeLine);
      myLogTable.lineVisible(-1);
    }
  }
}

void navPanel_statePopup(string baseDP) {
  dyn_string popup;

  // NCFObjectState vars
  string DPName="";
  int state;
  string message="Operator Overrule";
  bool force=true;
  
  // define the popupMenu
  LOG_DEBUG("navPanel.ctl:navPanel_statePopup|define popup for DP: "+baseDP);
 
  int idx=1;
  dynInsertAt(popup,"CASCADE_BUTTON,Set State, 1",idx++);
  dynInsertAt(popup,"CASCADE_BUTTON,Set Recursive State, 1",idx++);
  dynInsertAt(popup,"Set State",idx++);
  for (int i = 1; i <= mappinglen(stateName); i++)  {
    string aS="PUSH_BUTTON,"+mappingGetValue(stateName,i)+",1"+mappingGetKey(stateName,i)+",1";
    dynInsertAt(popup,aS,idx++);
  }  
  dynInsertAt(popup,"Set Recursive State",idx++);
  for (int i = 1; i <= mappinglen(stateName); i++)  {
    string aS="PUSH_BUTTON,"+mappingGetValue(stateName,i)+",2"+mappingGetKey(stateName,i)+",1";
    dynInsertAt(popup,aS,idx++);
  }
  
  LOG_DEBUG("navPanel.ctl:navPanel_statePopup|popup composed");
  if (popupMenu(popup,state)<0 || !state>0) {
    return;
  }
  
  LOG_DEBUG("navPanel.ctl:navPanel_statePopup|popup returned: "+state);
  bool recursive = false;
  if (state == 10 ) {
    state = 0;
  } else if (state == 20) {
    state = 0;
    recursive = true;    
  } else if (state < 200) {
    state -= 100;
  } else {
    state -= 200;
    recursive = true;
  }        

  LOG_DEBUG("navPanel.ctl:navPanel_statePopup|recursive="+recursive);
  bool ack=navFunct_acknowledgePanel("This will (re)set the state of "+baseDP+" to "+getStateName(state)+". Are you sure?");
  if (!ack) {
    LOG_DEBUG("navPanel.ctl:navPanel_statePopup|State change by operator cancelled");
    return;
  }
  LOG_DEBUG("navPanel.ctl:navPanel_statePopup|State change by operator confirmed");
  
  string database = dpSubStr(baseDP,DPSUB_SYS);
  string bareDP = dpSubStr(baseDP,DPSUB_DP);
  if (state >= 0 ) {
    if (!recursive) {
      LOG_DEBUG("navPanel.ctl:navPanel_statePopup|Operator sets "+baseDP+".status.state to "+getStateName(state)+ " (SINGLE)");
      DPName=baseDP+".status.state";
      dpSet(database+"__navObjectState.DPName",DPName,
            database+"__navObjectState.stateNr",state,
            database+"__navObjectState.message",message,
            database+"__navObjectState.force",force);
    } else {
      // we will write the info to the __resetObjectState point.
      // All existing stations, CCU's and MCU's will be connected to this point
      // via a ctl script that runs on each machine.
      // that script will do an update (if needed) for their own Database.
      LOG_DEBUG("navPanel.ctl:navPanel_statePopup|Operator sets "+baseDP+".status.state to "+getStateName(state)+" (RECURSIVE)");
      dpSet(database+"__resetObjectState.DPName",baseDP,
            database+"__resetObjectState.stateNr",state,
            database+"__resetObjectState.message",message);
    }

  }
  LOG_DEBUG("navPanel.ctl:navPanel_statePopup|end (re)set states reached");
}
