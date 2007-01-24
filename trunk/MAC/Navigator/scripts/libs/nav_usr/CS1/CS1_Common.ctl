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

#uses "nav_fw/gcf-util.ctl"


// check if the panel is placed on the right datatype
// aDP        = found datatpointtype
// aCheckType = required DPT

navViewCheckDpType(string aDp, string aCheckType){
  LOG_DEBUG("CheckType : ",aCheckType);
  LOG_DEBUG("DP        : ",$datapoint);
  LOG_DEBUG("dpTypeName: ",dpTypeName($datapoint));

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
  LOG_DEBUG("datapoint: ",aDP);
  LOG_DEBUG("index: " , index);
  dyn_string dpElements = splitDatapointPath(aDP);
  if(dynlen(dpElements) >= index)
  {
    result = dpElements[3];
  }
  return result;
}


//
// compactedArrayString(string)
//
// Given een array string ( '[ xx, xx, xx ]' ) this utility compacts the string
// by replacing series with range.
// Eg. [ lii001, lii002, lii003, lii005 ] --> [ lii001..lii003, lii005 ]
//
// ATTENTION !!!!!   this code is a duplicate from the code used in: LOFAR/MAC/APL/APLCommon/src/APLUtilities.cc
// If a structural bug is discovered. or the code has been modified then you also should have a look there !!!
//
string compactedArrayString(const string orgStr)
{

  LOG_DEBUG("orgStr: " , orgStr);
  // destroyable copy
  string baseString=orgStr;

  // strip the brackets
  string copy=strltrim(baseString," 	[");
  baseString =strrtrim(copy," 	]");

  // strip spaces and tabs from string
  strreplace(baseString," ","");
  strreplace(baseString,"	","");
  LOG_DEBUG("baseString: ", baseString);
  
	 
  //and split into vector
  dyn_string strVector= strsplit(baseString,',');
  if (dynlen(strVector) < 2) {
    return (orgStr);
  }
  LOG_DEBUG("Vector length: ",dynlen(strVector));
  LOG_DEBUG("First element: " , strVector[1]);
	
  //note: We assume that the format of each element is [xxxx]9999
  int firstDigit = strtok(strVector[1],"0123456789");
  //No position? then return original string
  if (firstDigit == -1) {
    return (orgStr);
  }
  LOG_DEBUG("First Digit of 0123456789 in " , strVector[1] , " at place: ", firstDigit); 
	

  string elemName = substr(strVector[1],0,firstDigit);
  string scanMask = elemName+"%ld";
  string outMask;
  int i = sprintf(outMask,"%s%%0%dld",elemName,strlen(strVector[1])-strlen(elemName));
  if (i < 0 ) {
    LOG_WARN("Error composing outMask");
    return (orgStr);
  }
  LOG_DEBUG("elemName: " , elemName);
  LOG_DEBUG("scanMask: " , scanMask);
  LOG_DEBUG("outMask: " , outMask);
	
  string result       = "[";
  long   prevValue    = -2;
  bool   firstElem    = true;
  bool   endOfArray   = false;
  int    elemsInRange = 0;
  int    nrElems      = dynlen(strVector);
	
  for (int idx=1; idx < nrElems+1; idx++) {
    LOG_DEBUG("-----------Loop ",idx,"-----------");
    long value;
    LOG_DEBUG("Value to scan: ",strVector[idx]);
    int scanResult=sscanf(strVector[idx],scanMask,value);
    LOG_DEBUG("ScanResult: ",scanResult);
    if ( scanResult <=0) {
      LOG_DEBUG("Element: " , strVector[idx] , " does not match mask " , scanMask , ". Returning original string.");
      return (orgStr);
    }
	  
    // contiquous numbering?
    if (value == prevValue+1) {
      elemsInRange++;
      prevValue = value;
      if (idx < nrElems) {		// when elements left
	continue;
      }
      endOfArray = true;
    }
	
    // broken range
    string valString="";
    int i = sprintf(valString,outMask,value);
    if (i < 0 ) {
      LOG_WARN("Error composing valString");
      return (orgStr);
    }		  


    LOG_DEBUG("value: ",value);
    LOG_DEBUG("prevValue: ",prevValue);
    LOG_DEBUG("firstElem: ",firstElem);
    LOG_DEBUG("elemsInRange: ",elemsInRange);
    LOG_DEBUG("endOfArray: ",endOfArray);

    string prevValString="";
    i = sprintf(prevValString,outMask,prevValue);
    if (i < 0 ) {
      LOG_DEBUG("Error composing prevValString");
      return (orgStr);
    }		  
    if (firstElem) {
      result += valString;
      firstElem = false;
    } else {
      if (elemsInRange == 1) {
        result += "," + valString;
      } else {
        if (elemsInRange == 2) {
          result += "," + prevValString;
        } else {
          result += ".." + prevValString;
        }
        if (!endOfArray) {
          result += "," + valString;
        }
      }
    }
    LOG_DEBUG("result: ",result);

    elemsInRange = 1;
    prevValue    = value;
  }
  return (result+"]");
}

