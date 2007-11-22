main(string query="", string fromSys="", string genAsSys="", bool addMissingLevels=TRUE, bool addDPtype=TRUE)
{  
  dyn_dyn_anytype tab;
  string remote = "";
  
  if(strlen(query) == 0 ||
     strlen(fromSys) == 0)
  {
    fprintf(stderr,"Usage: PVSS00ctrl MACExport.ctl <filter> <fromSys> [genAsSys] [addMissingLevels] [addDPtype]\n");
    fprintf(stderr,"filter            : Filter DP's based on wildcards. E.g.: P?C*\n");
    fprintf(stderr,"fromSys           : PVSS system to export the data from\n");
    fprintf(stderr,"genAsSys          : Systemname prefix for the generated datapoints may be \"\"\n");
    fprintf(stderr,"addMissingLevels  : flag to add missing levels, true or false\n");
    fprintf(stderr,"addDPtype         : flag to add the DP type to the output, true or false\n");
    fflush(stderr);
  }
  else
  {
    fprintf(stderr,"Filter          : %s\n",query);
    fprintf(stderr,"FromSys         : %s\n",fromSys);
    fprintf(stderr,"GenAsSys        : %s\n",genAsSys);
    fprintf(stderr,"AddMissingLevels: %d\n",addMissingLevels);
    fprintf(stderr,"AddDPtype:      : %d\n",addDPtype);
    fprintf(stderr,"State: Perform query...");
    fflush(stderr);

    if (fromSys != getSystemName())
    {
      remote = " REMOTE '" + fromSys + "'";
      strreplace(remote, ":", "");
    } 
    dpQuery("SELECT '_online.._value' FROM '" + query + "'" + remote + 
            " WHERE _DPT != \"GCFPaPsEnabled\"" +
            " SORT BY 0 ASC",
            tab);

    string genAsSys = getSystemName();
    if (strlen(genAsSys == 0))
    {
      genAsSys = getSystemName();
    }

    string curDPE;
    dyn_string items;
    for(int i = 2; i <= dynlen(tab); i++)
    {
      curDPE = tab[i][1];
      dynAppend(items,dpSubStr(curDPE, DPSUB_DP_EL));
    }
    if (addMissingLevels)
    {
      fprintf(stderr,"State: Add missing levels...");
      fflush(stderr);

      dyn_string splittedPath;
      int pathPartToDelete;
      bool found;
      for(int i = 1; i <= dynlen(items); i++)
      {
        curDPE = items[i];
        splittedPath = strsplit(curDPE, "_");
        pathPartToDelete = dynlen(splittedPath);
        found = false;
        while (!found && pathPartToDelete > 1)
        {
          strreplace(curDPE, "_" + splittedPath[pathPartToDelete], "");         
          found = (dynlen(dynPatternMatch(curDPE + ".*", items)) > 0);
          if (!found)
          {
            dynAppend(items, curDPE + ".dummy");
            pathPartToDelete - 1;
          }
        }
        curDPE = items[i];
        splittedPath = strsplit(curDPE, ".");
        pathPartToDelete = dynlen(splittedPath);
        found = false;
        while (!found && pathPartToDelete > 2)
        {
          strreplace(curDPE, "." + splittedPath[pathPartToDelete], "");
          found = (dynContains(items, curDPE) > 0);
          if (!found)
          {
            dynAppend(items, curDPE);
            pathPartToDelete - 1;
          }
        }
      }
    }
    fprintf(stderr,"State: Sorting list...");
    fflush(stderr);

    dynSortAsc(items);
   
    int dpTypeId;
    fprintf(stderr,"State: Write list to stdout...");
    fflush(stderr);

    for(int i = 1; i <= dynlen(items); i++)
    {
      curDPE = items[i];
      if (addDPtype)
      {
        if (dpExists(curDPE))
        {
          dpTypeId = dpElementType(curDPE);
        }
        else if (strpos(curDPE, ".dummy") > 0)
        {
          dpTypeId = DPEL_INT;
        }
        else
        {
          dpTypeId = 0;
        }
        fprintfUL(stdout, "%d %s\n", (dpTypeId == DPEL_TYPEREF || dpTypeId == DPEL_STRUCT ? 0 : dpTypeId), genAsSys + curDPE);
        fflush(stdout);

      }
      else
      {
        fprintfUL(stdout, "%s\n", genAsSys + curDPE);
        fflush(stdout);
      }  
    }
    fprintf(stderr,"State: Ready/Idle");
    fflush(stderr);
  }
}
