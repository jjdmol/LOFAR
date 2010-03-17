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
  * This will be used to fill: Antenna deltaX deltaY and deltaH fields for 
  * LBA and HBA antenna configurations. 
  *
  * Allowed for now are :  LBA-HBA When LBA is missing LBA_X will be used 
  * (if available)
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
  bool foundLBA  = false;
  
  bool bDebug    = false;
  int nr_ofAnt   = 0,
      index      = 1,
      OL         = 1,                // ArrayIndexnumber of dda_splittedLinesAntConfFile
      NB         = 2,                // ArrayIndexnumber of dda_splittedLinesAntConfFile
      H          = 3;                // ArrayIndexnumber of dda_splittedLinesAntConfFile;
       
  string strCurConfig;
  
  string strDataDir             = ""; 
  if (isdir("/opt/lofar/etc/") ) {
    strDataDir = "/opt/lofar/etc/";
  } else if ( isdir ("c:/data/CS20_CS010/data/configs/") ) {
    strDataDir = "c:/data/CS20_CS010/data/configs/";
  } else {
    DebugN("Could not find datadir to work with, leaving and no antenne data read.");
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
    if (strlen(str) > 0 && str[0] != " " && str[0] != "#") {
      strCurConfig=strtoupper(str);
      Debug("readStationConfigs.ctl:main|Reading  Config for: "+strCurConfig);

    
      // read fieldcenter
      sscanf(dynStr_fileContent[index++],"%*d %*s %f %f %f",centerOL,centerNB,centerH);
      if (bDebug) DebugN("Reading  Config for center OL,NB,H:" + centerOL + " " + centerNB + " " + centerH);

      // read nr of antennas
      sscanf(dynStr_fileContent[index++],"%d",nr_ofAnt);
      if (bDebug) DebugN("Nr of Antenna's in this Config: "+nr_ofAnt);
   
      // Select on allowed configurations
      if (strCurConfig == "LBA" || 
          strCurConfig == "HBA" ||
          (strCurConfig == "LBA_X" && !foundLBA)
          ) {
        DebugN("--> will be read");
        bool splitArray=false;   //LBAXarray is 48 lines, 1st col(x3) contains 0-47  2ndco(x3) contain 48-96 
        if (strCurConfig == "LBA") foundLBA=true;
        if (strCurConfig == "LBA_X") {
          strCurConfig="LBA";
          splitArray=true;
        }
      

        for (int ix = index; ix < nr_ofAnt + index; ix++ )
        {
       
          float deltaOL;
          float deltaNB;
          float deltaH;
          float deltaOL2;
          float deltaNB2;
          float deltaH2;
      
          // read new line of delta's
          if (splitArray) {
            sscanf(dynStr_fileContent[ix],"%lf %lf %lf %lf %lf %lf",deltaOL,deltaNB,deltaH,deltaOL2,deltaNB2,deltaH2);
          } else {
            sscanf(dynStr_fileContent[ix],"%lf %lf %lf",deltaOL,deltaNB,deltaH);
          }    
          if (bDebug) DebugN("OL :"+deltaOL+ " NB: "+ deltaNB+ " H: "+deltaH);
          if (bDebug) DebugN("ix: " + ix + " index: " + index);
          if (bDebug) DebugN("Filling array at index: "+ (ix+1-index));
          antConfFileOL[(ix+1-index)] = deltaOL;
          antConfFileNB[(ix+1-index)] = deltaNB;
          antConfFileH[(ix+1-index)]  = deltaH;
          if (splitArray) {
            if (bDebug) DebugN("OL2 :"+deltaOL2+ " NB2: "+ deltaNB2+ " H2: "+deltaH2);
            if (bDebug) DebugN("Filling array at index: "+ (ix+1-index+48));
            antConfFileOL[(ix+1-index+48)] = deltaOL2;
            antConfFileNB[(ix+1-index+48)] = deltaNB2;
            antConfFileH[(ix+1-index+48)]  = deltaH2;
          }
        }
      
        if (bDebug) DebugN("antConfFileOL: "+antConfFileOL);
      
        // All the reading has been done.
        dpSet("remoteStation."+strCurConfig+".centerOL",centerOL,
              "remoteStation."+strCurConfig+".centerNB",centerNB,
              "remoteStation."+strCurConfig+".centerH",centerH);
      
        int ix=nr_ofAnt;
        if (splitArray) ix *= 2;
        for (int i=1; i<= ix;i++) {
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
  }
  
  //
  // RemoteStation cycle
  //	
  
  dynStr_fileContent = lto_getFile_asDynStr(strRemoteStationConfFile);
  if (bDebug) DebugN("fileContent: "+dynStr_fileContent);
  
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

