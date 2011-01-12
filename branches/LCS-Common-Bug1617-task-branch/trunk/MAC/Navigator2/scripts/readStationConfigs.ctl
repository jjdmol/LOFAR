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
  global dyn_string dynStr_fileContent;
  global int index=1;
  global bool bDebug    = false;
  global bool norVecLBAFound=false;
  global bool norVecHBA0Found=false;
  global bool norVecHBA1Found=false;
  global bool rotMatLBAFound=false;
  global bool rotMatHBA0Found=false;
  global bool rotMatHBA1Found=false;
  global bool centerLBAFound=false;
  global bool centerHBAFound=false;
  global bool deltasLBAFound=false;
  global bool deltasHBAFound=false;
  global bool centerHBA0Found=false;
  global bool centerHBA1Found=false;

  
main()
{ 

  bool foundLBA  = false;
  
  string strCurConfig;
  
  string strDataDir             = ""; 
  if (isdir("/opt/lofar/etc/") ) {
    strDataDir = "/opt/lofar/etc/";
  } else if ( isdir ("d:/data/CS20_CS001/data/configs/") ) {
    strDataDir = "d:/data/CS20_CS001/data/configs/";
  } else {
    DebugN("Could not find datadir to work with, leaving and no antenne data read.");
    return;
  }
    
//  string strAntArrayConfFile      = strDataDir+"AntennaArrays.conf";
  string strAntArrayConfFile      = strDataDir+"AntennaField.conf";
  string strRemoteStationConfFile = strDataDir+"RemoteStation.conf";

   
  //
  // AntennaArray cycle
  //		
  dynStr_fileContent = lto_getFile_asDynStr(strAntArrayConfFile);
   

  while (dynlen(dynStr_fileContent) > 3  && index < dynlen(dynStr_fileContent)) {
    
    // read ConfigurationName
    string str1="",str2="";
    sscanf(dynStr_fileContent[index++],"%s %s",str1,str2);
    if (strlen(str1) <= 0 || str1[0] == " " || str1[0] == "#") {
      continue;
    }
      
    if (bDebug) DebugN("working on: ",dynStr_fileContent[index-1]);    
    if (bDebug) DebugN("str1: ",str1," str2: ",str2);
    
    if (strtoupper(str1) == "NORMAL_VECTOR" ) {
      if (strtoupper(str2) != "LBA" && strtoupper(str2) != "HBA0" && strtoupper(str2) != "HBA1") {
        DebugN("readStationConfigs.ctl | Unknown NORMAL_VECTOR target found: ", str2);
        continue;
      } else {
       // read Normal Vector
        processNormalVector(strtoupper(str2));
      }
    } else if (strtoupper(str1) == "ROTATION_MATRIX" ) {
      if (strtoupper(str2) != "LBA" && strtoupper(str2) != "HBA0" && strtoupper(str2) != "HBA1") {
        DebugN("readStationConfigs.ctl | Unknown ROTATION_MATRIX target found: ", str2);
        continue;
      } else {
       // read Rotation Matrix
        processRotationMatrix(strtoupper(str2));
      }
    } else if (strtoupper(str1) == "LBA" || strtoupper(str1) == "HBA") {
      strCurConfig=strtoupper(str1); 
        
      // read fieldcenter
      processFieldCenter(strCurConfig);

      // read field Deltas
      processFieldDeltas(strCurConfig);      
    } else if (strtoupper(str1) == "HBA0" || strtoupper(str1)=="HBA1") {
      // read fieldcenter
      processFieldCenter(strtoupper(str1));
    } else {
      DebugN(str1," -----> Unknown, will not be read");
    }
  }
    
  if (!norVecLBAFound ) {
    DebugN("Error? NORMAL_VECTOR for LBA not found.");
  }
  if (!norVecHBA0Found ) {
    DebugN("Error? NORMAL_VECTOR for HBA0 not found.");
  }
  if (!norVecHBA1Found ) {
    DebugN("Error? NORMAL_VECTOR for HBA1 not found.");
  }      
  if (!rotMatLBAFound ) {
    DebugN("Error? ROTATION_MATRIX for LBA not found.");
  }      
  if (!rotMatHBA0Found ) {
    DebugN("Error? ROTATION_MATRIX for HBA0 not found.");
  }      
  if (!rotMatHBA1Found ) {
    DebugN("Error? ROTATION_MATRIX for HBA1 not found.");
  }      
  if (!centerLBAFound ) {
    DebugN("Error? FieldCenter for LBA not found.");
  }      
  if (!centerHBAFound ) {
    DebugN("Error? FieldCenter for HBA not found.");
  }
  if (!deltasLBAFound ) {
    DebugN("Error? Deltas for LBA not found.");
  }      
  if (!deltasHBAFound ) {
    DebugN("Error? Deltas for HBA not found.");
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


dyn_string lto_getFile_asDynStr(string aFileName) {
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

void processNormalVector(string aS) {
  float fX=0,fY=0,fZ=0;
  sscanf(dynStr_fileContent[index++],"%*d %*s %lf %lf %lf",fX,fY,fZ);
  if (bDebug) DebugN("Reading NORMAL_VECTOR "+aS+" X,Y,Z :" + fX + " " + fY + " " + fZ);
  if (aS == "LBA" ) {
    norVecLBAFound=true;
    dpSet("remoteStation.LBA.NormalVector.X",fX,
          "remoteStation.LBA.NormalVector.Y",fY,
          "remoteStation.LBA.NormalVector.Z",fZ);    
  } else if (aS == "HBA0") {
    norVecHBA0Found=true;
    dpSet("remoteStation.HBA.HBA0.NormalVector.X",fX,
          "remoteStation.HBA.HBA0.NormalVector.Y",fY,
          "remoteStation.HBA.HBA0.NormalVector.Z",fZ);    
  } else if (aS == "HBA1") {
    norVecHBA1Found=true;
    dpSet("remoteStation.HBA.HBA1.NormalVector.X",fX,
          "remoteStation.HBA.HBA1.NormalVector.Y",fY,
          "remoteStation.HBA.HBA1.NormalVector.Z",fZ);    
  }
} 

void processRotationMatrix(string aS) {
  dyn_float fX,fY,fZ;
  int nr_rows=0;
  // read nr of rows
  sscanf(dynStr_fileContent[index++],"%d",nr_rows);
  if (bDebug) DebugN("index: "+(index-1)+" nr_rows: "+nr_rows);
  for (int i = 1; i <= nr_rows; i++) {
    sscanf(dynStr_fileContent[index++],"%lf %lf %lf",fX[i],fY[i],fZ[i]);
    if (bDebug) DebugN("Reading rotationMatrix "+aS+"line "+i+" X,Y,Z:" + fX[i] + " " + fY[i] + " " + fZ[i]);
  }
  index++;
  if (aS == "LBA" ) {
    rotMatLBAFound=true;
    dpSet("remoteStation.LBA.RotationMatrix.X",fX,
          "remoteStation.LBA.RotationMatrix.Y",fY,
          "remoteStation.LBA.RotationMatrix.Z",fZ);    
  } else if (aS == "HBA0") {
    rotMatHBA0Found=true;
    dpSet("remoteStation.HBA.HBA0.RotationMatrix.X",fX,
          "remoteStation.HBA.HBA0.RotationMatrix.Y",fY,
          "remoteStation.HBA.HBA0.RotationMatrix.Z",fZ);    
  } else if (aS == "HBA1") {
    rotMatHBA1Found=true;
    dpSet("remoteStation.HBA.HBA1.RotationMatrix.X",fX,
          "remoteStation.HBA.HBA1.RotationMatrix.Y",fY,
          "remoteStation.HBA.HBA1.RotationMatrix.Z",fZ);   
  }


}   

void processFieldCenter(string aS) {
  float fX=0,fY=0,fZ=0;
  sscanf(dynStr_fileContent[index++],"%*d %*s %lf %lf %lf",fX,fY,fZ);
  if (bDebug) DebugN("Reading  fieldcenter "+aS+"X,Y,Z:" + fX + " " + fY + " " + fZ);
  if (aS== "LBA") {
    centerLBAFound=true;
    dpSet("remoteStation."+aS+".centerX",fX,
        "remoteStation."+aS+".centerY",fY,
        "remoteStation."+aS+".centerZ",fZ);
  } else if (aS == "HBA") {
    centerHBAFound=true;
    dpSet("remoteStation."+aS+".centerX",fX,
        "remoteStation."+aS+".centerY",fY,
        "remoteStation."+aS+".centerZ",fZ);
  } else if (aS == "HBA0") {
    centerHBA0Found=true;
    dpSet("remoteStation.HBA.HBA0.centerX",fX,
          "remoteStation.HBA.HBA0.centerY",fY,
          "remoteStation.HBA.HBA0.centerZ",fZ);
  } else if (aS == "HBA1") {
    centerHBA1Found=true;
    dpSet("remoteStation.HBA.HBA1.centerX",fX,
          "remoteStation.HBA.HBA1.centerY",fY,
          "remoteStation.HBA.HBA1.centerZ",fZ);
  }
}

void processFieldDeltas(string aS) {
  int nr_ofAnt   = 0; 
  float deltaX;
  float deltaY;
  float deltaZ;
  dyn_float antConfFileX;
  dyn_float antConfFileY;
  dyn_float antConfFileZ;

  if (aS== "LBA") {
    deltasLBAFound=true;
  } else if (aS == "HBA") {
    deltasHBAFound=true;
  }

  // read nr of antennas
  sscanf(dynStr_fileContent[index++],"%d",nr_ofAnt);
  if (bDebug) DebugN("Nr of Antenna's in this Config: "+nr_ofAnt);
  DebugN(aS,"--> will be read");
  for (int ix = index; ix < nr_ofAnt + index; ix++ ) {
    // read new line of delta's
    sscanf(dynStr_fileContent[ix],"%lf %lf %lf",deltaX,deltaY,deltaZ);
    if (bDebug) DebugN("X :"+deltaX+ " Y: "+ deltaY+ " Z: "+deltaZ);
    if (bDebug) DebugN("ix: " + ix + " index: " + index);
    if (bDebug) DebugN("Filling array at index: "+ (ix+1-index));
    antConfFileX[(ix+1-index)] = deltaX;
    antConfFileY[(ix+1-index)] = deltaY;
    antConfFileZ[(ix+1-index)] = deltaZ;
  }
    
  int ix=nr_ofAnt;
  for (int i=1; i<= ix;i++) {
    string ant=(i-1);
    dpSet(aS+ant+".deltaX",antConfFileX[i],
          aS+ant+".deltaY",antConfFileY[i],
          aS+ant+".deltaZ",antConfFileZ[i]);
  }
  index +=nr_ofAnt+1;
}
    

