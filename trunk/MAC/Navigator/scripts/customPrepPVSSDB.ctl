// this script must reside in the <PVSS project>/scripts directory
main()
{
  dyn_string DPNames;
  int namelen,loopidx;
  string dpName,enabledValue;
	
  // Clean the database from leftover points

  //get all __enabled points under GCFPaPsEnabled
  DPNames = dpNames("*__enabled","GCFPaPsEnabled");
  namelen = dynlen(DPNames);
	
  // Found GCFPaPsEnabled points. If the are temporarily we will check if the 
  // corresponding LOFAR treepoints also exist. If that is the case we will remove
  // those points, in either case the enabled points will be removed also.
  if (namelen > 0) {
    // be aware, dyn_arrays start with 1!
    for (loopidx=1;loopidx <= namelen; loopidx++) {
      dpName=DPNames[loopidx];
			
      // Get the _original_value from the __enabled datapoint
      if (dpGet(dpName+".",enabledValue) == 0 ){

        // split found value on -
        // values can be :
        // perm-autoloaded
        // temp-autoloaded
        // permanent
        // temporary 
        if (substr(enabledValue,0,4) == "temp") {
      	  // 
      	  // Temporary datapoint, if the matching LOFAR treepoint exists it
      	  // should be removed also
      	  //
      	  string newString = substr(dpName,0,strpos(dpName,"__enabled"));
      	  
      	  if (dpExists(newString) ) {
       	    // 
      	    // treepoint exists, so remove
      	    //
      	    deleteDP(newString);
      	  	
      	    // also set an indication in GCFPaPsIndication that a point has been
      	    // deleted. After all there might be Navigators looking at it...
      	    setGCFPaPsIndication("d",newString);
      	  }
 	}
        deleteDP(dpName);
      }
    }
  } else {
    DebugTN("No GCFPaPsEnabled points where found.");
  }
  createInitialPoints();
}

bool deleteDP(string aDPName) {
  if (dpDelete(aDPName) > 0) {
    DebugTN("Error removing datapoint");
    return false;
  } else {
    DebugTN(aDPName + " removed from database");
    return true;
  }
}

void setGCFPaPsIndication(string state,string aDPName) {
  if (dpExists("__pa_PSIndication")) {
    if (dpSet("__pa_PSIndication",state+"|"+aDPName) == 0 ) {
      return;
    }
  }
  DebugTN("Error setting __pa_PSIndication to " + state + "|"+aDPName);
}

void createInitialPoints() {

  //MainCU's have a dPType LOFAR, stations have StnLOFAR
  dyn_string ds=dpTypes("LOFAR");
 


  if (dynlen(ds) == 1 ){ 					// MainCU
    dpCreate("LOFAR__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR__enabled.","autoloaded|LOFAR");	

    dpCreate("LOFAR_PermSW__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PermSW__enabled.","autoloaded|PermSW");

    dpCreate("LOFAR_ObsSW__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_ObsSW__enabled.","autoloaded|ObsSW");

    dpCreate("LOFAR_PIC__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PIC__enabled.","autoloaded|PIC");

    dpCreate("LOFAR_PIC_Core__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PIC_Core__enabled.","autoloaded|Core");

    dpCreate("LOFAR_PermSW_Core__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PermSW_Core__enabled.","autoloaded|Core");

    dpCreate("LOFAR_PIC_Core_CS001__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PIC_Core_CS001__enabled.","autoloaded|CS001");

    dpCreate("LOFAR_PIC_Core_CS008__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PIC_Core_CS008__enabled.","autoloaded|CS008");

    dpCreate("LOFAR_PIC_Core_CS010__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PIC_Core_CS010__enabled.","autoloaded|CS010");

    dpCreate("LOFAR_PIC_Core_CS016__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PIC_Core_CS016__enabled.","autoloaded|CS016");

    dpCreate("LOFAR_PermSW_Core_CS001__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PermSW_Core_CS001__enabled.","autoloaded|CS001");

    dpCreate("LOFAR_PermSW_Core_CS008__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PermSW_Core_CS008__enabled.","autoloaded|CS008");

    dpCreate("LOFAR_PermSW_Core_CS010__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PermSW_Core_CS010__enabled.","autoloaded|CS010");

    dpCreate("LOFAR_PermSW_Core_CS016__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PermSW_Core_CS016__enabled.","autoloaded|CS016");

  } else {				        		// Station
    dpCreate("LOFAR__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR__enabled.","autoloaded|StnLOFAR");

    dpCreate("LOFAR_PermSW__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PermSW__enabled.","autoloaded|StnPermSW");

    dpCreate("LOFAR_ObsSW__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_ObsSW__enabled.","autoloaded|StnObsSW");

    dpCreate("LOFAR_PIC__enabled", "GCFPaPsEnabled");
    dpSet("LOFAR_PIC__enabled.","autoloaded|StnPIC");
  }
}
