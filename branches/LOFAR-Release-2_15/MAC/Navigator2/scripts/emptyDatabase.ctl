main() {
  string dpType;
  dyn_string types = dpTypes("*");
  int i, len, i2, len2;
  len = dynlen(types);
  if (len > 0) {
    for (i=1;i<= len; i++) { 
      dpType = types[i];

      // skip internal datapoints       
      if (substr(dpType,0,1) != "_" ) {

        // Skip original ETM points
        if (dpType == "ANALOG1" ||
            dpType == "ANALOG2" ||
            dpType == "BIT_CONDITION" ||
            dpType == "COUNTER_SUB" ||
            dpType == "COUNTER1" ||
            dpType == "DRIVE1" ||
            dpType == "DRIVE2" ||
            dpType == "ExampleDP_BarTrend" ||
            dpType == "ExampleDP_Bit" ||
            dpType == "ExampleDP_DDE" ||
            dpType == "ExampleDP_Float" ||
            dpType == "ExampleDP_Int" ||
            dpType == "ExampleDP_Text" ||
            dpType == "LABOR_ANALOG" ||
            dpType == "LABOR_COUNTER" ||
            dpType == "MODE_CMD" ||
            dpType == "MODE_STATE" ||
            dpType == "PUMP1" ||
            dpType == "PUMP2" ||
            dpType == "SETPOINT" ||
            dpType == "SLIDE_VALVE_HAND1" ||
            dpType == "SLIDE_VALVE1" ||
            dpType == "SLIDE_VALVE2" ||
            dpType == "WH_SC_SERVICE" ||
            dpType == "WH_SC_SUB" ||
            dpType == "WH_SC1") {

            DebugN("Skipped original ETM type: ",dpType);
        } else {
  	  string dpName;
          dyn_string names = dpNames("*",dpType);  
          len2 = dynlen(names);
  	  if (len2 > 0) {
    	    for (i2 = 1; i2 <= len2; i2++) {
      	      dpName = names[i2];
	      dpDelete(dpName);
  	      DebugN(dpName + " deleted");
    	    }
    	  }
    	  dpTypeDelete(dpType);
	  DebugN(dpType + " deleted");
        }
      }
    }
  }
}
