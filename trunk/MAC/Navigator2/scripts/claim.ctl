// *******************************************
// Name : ClaimManager
// *******************************************
// Description:
//    This is the script that reacts to a claim.
//    A 'claim' is used to retrieve the name of a previously allocated
//    'temporary' datapoint
//
//    A claim must be made via:
//
//    dpSet( 
//      "ClaimManager.request.typeName", "Observation",
//      "ClaimManager.request.newObjectName", "MYOBSERVATION" );
//
//
//    The Claim Manager has a global array of claimed datapoints
//    for better performance. If the claimmanager runs on the mainserver
//    the manager has write rights and is responsible to fill the cache values
//    dpSet("ClaimManager.cache.typeNames",*add the new typename to the dyn_string*,
//          "ClaimManager.cache.newObjectNames",*add the new ObjectName to the dyn_string*,
//          "ClaimManager.cache.DPNames",*add the new DPName to the dyn_string*,
//          "ClaimManager.cache.claimDates",*add the new ClaimDate to the dyn_string*)
//
//    When a client connects to the maindatabase he will fill his internal cache map (gClaimdTypes)
//    from the ClaimManager.Cache  arrays.  This will also be done after a disconnect/reconnect
//    The client will take a dpConnect on the ClaimManager.response fields and will update his 
//    internal cache map via the callback from this also (if the point doesn't exist yet)
//
//    To check distributed connections a dpconnect on the distmanager is needed for the clients
//
// Returns:
//    None
//
// *******************************************

#uses "GCFCommon.ctl"

global mapping g_ClaimedTypes;

global bool bDebug = false;
// needed for dpDisconnect after a connect and a resync
global bool isConnected = false;
// is the machine we are running from a client or not
global bool isClient = true;
// contains the old known system to be able to check of the dist callback was caused by the master
global dyn_int oldSystemList;
// contains all known claimable DPTypes
global dyn_string claimableTypes;


void main()
{
  
  DebugN( "***************************" );
  DebugN( "ClaimManager is starting   " );
  DebugN( "***************************" );
  
  // append all Claimable types, for now only Observations needed
  dynAppend(claimableTypes,"Observation");
  
  
  if (bDebug){
    DebugN("SystemID: "+ getSystemId() + " MainDBName: " + MainDBName );
    DebugN("SystemID(MainDBName) : " + MainDBID + "GetSystemName: "+getSystemName());
  }
  // Find out if we are a client or a master system
  if (getSystemId() == MainDBID) {
    if (bDebug) DebugN("Running on Master System");
    isClient=false;
  } else {
      if (bDebug) DebugN("Running on Client System");
  }
  
  // check if datapoints for all types are available, if not, then create them
  checkAndCreateDPs();
  
  if (isClient) {
  	// Routine to connect to _DistConnections.ManNums.
  	// This point keeps a dyn_int array with all active distributed connections
  	// and will generate a callback everytime a station goes off-, or on- line

  	if (dpExists("_DistConnections.Dist.ManNums")) {
    	dpConnect("distSystemTriggered",true,"_DistConnections.Dist.ManNums");
  	} else {
    	DebugN("_DistConnections point not found, no trigger available for dist System updates.");  
  	}    
  } else {
    startLocalClaim();
  }  
}

