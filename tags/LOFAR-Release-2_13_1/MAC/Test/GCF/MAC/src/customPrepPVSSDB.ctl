main()
{
  deleteDPs("TTypeA");
  deleteDPs("TTypeB");
  deleteDPs("TTypeC");
  deleteDPs("TTypeD");
  deleteDPs("TTypeE");
  deleteDPs("TTypeF");
  deleteDPs("TTypeG");
  dpCreate("A_K", "TTypeB");
  dpCreate("A_L", "TTypeE");
  dpCreate("B_A_BRD1", "TTypeF");
}

deleteDPs(string type)
{
  string dpName;
  dyn_string names = dpNames("*",type);  
  int i, len;
  len = dynlen(names);
  if (len > 0)
  {
    for (i = 1; i <= len; i++)
    {
      dpName = names[i];
      dpDelete(dpName);
      DebugN(dpName + " deleted");
    }
  }
}