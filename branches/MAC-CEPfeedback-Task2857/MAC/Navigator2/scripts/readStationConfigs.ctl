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
  global int nr_HBA=0;
  global int nr_LBA=0;
  global bool norVecLBAFound=false;
  global bool norVecHBAFound=false;
  global bool norVecHBA0Found=false;
  global bool norVecHBA1Found=false;
  global bool rotMatLBAFound=false;
  global bool rotMatHBAFound=false;
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
  } else if ( isdir ("d:/data/MAC-CEPfeedback2857-CS001/data/configs/") ) {
    strDataDir = "d:/data/MAC-CEPfeedback2857-CS001/data/configs/";
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
      if (strtoupper(str2) != "LBA" && strtoupper(str2) != "HBA0" && strtoupper(str2) != "HBA1" && strtoupper(str2) != "HBA") {
        DebugN("readStationConfigs.ctl | Unknown NORMAL_VECTOR target found: ", str2);
        continue;
      } else {
       // read Normal Vector
        processNormalVector(strtoupper(str2));
      }
    } else if (strtoupper(str1) == "ROTATION_MATRIX" ) {
      if (strtoupper(str2) != "LBA" && strtoupper(str2) != "HBA0" && strtoupper(str2) != "HBA1" && strtoupper(str2) != "HBA") {
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
  if (!norVecHBA0Found && !norVecHBAFound ) {
    DebugN("Error? NORMAL_VECTOR for HBA0 not found.");
  }
  if (!norVecHBA1Found && !norVecHBAFound) {
    DebugN("Error? NORMAL_VECTOR for HBA1 not found.");
  }      
  if (!rotMatLBAFound ) {
    DebugN("Error? ROTATION_MATRIX for LBA not found.");
  }      
  if (!rotMatHBA0Found && !rotMatHBAFound) {
    DebugN("Error? ROTATION_MATRIX for HBA0 not found.");
  }      
  if (!rotMatHBA1Found && !rotMatHBAFound) {
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
  
  
  // calculate the rotated values and write back into the database
  if (deltasLBAFound && rotMatLBAFound) {
    calcRotated("LBA");
  } else {
    DebugN("LBA values or LBA rotationmatrix missing, no rotated values calulated");
  }
  if (deltasHBAFound && (rotMatHBAFound || rotMatHBA0Found || rotMatHBA1Found)) {
    calcRotated("HBA");
  } else {
    DebugN("HBA values or HBA(0)(1) rotationmatrix missing, no rotated values for HBA calulated");
  }
  if (deltasHBAFound && rotMatHBA0Found) {
    calcRotated("HBA0");
  } else {
    DebugN("HBA values or HBA0 rotationmatrix missing, no rotated values for HBA0 calulated");
  }
  if (deltasHBAFound && rotMatHBA1Found) {
    calcRotated("HBA1");
  } else {
    DebugN("HBA values or HBA1 rotationmatrix missing, no rotated values for HBA1 calulated");
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
        dpSet("LOFAR_PIC_StationInfo.stationID",value[2]);
      }
    }
    
    if (strpos(dynStr_fileContent[index],"RS.N_RSPBOARDS")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        dpSet("LOFAR_PIC_StationInfo.N_RSPBoards",value[2]);
      }
    }
    
    if (strpos(dynStr_fileContent[index],"RS.N_TBBOARDS")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        dpSet("LOFAR_PIC_StationInfo.N_TBBoards",value[2]);
      }
    }

    if (strpos(dynStr_fileContent[index],"RS.N_LBAS")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        dpSet("LOFAR_PIC_StationInfo.N_LBAS",value[2]);
      }
    }
      
    if (strpos(dynStr_fileContent[index],"RS.N_HBAS")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        dpSet("LOFAR_PIC_StationInfo.N_HBAS",value[2]);
      }
    }
  
    if (strpos(dynStr_fileContent[index],"RS.HBA_SPLIT")>-1) {
      dyn_string value = strsplit(dynStr_fileContent[index],"=");
      if (dynlen(value) > 1) {
        if (substr(value[2],0,1) == "N" ||
            substr(value[2],0,1) == "n" ||
            substr(value[2],0,1) == "F" ||
            substr(value[2],0,1) == "f") {
          dpSet("LOFAR_PIC_StationInfo.HBA_Split",false);
        } else {
          dpSet("LOFAR_PIC_StationInfo.HBA_Split",true);
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
          dpSet("LOFAR_PIC_StationInfo.wide_LBAS",false);
        } else {
          dpSet("LOFAR_PIC_StationInfo.wide_LBAS",true);
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
    dpSet("LOFAR_PIC_StationInfo.LBA.NormalVector.X",fX,
          "LOFAR_PIC_StationInfo.LBA.NormalVector.Y",fY,
          "LOFAR_PIC_StationInfo.LBA.NormalVector.Z",fZ);    
  } else if (aS == "HBA0") {
    norVecHBA0Found=true;
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA0.NormalVector.X",fX,
          "LOFAR_PIC_StationInfo.HBA.HBA0.NormalVector.Y",fY,
          "LOFAR_PIC_StationInfo.HBA.HBA0.NormalVector.Z",fZ);    
  } else if (aS == "HBA1") {
    norVecHBA1Found=true;
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA1.NormalVector.X",fX,
          "LOFAR_PIC_StationInfo.HBA.HBA1.NormalVector.Y",fY,
          "LOFAR_PIC_StationInfo.HBA.HBA1.NormalVector.Z",fZ);    
  } else if (aS == "HBA") {      // use HBA0 for foreign stations (1 HBA field)
    norVecHBAFound=true;
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA0.NormalVector.X",fX,
          "LOFAR_PIC_StationInfo.HBA.HBA0.NormalVector.Y",fY,
          "LOFAR_PIC_StationInfo.HBA.HBA0.NormalVector.Z",fZ);    
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
    dpSet("LOFAR_PIC_StationInfo.LBA.RotationMatrix.X",fX,
          "LOFAR_PIC_StationInfo.LBA.RotationMatrix.Y",fY,
          "LOFAR_PIC_StationInfo.LBA.RotationMatrix.Z",fZ);    
  } else if (aS == "HBA0") {
    rotMatHBA0Found=true;
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.X",fX,
          "LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.Y",fY,
          "LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.Z",fZ);    
  } else if (aS == "HBA1") {
    rotMatHBA1Found=true;
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA1.RotationMatrix.X",fX,
          "LOFAR_PIC_StationInfo.HBA.HBA1.RotationMatrix.Y",fY,
          "LOFAR_PIC_StationInfo.HBA.HBA1.RotationMatrix.Z",fZ);   
  } else if (aS == "HBA") {      // use HBA0 for foreign stations (1 HBA field)
    rotMatHBAFound=true;
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.X",fX,
          "LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.Y",fY,
          "LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.Z",fZ);   
  }


}   