// *******************************************
// Name : distSystemTriggered
// *******************************************
// Description:
// Callback that a distSystem has connected/disconnected
//
// we have to look if it was triggered by the master or another station
// if it was the master we need a resync of the g_ClaimedTypes map to avoid 
// differences
//
// Returns:
//    None
// *******************************************
void distSystemTriggered(string dp1, dyn_int systemList) {

  if (bDebug) DebugN("claim.ctl:distSystemTriggered |distSystemTriggered: "+ systemList);
  if (bDebug) DebugN("claim.ctl:distSystemTriggered |oldList: "+ oldSystemList);
  
  // both lists contained the MainSystem, nothing to be done
  if ((dynContains(oldSystemList,MainDBID) > 0) && 
      (dynContains(systemList,MainDBID) > 0 )) {
     if (bDebug) DebugN("Master in both lists available, nothing to be done");
    oldSystemList = systemList;
    return;
  }
  
  //oldlist contains master, but new list not ==> master disappeared 
  if ((dynContains(oldSystemList,MainDBID) > 0 ) &&
      (dynContains(systemList,MainDBID) < 1 )) {
     if (bDebug) DebugN("Master went offline, connection to MasterDB lost");
    oldSystemList = systemList;
    return;
  }
      
  //oldlist contains no master, nor does new list  ==> nothing to be done 
  if ((dynContains(oldSystemList,MainDBID) < 1 ) &&
      (dynContains(systemList,MainDBID) < 1 )) {
     if (bDebug) DebugN("Master in no list avaliable, nothing to be done");
    oldSystemList = systemList;
    return;
  }

  oldSystemList = systemList;
  // in the other case the oldList didn't contain the master, while the new list did contain it.
  // this indicates that the master was offline and did reconnect. We will have to resync the 
  // g_ClaimedTypes internal mapping and (re) connect to the ClaimManager.Response DP
 
  if (bDebug) DebugN("claim.ctl:distSystemTriggered |client triggered with Master, so sync needed");
  syncClient();
  
  
  if (isConnected) {
    dpDisconnect("clientAddClaimCallback",
    	MainDBName+"ClaimManager.response.typeName", 
    	MainDBName+"ClaimManager.response.newObjectName",
    	MainDBName+"ClaimManager.response.DPName",
    	MainDBName+"ClaimManager.response.claimDate"); 
  }
  
  // Connect to the dp elements that we use to receive
  // a new claim in the MainDB
  dpConnect( "clientAddClaimCallback", 
    true,         
    MainDBName+"ClaimManager.response.typeName", 
    MainDBName+"ClaimManager.response.newObjectName",
    MainDBName+"ClaimManager.response.DPName",
    MainDBName+"ClaimManager.response.claimDate"); 
  isConnected = true; 
  
  // start the local Claim mechanism
  startLocalClaim();
} 

// *******************************************
// Name : syncClient
// *******************************************
// Description:
//    This will (re)load the internal mapping from the 
//    ClaimManager.cache strings 
//
// Returns:
//    None
// *******************************************
void syncClient() {

  dyn_string typeNames;
  dyn_string newObjectNames;
  dyn_string DPNames;
  dyn_time   claimDates;
  mapping    tmp;
  
  if (bDebug) DebugN("claim.ctl:syncClient|client sync entered.");

  // empty the mapping
  mappingClear(g_ClaimedTypes);
  
  // because it always takes some time for the databases to be ready while the are connecting we will have to wait here till the MainDB 
  // is fully reachable
  int retry=0;
  while (!dpExists(MainDBName+"ClaimManager.cache.typeNames") & retry < 120) {
    delay(2);
    retry++;
    if (retry >= 120) {
      if (bDebug) DebugN("claim.ctl:syncClient| connecting to mainDB retry longer then 2 minutes, mainDB still not ready?");
      break;
    }
  }	
      
  // get all lists of claimed datapoints 
  dpGet(MainDBName+"ClaimManager.cache.typeNames",typeNames,
        MainDBName+"ClaimManager.cache.newObjectNames",newObjectNames,
        MainDBName+"ClaimManager.cache.DPNames",DPNames,
        MainDBName+"ClaimManager.cache.claimDates",claimDates);
  
  for (int t = 1 ; t <= dynlen(typeNames); t++) {
    
    // if the found type is not available in the local mapping, claim a spot
    // and fill it.
    if( !mappingHasKey( g_ClaimedTypes, typeNames[t] )) {
      g_ClaimedTypes[ typeNames[t] ] = tmp;
  
  	  // Allocate the right places in our global variable and fill it
  	  g_ClaimedTypes[ typeNames[t] ][ "DP"        ] = makeDynString(DPNames[t]);
  	  g_ClaimedTypes[ typeNames[t] ][ "NAME"      ] = makeDynString(newObjectNames[t]);
  	  g_ClaimedTypes[ typeNames[t] ][ "CLAIMDATE" ] = makeDynTime(claimDates[t]);
    } else {
    
    // we did have the type, so we can add this entry to it
      dynAppend(g_ClaimedTypes[ typeNames[t] ][ "DP"        ],DPNames[t]);
      dynAppend(g_ClaimedTypes[ typeNames[t] ][ "NAME"      ],newObjectNames[t]);
      dynAppend(g_ClaimedTypes[ typeNames[t] ][ "CLAIMDATE" ],claimDates[t]);
    } 
    if (bDebug) DebugN("Setting DPNames[t].claim.name : "+dpSubStr(DPNames[t],DPSUB_DP) + " --> "+newObjectNames[t]);
    dpSet(dpSubStr(DPNames[t],DPSUB_DP)+".claim.name",newObjectNames[t],dpSubStr(DPNames[t],DPSUB_DP)+".claim.claimDate",claimDates[t]); 
  }
  showMapping("syncClient");

}

