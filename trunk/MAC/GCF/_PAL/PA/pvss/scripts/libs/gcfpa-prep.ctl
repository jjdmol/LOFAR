main()
{
	dyn_dyn_string xxdepes;
	dyn_dyn_int xxdepei;
	dyn_string types;
	types = dpTypes("LPT_*");
	
	handleType(types, "LPT_CHAR", DPEL_CHAR);
	handleType(types, "LPT_STRING", DPEL_STRING);
	handleType(types, "LPT_BOOL", DPEL_BOOL);
	handleType(types, "LPT_BLOB", DPEL_BLOB);
	handleType(types, "LPT_DOUBLE", DPEL_FLOAT);
	handleType(types, "LPT_INTEGER", DPEL_INT);
	handleType(types, "LPT_UNSIGNED", DPEL_UINT);
	handleType(types, "LPT_DYNCHAR", DPEL_DYN_CHAR);
	handleType(types, "LPT_DYNSTRING", DPEL_DYN_STRING);
	handleType(types, "LPT_DYNBOOL", DPEL_DYN_BOOL);
	handleType(types, "LPT_DYNBLOB", DPEL_DYN_BLOB);
	handleType(types, "LPT_DYNDOUBLE", DPEL_DYN_FLOAT);
	handleType(types, "LPT_DYNINTEGER", DPEL_DYN_INT);
	handleType(types, "LPT_DYNUNSIGNED", DPEL_DYN_UINT);

	types = dpTypes("GCF*");
	if (!dynContains(types, "GCFTempRef"))
	{
		xxdepes[1] = makeDynString ("GCFTempRef");
		xxdepei[1] = makeDynInt (DPEL_STRING);
		dpTypeCreate(xxdepes,xxdepei);
	}
	else
	{
  	deleteDPs("GCFTempRef");
	}

	if (!dynContains(types, "GCFDataType"))
	{
		xxdepes[1] = makeDynString ("GCFDataType", "");
		xxdepes[2] = makeDynString ("", "panelAdmin");
		xxdepes[3] = makeDynString ("", "permanentStatus");
		xxdepei[1] = makeDynInt (DPEL_STRUCT);
		xxdepei[2] = makeDynInt (0, DPEL_DYNSTRING);
		xxdepei[2] = makeDynInt (0, DPEL_BOOL);
		dpTypeCreate(xxdepes,xxdepei);
	}

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