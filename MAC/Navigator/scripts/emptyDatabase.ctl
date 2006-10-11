main()
{
  string dpType;
  dyn_string types = dpTypes("*");
  int i, len, i2, len2;
  len = dynlen(types);
  if (len > 0)
  {
    for (i=1;i<= len; i++)
    { 
      dpType = types[i];
     	
     	if (substr(dpType,0,1) != "_" 
     	    && substr(dpType,0,7) != "ANALOG2"
     	    && substr(dpType,0,15) != "ExampleDP_Float" )
     	{
	     	DebugN("DPType: "+dpType);
				string dpName;
  		  dyn_string names = dpNames("*",dpType);  
        len2 = dynlen(names);
  			if (len2 > 0)
  			{
    			for (i2 = 1; i2 <= len2; i2++)
    			{
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