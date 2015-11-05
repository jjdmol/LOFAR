//# readStationBGPConnections.ctl
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

bool showDebug = false;

/**
  * Controller that will be run once during ccu startup.
  * it will search for different conf files in /opt/lofar/etc/
  * and will fill the pvss database types for those config files.
  */
main()
{ 

  
  // first empty old settings
  
  emptyIONodes();
       
  string strDataDir     = "";
  if (isdir("/opt/lofar/etc/") ) {
    strDataDir = "/opt/lofar/etc/";
  } else if ( isdir ("d:/data/TRUNK-CCU001/data/configs/") ) {
    strDataDir = "d:/data/TRUNK-CCU001/data/configs/";
  } else {
    DebugN("Could not find datadir to work with, leaving and no antenne data read.");
    return;
  }
    
  readBGPDat(strDataDir+"RSPConnections_CCU.dat");
  readCobaltDat(strDataDir+"RSPConnections_Cobalt.dat");

}  

void readBGPDat(string strBGPDatFile) {
  dyn_string dynStr_BGPFile;
   
  // first read the file
  dynStr_BGPFile = lto_getFile_asDynStr(strBGPDatFile); 
  
  DebugN("Filling Database from file " + strBGPDatFile);
  for (int index=1;index <= dynlen(dynStr_BGPFile);index++) {
    if (strpos(dynStr_BGPFile[index],"#") < 0 || strpos(dynStr_BGPFile[index],"#") > 4) {
      
      dyn_string linesplitted=strsplit(dynStr_BGPFile[index]," \t");
      if (showDebug) DebugN(index+" :"+linesplitted);

      string station    = linesplitted[1];
      string rspstr     = linesplitted[2];
      string ionode     = linesplitted[3];
      string ip         = linesplitted[4];
      string mac        = linesplitted[5];
      string macForeign = "";
      string rspForeign = "";
      // the station/mac/ip places are for the cases were rsp1 can be the 2nd ear or a foreign station
      // if a foreign station is used they will be in the list as R(00-01)_BG(1-3)_(DE,FR,SE,UK)(601-608)
      // and the real ionode can be found based on the shared ipnr
      // then the info will go to the 2nd station in the database
        
      int stationPlace=1;
      if (strpos(ionode,"R00") >= 0) {
        stationPlace=0;
      }
      if (strpos(ionode,"BG") >= 0) {
        if (showDebug) DebugN(" ionode contains BG router name, trying to find real ionode for connection based on ip: "+ip);
        ionode="";
        // check list based on ipnr and find the real ionode
        for (int idx=1;idx <= index;idx++) {
          if (strpos(dynStr_BGPFile[idx],ip) >= 0) {
            if (showDebug) DebugN(" found match for ip in: " + dynStr_BGPFile[idx]);
            dyn_string  sp = strsplit(dynStr_BGPFile[idx]," \t"); 
            ionode= sp[3];
            macForeign = mac;
	    //foreign stations always connected to HBA1 (= RSP1)  allthough the foreign station will say RSP0
            rspForeign = "RSP"+1;
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
      if (showDebug) DebugN( "node: "+ionode+ "  ioname: "+ioname+"  rspfull: " + rspstr+ "  rsp[2]" + rsp[2]
                             + " ip: "+ip+ " mac: "+mac);
      if (dpExists(ioname)) {
        dpSet(ioname+".IP"+stationPlace,ip);
        if (macForeign != "") {
          dpSet(ioname+".MACForeign",macForeign);
          dpSet(ioname+".station1",station);          
        } else {
          dpSet(ioname+".MAC"+stationPlace,mac);
          dpSet(ioname+".station0",station);
        }
        if (rspForeign != "") {
          dpSet(ioname+".RSPForeign",rspForeign);       
          dpSet(ioname+".RSP1","RSP"+rsp[2]);          
	      } else {
          dpSet(ioname+".RSP"+stationPlace,"RSP"+rsp[2]);          
	      }
      } else {
          DebugN(ionode+" gives wrong dp: " , ioname);
      }
    }
  }
  DebugN("Ready");
}

void readCobaltDat(string strCobaltDatFile) {
  dyn_string dynStr_CobaltFile;
  int iPos=-1;
  mapping cobaltConnections;
  
  cobaltConnections[ "NODE"  ]    = makeDynString();
  cobaltConnections[ "STATION" ] = makeDynString();
  cobaltConnections[ "RSP"  ]     = makeDynString();
  cobaltConnections[ "IP"  ]      = makeDynString();
  cobaltConnections[ "MAC"  ]     = makeDynString();
  
   
  // first read the file
  dynStr_CobaltFile = lto_getFile_asDynStr(strCobaltDatFile); 
  
  if (showDebug) DebugN("Filling Database from file " + strCobaltDatFile);
  for (int index=1;index <= dynlen(dynStr_CobaltFile);index++) {
    if (strpos(dynStr_CobaltFile[index],"#") < 0 || strpos(dynStr_CobaltFile[index],"#") > 4) {
     
      if (strlen(dynStr_CobaltFile[index]) < 1) continue;
      
      string aLine = dynStr_CobaltFile[index];
      
      dyn_string linesplitted=strsplit(aLine," \t");
      if (dynlen(linesplitted) < 5) {
        if (showDebug) DebugN("ERROR : inputline has to few parameters: "+aLine);
        continue;
      }
        
      if (showDebug) DebugN(index+" :"+linesplitted);

      string station    = linesplitted[1];
      string rspstr     = linesplitted[2];
      string ionode     = linesplitted[3];
      string ip         = linesplitted[4];
      string mac        = linesplitted[5];
        
      if (ionode == "" ) {
        DebugN("didnt find nodeName. skipping....");
        continue;
      } else {
        if (showDebug) DebugN("node match found: "+ionode);
      }  
      dyn_string rspsplit= strsplit(rspstr,"_");
      int rsp= rspsplit[2];
      
      // Is this an existing node or a new one
      if (dynlen(cobaltConnections[  "NODE" ]) >= 1)  iPos = dynContains(cobaltConnections[  "NODE"  ], ionode );  
  
      if( iPos < 1 ){
        dynAppend( cobaltConnections[  "NODE"  ], ionode );
        dynAppend( cobaltConnections[  "RSP"  ], "");
        dynAppend( cobaltConnections[  "STATION"  ], "" );
        dynAppend( cobaltConnections[  "IP"  ], "" );
        dynAppend( cobaltConnections[  "MAC"  ], "" );
        iPos = dynlen( cobaltConnections[  "NODE"  ] );
      }  
      cobaltConnections[ "MAC"  ][iPos]     = mac;
      string stations =  cobaltConnections[ "STATION" ][iPos];
      if (stations != "") stations+=" ";
      stations += station;
      cobaltConnections[ "STATION" ][iPos] = stations;
      
      string rsps =  cobaltConnections[ "RSP" ][iPos];
      if (rsps != "") rsps += " ";
      rsps += rsp;
      cobaltConnections[ "RSP" ][iPos] = rsps;

      string ips =  cobaltConnections[ "IP" ][iPos];
      if (ips != "") ips += " ";
      ips += ip;
      cobaltConnections[ "IP" ][iPos] = ips;
    }
  }
  
  dyn_string compareList;
  
  // now empty the mapping into the NICs in the DB
  
  for (int i=1; i<= dynlen(cobaltConnections[ "NODE" ]); i++) {
                              
    string ionode   = cobaltConnections[ "NODE" ][i];
    string mac      = cobaltConnections[ "MAC" ][i];
    string stations = cobaltConnections[ "STATION" ][i];
    string rsps     = cobaltConnections[ "RSP" ][i];
    string ips      = cobaltConnections[ "IP" ][i];
    
    string node       = "";

    string nic        = "";
    dyn_string aDS = strsplit(ionode,"-");
    node = aDS[1];
    nic = aDS[2];
    int nNr = substr(nic,strlen(nic)-1);
    int nodeNr = substr(node,3);
    int nicNr = (((nodeNr)-1)*4)+(nNr-1);
    dyn_string stationList=strsplit(stations," ");
    dyn_string RSPList=strsplit(rsps," ");
    dyn_string IPList=strsplit(ips," ");

    int ethNr = nNr+1;
    
    string dpname = CEPDBName+"LOFAR_PIC_Cobalt_"+strtoupper(node)+"_CobaltNIC"+navFunct_formatInt(nicNr,"99");

    if (showDebug) DebugN( "node: "+node+ "  nic: "+nic +"  stationList: " + stationList+ "  RSPList" + RSPList
                             + " IPList: "+IPList+ " mac: "+mac+" dp: "+dpname);
    
    if (showDebug) {
      for (int j=1; j<= dynlen(stationList); j++) {
        string comp = stationList[j]+ " " + RSPList[j]+ " " + ionode + " " + IPList[j] + " " +mac;
        dynAppend(compareList, comp);
      }
    }

    int cpu=0;
    if (nNr >1) cpu=1;
    string eth = "eth"+ethNr;
    
    if (dpExists(dpname)) {
      dpSet(dpname+".Node",node);
      dpSet(dpname+".CPU",cpu);
      dpSet(dpname+".Interface",eth);
      dpSet(dpname+".MAC",mac);
      dpSet(dpname+".RSPList",RSPList);
      dpSet(dpname+".IPList",IPList);
      dpSet(dpname+".stationList",stationList);
    } else {
     DebugN(ionode+" gives wrong dp: " + dpname);
    }
  }
  if (showDebug) {
    dynSort(compareList);
    for (int j=1; j <= dynlen(compareList); j++) {
      DebugN(compareList[j]);
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
      dpSet(dp+".MACForeign","");
    } else {
      DebugN("wrong dp found: "+dp+".station0");
    }
  }
}
