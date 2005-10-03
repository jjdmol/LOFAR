
///////////////////////////////////////////////////////////////////////////
// Function dpAccessable: Checks whether the given dpName is accessable and
//                    existing (in case of a distributed system.
//
// Input : datapoint name, including systemName
// Output: TRUE, if accessable and existing,
//         FALSE, if not accessable and/or not existing
///////////////////////////////////////////////////////////////////////////

global dyn_int g_distributedSystems;
global int g_distSysVarSem = 0;

void gcfStartFollowDistSystems()
{
  g_distSysVarSem = 0;
  dpConnect("followDistributedSystems", "_DistManager.State.SystemNums");
}

void followDistributedSystems(string dp, dyn_int value)
{
  while (g_distSysVarSem > 0) // wait until the "semaphore" is freed by the dpAccessable method
  ;
  g_distributedSystems = value;
}

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
      return FALSE;
    }
  }
  else
  {
    return FALSE;
  }
}
