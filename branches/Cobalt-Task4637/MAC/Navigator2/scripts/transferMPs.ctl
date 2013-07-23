#uses "da.ctl"

// To copy all _mp defined functionality (archives , dp_fct etc to the real DP's
// we use this script after setup (or each time that the scriptInfo.paramDone is set to false)
// we run this script
//
bool bDebug=false;

void main()
{
  // connect to debugflag to be able to switch debug on/off during run
  if (dpExists("scriptInfo.transferMPs.debug")) {
    dpConnect("debugCB",true,"scriptInfo.transferMPs.debug");
  } else {
    DebugTN("transferMPs.ctl:main|scriptInfo.transferMPs.runDone point not found in Database");  
  } 

  if (dpExists("scriptInfo.transferMPs.runDone")) {
    dpConnect("startTransferMP",true,"scriptInfo.transferMPs.runDone");
  } else {
    DebugTN("transferMPs.ctl:main|scriptInfo.transferMPs.runDone point not found in Database");  
  } 

  //Check all .state and .childState points in the database if they are invalid, invalid points mean that they never have been initialised and thus are new
  // in the database. These will be set to operational initialy
  if (setOperational()) {
    DebugTN("transferMPs.ctl:main|set new points to Operational done");  
  } else {
    DebugTN("transferMPs.ctl:main|set new points to Operational failed");
  }
}

private void debugCB(string dp1, bool debug) {
  if (bDebug != debug) bDebug=debug;
}

// Transfer all _mp_settings to all DP's
private void startTransferMP(string dp1, bool done ) {
  int        i, j, k, l, ll, l1, iError;
  dyn_float  df;
  dyn_string dsConfigs, ds;
  string     sDestinationDPE;
  
  if (done) return;
  
  DebugTN("transferMPs.ctl:main|start transfer of MPconfigs to all DP's");     

    
  string query="SELECT '_original.._value' FROM '_mp_*'";
  dyn_dyn_anytype tab;
  dyn_string dps;
  dpQuery(query,tab);
  string olddp="";
  for(int z=2;z<=dynlen(tab);z++) {
    string dp = dpSubStr(tab[z][1],DPSUB_DP);
    if (olddp != dp) {
      olddp=dp;
      if (!skipDP(dp)) {
        dynAppend(dps,dp);
      }
    }
  }

  for (k=1;k <= dynlen(dps); k++) {
    string dpstr = dps[k];
    dyn_string dsDpes = dpNames( dpstr + ".**"),
               dsDps = dpNames("*",dpTypeName(dpstr));
  
    // no datapoints found
    if ( dynlen(dsDps) > 1 ) {
    
      l = dynlen(dsDpes);
      l1 = dynlen(dsDps);
      ll = l * (l1 - 1);
      // !!! dynlen(dsDpes) * dynlen(dsDps) verwenden mit der 2-Sek-Verzögerung

      for ( i = 1; i <= l; i++ ) {
        if ( strpos(dsDpes[i],".") < 1 ) dsDpes[i] += ".";
        dsConfigs = dpNames( dsDpes[i] + ":*" );
        for ( j = dynlen(dsConfigs); j > 0; j--) {
          strreplace(dsConfigs[j], dsDpes[i] + ":", "");
        }
        if ( dynlen(dsConfigs) < 1 ) continue;
    
        for ( j = 1; j <= dynlen(dsDps); j++ ) {

          if ( dpSubStr(dsDps[j],DPSUB_DP) == dpSubStr(dsDpes[i],DPSUB_DP) ) {
            continue;
          }
          sDestinationDPE = dsDpes[i];
          strreplace( sDestinationDPE, dpSubStr(dsDpes[i], DPSUB_DP), dpSubStr(dsDps[j], DPSUB_DP));
          daCheckDPE(sDestinationDPE);

          dpCopyConfig(dsDpes[i], sDestinationDPE, dsConfigs, iError);
        }
      }
    }
  }
  dpSet("scriptInfo.transferMPs.runDone",true);
  DebugTN("MPTransfer Done.");
}

private bool setOperational() {

  dyn_dyn_anytype tab;
  string dp="";
  string query="";
  int z;
  int aVal;
  int err;

  query = "SELECT '_online.._invalid' FROM '{**.**.status.childState,**.**.status.state}' WHERE '_online.._invalid' == 1";
 
  if (bDebug) DebugN("transferMPs.ctl:setOperational|Query: ",query);
  err = dpQuery(query, tab);
   
  if (err < 0) {
    if (bDebug) DebugN("transferMPs.ctl:setOperational|Error " + err + " while getting query.");
    return false;
  }
 
  for(z=2;z<=dynlen(tab);z++) {

    dp = tab[z][1];
    // filter out the _mp_ points
    if (strpos(dp,"_mp_") < 0) {
      dpSet(dp,10);
    }
  }
  return true;
}

bool skipDP(string dp) {
// We only want to look at our own DB points that have a _mp_ sadly enough PVSS 
// keeps some of their own datapoints in the database, and there is no way to 
// determine what are theirs and what are or own points, other then a hard compare.
  if (strpos(dp,"_mp__") > -1) return true;
  if (strpos(dp,"WH_SC_SUB") > -1) return true;
  if (strpos(dp,"WH_SC1") > -1) return true;
  if (strpos(dp,"ANALOG1") > -1) return true;
  if (strpos(dp,"ANALOG2") > -1) return true;
  if (strpos(dp,"DRIVE1") > -1) return true;
  if (strpos(dp,"DRIVE2") > -1) return true;
  if (strpos(dp,"SETPOINT") > -1) return true;
  if (strpos(dp,"SLIDE_VALVE1") > -1) return true;
  if (strpos(dp,"SLIDE_VALVE2") > -1) return true;
  if (strpos(dp,"PUMP2") > -1) return true;
  if (strpos(dp,"PUMP1") > -1) return true;
  if (strpos(dp,"BIT_CONDITION") > -1) return true;
  if (strpos(dp,"COUNTER1") > -1) return true;
  if (strpos(dp,"COUNTER_SUB") > -1) return true;
  if (strpos(dp,"LABOR_ANALOG") > -1) return true;
  if (strpos(dp,"LABOR_COUNTER") > -1) return true;
  if (strpos(dp,"SLIDE_VALVE_HAND1") > -1) return true;
  if (strpos(dp,"WH_SC_SERVICE") > -1) return true;
  if (strpos(dp,"MODE_CMD") > -1) return true;
  return false;
}
