
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
  string dpSystemName = strrtrim(dpSubStr(dpName,DPSUB_SYS),":");
  if(getSystemName()==(dpSystemName+":"))
  {
    return TRUE;
  }
  else if (getSystemName()!=(dpSystemName+":"))
  {
    dyn_int distributedSystems;
    dpGet("_DistManager.State.SystemNums", distributedSystems);
    //Check if dpSystemName is in the distributedSystems list
    if (dynlen(distributedSystems)>0)
    {
      for(int i=1; i<=dynlen(distributedSystems); i++)
      {
        if(getSystemName(distributedSystems[i])==(dpSystemName+":"))  //if the system is reacheable
        {
          return dpExists(dpName);
        }
      }
    }
    return FALSE;
  }
}
