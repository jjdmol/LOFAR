//# gcfnav-pmlinterface.ctl
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
//# This PVSS control script provides an interface to the gcf pml scripts. It hides 
//# the administration of clientID's from the caller
//#

#uses "gcfpa-pml.ctl"

global unsigned g_PAclientId = 0;  // client ID used during all communication with the PML

///////////////////////////////////////////////////////////////////////////
//Function navPMLinitialize
//
// open the connection with the PML
//
///////////////////////////////////////////////////////////////////////////
void navPMLinitialize()
{
  DebugTN("navPMLinitialize");
	g_PAclientId = gcfInit("pmlCallback");
}  

///////////////////////////////////////////////////////////////////////////
//Function navPMLterminate
//
// closes the connection with the PML
///////////////////////////////////////////////////////////////////////////
void navPMLterminate()
{
  DebugTN("navPMLterminate");
	gcfLeave(g_PAclientId);
}  

///////////////////////////////////////////////////////////////////////////
//Function navPMLunloadPropertySet
//
// unloads the propertyset of the datapoint.
///////////////////////////////////////////////////////////////////////////
bool navPMLunloadPropertySet(string datapoint)
{
  DebugTN("navPMLunloadPropertySet",datapoint);
  gcfUnloadPS(g_PAclientId,datapoint);
}

///////////////////////////////////////////////////////////////////////////
//Function navPMLloadPropertySet
//
// loads the propertyset of the datapoint
///////////////////////////////////////////////////////////////////////////
void navPMLloadPropertySet(string datapoint)
{
  DebugTN("navPMLloadPropertySet",datapoint);
  gcfLoadPS(g_PAclientId,datapoint);
}

///////////////////////////////////////////////////////////////////////////
//Function navPMLconfigurePropertySet
//
// configures the propertyset of the datapoint
///////////////////////////////////////////////////////////////////////////
void navPMLconfigurePropertySet(string psScope, string psApcName)
{
  DebugTN("navPMLconfigurePropertySet",psScope,psApcName);
  gcfConfigurePS(g_PAclientId,psScope,psApcName);
}

///////////////////////////////////////////////////////////////////////////
// pmlCallback - this function is called by the PropertyAgent when a (un)load request
//               has finished. 
// response array contents:
// [1] : response ("loaded","unloaded","configured","gone")
// [2] : datapoint name
// [3] : result ("OK","failed")
///////////////////////////////////////////////////////////////////////////
void pmlCallback(dyn_string response)
{
  DebugTN("pmlCallback:",response);
  if(response[1] == "loaded")
  {
    if(response[3] == "OK")
    {
    }
    else
    {
    }
  }
  else if(response[1] == "unloaded")
  {
    if(response[3] == "OK ")
    {
    }
    else
    {
    }
  }
  else if(response[1] == "configured")
  {
    if(response[3] == "OK ")
    {
    }
    else
    {
    }
  }
  else if(response[1] == "gone")
  {
  }
  else
  {
  }
}