void processFieldCenter(string aS) {
  float fX=0,fY=0,fZ=0;
  sscanf(dynStr_fileContent[index++],"%*d %*s %lf %lf %lf",fX,fY,fZ);
  if (bDebug) DebugN("Reading  fieldcenter "+aS+"X,Y,Z:" + fX + " " + fY + " " + fZ);
  if (aS== "LBA") {
    centerLBAFound=true;
    dpSet("LOFAR_PIC_StationInfo."+aS+".centerX",fX,
        "LOFAR_PIC_StationInfo."+aS+".centerY",fY,
        "LOFAR_PIC_StationInfo."+aS+".centerZ",fZ);
  } else if (aS == "HBA") {
    centerHBAFound=true;
    dpSet("LOFAR_PIC_StationInfo."+aS+".centerX",fX,
        "LOFAR_PIC_StationInfo."+aS+".centerY",fY,
        "LOFAR_PIC_StationInfo."+aS+".centerZ",fZ);
  } else if (aS == "HBA0") {
    centerHBA0Found=true;
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA0.centerX",fX,
          "LOFAR_PIC_StationInfo.HBA.HBA0.centerY",fY,
          "LOFAR_PIC_StationInfo.HBA.HBA0.centerZ",fZ);
  } else if (aS == "HBA1") {
    centerHBA1Found=true;
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA1.centerX",fX,
          "LOFAR_PIC_StationInfo.HBA.HBA1.centerY",fY,
          "LOFAR_PIC_StationInfo.HBA.HBA1.centerZ",fZ);
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
  

  // read nr of antennas
  sscanf(dynStr_fileContent[index++],"%d",nr_ofAnt);

  if (aS== "LBA") {
    deltasLBAFound=true;
    nr_LBA=nr_ofAnt;
  } else if (aS == "HBA") {
    deltasHBAFound=true;
    nr_HBA=nr_ofAnt;
  }

  if (bDebug) DebugN("Nr of Antenna's in this Config: "+nr_ofAnt);
  DebugN(aS,"--> will be read");
  for (int ix = index; ix < nr_ofAnt + index; ix++ ) {
    // read new line of delta's
    sscanf(dynStr_fileContent[ix],"%lf %lf %lf",deltaX,deltaY,deltaZ);
    if (bDebug) DebugN("X :"+deltaX+ " Y: "+ deltaY+ " Z: "+deltaZ);
    if (bDebug) DebugN("ix: " + ix + " index: " + index);
    if (bDebug) DebugN("Filling array at index: "+ (ix+1-index));
    
    if (deltasHBAFound) {
      int c=ix+1-index;
    }
    antConfFileX[(ix+1-index)] = deltaX;
    antConfFileY[(ix+1-index)] = deltaY;
    antConfFileZ[(ix+1-index)] = deltaZ;
  }
    
  int ix=nr_ofAnt;
  for (int i=0; i< ix;i++) {
    string ant=aS;
    if (ant=="HBA") {
      if (i < 10) {
        ant+="0"+i;
      } else if ( i >= 10) {
        ant+=i;
      }
    } else if (aS=="LBA") {
      if (i < 10) {
        ant+="00"+i;
      } else if ( i >= 10 && i < 100) {
        ant+="0"+i;
      } else if ( i >= 100) {
        ant+=i;
      }
    }    
    
    dpSet("LOFAR_PIC_"+ant+".common.deltaX",antConfFileX[i+1],
          "LOFAR_PIC_"+ant+".common.deltaY",antConfFileY[i+1],
          "LOFAR_PIC_"+ant+".common.deltaZ",antConfFileZ[i+1]);
  }
  index +=nr_ofAnt+1;
    
}
    
