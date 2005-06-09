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
//# All user panel scripts functions
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
        RootPanelOn("navigator/views/" + panelName,
                    panelName,
                    makeDynString("$datapoint:" + datapoint + addingPart));
        LOG_TRACE("Navigate to[undocked]: ",datapoint + addingPart);
      }
    }
  }
}


