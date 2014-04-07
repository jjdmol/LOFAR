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
#uses "navFunct.ctl"


/**
  * Controller that will be run once during ccu startup.
  * it will search for different conf files in /opt/lofar/etc/
  * and will fill the pvss database types for those config files.
  */
main()
{ 

  bool showDebug = false;
  
  // first empty old settings
  
  emptyIONodes();
       
  string strCurConfig;
  string strDataDir     = "";
  string strDataDir             = ""; 
  if (isdir("/opt/lofar/etc/") ) {
    strDataDir = "/opt/lofar/etc/StaticMetaData/";
  } else if ( isdir ("d:/data/MAC-CEPfeedback2857-CCU001/data/configs/") ) {
    strDataDir = "d:/data/MAC-CEPfeedback2857-CCU001/data/configs/";
  } else {
    DebugN("Could not find datadir to work with, leaving and no antenne data read.");
    return;
  }
  
  string strRSPDatFile   = strDataDir+"RSPConnections_CCU.dat";

  dyn_string dynStr_RSPfile;
   
  // first read the file
  dynStr_RSPfile = lto_getFile_asDynStr(strRSPDatFile); 
  
  DebugN("Filling Database from file " + strRSPDatFile);
  for (int index=1;index <= dynlen(dynStr_RSPfile);index++) {
    if (strpos(dynStr_RSPfile[index],"#") < 0 || strpos(dynStr_RSPfile[index],"#") > 4) {
      
      dyn_string linesplitted=strsplit(dynStr_RSPfile[index]," \t");
      if (showDebug) DebugN(index+" :"+linesplitted);

      string station= linesplitted[1];
      string rspstr = linesplitted[2];
      string ionode = linesplitted[3];
      string ip     = linesplitted[4];
      string mac    = linesplitted[5];
      // if rack = 1 but rack 1 is not in use, then skip line
      if (strpos(ionode,"R01") >= 0 && !navFunct_isBGPSwitch()) {
        continue;
      } else if (strpos(ionode,"R00") >= 0 && navFunct_isBGPSwitch()) {
        continue;
      }
      // the station/mac/ip places are for the cases were rsp1 can be the 2nd ear or a foreign station
      // if a foreign station is used they will be in the list as R(00-01)_BG(1-3)_(DE,FR,SE,UK)(601-608)
      // and the real ionode can be found based on the shared ipnr
      // then the info will go to the 2nd station in the database
        
      int stationPlace=1;
      if (strpos(ionode,"BG") < 0) {
        stationPlace = 0;
      } else {
        if (showDebug) DebugN(" ionode contains BG router name, trying to find real ionode for connection based on ip: "+ip);
        ionode="";
        // check list based on ipnr and find the real ionode
        for (int idx=1;idx <= index;idx++) {
          if (strpos(dynStr_RSPfile[idx],ip) >= 0) {
            if (showDebug) DebugN(" found match for ip in: " + dynStr_RSPfile[idx]);
            dyn_string  sp = strsplit(dynStr_RSPfile[idx]," \t"); 
            ionode= sp[3];
            break;
          }
        }
      }
        
      if (ionode == "" ) {
        DebugN("Found BG name in ionode, but couldn't find ip match. skipping....");
        continue;
      } else {
        if (showDebug) DebugN("ionode match found: "+ionode); 
      }  
      
      string ioname = "LOFAR_PIC_"+navFunct_CEPName2DPName(ionode);
 
                
               
      dyn_string rsp = strsplit(rspstr,"_");
      int nr = rsp[2];
      if (showDebug) DebugN( "node: "+ionode+ "  rspfull: " + rspstr+ "  rsp[2]" + rsp[2]+ " nr: "+nr
                             + " ip: "+ip+ " mac: "+mac);
      if (dpExists(ioname)) {
        dpSet(ioname+".station"+stationPlace,station);
        dpSet(ioname+".IP"+stationPlace,ip);
        dpSet(ioname+".MAC"+stationPlace,mac);
        
      } else {
          DebugN(ionode+" gives wrong dp: " , ioname);
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

void emptyIONodes() {
  for (int i = 0; i < 64;i++) {
    string ext = "";
    if (i < 10) ext = "0";
    string dp = "LOFAR_PIC_BGP_Midplane"+navFunct_IONode2Midplane(i)+"_IONode"+ext+i;  
    if (dpExists(dp+".station0")) {
      dpSet(dp+".station0","");
      dpSet(dp+".IP0","");
      dpSet(dp+".MAC0","");
      dpSet(dp+".station1","");
      dpSet(dp+".IP1","");
      dpSet(dp+".MAC1","");
    } else {
      DebugN("wrong dp found: "+dp+".station0");
    }
  }
}