void calcRotated(string aS) {
  dyn_float rotX,rotY,rotZ;
  float centerX,centerY,centerZ;
  float X,Y,Z;
  float x1=0,x2=0,y1=0,y2=0,x3=0,x4=0,y3=0,y4=0;

  
  if (aS=="LBA") {
    dpGet("LOFAR_PIC_StationInfo.LBA.RotationMatrix.X",rotX,"LOFAR_PIC_StationInfo.LBA.RotationMatrix.Y",rotY,"LOFAR_PIC_StationInfo.LBA.RotationMatrix.Z",rotZ,
          "LOFAR_PIC_StationInfo.LBA.centerX",centerX,"LOFAR_PIC_StationInfo.LBA.centerY",centerY,"LOFAR_PIC_StationInfo.LBA.centerZ",centerZ);
    dyn_float result=rotate(centerX,centerY,centerZ,rotX,rotY,rotZ);
    dpSet("LOFAR_PIC_StationInfo.LBA.centerX",result[1],"LOFAR_PIC_StationInfo.LBA.centerY",result[2],"LOFAR_PIC_StationInfo.LBA.centerZ",result[3]);
    
  } else if (aS=="HBA" ) {
    dpGet("LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.X",rotX,"LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.Y",rotY,"LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.Z",rotZ,
          "LOFAR_PIC_StationInfo.HBA.centerX",centerX,"LOFAR_PIC_StationInfo.HBA.centerY",centerY,"LOFAR_PIC_StationInfo.HBA.centerZ",centerZ);
    dyn_float result=rotate(centerX,centerY,centerZ,rotX,rotY,rotZ);
    dpSet("LOFAR_PIC_StationInfo.HBA.centerX",result[1],"LOFAR_PIC_StationInfo.HBA.centerY",result[2],"LOFAR_PIC_StationInfo.HBA.centerZ",result[3]);

  } else if (aS == "HBA0") {
    dpGet("LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.X",rotX,"LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.Y",rotY,"LOFAR_PIC_StationInfo.HBA.HBA0.RotationMatrix.Z",rotZ,
          "LOFAR_PIC_StationInfo.HBA.HBA0.centerX",centerX,"LOFAR_PIC_StationInfo.HBA.HBA0.centerY",centerY,"LOFAR_PIC_StationInfo.HBA.HBA0.centerZ",centerZ);
    dyn_float result=rotate(centerX,centerY,centerZ,rotX,rotY,rotZ);
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA0.centerX",result[1],"LOFAR_PIC_StationInfo.HBA.HBA0.centerY",result[2],"LOFAR_PIC_StationInfo.HBA.HBA0.centerZ",result[3]);

  } else if (aS== "HBA1") {
    dpGet("LOFAR_PIC_StationInfo.HBA.HBA1.RotationMatrix.X",rotX,"LOFAR_PIC_StationInfo.HBA.HBA1.RotationMatrix.Y",rotY,"LOFAR_PIC_StationInfo.HBA.HBA1.RotationMatrix.Z",rotZ,
          "LOFAR_PIC_StationInfo.HBA.HBA1.centerX",centerX,"LOFAR_PIC_StationInfo.HBA.HBA1.centerY",centerY,"LOFAR_PIC_StationInfo.HBA.HBA1.centerZ",centerZ);
    dyn_float result=rotate(centerX,centerY,centerZ,rotX,rotY,rotZ);
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA1.centerX",result[1],"LOFAR_PIC_StationInfo.HBA.HBA1.centerY",result[2],"LOFAR_PIC_StationInfo.HBA.HBA1.centerZ",result[3]);

  }  
  
  if (aS == "HBA") {
    for (int i=0;i < nr_HBA; i++) {
      string hba="HBA";
      if (i < 10) {
        hba+="0"+i;
      } else if ( i >= 10) {
        hba+=i;
      }
      dpGet("LOFAR_PIC_"+hba+".common.deltaX",X,"LOFAR_PIC_"+hba+".common.deltaY",Y,"LOFAR_PIC_"+hba+".common.deltaZ",Z);
      dyn_float result=rotate(X,Y,Z,rotX,rotY,rotZ);
       // keep x y from hba0 and hba1 for rotationcalculations hba0
       // keep x y from hba24 and hba25 for rotationcalculations hba1
      if (i == 0) {
        x1=result[1];
        y1=result[2];
      } else if (i == 1) {
        x2=result[1];
        y2=result[2];
      } else if (i == 24) {
        x3=result[1];
        y3=result[2];
      } else if (i == 25) {
        x4=result[1];
        y4=result[2];
      } 

      dpSet("LOFAR_PIC_"+hba+".common.deltaX",result[1],"LOFAR_PIC_"+hba+".common.deltaY",result[2],"LOFAR_PIC_"+hba+".common.deltaZ",result[3]);
    }
    // calculate rotationcorner of this field
    float dx1=x2-x1;
    float dy1=y2-y1;
    float angle1 = 0;
    if (dx1 != 0) {
      angle1 = rad2deg(atan(dy1/dx1));
    }
    if (bDebug) DebugN("x1,x2",x1,x2);
    if (bDebug) DebugN("Y1,y2",y1,y2);
    if (bDebug) DebugN("dy1,dx1",dy1,dx1);
    if (bDebug) DebugN("atan(dy1/dx1):" ,atan(dy1/dx1));
    if (bDebug) DebugN("Angle for  HBA0 = ",angle1);
    float dx2=x4-x3;
    float dy2=y4-y3;
    float angle2=0;
    if (dx2 != 0) {
      angle2 = rad2deg(atan(dy2/dx2));
    }
    if (bDebug) DebugN("x3,x4",x3,x4);
    if (bDebug) DebugN("Y3,y4",y3,y4);
    if (bDebug) DebugN("dy2,dx2",dy2,dx2);
    if (dx2 != 0) {
      if (bDebug) DebugN("atan(dy2/dx2):" ,atan(dy2/dx2));
    }
    if (bDebug) DebugN("Angle for  HBA1 = ",angle2);
    dpSet("LOFAR_PIC_StationInfo.HBA.HBA0.rotation",angle1,
          "LOFAR_PIC_StationInfo.HBA.HBA1.rotation",angle2);
  } else if (aS == "LBA" )  {
    for (int i=0;i < nr_LBA; i++) {
      string lba="LBA";
      if (i < 10) {
        lba+="00"+i;
      } else if ( i >= 10 && i < 100) {
        lba+="0"+i;
      } else if ( i >= 100) {
        lba+=i;
      }
                  
      dpGet("LOFAR_PIC_"+lba+".common.deltaX",X,"LOFAR_PIC_"+lba+".common.deltaY",Y,"LOFAR_PIC_"+lba+".common.deltaZ",Z);
      dyn_float result=rotate(X,Y,Z,rotX,rotY,rotZ);
      dpSet("LOFAR_PIC_"+lba+".common.deltaX",result[1],"LOFAR_PIC_"+lba+".common.deltaY",result[2],"LOFAR_PIC_"+lba+".common.deltaZ",result[3]);
    }
  }
  
}

dyn_float rotate(float X, float Y, float Z, dyn_float rotX, dyn_float rotY, dyn_float rotZ) {
  dyn_float result=makeDynString(0,0,0);

  result[1]=(X*rotX[1])+(Y*rotX[2])+(Z*rotX[3]);
  result[2]=(X*rotY[1])+(Y*rotY[2])+(Z*rotY[3]);
  result[3]=(X*rotZ[1])+(Y*rotZ[2])+(Z*rotZ[3]);
    
  return result;
}
