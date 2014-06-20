
global dyn_string strHighlight;    // What symbols to highligth
global mapping    Observations;    // 
global dyn_string strPlannedObs;

// The following global variables are used to hold
// the name of the datapoints plus the actual name
// This is used to quickly determine the name of the real datapoint
// from the graphical user interface
global dyn_string strClaimDPName;      // datapoint that was claimed
global dyn_string strClaimObjectName;  // Actual object name


// ****************************************
// Name : NameToRealName
// ****************************************
// Description:
//    Accepts a name and will return:
//
//    1)  The name of the datapoint
//        Example:  'LOFAR" -> 'LOFAR'
//
//    2)  The name of the actual 'Claimed' datapoint
//        when the name refers to a temporary datapoint
//
// Returns:
//    The DP nam eof the actual datapoint
//
// Restrictions:
//    The 'Claimed' objects have a name
// ***************************************

string NameToRealName( string strName )
{
  if( dpExists( strName ))       // When the name refers to an actual datapoint
    return strName;              // then just return that name
  
  // Do we know the 'Claimed' name
  int iPos = dynContains( strClaimObjectName, strName );
  
  if( iPos < 1 )
    return "";
  else
     return strClaimDPName[ iPos ];

}

// ****************************************
// Name : QueryConnectClaims
// ****************************************
// Description:
//    Establish a query connect to get all
//    claimed datapoints
//
// Returns:
//    None
// ***************************************

void QueryConnectClaims()
{
  // Local data
  string strQuery = "SELECT '.name:_original.._value, .Claim.ClaimDate:_original.._value' FROM '*' WHERE _DPT = \"ObsCtrl\"";

  DebugN( "*** Doing a query for : QueryConnectClaims() " );
 
  // Triggr a single query that gets an update when one 
  // claim changes
  dpQueryConnectSingle( "QueryConnectClaim_Callback", 1, "ident_claim", strQuery, 50 );
}

void QueryConnectClaim_Callback(
  string strIdent,
  dyn_dyn_anytype aResult    
)
{
  // Locla data
  int iPos;
  string strDP;
  string strName;
  time tClaimDate;
  bool bClaimed;
  
  DebugN( "QueryConnectClaim_Callback() has " + dynlen( aResult ) + " results" );
  
  if( dynlen( aResult ) < 2 )
      return;
  
  // Iterate through the results
  for( int t = 2; t <= dynlen( aResult ); t++)
  {
    strDP      = aResult[t][1];
    strName    = aResult[t][2];
    tClaimDate = aResult[t][3];
    
    // We are claimed when the date is not 1970
    bClaimed = year( tClaimDate ) != 1970;
  
    // Do we already have this name 
    iPos = dynContains( strClaimDPName, strDP );  
    
    // When we have the claim, and the datapoint is now 'not claimed'
    // then 
    if( !bClaimed && (iPos > 0))
    {
      dynRemove( strClaimDPName     , iPos );
      dynRemove( strClaimObjectName , iPos );
    }
    
    // When we do not have the item and it gets 'Claimed'
    // then we have to add it !
    if( bClaimed && (iPos < 1 ))
    {
      dynAppend(  strClaimDPName     , strDP   );
      dynAppend(  strClaimObjectName , strName );
    }
  }  
}

// ****************************************
// Name : QueryConnectObservations
// ****************************************
// Description:
//    Establish a query connect that gives us the current
//    status of all observations.
//    This information is needed to update the 'Planned', 'Running' and 'Finished'
//    observation tables
//
// Returns:
//    None
// ***************************************

void QueryConnectObservations()
{
  string strQuery = "SELECT '.name:_original.._value, .state:_original.._value, .childState:_original.._value, .stationList:_original.._value' FROM '*' WHERE _DPT = \"ObsCtrl\"";

  // Define a global variable of type mapping
  // that holds the various properties of the observations
  Observations[ "DP"          ] = makeDynString();                    
  Observations[ "NAME"        ] = makeDynString();
  Observations[ "STATE"       ] = makeDynInt();
  Observations[ "CHILDSTATE"  ] = makeDynInt();
  Observations[ "STATIONLIST" ] = makeDynString();
  
  // Triggr a single query that gets an update when one 
  // observation changes
  dpQueryConnectSingle( "QueryConnectObservations_Callback", 1, "ident_observations", strQuery, 50 );
}

// ****************************************
// Name : QueryConnectObservations_Callback
// ****************************************
// Description:
//    This is the callback that receives infoabout observations
//
// Returns:
//    None
// ***************************************

void QueryConnectObservations_Callback( 
  string strIdent,
  dyn_dyn_anytype aResult 
)
{
  int iPos;
  dyn_string strTableDP;
  dyn_string strTableName;
  dyn_int    iTableState;
  dyn_int    iTableChildState;
  dyn_string strTableStationList;
  
  DebugN( "QueryConnectObservations_Callback() Number of observations in message = " + dynlen( aResult ) );
  
  for( int t = 2; t <= dynlen( aResult ); t++)
  {  
    string strDP = aResult[t][1];
    
    // Is this an existing observation or a new one
    iPos = dynContains( Observations[ "DP"         ], strDP );  
  
    if( iPos < 1 ){
      dynAppend( Observations[ "DP"          ], strDP );
      iPos = dynlen( Observations[ "DP" ] );
    }  

    // Now store the values 
    Observations[ "NAME"        ][iPos] = aResult[t][2];
    Observations[ "STATE"       ][iPos] = aResult[t][3];
    Observations[ "CHILDSTATE"  ][iPos] = aResult[t][4];
    Observations[ "STATIONLIST" ][iPos] = aResult[t][5];
  }

                      
}  
