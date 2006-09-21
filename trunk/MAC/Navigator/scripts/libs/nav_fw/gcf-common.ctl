//# gcf-common.ctl
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
//# Common GCF functions
//# 

#uses "nav_fw/gcf-util.ctl"

global dyn_int g_distributedSystems;
global int g_distSysVarSem = 0;

void gcfStartFollowDistSystems()
{
  g_distSysVarSem = 0;
  dpConnect("followDistributedSystems", "_DistManager.State.SystemNums");
}

void followDistributedSystems(string dp, dyn_int value)
{
  while (g_distSysVarSem > 0) delay(0, 10); // wait until the "semaphore" is freed by the dpAccessable method
  g_distributedSystems = value;
}

///////////////////////////////////////////////////////////////////////////
// Function dpAccessable: Checks whether the given dpName is accessable and
//                    existing (in case of a distributed system.
//
// Input : datapoint name, including systemName
// Output: TRUE, if accessable and existing,
//         FALSE, if not accessable and/or not existing
///////////////////////////////////////////////////////////////////////////
bool dpAccessable(string dpName)
{
  if (dpExists(dpName))
  {
    string dpSystemName = strrtrim(dpSubStr(dpName, DPSUB_SYS), ":");
    if (getSystemName() == (dpSystemName + ":"))
    {
      return TRUE;
    }
    else
    {
      g_distSysVarSem++; // set "semaphore"
      // check if the dist. system with the systemname from the dpName is reachable
      if (dynlen(g_distributedSystems) > 0)
      {
        // Check whether the first part of the dpName is a valid system name 
        // and the name of a reachable dist. system
        dyn_string splitedDPName = strsplit(dpName, ':');
        for (int i = 1; i <= dynlen(g_distributedSystems); i++)
        {
          if (getSystemName(g_distributedSystems[i]) == (splitedDPName[1] + ":"))
          {
            g_distSysVarSem--; // release "semaphore"
            return TRUE;
          }          
        }
      }
      g_distSysVarSem--; // release "semaphore"
      LOG_WARN("Not accessible: ",dpName);
      return FALSE;
    }
  }
  else
  {
    return FALSE;
  }
}
