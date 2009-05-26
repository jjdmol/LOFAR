//# readStationConfigs.ctl
//#
//#  Copyright (C) 2007-2008
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
#uses "GCFCommon.ctl"

/**
  * Controller that will be run once during station startup.
  * it will search for different conf files in /opt/lofar/etc/
  * and will fill the pvss database types for those config files.
 
  * For now we have: 

  * AntennaArrays.conf.
  * This will be used to fill: Antenna deltaX deltaY and deltaH fields for LBA and HBA
  * antenna configurations. 
  *
  * Allowed for now are :  LBA-HBA
  *
  * The files contain fieldcenter coordinates in OL-NB-Height
  * and Antenna positions in OL-NB-Height Offsets from the fieldCenter.
  * As FieldCenter now the GPS coordinates are taken, this is not correct
  *
  * For future compatibility we have to consider other earth coordinates also
  
  * RemoteStation.conf
  * This will fill the RemoteStation point with all data available for this station.
  *
  */
main()
{ 

  bool showDebug = false;
       
  string strCurConfig;
  string strDataDir     = "";
  string strDataDir             = ""; 
  if (isdir("/opt/lofar/etc/") ) {
    strDataDir = "/opt/lofar/etc/StaticMetaData/";
  } else if ( isdir ("c:/data/CS20_CCU001/data/configs/") ) {
    strDataDir = "c:/data/CS20_CCU001/data/configs/";
  } else {
    DebugN("Could not find datadir to work with, leaving and no antenne data read.");
    return;
  }
  
  string strRSPDatFile   = strDataDir+"RSPConnections.dat";
  string strMACDatFile   = strDataDir+"MAC+IP.dat";

  dyn_string dynStr_fileContent;
   
  // first read the RSP
  dynStr_fileContent = lto_getFile_asDynStr(strRSPDatFile);
  DebugN("Filling Database from file " + strRSPDatFile);
  for (int index=1;index <= dynlen(dynStr_fileContent);index++) {
    if (strpos(dynStr_fileContent[index],"#") < 0 || strpos(dynStr_fileContent[index],"#") > 4) {
      dyn_string linesplitted=strsplit(dynStr_fileContent[index]," \t");
      if (showDebug) DebugN(index+" :"+linesplitted);
      // check if stationname is not empty
      if (dynlen(linesplitted) == 3 && linesplitted[3] != "") {
        // if BGPNode allready in dplist, rewrite contents.
        // if not, add to dp's and fill contents.
        string node= linesplitted[3];
        if (!dpExists(node) ){
          dpCreate(node, "BGPConnectionInfo");
        }
        
        dpSet(node+".station",linesplitted[1]);
        dyn_string rsp = strsplit(linesplitted[2],"_");
        int nr = rsp[2];
        if (showDebug) DebugN( "node: "+node+ "  rspfull: " + linesplitted[2]+ "  rsp[2]" + rsp[2]+ "nr: "+nr);
        dpSet(node+".RSPBoard",nr);
      }   
    }
  }
  DebugN("Ready");

  // now read MAC+IP
  dynStr_fileContent = lto_getFile_asDynStr(strMACDatFile);
  DebugN("Filling Database from file " + strMACDatFile);
  for (int index=1;index <= dynlen(dynStr_fileContent);index++) {
    if (strpos(dynStr_fileContent[index],"#") < 0 || strpos(dynStr_fileContent[index],"#") > 4) {
      dyn_string linesplitted=strsplit(dynStr_fileContent[index]," \t");
      if (showDebug) DebugN(index+" :"+linesplitted);
      // check if stationname is not empty
      if (dynlen(linesplitted) == 3 && linesplitted[1] != "") {
        // if BGPNode allready in dplist, add contents.
        // if not, add to dp's and fill contents.
        string node= linesplitted[1];
        if (!dpExists(node) ){
          dpCreate(node, "BGPConnectionInfo");
        }
        
        dpSet(node+".IP",linesplitted[2]);
        dpSet(node+".MAC",linesplitted[3]);
      }   
    }
  }
  DebugN("Ready");
}

dyn_string lto_getFile_asDynStr(string aFileName)
{
  // Local vars
  dyn_string aFile_asDynStr;     // dyn_string returnvalue
  string     aFile_asStr;

  file f;                        // our file
  int err;                           // error code
  
  f=fopen(aFileName,"r");        // open for reading
  if (f > 0) 
  {
    err=ferror(f);                 // export error

    if (err!=0) 
    {
      DebugN("readStationConfigs.ctl:lto_getFile_asDynStr|Error during read no. " + err);
    }
    else
    {
      if ( fileToString (aFileName, aFile_asStr) )
      {
        aFile_asDynStr = strsplit(aFile_asStr, "\n");
      }	
    }
    fclose(f); // close file
  } else {
    DebugN("readStationConfigs.ctl:lto_getFile_asDynStr|Error opening file: " + aFileName);
  }
    
  return aFile_asDynStr;
}
