//# CS1_Common.ctl
//#
//#  Copyright (C) 2002-2004
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

//#
//# All user panel scripts functions. The scipts in the script file are user script
//# which are an example of visualisations of (parts of) the LOFAR instrument.
//# And are NOT part of the MAC Navigator framework.
//#


// check if the panel is placed on the right datatype
// aDP        = found datatpointtype
// aCheckType = required DPT

navViewCheckDpType(string aDp, string aCheckType){
  //DebugN(aCheckType);
  //DebugN(dpTypeName($datapoint));
  if (aCheckType != dpTypeName($datapoint)){
    ChildPanelOnCentralModal("objects/nav_fw/warning_wrong_datatype.pnl",
	                     "WRONG datapointtype",
	                     makeDynString("$datapointType:" + aCheckType,
	                     "$typeName:" +dpTypeName($datapoint)));
   
  }
}

//
// get part at place <index> from a datapointpath <aDP>
//
string getPathComponent(string aDP,int index) {
  string result="";
  //DebugTN("datapoint: "+aDP);
  //DebugTN("index: " + index);
  dyn_string dpElements = splitDatapointPath(aDP);
  if(dynlen(dpElements) >= index)
  {
    result = dpElements[3];
  }
  return result;
}

