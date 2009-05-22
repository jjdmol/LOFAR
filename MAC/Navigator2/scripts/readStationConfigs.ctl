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

  bool showDebug = true;
  int nr_ofAnt   = 0,
      index      = 1,
      OL         = 1,                // ArrayIndexnumber of dda_splittedLinesAntConfFile
      NB         = 2,                // ArrayIndexnumber of dda_splittedLinesAntConfFile
      H          = 3;                // ArrayIndexnumber of dda_splittedLinesAntConfFile;
       
  string strCurConfig;
  
  string strDataDir             = ""; 
  if (isDir("/opt/lofar/etc/") ) {
    strDataDir = "/opt/lofar/etc/";
  } else if ( isDir ("c:/data/CS20_CS010/data/configs/") ) {
    strDataDir = "c:/data/CS20_CS010/data/configs/";
  } else {
    DebugN("Could not find datadair to work with, leaving and no antenne data read.");
    return;
  }
    
  string strAntArrayConfFile      = strDataDir+"AntennaArrays.conf";
  string strRemoteStationConfFile = strDataDir+"RemoteStation.conf";

  dyn_string dynStr_fileContent;
   
  //
  // AntennaArray cycle
  //		
  dynStr_fileContent = lto_getFile_asDynStr(strAntArrayConfFile);

  while (dynlen(dynStr_fileContent) > 3  && index < dynlen(dynStr_fileContent)) {
    float centerOL,
          centerNB,
          centerH;
    dyn_float antConfFileOL;
    dyn_float antConfFileNB;
    dyn_float antConfFileH;
    // read ConfigurationName
    string str;
    sscanf(dynStr_fileContent[index++],"%s",str);
    strCurConfig=strtoupper(str);
    Debug("readStationConfigs.ctl:main|Reading  Config for: "+strCurConfig);

    
    // read fieldcenter
    sscanf(dynStr_fileContent[index++],"%*d %*s %f %f %f",centerOL,centerNB,centerH);
    //DebugN("Reading  Config for center OL,NB,H:" + centerOL + " " + centerNB + " " + centerH);

    // read nr of antennas
    sscanf(dynStr_fileContent[index++],"%d",nr_ofAnt);
    //DebugN("Nr of Antenna's in this Config: "+nr_ofAnt);
   
    // Select on allowed configurations
    if ((strpos(strCurConfig,"LBA") > -1 && strlen(strCurConfig) == 3) || 
         (strpos(strCurConfig,"HBA") > -1 && strlen(strCurConfig) == 3)) {
      DebugN("--> will be read");

      for (int ix = index; ix < nr_ofAnt + index; ix++ )
      {
       
        float deltaOL;
        float deltaNB;
        float deltaH;
      
        // read new line of delta's
        sscanf(dynStr_fileContent[ix],"%lf %lf %lf",deltaOL,deltaNB,deltaH);
        //DebugN("OL :"+deltaOL+ " NB: "+ deltaNB+ " H: "+deltaH);
        //DebugN("ix: " + ix + " index: " + index);
        antConfFileOL[(ix+1-index)] = deltaOL;
        antConfFileNB[(ix+1-index)] = deltaNB;
        antConfFileH[(ix+1-index)]  = deltaH;
      }
      
      //DebugN("antConfFileOL: "+antConfFileOL);
      
      // All the reading has been done.
      dpSet("remoteStation."+strCurConfig+".centerOL",centerOL,
            "remoteStation."+strCurConfig+".centerNB",centerNB,
            "remoteStation."+strCurConfig+".centerH",centerH);
      
      for (int i=1; i<= nr_ofAnt;i++) {
        string ant=(i-1);
        dpSet(strCurConfig+ant+".deltaX",antConfFileOL[i],
              strCurConfig+ant+".deltaY",antConfFileNB[i],
              strCurConfig+ant+".deltaH",antConfFileH[i]);
      }
    } else {
      DebugN("--> will be skipped");
    }
    index +=nr_ofAnt+1;
  }
  
  //
  // RemoteStation cycle
  //	
  
  dynStr_fileContent = lto_getFile_asDynStr(strRemoteStationConfFile);
  //DebugN("fileContent: "+dynStr_fileContent);
  
  for (index=1;index <= dynlen(dynStr_fileContent);index++) {
    if (strpos(dynStr_fileContent[index],"RS.STATION_ID")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        dpSet("remoteStation.stationID",value[2]);
      }
    }
    
    if (strpos(dynStr_fileContent[index],"RS.N_RSPBOARDS")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        dpSet("remoteStation.N_RSPBoards",value[2]);
      }
    }
    
    if (strpos(dynStr_fileContent[index],"RS.N_TBBOARDS")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        dpSet("remoteStation.N_TBBoards",value[2]);
      }
    }

    if (strpos(dynStr_fileContent[index],"RS.N_LBAS")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        dpSet("remoteStation.N_LBAS",value[2]);
      }
    }
      
    if (strpos(dynStr_fileContent[index],"RS.N_HBAS")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        dpSet("remoteStation.N_HBAS",value[2]);
      }
    }
  
    if (strpos(dynStr_fileContent[index],"RS.HBA_SPLIT")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        if (substr(value[2],0,1) == "N" ||
            substr(value[2],0,1) == "n" ||
            substr(value[2],0,1) == "F" ||
            substr(value[2],0,1) == "f") {
          dpSet("remoteStation.HBA_Split",false);
        } else {
          dpSet("remoteStation.HBA_Split",true);
        }
      }
    }
  

    if (strpos(dynStr_fileContent[index],"RS.WIDE_LBAS")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        if (substr(value[2],0,1) == "N" ||
            substr(value[2],0,1) == "n" ||
            substr(value[2],0,1) == "F" ||
            substr(value[2],0,1) == "f") {
          dpSet("remoteStation.wide_LBAS",false);
        } else {
          dpSet("remoteStation.wide_LBAS",true);
        }
      }
    }
  }
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