// *******************************************
// Name : clientAddClaimCallback
// *******************************************
// Description:
//    This is the callback that is triggered
//    when the master gives out a new Claim.
//    we need to determine if the combo was
//    allready in the map. If not we need to 
//    add it.
//
//    if it was we might need to refresh the 
//    Claimdate or overwrite newObjectName for reuse.
//
// Returns:
//    None
// *******************************************
    
void clientAddClaimCallback(
  string strDP1, string typeName,
  string strDP2, string newObjectName,
  string strDP3, string DPName,
  string strDP4, time   claimDate
)
{
  
  if (bDebug) DebugN("claim.ctl:clientAddClaimCallback| entered.");
  if (bDebug) DebugN("claim.ctl:clientAddClaimCallback|typeName  : " + typeName);
  if (bDebug) DebugN("claim.ctl:clientAddClaimCallback|objectName: " + newObjectName);
  if (bDebug) DebugN("claim.ctl:clientAddClaimCallback|DPName    : " + DPName);
  if (bDebug) DebugN("claim.ctl:clientAddClaimCallback|claimDate : " + claimDate);
  // local data
  mapping tmp;
  int index = -1;

  if (DPName == "") {
    if (bDebug) DebugN("claim.ctl:clientAddClaimCallback|Empty DPName, no claim needed");
    return;
  } 
  if( mappingHasKey( g_ClaimedTypes, typeName )){  // when we know the type
    // if length is 0 then we just need to add
    if( !dynlen( g_ClaimedTypes[ typeName ][ "DP" ] )) {
        if (bDebug) DebugN("claim.ctl:clientAddClaimCallback|Adding entry because 0 entries in type");
	  	dynAppend(g_ClaimedTypes[ typeName ][ "DP"        ],DPName);
  		dynAppend(g_ClaimedTypes[ typeName ][ "NAME"      ],newObjectName);
  		dynAppend(g_ClaimedTypes[ typeName ][ "CLAIMDATE" ],claimDate); 
    } else {
       
  		// Look at all elements to see if you can find a claimed object
  		// that has the right name
  		for( int t = 1; (t <= dynlen( g_ClaimedTypes[ typeName ][ "DP" ] )) && (index == -1 ); t++) {
 				// if DPname && ObjectName is the same we only need to set the new claimDate
        if ((g_ClaimedTypes[ typeName ][ "DP"   ][t] == DPName) &&
            (g_ClaimedTypes[ typeName ][ "NAME" ][t] == newObjectName)) {

          if (bDebug) DebugN("claim.ctl:clientAddClaimCallback|DPName & ObjectName same, so only refresh claimdate");
          
          dynRemove(g_ClaimedTypes[ typeName ][ "CLAIMDATE" ],t);
          dynInsertAt(g_ClaimedTypes[ typeName ][ "CLAIMDATE" ],claimDate,t);
          index=t;
          
        // in case of aDPname is the same but the newObjectName differs then the spot 
        // would have to be reclaimed, so we fill in the new Object Name
        } else if ( (g_ClaimedTypes[ typeName ][ "DP"   ][t] == DPName) &&
            				(g_ClaimedTypes[ typeName ][ "NAME" ][t] != newObjectName)) {
          if (bDebug) DebugN("claim.ctl:clientAddClaimCallback|Found existing DP, but new Name, so overwrite to reuse");
          dynRemove(  g_ClaimedTypes[ typeName ][ "CLAIMDATE" ],t);
          dynInsertAt(g_ClaimedTypes[ typeName ][ "CLAIMDATE" ],claimDate,t);
          dynRemove(  g_ClaimedTypes[ typeName ][ "NAME"      ],t);
          dynInsertAt(g_ClaimedTypes[ typeName ][ "NAME"      ],newObjectName,t);
          index=t;
        }
      }
      int maxMapSize;
      // get all dpNames from this type to be able to check if the map doesn't go out of size (indicates an error)
      if (isClient) {
  	    maxMapSize = dynlen(dpNames("*","Stn"+typeName));
      } else {
	      maxMapSize = dynlen(dpNames("*",typeName));                    
      }
                
      // if index still -1, the name was not found and the entry can be added to the map
      if (index == -1 ) {
        // if length goes over maxMapSize there has been en error sometimes. Since the master
        // claimManager should take care that (for now, might be different in the future)
        if (dynlen( g_ClaimedTypes[ typeName ][ "DP"        ]) >= maxMapSize+1) {
          DebugN("claim.ctl:clientAddClaimCallback|ERROR!!!!!, internal claim mapping will exceed maxMapSize( "+maxMapSize+")");
        } else {
          if (bDebug) DebugN("claim.ctl:clientAddClaimCallback|No matches found, so just append to mapping");
   
	  	    dynAppend(g_ClaimedTypes[ typeName ][ "DP"        ],DPName);
  		    dynAppend(g_ClaimedTypes[ typeName ][ "NAME"      ],newObjectName);
  		    dynAppend(g_ClaimedTypes[ typeName ][ "CLAIMDATE" ],claimDate); 
        }
      } 
    }
  } else {
    if (typeName != "") {
	  	if( bDebug ) DebugN( "claim.ctl:clientAddClaimCallback|clientClaim() allocating new element in mapping because type wasn't found yet" );
  	  g_ClaimedTypes[ typeName ] = tmp;
  
  		// Allocate the right places in our global variable
  		g_ClaimedTypes[ typeName ][ "DP"        ] = makeDynString(DPName);
  		g_ClaimedTypes[ typeName ][ "NAME"      ] = makeDynString(newObjectName);
  		g_ClaimedTypes[ typeName ][ "CLAIMDATE" ] = makeDynTime(claimDate); 
    }
    dpSet(dpSubStr(DPName,DPSUB_DP)+".claim.name",newObjectName,dpSubStr(DPName,DPSUB_DP)+".claim.claimDate",claimDate); 
  }  
  
  if (bDebug) showMapping("clientAddClaimCallback");
}

