main()
{
	dyn_dyn_string xxdepes;
	dyn_dyn_int xxdepei;
	dyn_string types;

	types = dpTypes("GCF*");
	handleType(types, "GCFPaPsEnabled", DPEL_STRING);
	handleType(types, "GCFPaPsIndication", DPEL_STRING);
	handleType(types, "GCFDistPort", DPEL_BLOB);
	string type = "GCFWatchDog";
	if (dynContains(types, type))
	{
		deleteDPs(type);
	}
	xxdepes[1] = makeDynString (type, "");
	xxdepes[2] = makeDynString ("", "sys");
	xxdepes[3] = makeDynString ("", "man");
	xxdepei[1] = makeDynInt (DPEL_STRUCT);
	xxdepei[2] = makeDynInt (0, DPEL_STRING);
	xxdepei[3] = makeDynInt (0, DPEL_STRING);
	dpTypeCreate(xxdepes,xxdepei);
	DebugN("Add type " + type);
	dpCreate("__gcf_wd", type);
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
      if (dpName != getSystemName() + "__gcfportAPI_DPAserver")
      {
      	dpDelete(dpName);
      	DebugN(dpName + " deleted");
      }
    }
  }
  dpTypeDelete(type);
}

void handleType(dyn_string types, string type, int dpelType)
{
	if (dynContains(types, type))
	{
		deleteDPs(type);
	}
	dyn_dyn_string xxdepes;
	dyn_dyn_int xxdepei;
	xxdepes[1] = makeDynString (type);
	xxdepei[1] = makeDynInt (dpelType);
	dpTypeCreate(xxdepes,xxdepei);
	DebugN("Add type " + type);
}