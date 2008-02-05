//# gcf-util.ctl
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
//#  $Id$

//#
//# General util functions:



////////////////////////////////////////////////////////////////////////////////
//
// Function showPopupMessage (caption, message)
//
// Puts up a modal-dialog with the given caption displaying the given message,
//
////////////////////////////////////////////////////////////////////////////////
void showPopupMessage(string caption, string message) {
  	dyn_string messageboxParams = makeDynString("$1:"+message);
  	ChildPanelOnCentral("vision/MessageWarning",caption, messageboxParams);
}

//////////////////////////////////////////////////////////////////////////////////
//
// Function progressBar
//
// With this function a progress bar can be made. The progress is shown
// horizontal and is going from the left to the right.
// [0                                  100%]
// 
// Input: 1. maximum value (range) == 100%
//        2. current value to show in the progress bar.
//
///////////////////////////////////////////////////////////////////////////////////
void progressBar(float Maximum, float value) {
	LOG_DEBUG("progressBar: ", Maximum, value);

	float Minimum = 0;
	int	  waarde;
	float positie;

	if (value > Minimum) {	// make progressBar visible
		setValue("progressBar", "visible", TRUE);
		setValue("progressBar_border", "visible", TRUE);
	}

	// set bar to given value
	setValue("progressBar", "scale", value/Maximum, 1.0);

	if (Maximum >= value) {	// hide progressbar when end is reached
		delay(0, 200);
		setValue("progressBar", "visible", FALSE);
		setValue("progressBar_border", "visible", FALSE);
	}
}




///////////////////////////////////////////////////////////////////////////
//
// Function getArmFromStation
// 
// Will return the armName based upon the stationName. 
// for now (CS1 fase) it will return Core, later something smart has to be
// done to give the other ringnames correctly.
// It is needed to get the correct datapoints in constructions like:
// LOFAR_PermSW_Core_CS010.state when you get a CS010:LOFAR_PermSW.state
// change.
//
// Added 3-4-2007 A.Coolen
///////////////////////////////////////////////////////////////////////////
string getArmFromStation(string stationName) {

  	string armName="";
  	if (substr(stationName,0,2) == "CS") {
    	armName="Core";
  	}
  	return armName;
}




///////////////////////////////////////////////////////////////////////////
//
// Function queryDatabase
//
// Query's the (distributed)database according the given options
//
// Input: 1. Attribute (E.g. "_online.._value")
//        2. Datapoint name, including systemName
//        3. How many first items must be retrieved via the query!!!
//        4. Search depth (relative, from current position)
//        5. Use function progressBar to display a progressBar
//
// Output: dyn_string with the resultsm exlusief the current datapointPath
//         
///////////////////////////////////////////////////////////////////////////
dyn_string queryDatabase(string attribute, string datapointPath, int first, int searchDepth, bool useProgressBar) {
 	LOG_DEBUG("queryDatabase: ", attribute, datapointPath, first, searchDepth, useProgressBar);
  	dyn_string output;
  	dyn_dyn_anytype tab;
  	string fullDpName;
  	int currentDepth = dynlen(strsplit(datapointPath, "_"));
  	string datapointPathOriginal = datapointPath;
  	int elementIndex;
  	int outputCounter = 1;
  	bool dpIsReference = false;
  	dyn_string reference;
  	string REMOTESYSTEM = "";
  	string firstXResults = "";
  	
  	checkForReference(datapointPath, reference, dpIsReference);
  	if (dpIsDistributed(datapointPath)) {
    	if (dpIsReference) {
      		REMOTESYSTEM = " REMOTE '" + strrtrim(dpSubStr(reference[2], DPSUB_SYS), ":") + "'";
    	}
		else {
      		REMOTESYSTEM = " REMOTE '" + strrtrim(dpSubStr(datapointPath, DPSUB_SYS), ":") + "'";
		}
  	}
  	//How many items must be retrieved (this is a DP-Type)
  	if (first > 0) {
  	  	firstXResults = " FIRST " + first;
  	}
        

        string query = "SELECT '" + attribute + "' FROM '" + datapointPath  + firstXResults + REMOTESYSTEM + "'";
        //DebugN("Query : " + query);
  	dpQuery(query, tab);
	//DebugN("Results : " + tab);
        

  	int maximumCount = dynlen(tab);
  	int maximumCounter = 0;
  	int i = 2;
  	if (dynlen(tab) >= 2) {
    	dyn_dyn_string dds;
    	string fullDpName;
    	int curDPTElevel = 2;
    	int elType, elNTypesItemLen;
    	dyn_string levels;
    	for (i = 2; i <= dynlen(tab); i++) {
      		int functionOk;
      		fullDpName = tab[i][1];
                //DebugN("fullDpName found: "+fullDpName);
                //DebugN("currentDepth    : "+currentDepth);
                //DebugN("searchDepth     : "+searchDepth);

      		if ((dynlen(strsplit(fullDpName, "_")) <= (currentDepth + searchDepth)) || (searchDepth == 0)) {
        		dyn_string dpes = dpNames(fullDpName);
                	//DebugN(" dpes found     : "+dpes);
        		int dpesLen = dynlen(dpes);
        		for (int j = 1; j <= dpesLen; j++) {
          			if (dpElementType(dpes[j]) != DPEL_TYPEREF && dpElementType(dpes[j]) != DPEL_STRUCT) {
            			output[outputCounter] = dpes[j];          
            			outputCounter++;
          			}
        		}
      		}
      		maximumCounter++;
      		//if the progressBar must be used
      		if (useProgressBar) {
        		progressBar(maximumCount, maximumCounter);
      		}
    	}//end of for loop
  	}//end of if

  	//Hide the progress bar
  	if (useProgressBar) {
    	progressBar(maximumCount, maximumCount);
  	}
  	dynSortAsc(output); //sort the dyn_string output (alphanumeric)
        //DebugN("querydatabase returns: "+output);
  	return output;
}

///////////////////////////////////////////////////////////////////////////
//
// Function compactedArrayString(string)
//
// Given een array string ( '[ xx, xx, xx ]' ) this utility compacts the string
// by replacing series with range.
// Eg. [ lii001, lii002, lii003, lii005 ] --> [ lii001..lii003, lii005 ]
//
// ATTENTION !!!!!   this code is a duplicate from the code used in: LOFAR/MAC/APL/APLCommon/src/APLUtilities.cc
// If a structural bug is discovered. or the code has been modified then you also should have a look there !!!
//
///////////////////////////////////////////////////////////////////////////

string compactedArrayString(const string orgStr) {

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
    	} 
    	else {
      		if (elemsInRange == 1) {
        		result += "," + valString;
      		}
      		else {
        		if (elemsInRange == 2) {
          			result += "," + prevValString;
        		}
        		else {
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

///////////////////////////////////////////////////////////////////////////
//
// Function expandedArrayString(string)
//
// Given een array string ( '[ xx..xx, xx ]' ) this utility expands the string
// by replacing ranges with the fill series.
// Eg. [ lii001..lii003, lii005 ] --> [ lii001, lii002, lii003, lii005 ]
//
//
// ATTENTION !!!!!   this code is a duplicate from the code used in: LOFAR/MAC/APL/APLCommon/src/APLUtilities.cc
// If a structural bug is discovered. or the code has been modified then you also should have a look there !!!
//
///////////////////////////////////////////////////////////////////////////

string expandedArrayString(const string	orgStr) {
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
  	} 
  	else {
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
    	} 
    	else {	// yes, try to get second element.
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
      		}
      		else {
        		result += "," + valString;
      		}
    	}
  	}

  	return (result+"]");
}