// *******************************************
// Name : startLocalClaim
// *******************************************
// Description:
// Here the  claiming mechanism will start
// Obvious it's the same for master as well as client, 
// since they bove have their own internal mapping.
//
// Returns:
//    None
// *******************************************
void startLocalClaim() {

  // Connect to the dp elements that we use to receive
  // a new claim
  dpConnect( "claimCallback", 
    false,         
    "ClaimManager.request.typeName", 
    "ClaimManager.request.newObjectName" ); 

}

// *******************************************
// Name : claimCallback
// *******************************************
// Description:
//    This is the callback that is triggered
//    when a client does a claim.
//
//    A claim must be made via:
//
//    dpSet( 
//      "ClaimManager.request.typeName", "Observation",
//      "ClaimManager.request.newObjectName", "MYOBSERVATION" );
//
// Returns:
//    None
// *******************************************
    
void claimCallback(
  string strDP1, string strTypeName,
  string strDP2, string strNewObjectName   
)
{
  // Local data
  string strDP="";
  time claimDate;
  bool error=false;
  
  if( bDebug ) DebugN( "claim.ctl:claimCallback| request for " + strTypeName + "," + strNewObjectName );
  
  // (1) We have a new claim
  // First verify the datapoint type 
  if (isClient) {
    if( ! mappingHasKey( g_ClaimedTypes, strTypeName )) {
      DebugN("claim.ctl:claimCallback|ERROR!!!!  Client was asked for DPtype: "+strTypeName+" But has no such type local.");
      error=true;    
    }
  } else {
    if (bDebug) DebugN("claim.ctl:claimCallback|Check if Type is in mapping"); 
   	VerifyDatapointType( strTypeName );
  }
  
  // (2) We should have the right type. ( from step (1) )
  // now look for a free place
  if (!error) {
    strDP = FindFreeClaim( strTypeName, strNewObjectName, claimDate );
          
    // (3) Set the 'ClaimDate' of the datapoint
    // that we've found and the master should add the response to the Cache arrays
    if (!isClient && strDP != "") {
      
      if (bDebug) DebugN("claim.ctl:claimCallback|Master Cache action required");
      dyn_string typeNames;
      dyn_string newObjectNames;
      dyn_string DPNames;
      dyn_time   claimDates;

      // get all lists of claimed datapoints 
      dpGet("ClaimManager.cache.typeNames",typeNames,
        		"ClaimManager.cache.newObjectNames",newObjectNames,
        		"ClaimManager.cache.DPNames",DPNames,
        		"ClaimManager.cache.claimDates",claimDates); 
      
      // we need to check if the data is allready in the cacheArrays, to avoid duplication.
      // same type and same name and same dpname  only the claimdate might need to be changed.
      // same type and same dpName but different name means it is reused.
      // else we need to append
      
      bool found = false;
      for (int i=1; i<= dynlen(typeNames); i++) {
        
        // same only claimdate can be altered
        if (typeNames[i] == strTypeName &&
            newObjectNames[i] == strNewObjectName &&
            DPNames[i] == dpSubStr(strDP,DPSUB_DP) && !found) {
          
          if (bDebug) DebugN("claim.ctl:claimCallback|Found type and Objectname and DPname are the same, refresh Cache entry Claimdate");
          dynRemove(  claimDates,i); 
          dynInsertAt(claimDates,claimDate,i);
          found = true;
          exit;
        }
        
        // item has been reused
 	      if (typeNames[i] == strTypeName &&
   	        newObjectNames[i] != strNewObjectName &&
     	      DPNames[i] == dpSubStr(strDP,DPSUB_DP) && !found) {
                
          if (bDebug) DebugN("claim.ctl:claimCallback|type and DPname are the same, but ObjectName is different, Cache entry has been reused");
                
       	  dynRemove(  claimDates,i);                
       	  dynInsertAt(claimDates,claimDate,i);
         	dynRemove(  newObjectNames,i);
         	dynInsertAt(newObjectNames,strNewObjectName,i);
          found=true;
          exit;
       	}    
   		}       
      
      if (!found) {
      
        if (bDebug) DebugN("claim.ctl:claimCallback|Claim needs to be added to Cache"); 
	      // append new info
  	    dynAppend(typeNames,strTypeName);
    	  dynAppend(newObjectNames,strNewObjectName);
      	dynAppend(DPNames,dpSubStr(strDP,DPSUB_DP));
      	dynAppend(claimDates,claimDate);
      }
      
      // write back
      dpSet("ClaimManager.cache.typeNames",typeNames,
        		"ClaimManager.cache.newObjectNames",newObjectNames,
        		"ClaimManager.cache.DPNames",DPNames,
        		"ClaimManager.cache.claimDates",claimDates);  
    }
    
    if (bDebug) DebugN("claim.ctl:claimCallback|Set ClaimManagers Response point");
    // for master and client, if not strDP == empty
    dpSet(
      "ClaimManager.response.DPName"       , dpSubStr(strDP,DPSUB_DP),
      "ClaimManager.response.typeName"     , strTypeName,
      "ClaimManager.response.newObjectName", strNewObjectName,
      "ClaimManager.response.claimDate"    , claimDate );
  }

}

