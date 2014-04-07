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
       
  string strCurConfig;
  string strDataDir     = "";
  string strDataDir             = ""; 
  if (isdir("/opt/lofar/etc/") ) {
    strDataDir = "/opt/lofar/etc/StaticMetaData/";
  } else if ( isdir ("d:/data/TRUNK-CCU001/data/configs/") ) {
    strDataDir = "d:/data/TRUNK-CCU001/data/configs/";
  } else {
    DebugN("Could not find datadir to work with, leaving and no antenne data read.");
    return;
  }
  
  string strRSPDatFile   = strDataDir+"RSPConnections.dat";
  string strMACDatFile   = strDataDir+"MAC+IP.dat";

  dyn_string dynStr_RSPfile;
  dyn_string dynStr_MACfile;
   
  // first read the files
  dynStr_RSPfile = lto_getFile_asDynStr(strRSPDatFile);
  dynStr_MACfile = lto_getFile_asDynStr(strMACDatFile);
  
  DebugN("Filling Database from file " + strRSPDatFile);
  for (int index=1;index <= dynlen(dynStr_RSPfile);index++) {
    if (strpos(dynStr_RSPfile[index],"#") < 0 || strpos(dynStr_RSPfile[index],"#") > 4) {
      dyn_string linesplitted=strsplit(dynStr_RSPfile[index]," \t");
      if (showDebug) DebugN(index+" :"+linesplitted);
      // check if stationname is not empty
      if (dynlen(linesplitted) == 3 && linesplitted[3] != "") {

        string ionode = linesplitted[3];
        string ioname = "LOFAR_PIC_"+navFunct_CEPName2DPName(ionode);
 
        int stationPlace=1;
        if (strpos(ionode,"BG") < 0) {
          stationPlace = 0;
        } else {
          if (showDebug) DebugN("BGP in name found: " + ionode);
          continue;
        }
        int rackPlace=1;
        if (strpos(ionode,"R00") < 0) {
            rackPlace=0;
        }
        
        dyn_string rsp = strsplit(linesplitted[2],"_");
        int nr = rsp[2];
        if (showDebug) DebugN( "node: "+ionode+ "  rspfull: " + linesplitted[2]+ "  rsp[2]" + rsp[2]+ "nr: "+nr);
        if (dpExists(ioname)) {
          dpSet(ioname+".station"+stationPlace,linesplitted[1]);
//          dpSet(ioname+".RSPBoard"+rackPlace,nr);
        } else {
            DebugN(ionode+" gives wrong dp: " , ioname);
        }
      }   
    }
  }
  DebugN("Ready");

  // now read MAC+IP
  DebugN("Filling Database from file " + strMACDatFile);
  for (int index=1;index <= dynlen(dynStr_MACfile);index++) {
    if (strpos(dynStr_MACfile[index],"#") < 0 || strpos(dynStr_MACfile[index],"#") > 4) {
      dyn_string linesplitted=strsplit(dynStr_MACfile[index]," \t");
      
      if (showDebug) DebugN(index+" :"+linesplitted);
      // check if stationname is not empty
      if (dynlen(linesplitted) == 3 && linesplitted[1] != "") {
        string ionode= linesplitted[1];
        if (strpos(ionode,"R02") >= 0) {
          continue;
        }
        int rackPlace=1;
        if (strpos(ionode,"R00") < 0) {
            rackPlace=0;
        }

        string ioname = "LOFAR_PIC_"+navFunct_CEPName2DPName(ionode);

        
        int stationPlace=1;
        
        if (strpos(ionode,"BG") < 0) {
          stationPlace = 0;
        } else {
          if (showDebug) DebugN("BGP in name found: " + ionode);
          // we need te find out the foreign connection info out of two files
          // first we need the Station and rspinfo that belong to thies node
          dyn_string rspinfo = dynPatternMatch("*"+ionode+"*",dynStr_RSPfile);
          // take stationname and rspnr
          if (dynlen(rspinfo) < 1) {
            DebugN("no match in RSPconnections.dat for pattern: "+ ionode);
            continue;
           }
          dyn_string spl=strsplit(rspinfo[1]," \t");
          string station = spl[1];
          dyn_string rsp = strsplit(spl[2],"_");
          int nr = rsp[2];
          
          
          //now look for match on ip nr in same file
          dyn_string ipinfo = dynPatternMatch("*"+linesplitted[2]+"*",dynStr_MACfile);
          // there must be 2 hits, 1st is the right node, 2nd is the initial line with BG
          if (dynlen(ipinfo) < 2) {
            DebugN("couldn't find match on ip for :"+ionode+" with pattern "+"*"+linesplitted[2]+"*");
            continue;
          }
          spl=strsplit(ipinfo[1]," \t");
          ionode= spl[1];
          ioname = "LOFAR_PIC_"+navFunct_CEPName2DPName(ionode);
          if (dpExists(ioname)) {
            dpSet(ioname+".station"+stationPlace,station);
//            dpSet(ioname+".RSPBoard"+rackPlace,nr);
          } else {
            DebugN(ionode+" gives wrong dp: " , ioname);
            continue;
          }          
        } 

        if (dpExists(ioname)) {
          dpSet(ioname+".IP"+rackPlace,linesplitted[2]);
          dpSet(ioname+".MAC"+rackPlace,linesplitted[3]);
        } else {
            DebugN(ionode+" gives wrong dp: " , ioname);
        }
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
