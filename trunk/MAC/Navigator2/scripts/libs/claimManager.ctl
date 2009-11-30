// claimManager.ctl
//
//  Copyright (C) 2002-2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
///////////////////////////////////////////////////////////////////
// claimManager functions
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// claimManager_nameToRealName		         : returns the DPname of the actual datapoint
// claimManager_queryConnectClaims         : Establish a query connect to get all claimed datapoints
// claimManager_queryConnectClaim_Callback : Callback for the above query connect



// the name of the datapoints plus the actual name
// This is used to quickly determine the name of the real datapoint
// from the graphical user interface
global dyn_string strClaimDPName;      // datapoint that was claimed
global dyn_string strClaimObjectName;  // Actual object name


// ****************************************
// Name : claimManager_nameToRealName
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
//    The DP name of the actual datapoint
//
// Restrictions:
//    The 'Claimed' objects have a name
// ***************************************

string claimManager_nameToRealName( string strName )
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
// Name : claimManager_realNameToName
// ****************************************
// Description:
//    Accepts a temp datapoint and will return:
//
//    1)  The name of the datapoint
//        Example:  'LOFAR" -> 'LOFAR'
//
//    2)  The name of the actual 'Claimed' datapoint
//        when the name refers to a temporary datapoint
//
// Returns:
//    The DP name of the claimed datapoint
//
// Restrictions:
//    The 'Claimed' objects have a name
// ***************************************

string claimManager_realNameToName( string strName )
{
  
  // Do we know the 'Claimed' name
  int iPos = dynContains( strClaimDPName, strName );
  
  if( iPos < 1 )
    return "";
  else
     return strClaimObjectName[ iPos ];

}

// ****************************************
// Name : claimManager_queryConnectClaims
// ****************************************
// Description:
//    Establish a query connect to get all
//    claimed datapoints
//
// Returns:
//    None
// ***************************************

void claimManager_queryConnectClaims()
{
  // Local data
  string strQuery = "SELECT '.claim.name:_original.._value, .claim.claimDate:_original.._value' FROM 'LOFAR_ObsSW_*' WHERE _DPT = \"Observation\"";

  LOG_DEBUG( "claimManager.ctl:claimManager_queryConnectClaims|*** Doing a query for : claimManager_QueryConnectClaims() " );
 
  // Trigger a single query that gets an update when one 
  // claim changes
  if (dpQueryConnectSingle( "claimManager_queryConnectClaim_Callback", 1, "ident_claim", strQuery, 50 ) == -1) {
    LOG_ERROR( "claimManager.ctl:claimManager_queryConnectClaims|dpQueryConnectSingle failed" );
  }
}

void claimManager_queryConnectClaim_Callback(
  string strIdent,
  dyn_dyn_anytype aResult    
)
{
  // Locla data
  int iPos;
  string aDP;
  string strDP;
  string strName;
  time tClaimDate;
  bool bClaimed;
  
  LOG_DEBUG( "claimManager.ctl:claimManager_queryConnectClaim_Callback| has " + dynlen( aResult ) + " results" );
  LOG_DEBUG( "claimManager.ctl:claimManager_queryConnectClaim_Callback| "+aResult);
  if( dynlen( aResult ) < 2 )
      return;
  
  // Iterate through the results
  for( int t = 2; t <= dynlen( aResult ); t++)
  {
    aDP        = aResult[t][1];
    strName    = aResult[t][2];
    tClaimDate = aResult[t][3];
    
    strDP=dpSubStr(aDP,DPSUB_DP);
    
    LOG_DEBUG("claimManager.ctl:claimManager_queryConnectClaim_Callback| strDP     : "+ strDP);
    LOG_DEBUG("claimManager.ctl:claimManager_queryConnectClaim_Callback| strName   : "+strName);
    LOG_DEBUG("claimManager.ctl:claimManager_queryConnectClaim_Callback| tClaimDate: "+tClaimDate);
    
    // We are claimed when the date is not 1970
    bClaimed = year( tClaimDate ) != 1970;
  
    // Do we already have this name 
    iPos = dynContains( strClaimDPName, strDP );  
    
    LOG_DEBUG("claimManager.ctl:claimManager_queryConnectClaim_Callback| found old claim at postion: "+ iPos);

    
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
    } else if( bClaimed && (iPos > 0 )) {
      // When we do have the item and it gets 'Claimed'
      // then we have to alter the 
      strClaimObjectName[iPos] = strName;
    }
  }  
}