// *******************************************
// Name : VerifyDatapointType
// *******************************************
// Description:
//    Verify our global variable to see 
//    wether we already know this datapoint type
//    if not we will take it from the database and fill
//    our global map with it
//
// Returns:
//    None
// *******************************************

void VerifyDatapointType( string strType )
{
  // Local data
  mapping tmp;
  dyn_time ClaimDate;
  dyn_string strDPNames;
  dyn_string strDPClaimDate;
  dyn_time tCurrentTime;
  int t;
    
  if( mappingHasKey( g_ClaimedTypes, strType )){  // when we know the type
    if( bDebug ) DebugN( "claim.ctl:VerifyDatapointType|type is already known" );
    return;                                  // then we are ready
  }  

  if( bDebug ) DebugN( "claim.ctl:VerifyDatapointType|allocating new element in mapping" );
  
  g_ClaimedTypes[ strType ] = tmp;
  
  // Allocate the right places in our global variable
  g_ClaimedTypes[ strType ][ "DP"        ] = makeDynString();
  g_ClaimedTypes[ strType ][ "NAME"      ] = makeDynString();
  g_ClaimedTypes[ strType ][ "CLAIMDATE" ] = makeDynTime(); 
  
  if( bDebug ) DebugN( "claim.ctl:VerifyDatapointType| looking for DP type " + strType ); 
  
  g_ClaimedTypes[ strType ]["DP"] = dpNames("*", strType );
  
  if( bDebug ) DebugN( "claim.ctl:VerifyDatapointType| number of instances=" + dynlen( g_ClaimedTypes[ strType ][ "DP"   ] ) );

  // When we find no instances, then just exit
  if( !dynlen(  g_ClaimedTypes[ strType ][ "DP"   ] ))
    return; 
  
  // Determine the dp names of the elements 'ClaimDate' and 'name'
  for( t = 1; t <= dynlen( g_ClaimedTypes[ strType ][ "DP"   ] ); t++)
  {
     dynAppend( strDPNames    , g_ClaimedTypes[ strType ][ "DP"   ][t] + ".claim.name" );
     dynAppend( strDPClaimDate, g_ClaimedTypes[ strType ][ "DP"   ][t] + ".claim.claimDate" );
  }

  // Note:
  // Some functions ( like dpGet() ) do not like it when you pas them an 
  // element of a mapping
  // ( dpGet() fails when you use the element sin the mapping )
  // That is why we pass the mapping elements to a function.
  // The function 'GetClaimInfo' will accept the elements
  // of the mapping and dpGet() will be happy !
  
  // Now do one big dpGet() to find the name and ClaimDate
  // of every possible datapoint
  GetClaimInfo( 
    strDPNames    , g_ClaimedTypes[ strType ][ "NAME"      ],
    strDPClaimDate, g_ClaimedTypes[ strType ][ "CLAIMDATE" ] );
  
  if (bDebug) showMapping("VerifyDatapointType");
 
}

