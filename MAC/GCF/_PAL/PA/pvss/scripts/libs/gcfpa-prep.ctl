main()
{
	dyn_dyn_string xxdepes;
	dyn_dyn_int xxdepei;
	dyn_string types;

	types = dpTypes("GCF*");
	handleType(types, "GCFPaPsEnabled", DPEL_BOOL);
	handleType(types, "GCFPaPsIndication", DPEL_STRING);
	handleType(types, "GCFDistPort", DPEL_BLOB);
	handleType(types, "GCFWDGoneSys", DPEL_UINT);
	dpCreate("__gcf_WDGoneSys", "GCFWDGoneSys");
	dpCreate("__pa_PSIndication", "GCFPaPsIndication");
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
      if (dpName != getSystemName() + "__gcf_DPA_server")
      {
      	dpDelete(dpName);
      	DebugN(dpName + " deleted");
      }
    }
  }
}

void handleType(dyn_string types, string type, int dpelType)
{
	if (!dynContains(types, type))
	{
		dyn_dyn_string xxdepes;
		dyn_dyn_int xxdepei;
		xxdepes[1] = makeDynString (type);
		xxdepei[1] = makeDynInt (dpelType);
		dpTypeCreate(xxdepes,xxdepei);
		DebugN("Add type " + type);
	}
	else
	{
		deleteDPs(type);
	}
}