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

#uses "nav_fw/gcfpa-pml.ctl"
#uses "nav_fw/gcf-util.ctl"

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
  if (g_PAclientId == 0) 
    g_PAclientId = gcfInit("pmlCallback");
  else
    LOG_WARN("PML already initialized");
  
  if (g_PAclientId == 0) 
  {
    LOG_FATAL("PML not properly initialized");
  }
  else
  {
    dpConnect("HandlePanelMessage", false, DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_MESSAGE);
    LOG_INFO("PML initialized. Using ID:", g_PAclientId);
  }
}  

void HandlePanelMessage(string dp, string msg)
{
  dyn_string splittedMsg = strsplit(msg, "|");  
  if ((dynlen(splittedMsg) == 2) && (splittedMsg[1] == "PML_UNLOAD"))
  {
    string datapoint = splittedMsg[2];
    if (!navPMLisAutoLoaded(datapoint))
    {
      LOG_TRACE("PML unload propertyset", g_PAclientId, datapoint);
      gcfUnloadPS(g_PAclientId, datapoint);
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//Function navPMLterminate
//
// closes the connection with the PML
///////////////////////////////////////////////////////////////////////////
void navPMLterminate(bool inTerminate = false)
{
  LOG_TRACE("navPMLterminate");
  if (g_PAclientId != 0)
  {
    gcfLeave(g_PAclientId, inTerminate);
    g_PAclientId = 0;
  }
}  

///////////////////////////////////////////////////////////////////////////
//Function navPMLloadPropertySet
//
// loads the propertyset of the datapoint
///////////////////////////////////////////////////////////////////////////
void navPMLloadPropertySet(string datapoint)
{
  LOG_TRACE("navPMLloadPropertySet", datapoint);
  if (!navPMLisAutoLoaded(datapoint))
  {
    gcfLoadPS(g_PAclientId, datapoint);
  }
}

///////////////////////////////////////////////////////////////////////////
//Function navPMLunloadPropertySet
//
// unloads the propertyset of the datapoint.
///////////////////////////////////////////////////////////////////////////
bool navPMLunloadPropertySet(string datapoint)
{
  LOG_TRACE("navPMLunloadPropertySet", datapoint);
  if (dpExists(datapoint))
  {
  	// in this way the context can be switched from the terminated panel to a still running 
  	// panel (the navigator)
    dpSet(DPNAME_NAVIGATOR + g_navigatorID + "." + ELNAME_MESSAGE, "PML_UNLOAD|" + datapoint);
  }
  else
  {
    LOG_TRACE("navPMLunloadPropertySet -- Datapoint does not exist", datapoint);
  }
}

///////////////////////////////////////////////////////////////////////////
//Function navPMLconfigurePropertySet
//
// configures the propertyset of the datapoint
///////////////////////////////////////////////////////////////////////////
void navPMLconfigurePropertySet(string psScope, string psApcName)
{
  LOG_TRACE("navPMLconfigurePropertySet", psScope, psApcName);
  gcfConfigurePS(g_PAclientId, psScope, psApcName);
}

///////////////////////////////////////////////////////////////////////////
// pmlCallback - this function is called by the PropertyAgent when a (un)load request
//               has finished. 
// response array contents:
// [1] : response ("loaded", "unloaded", "configured", "gone")
// [2] : datapoint name
// [3] : result ("OK", "failed")
///////////////////////////////////////////////////////////////////////////
void pmlCallback(dyn_string response)
{
  LOG_DEBUG("pmlCallback:", LOG_DYN(response));
  if (response[1] == "loaded")
  {
    if (response[3] == "OK")
    {
    }
    else
    {
    }
  }
  else if (response[1] == "unloaded")
  {
    if (response[3] == "OK ")
    {
    }
    else
    {
    }
  }
  else if (response[1] == "configured")
  {
    if (response[3] == "OK ")
    {
    }
    else
    {
    }
  }
  else if (response[1] == "gone")
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
  bool autoLoaded = false;
  LOG_TRACE("navPMLisAutoLoaded", datapoint);
  if (dpAccessable(datapoint))
  {
    // check if the propertyset is autoloaded by GCF
    if (dpAccessable(datapoint + NAVPML_DPNAME_ENABLED))
    {
      string enabled = "";
      dpGet(datapoint + NAVPML_DPNAME_ENABLED + ".", enabled);
      if (strpos(enabled, NAVPML_ENABLED_AUTOLOAD) == 0)
      {
        autoLoaded = true;
      }
    }
  }
  else
  {
    LOG_TRACE("navPMLloadPropertySet -- Datapoint does not exist", datapoint);
  }
  return autoLoaded;
}


///////////////////////////////////////////////////////////////////////////
//Function navPMLisTemporary
//
// returns true if the propertyset is temporary
///////////////////////////////////////////////////////////////////////////
bool navPMLisTemporary(string datapoint)
{
  bool temporary = false;

  // check if the propertyset is temporary by GCF
  if (strpos(datapoint, NAVPML_DPNAME_ENABLED) > 0)
  {
    if (dpAccessable(datapoint))
    {
      string enabled = "";
      dpGet(datapoint + ".", enabled);
      LOG_TRACE("navPMLisTemporary[content enabled]", enabled);
      if (strpos(enabled, NAVPML_ENABLED_TEMP) == 0)
      {
        temporary = true;
      }
    }
  }
  LOG_TRACE("navPMLisTemporary[T/F]", temporary);
  return temporary;
}