void GetClaimInfo(
  dyn_string &strDPNames, dyn_string &strReceiveName,
  dyn_string &strDPClaimDate, dyn_time &tReceiveDates 
)
{
  dpGet( 
    strDPNames    , strReceiveName,
    strDPClaimDate, tReceiveDates );
  
}    

// *******************************************
// Name : FindFreeClaim
// *******************************************
// Description:
//    Look for datapoint that is 'Free'.
//    The Client will only look in his local cache if the claim is known
//    The master has the right to write out new claims
//
//    if the mapping for a certain strType exceeds the number of DP's for that type
//    the oldest Claim will be found and that entry will be overwritten.
//
//    Free means that the 'ClaimDate' is 1970.
//
// Returns:
//    None
// *******************************************

string  FindFreeClaim( 
  string strTypeName,
  string strNewObjectName,
  time   &claimDate
)
{
  // Local data
  string strDP="";
    
  int iIndex = -1;          // Index of the item that we find ( if any )
  
  if( bDebug ) DebugN( "claim.ctl:FindFreeClaim| is looking for a a" + strTypeName );

  // When we have no instances ( then they probably requested the wrong type )


    if( !dynlen( g_ClaimedTypes[ strTypeName ][ "CLAIMDATE" ] ))
    return "";

  // Look at all elements to see if you can find a claimed object
  // that has the right name
  for( int t = 1; (t <= dynlen( g_ClaimedTypes[ strTypeName ][ "CLAIMDATE" ] )) && (iIndex == -1 ); t++)
  {
    if( 
     (year(  g_ClaimedTypes[ strTypeName ][ "CLAIMDATE" ][t]) != 1970 ) &&
     (g_ClaimedTypes[ strTypeName ][ "NAME" ][t] == strNewObjectName ) )
    {  
      if( bDebug ) DebugN( "claim.ctl:FindFreeClaim|  we found an existing claim for :" + strNewObjectName );
      iIndex = t;      
    } 
  } 
  
  // the next entries will prepare to add data, so they are only allowed for the Master ClaimManager
  if (!isClient) { 

    // We did not find a match by name, so we are just going to look for a free element
    if( iIndex == -1 ) {      
      for( int t = 1; (t <= dynlen( g_ClaimedTypes[ strTypeName ][ "CLAIMDATE" ] )) && (iIndex ==-1); t++) {
        // When the year is empty ( e.g. 1970 ) then it marks a free datapoint
        if( year(  g_ClaimedTypes[ strTypeName ][ "CLAIMDATE" ][t]) == 1970 ) {
          
          if (bDebug) DebugN("claim.ctl:FindFreeClaim|We found a free objectPlace.");
          iIndex = t;
        }  
    	}
  	}
  
    // we didn't find a free element, so we need to find the oldest claimDate and reuse that
    if( iIndex == -1 ) {
      time old = getCurrentTime();      
 
      if (bDebug) DebugN("claim.ctl:FindFreeClaim|We need to reuse the oldest claimed entry");

      for( int t = 1; (t <= dynlen( g_ClaimedTypes[ strTypeName ][ "CLAIMDATE" ] )) && (iIndex ==-1); t++) {
        if( g_ClaimedTypes[ strTypeName ][ "CLAIMDATE" ][t] < old ) {
          old = g_ClaimedTypes[ strTypeName ][ "CLAIMDATE" ][t];
          iIndex = t;
        }
      }
    }
  }
     
    
  if( iIndex > 0 )
  {
    // we have found an index that is free
    // Lets claim that place
    claimDate = getCurrentTime();
    
    // Update our mapping so that we know what is claimed
    g_ClaimedTypes[ strTypeName ][ "CLAIMDATE" ][iIndex] = claimDate;
    g_ClaimedTypes[ strTypeName ][ "NAME"      ][iIndex] = strNewObjectName;
      
    // Return the DP name of the claimed datapoint
    strDP = g_ClaimedTypes[ strTypeName ][ "DP" ][iIndex];
      
    if( bDebug ) DebugN( "claim.ctl:FindFreeClaim| found datapoint : " + strDP );
     
    if (!isClient) {
      dpSet( 
        strDP + ".claim.name", strNewObjectName,
        strDP + ".claim.claimDate", claimDate);
    }
  }
  
  if (bDebug) showMapping("FindFreeClaim");
  
  // Return the result
  return strDP;
}  