//
// expandedArrayString(string)
//
// Given een array string ( '[ xx..xx, xx ]' ) this utility expands the string
// by replacing ranges with the fill series.
// Eg. [ lii001..lii003, lii005 ] --> [ lii001, lii002, lii003, lii005 ]
//
//
// ATTENTION !!!!!   this code is a duplicate from the code used in: LOFAR/MAC/APL/APLCommon/src/APLUtilities.cc
// If a structural bug is discovered. or the code has been modified then you also should have a look there !!!
//
string expandedArrayString(const string	orgStr)
{
  LOG_DEBUG("orgString: " , orgStr);
  // any ranges in the string?
  if (strpos(orgStr,"..") < 0) {
    return (orgStr);						// no, just return original
  }

  string	baseString=orgStr;				// destroyable copy
  // strip the brackets
  string copy=strltrim(baseString," 	[");
  baseString =strrtrim(copy," 	]");

  // strip spaces and tabs from string
  strreplace(baseString," ","");
  strreplace(baseString,"	","");
  LOG_INFO("baseString: " , baseString);
  
  //and split into vector
  dyn_string strVector= strsplit(baseString,',');
  LOG_DEBUG("Vector length: ",dynlen(strVector));
  LOG_DEBUG("First element: " , strVector[1]);

  //note: We assume that the format of each element is [xxxx]9999
  int firstDigit = strtok(strVector[1],"0123456789");
  //No position? then return original string
  LOG_DEBUG("first occurance of 0123456789 at place: ", firstDigit);
  if (firstDigit == -1) {
    return (orgStr);
  }	

  // construct scanmask and outputmask.
  string elemName = substr(strVector[1],0,firstDigit);
  string scanMask = elemName+"%ld";
  int		nrDigits;
  LOG_DEBUG("Pos .. :",strpos(strVector[1],".."));

  if (strpos(strVector[1],"..") >-1 ) {	// range element?
    nrDigits = ((strlen(strVector[1]) - 2)/2) - strlen(elemName);
  } else {
    nrDigits = strlen(strVector[1]) - strlen(elemName);
  } 
  LOG_DEBUG("elemName: ", elemName);
  LOG_DEBUG("scanMask: ", scanMask);
  LOG_DEBUG("nrDigits: ", nrDigits);


  string outMask;
  int i = sprintf(outMask,"%s%%0%dld",elemName,nrDigits);
  if (i < 0 ) {
    LOG_WARN("Error composing outMask");
    return (orgStr);
  }
  LOG_DEBUG("outMask: ", outMask);

  // handle all elements
  string result  = "[";
  bool	 firstElem = true;
  int	 nrElems   = dynlen(strVector);

  for (int idx = 1; idx < nrElems+1; idx++) {
    long	firstVal;
    long	lastVal;
    LOG_DEBUG("Working on: " , strVector[idx]);

    // should match scanmask.
    if (sscanf(strVector[idx], scanMask, firstVal) <= 0) {
      LOG_DEBUG("Element: " + strVector[idx] + " does not match mask " + 
               scanMask + ". Returning original string.");
      return (orgStr);
    }
    LOG_DEBUG("firstVal: " ,firstVal);

    // range element?
    int rangePos=strpos(strVector[idx],"..");
    LOG_DEBUG("rangePos: ",rangePos);

    if (rangePos < 0) {
      lastVal = firstVal;
    } else {	// yes, try to get second element.
      LOG_DEBUG("Input for sscanf: " , substr(strVector[idx],rangePos+2,strlen(strVector[idx])-(rangePos+2)));

      if (sscanf(substr(strVector[idx],rangePos+2,strlen(strVector[idx])-(rangePos+2)), scanMask, lastVal) <= 0) {
      	LOG_DEBUG("Second part of element: " + strVector[idx] + " does not match mask " + 
                scanMask + ". Returning original string.");
      	return (orgStr);
      }
			
      // check range
      if (lastVal < firstVal) {
        LOG_WARN("Illegal range specified in " + strVector[idx] +
                ". Returning orignal string");
        return (orgStr);
      }
    }

    
    // finally construct one or more elements
    for	(long val = firstVal ; val <= lastVal; val++) {
      string valString="";
      int i = sprintf(valString,outMask,val);
      if (i < 0 ) {
        LOG_DEBUG("Error composing valString");
        return (orgStr);
      }
      if (firstElem) {
        result += valString;
        firstElem = false;
      } else {
        result += "," + valString;
      }
    }
  }

  return (result+"]");
}

