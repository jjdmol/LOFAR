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
#uses "gcf-util.ctl"

global unsigned g_PAclientId = 0;  // client ID used during all communication with the PML
global string    NAVPML_DPNAME_ENABLED   = "__enabled";
global string    NAVPML_ENABLED_PERM     =  "permanent";
global string    NAVPML_ENABLED_TEMP     =  "temporary";
global string    NAVPML_ENABLED_AUTOLOAD =  "autoloaded";

///////////////////////////////////////////////////////////////////////////
//Function navPMLinitialize
//
// open the connection with the PML
//
///////////////////////////////////////////////////////////////////////////
void navPMLinitialize()
{
  LOG_TRACE("navPMLinitialize");
	g_PAclientId = gcfInit("pmlCallback");
}  

///////////////////////////////////////////////////////////////////////////
//Function navPMLterminate
//
// closes the connection with the PML
///////////////////////////////////////////////////////////////////////////
void navPMLterminate()
{
  LOG_TRACE("navPMLterminate");
	gcfLeave(g_PAclientId);
}  

///////////////////////////////////////////////////////////////////////////
//Function navPMLunloadPropertySet
//
// unloads the propertyset of the datapoint.
///////////////////////////////////////////////////////////////////////////
bool navPMLunloadPropertySet(string datapoint)
{
  LOG_TRACE("navPMLunloadPropertySet",datapoint);
  if(dpExists(datapoint))
  {
    if(!navPMLisAutoLoaded(datapoint))
    {
  	  gcfUnloadPS(g_PAclientId,datapoint);
    }
	}
	else
	{
	  LOG_TRACE("navPMLunloadPropertySet -- Datapoint does not exist",datapoint);
	}
}

///////////////////////////////////////////////////////////////////////////
//Function navPMLloadPropertySet
//
// loads the propertyset of the datapoint
///////////////////////////////////////////////////////////////////////////
void navPMLloadPropertySet(string datapoint)
{
  LOG_TRACE("navPMLloadPropertySet",datapoint);
  if(dpExists(datapoint))
  {
    if(!navPMLisAutoLoaded(datapoint))
    {
      gcfLoadPS(g_PAclientId,datapoint);
    }
	}
	else
	{
	  LOG_TRACE("navPMLloadPropertySet -- Datapoint does not exist",datapoint);
	}
}

///////////////////////////////////////////////////////////////////////////
//Function navPMLconfigurePropertySet
//
// configures the propertyset of the datapoint
///////////////////////////////////////////////////////////////////////////
void navPMLconfigurePropertySet(string psScope, string psApcName)
{
  LOG_TRACE("navPMLconfigurePropertySet",psScope,psApcName);
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
  LOG_TRACE("pmlCallback:",response);
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

///////////////////////////////////////////////////////////////////////////
//Function navPMLisAutoLoaded
//
// returns true if the propertyset is auto loaded
///////////////////////////////////////////////////////////////////////////
bool navPMLisAutoLoaded(string datapoint)
{
  bool autoLoaded=false;
  LOG_TRACE("navPMLisAutoLoaded",datapoint);
  if(dpExists(datapoint))
  {
    // check if the propertyset is autoloaded by GCF
    if(dpExists(datapoint + NAVPML_DPNAME_ENABLED))
    {
      string enabled="";
      dpGet(datapoint + NAVPML_DPNAME_ENABLED,enabled);
      if(enabled == NAVPML_ENABLED_AUTOLOAD)
      {
        autoLoaded = true;
      }
    }
  }
  else
 {
    LOG_TRACE("navPMLloadPropertySet -- Datapoint does not exist",datapoint);
  }
  return autoLoaded;
}