// *******************************************
// Name : checkAndCreateDPs
// *******************************************
// Description:
//    Checks if all DP's for a given type
//    are available, and if not, then will create them
//
// Returns:
//    None
// *******************************************
void checkAndCreateDPs() {
  
  // loop over all claimable types and check if the needed DP's allready exist
  for (int i=1; i<= dynlen(claimableTypes); i++) {
    // check for Temp Observation points, we need 100 off all of them, if one is not available we
    // will create it on the fly
    //For Master the temppoints are:
    //
    //    DPType              DP's
    //    Observation:        LOFAR_ObsSW_TempObs0001-0100
    //    ObservationControl: LOFAR_ObsSW_TempObs0001-0100_ObservationControl
    // 
    // And for the Stations:   
    //
    //    DPType          DP's
    //    StnObservation: LOFAR_ObsSW_TempObs0001-0100
    //    BeamControl:    LOFAR_ObsSW_TempObs0001-0100_BeamControl
    //    CalControl:     LOFAR_ObsSW_TempObs0001-0100_CalibrationControl
    //    TBBControl:     LOFAR_ObsSW_TempObs0001-0100_TBBControl
    //
    //And for CCU:
    //    DPType          DP's
    //    CEPObservation: LOFAR_ObsSW_TempObs0001-0100
    //    OnlineControl:  LOFAR_ObsSW_TempObs0001-0100_OnlineControl
    //    BGLAppl:        LOFAR_ObsSW_TempObs0001-0100_OnlineControl_BGLAppl
    //    BGLProc:        LOFAR_ObsSW_TempObs0001-0100_OnlineControl_BGLAppl_BGLProc
    //    StorageAppl:    LOFAR_ObsSW_TempObs0001-0100_OnlineControl_StorageAppl
    

    
    if (claimableTypes[i] == "Observation") {
      //different points for Client and Master
      for (int j=1;j<=100;j++) {
        string pre;
        if (j < 10) {
          pre ="000"+j;
        } else if (j < 100) {
          pre ="00"+j;
        } else if (j < 1000) {
          pre = "0"+j;
        }
        
        if (isClient) {
          if (substr(getSystemName(),0,3) == "CCU") {
            //CEPObservation
            if (!dpExists("LOFAR_ObsSW_TempObs"+pre)) {
              dpCreate("LOFAR_ObsSW_TempObs"+pre,"CEPObservation");
            }
            //OnlineControl
            if (!dpExists("LOFAR_ObsSW_TempObs"+pre+"_OnlineControl")) {
              dpCreate("LOFAR_ObsSW_TempObs"+pre+"_OnlineControl","OnlineControl");
            }
            //BGPAppl
            if (!dpExists("LOFAR_ObsSW_TempObs"+pre+"_OnlineControl_BGPAppl")) {
              dpCreate("LOFAR_ObsSW_TempObs"+pre+"_OnlineControl_BGPAppl","BGPAppl");
            }
            //BGPProc
            if (!dpExists("LOFAR_ObsSW_TempObs"+pre+"_OnlineControl_BGPAppl_BGPProc")) {
              dpCreate("LOFAR_ObsSW_TempObs"+pre+"_OnlineControl_BGPAppl_BGPProc","BGPProc");
            }
            //StorageAppl
            if (!dpExists("LOFAR_ObsSW_TempObs"+pre+"_OnlineControl_StorageAppl")) {
              dpCreate("LOFAR_ObsSW_TempObs"+pre+"_OnlineControl_StorageAppl","StorageAppl");
            }
          } else {
            //StnObservation
            if (!dpExists("LOFAR_ObsSW_TempObs"+pre)) {
              dpCreate("LOFAR_ObsSW_TempObs"+pre,"StnObservation");
            }
            //BeamControl
            if (!dpExists("LOFAR_ObsSW_TempObs"+pre+"_BeamControl")) {
              dpCreate("LOFAR_ObsSW_TempObs"+pre+"_BeamControl","BeamControl");
            }
            //CalControl
            if (!dpExists("LOFAR_ObsSW_TempObs"+pre+"_CalibrationControl")) {
              dpCreate("LOFAR_ObsSW_TempObs"+pre+"_CalibrationControl","CalibrationControl");
            }
            //TBBControl
            if (!dpExists("LOFAR_ObsSW_TempObs"+pre+"_TBBControl")) {
              dpCreate("LOFAR_ObsSW_TempObs"+pre+"_TBBControl","TBBControl");
            }
          }
        } else {
          //Observation
          if (!dpExists("LOFAR_ObsSW_TempObs"+pre)) {
            dpCreate("LOFAR_ObsSW_TempObs"+pre,"Observation");
          } 
          //ObservationControl
          if (!dpExists("LOFAR_ObsSW_TempObs"+pre+"_ObservationControl")) {
            dpCreate("LOFAR_ObsSW_TempObs"+pre+"_ObservationControl","ObservationControl");
          }
        } // end iscClient
      } // end Observation counter loop
    } // end Observation Type
  } // end claimable Types loop
}

// *******************************************
// Name : showMapping
// *******************************************
// Description:
//    Prints all key value pairs in the mapping
//
// Returns:
//    None
// *******************************************
void showMapping(string caller) {
  DebugN( "claim.ctl:showMapping|After " + caller + " local mapping contains now: " );
  for (int i = 1; i <= mappinglen(g_ClaimedTypes); i++) { 
  	Debug("mappingGetKey", i, " = "+mappingGetKey(g_ClaimedTypes, i));  
		DebugN("  mappingGetValue", i, " = "+mappingGetValue(g_ClaimedTypes, i));
  }
}
    
