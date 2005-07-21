//# apl-view.ctl
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

//#
//# All user panel scripts functions. The scipts in the script file are user script
//# which are an example of visualisations of (parts of) the LOFAR instrument.
//# And are NOT part of the MAC Navigator framework.
//#

#uses "gcfnav_view.ctl"

//////////////////////////////////////////////////////////////////////////////////
// Function: aplViewNavigateTo. 
//           With the parameters given this function will decide which datapoint
//           name must be used to correctly navigate to this datapoint.
//
// Input: 1. $datapoint
//        2. $referenceDatapoint
//        3. addingPart, combined with 2 or 3, dp to navigate to
//        4. panelName, if undocked
//
// Output: 1. new datapoint name set in navigator dp, navigator trigger
//         2.
///////////////////////////////////////////////////////////////////////////////////
void aplViewNavigateTo(string datapoint, string referenceDatapoint, string addingPart, string panelName)
{
  if(dpAccessable(datapoint + addingPart))
  {
    if ("LOFAR Navigator" == myPanelName())
    {
      //If the datapoint is a reference, use the reference to navigate to!!
      if(referenceDatapoint=="") 
      {
        navConfigTriggerNavigatorRefreshWithDP(datapoint + addingPart);        
        LOG_TRACE("Navigate to: ",datapoint + addingPart);
      }
      else
      {
        navConfigTriggerNavigatorRefreshWithDP(referenceDatapoint + addingPart);
        LOG_TRACE("Navigate to: ",referenceDatapoint + addingPart);
      }
    }
    else // Panel is undocked.
    {  
      if(panelName!="")
      {
        RootPanelOn("nav_usr/STS/" + panelName,
                    panelName,
                    makeDynString("$datapoint:" + datapoint + addingPart));
        LOG_TRACE("Navigate to[undocked]: ",datapoint + addingPart);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////
// Function: setBackGroundColorAEM
//           Sets the background color and set the visibility of
//           the "icon_maintenance". Maintenance overrules and error status
//           of the object
//
// Input: 1. maintenance status
//        2. object status
//        3. invalid flag
///////////////////////////////////////////////////////////////////////////////////
setBackGroundColorAEM(string dp1, unsigned maintenance,
                      string dp2, unsigned status,
                      string dp3, bool invalid)
{
  
  if(!invalid) // Datapoint has a valid state.
  {
    setValue("icon_maintenance", "visible", (maintenance == 1));
    //Maintenance has priority above error status
    if (maintenance == 1)
    {
      setValue("backGround", "backCol", "Lofar_maintenance");
    }
    else if (status==1)    //object is in error status
    {
      setValue("backGround", "backCol", "Red");
    }
    else if (status==0)  //object has a correct status
    {
      setValue("backGround", "backCol", "Lofar_device_active");
    }
    else // if any other state => givce the color red
    {
      setValue("backGround", "backCol", "Red");
    }
  }
  else
  {
    setValue("backGround", "backCol", "_dpdoesnotexist");
  }
}

///////////////////////////////////////////////////////////////////////////
// Function: showVersion   
//           Displays the version of the PIC object in txt_version
//
// Input : 1. $datapoint.version
// Output: 1. version displayed in txt_version
///////////////////////////////////////////////////////////////////////////
showVersion(string dp1, string version,
            string dp2, bool invalid)
{
  if(!invalid) // Datapoint has a valid state.
  {
    if (version !="")
    {
      setValue("txt_version", "text", "ver: " +version);
    }
    else
    {
      setValue("txt_version", "text", "ver: x:x");
    }
  }
  else
  {
    setValue("txt_version", "text", "ver: x:x");    
  }
}

///////////////////////////////////////////////////////////////////////////
// Function: navViewShowTemp   
//           Displays the temperature in txt_temperature
//
// Input : 1. $datapoint.temperature [float]
// Output: 1. temperature displayed in txt_temperature
///////////////////////////////////////////////////////////////////////////
navViewShowTemp(string dp1, float temperature,
                string dp2, bool invalid)
{
  //Display the temperature with with format xxx if the datapoint is valid
  if(!invalid)
  {
    setValue("txt_temperature", "text", temperature);  
    setValue("txt_temperature", "visible", TRUE);
  }
  else
  {
    setValue("txt_temperature", "visible", FALSE);  
  }
}


/////////////////////////////////////////////////////////////////////
// Function: RCUContextMenu
//           Creates and handles the Right Mouse Button menu for the 
//           RCU, on the SubRack panel
//
// Input : 1. object status
//         2. maintenance status
// Output: 1. change of status
/////////////////////////////////////////////////////////////////////
void RCUContextMenu()
{
  string txt_maintenance, txt_status;
  int Answer, status, maintenance;
  bool bOK;       //Variable with value FALSE
  
  if(dpAccessable($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +".status:_original.._value"))
  {
  dpGet($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +".status:_original.._value", status);
  dpGet($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_Maintenance.status:_original.._value", maintenance);

  BuildContextMenu(status, maintenance, Answer);

    // Compute the chosen option
    switch (Answer)
    {
    case 2:
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +".status:_original.._value", 0);
        break;
    case 3:
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +".status:_original.._value", 1);
        break;
    case 10:    
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_Maintenance.status:_original.._value", 0);
        break;
    case 11:
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_Maintenance.status:_original.._value", 1);
      break;
    default:
      break;
    }       
  }  
  else //$configDatapoint is not Accessable
  {
    popupMenu(g_ContectMenuDpAccessableErrorText, Answer); 
  }
}


/////////////////////////////////////////////////////////////////////
// FunctionName: BuildContextMenu
//               Builds the contextMenu for setting object status 
//               and maintenance
//
// Input : 1. object status
//         2. maintenance status
// Output: 1. contextment
/////////////////////////////////////////////////////////////////////
void BuildContextMenu(int status, int maintenance, int &Answer)
{
  string txt_status, txt_maintenance;
  //Define the content of the contextmenu
  if (status==1)
    txt_status = "PUSH_BUTTON, Set status to -> OK, 2, 1";
  else
    txt_status = "PUSH_BUTTON, Set status to -> Error, 3, 1";

  if (maintenance==1)
    txt_maintenance = "PUSH_BUTTON, Turn off maintenance, 10, 1";
  else
    txt_maintenance = "PUSH_BUTTON, Turn on maintenance, 11, 1";
  
  // Display the contextmenu
  if  ((status==-1) && (maintenance!=-1))
  {
    popupMenu(makeDynString(txt_maintenance), Answer);
  }
  else if  ((status!=-1) && (maintenance==-1))
  {
    popupMenu(makeDynString(txt_status), Answer);
  }
  else // ((status!=-1) && (maintenance!=-1))
  {
    popupMenu(makeDynString(txt_maintenance, "SEPARATOR", txt_status), Answer);
  }
}


/////////////////////////////////////////////////////////////////////
// Function: AntennaContextMenu
//           Creates and handles the Right Mouse Button menu for the
//           LFA and HFA antenna, on the SubRack panel
//
// Input : 1. object status
//         2. maintenance status
// Output: 1. change of status
/////////////////////////////////////////////////////////////////////
void AntennaContextMenu(string antenna)
{
  string txt_maintenance;
  int Answer, status, maintenance;
  bool bOK;       //Variable with value FALSE
  if(dpAccessable($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_online.._value"))
  {
    dpGet($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value", status);
    dpGet($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + "_Maintenance.status:_original.._value", maintenance);
    BuildContextMenu(status, maintenance, Answer);
    // Compute the chosen option
    switch (Answer)
    {
      case 2:
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value", 0);
        break;
      case 3:
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value", 1);
        break;
      case 10:
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + "_Maintenance.status:_original.._value", 0);
        break;
      case 11:
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + "_Maintenance.status:_original.._value", 1);
        break;
      default:
        break;
    }       
  }  
  else //$configDatapoint is not Accessable
  {
    popupMenu(g_ContectMenuDpAccessableErrorText, Answer); 
  }
}


/////////////////////////////////////////////////////////////////////
// Function: APContextMenu
//           Creates and handles the Right Mouse Button menu for the
//           AP, on the SubRack panel
//
// Input : 1. object status
//         2. maintenance status
// Output: 1. change of status
/////////////////////////////////////////////////////////////////////
void APContextMenu()
{
  // Local data
  dyn_string txt;
  int Answer;
  int status;
  bool bOK;       //Variable with value FALSE
  if(dpAccessable($datapoint + "_Board0_AP"+ $APNr + ".status:_original.._value"))
  {
    dpGet($datapoint + "_Board0_AP"+ $APNr + ".status:_original.._value", status);
    BuildContextMenu(status, -1, Answer);

    // Compute the chosen option
    switch (Answer)
    {
    case 2:
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +".status:_original.._value", 0);
        break;
    case 3:
        dpSetWait($datapoint + "_Board0_AP"+ $APNr +".status:_original.._value", 1);
        break;
    default:
      break;
    }
  }
  else //$configDatapoint is not Accessable
  {
    popupMenu(g_ContectMenuDpAccessableErrorText, Answer);
  }        
}

/////////////////////////////////////////////////////////////////////
// Function: BPContextMenu
//           Creates and handles the Right Mouse Button menu for the
//           BP, on the SubRack panel
//
// Input : 1. object status
//         2. maintenance status
// Output: 1. change of status
/////////////////////////////////////////////////////////////////////
void BPContextMenu()
{
  string txt_maintenance, txt_status;
  int Answer, maintenance, status;
  bool bOK;       //Variable with value FALSE
  if(dpAccessable($datapoint + "_Board0_BP.status:_original.._value"))
  {
    dpGet($datapoint + "_Board0_BP.status:_original.._value", status);
    dpGet($datapoint + "_Board0_Maintenance.status:_original.._value", maintenance);
    BuildContextMenu(status, maintenance, Answer);

    // Compute the chosen option
    switch (Answer)
    {
    case 2:
      dpSetWait($datapoint + "_Board0_BP.status:_original.._value", 0);
      break;
    case 3:
      dpSetWait($datapoint + "_Board0_BP.status:_original.._value", 1);
      break;
    case 10:    
      dpSetWait($datapoint + "_Board0_Maintenance.status:_original.._value", 0);
      //dpActivateAlert($datapoint + "_Board0_BP.status", bOK);
      break;
    case 11:
      dpSetWait($datapoint + "_Board0_Maintenance.status:_original.._value", 1);
      //dpDeactivateAlert($datapoint + "_Board0_BP.status", bOK);
      break;
    default:
      break;
    }
  }  
  else //$configDatapoint is not Accessable
  {
    popupMenu(g_ContectMenuDpAccessableErrorText, Answer);
  }     
}






///////////////////////////////////////////////////////////
//
// Function: Display for a certain time a text
// OLD, not longer used
///////////////////////////////////////////////////////////
/*
void DisplayText(string ObjectName, string Text, string Value)
{
    setValue(ObjectName, "text", Text);
    delay(0,600);
    setValue(ObjectName, "text", Value);
}*/

/////////////////////////////////////////////////////////////////////
//
// Function: Creates and handles the RMB menu for the antennas
// FunctionName: AntennaContextMenu
// OLD IS NOT LONGER USED!!!!
/////////////////////////////////////////////////////////////////////
/*
void AntennaContextMenuMain(string antenna)
{
  string txt_maintenance;
  int Answer, status, maintenance;
  bool bOK;       //Variable with value FALSE
  if(dpAccessable($datapoint + "_Rack"+ $RackNr + "_SubRack" +$SubrackNr + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value"))
  {
    dpGet($datapoint + "_Rack"+ $RackNr + "_SubRack" +$SubrackNr + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value", status);
    dpGet($datapoint + "_Rack"+ $RackNr + "_SubRack" +$SubrackNr + "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + "_Maintenance.status:_original.._value", maintenance);
    BuildContextMenu(status, maintenance, Answer);
  
    // Compute the chosen option
    switch (Answer)
    {
      case 2:
        dpSetWait($datapoint + "_Rack"+ $RackNr + "_SubRack" +$SubrackNr +  "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value", 0);
        break;
      case 3:
        dpSetWait($datapoint + "_Rack"+ $RackNr + "_SubRack" +$SubrackNr +  "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + ".status:_original.._value", 1);
        break;
      case 10:
        dpSetWait($datapoint + "_Rack"+ $RackNr + "_SubRack" +$SubrackNr +  "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + "_Maintenance.status:_original.._value", 0);
        break;
      case 11:
        dpSetWait($datapoint + "_Rack"+ $RackNr + "_SubRack" +$SubrackNr +  "_Board0_AP"+ $APNr +"_RCU"+ $RCUNr +"_" + antenna + "_Maintenance.status:_original.._value", 1);
        break;
      default:
        break;
    }       
  }  
  else //$configDatapoint is not Accessable
  {
    dyn_string txt = makeDynString("PUSH_BUTTON, Configuration not possible, 2, 0");
    popupMenu(g_ContectMenuDpAccessableErrorText, Answer);
  }
}*/