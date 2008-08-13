//#  StringUtil.cc: implementation of the string utilities class.
//#
//#  Copyright (C) 2002-2005
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <Common/StringUtil.h>
#include <iostream>
#include <stdarg.h>
#include <time.h>

namespace LOFAR
{
  
vector<string> StringUtil::split(const string& s, char c)
{
	vector<string> 		v;
	string::size_type 	i, j;
	string				substring;
	i = j = 0;
	while (j != string::npos) {
		j = s.find(c,i);
		if (j == string::npos) {
			substring = s.substr(i);
		} 
		else {
			substring = s.substr(i,j-i);
		}
		ltrim(substring);
		rtrim(substring);
		v.push_back(substring);
		i = j + 1;
    }
    return (v);
  }

//
// formatString(format, ...) --> string
//
// Define a global function the accepts printf like arguments and returns 
// a string.
//
const string formatString(const	char* format, ...) {
	char		tmp_cstring[10240];
	va_list		ap;

	va_start (ap, format);
	vsnprintf(tmp_cstring, sizeof(tmp_cstring), format, ap);
	va_end   (ap);

	return   string(tmp_cstring);
}

//
// timeString(aTime [,format]) --> string
//
// Define a global function that convert a timestamp into a humanreadable 
// format.
//
const string timeString(time_t		aTime, 
							 bool		gmt,
							 char* 		format)
{
	char	theTimeString [256];
	strftime(theTimeString, 256, format, gmt ? gmtime(&aTime) 
														: localtime(&aTime));

	return (theTimeString);
}

//
// rtrim(char* CString [,len=0])
//
// Skip trailing spaces. If len of string is already known at invocation
// it may be given thus avoiding a relative expensive strlen call.
//
// NOTE: original string is truncated!
//
int32 rtrim(char*	aCstring, int32		len, char*	whiteSpace)
{
	if (!aCstring || !(*aCstring)) {		// aCstring must be valid
		return (0);
	}

	if (!len || aCstring[len] != '\0') {	// len unknown or wrong?
		len = strlen(aCstring);				// recalculate it.
	}
	--len;									// set on last char

	while ((len >= 0) && (strchr(whiteSpace, aCstring[len]))) {
		aCstring[len--] = '\0';
	}

	return (len+1);
}

//
// char* ltrim(char*	Cstring)
//
// skip leading spaces. A pointer to the first non-whitespace char is
// returned.
char*	ltrim(char*	aCstring, char*	whiteSpace)
{
	aCstring += strspn(aCstring, whiteSpace);

	return (aCstring);
}

string	toUpper(string str)
{
  transform(str.begin(), str.end(), str.begin(), toupper);
  return str;
}

string	toLower(string str)
{
  transform(str.begin(), str.end(), str.begin(), tolower);
  return str;
}

bool	StringToBool(const string& aString) throw(Exception)
{
	char	firstChar = aString.c_str()[0];
	if ((firstChar == 't') || (firstChar == 'T') || (firstChar == '1') || (firstChar == 'Y') || (firstChar == 'y'))
		return (true);

	if ((firstChar == 'f') || (firstChar == 'F') || (firstChar == '0') || (firstChar == 'N') || (firstChar == 'n'))
		return (false);

	THROW (Exception, aString + " is not a boolean value");
}	

int16	StringToInt16(const string& aString, const char* fmt) throw(Exception)
{
	int16		theShort;
	if ((fmt ? sscanf(aString.c_str(), fmt, &theShort) : 
			   sscanf(aString.c_str(), "%hd", &theShort)) != 1) {
		THROW (Exception, aString + " is not an short value");
	}

	return (theShort);
}	

uint16	StringToUint16(const string& aString, const char* fmt) throw(Exception)
{
	uint16		theUshort;
	if ((fmt ? sscanf(aString.c_str(), fmt, &theUshort) : 
			   sscanf(aString.c_str(), "%hu", &theUshort)) != 1) {
		THROW (Exception, aString + " is not an unsigned short value");
	}

	return (theUshort);
}	

int32	StringToInt32(const string& aString, const char* fmt) throw(Exception)
{
	int32		theInt;
	if ((fmt ? sscanf(aString.c_str(), fmt, &theInt) : 
			   sscanf(aString.c_str(), "%d", &theInt)) != 1) {
		THROW (Exception, aString + " is not an integer value");
	}

	return (theInt);
}	

uint32	StringToUint32(const string& aString, const char* fmt) throw(Exception)
{
	uint32		theUint;
	if ((fmt ? sscanf(aString.c_str(), fmt, &theUint) : 
			   sscanf(aString.c_str(), "%u", &theUint)) != 1) {
		THROW (Exception, aString + " is not an unsigned integer value");
	}

	return (theUint);
}	

#if HAVE_LONG_LONG
int64	StringToInt64(const string& aString, const char* fmt) throw(Exception)
{
	int64		theLong;
	if ((fmt ? sscanf(aString.c_str(), fmt, &theLong) : 
			   sscanf(aString.c_str(), "%lld", &theLong)) != 1) {
		THROW (Exception, aString + " is not a long integer value");
	}

	return (theLong);
}	

uint64	StringToUint64(const string& aString, const char* fmt) throw(Exception)
{
	uint64		theUlong;
	if ((fmt ? sscanf(aString.c_str(), fmt, &theUlong) : 
			   sscanf(aString.c_str(), "%llu", &theUlong)) != 1) {
		THROW (Exception, aString + " is not an unsigned long integer value");
	}

	return (theUlong);
}	
#endif

float	StringToFloat(const string& aString, const char* fmt) throw(Exception)
{
	float		theFloat;
	if ((fmt ? sscanf(aString.c_str(), fmt, &theFloat) : 
			   sscanf(aString.c_str(), "%g", &theFloat)) != 1) {
		THROW (Exception, aString + " is not a float value");
	}

	return (theFloat);
}	


double	StringToDouble(const string& aString, const char* fmt) throw(Exception)
{
	double		theDouble;
	if ((fmt ? sscanf(aString.c_str(), fmt, &theDouble) : 
			   sscanf(aString.c_str(), "%lg", &theDouble)) != 1) {
		THROW (Exception, aString + " is not a double value");
	}

	return (theDouble);
}	

//
// compactedArrayString(string)
//
// Given een array string ( '[ xx, xx, xx ]' ) this utility compacts the string
// by replacing series with range.
// Eg. [ lii001, lii002, lii003, lii005 ] --> [ lii001..lii003, lii005 ]
//
// ----------------------- ATTENTION !!!----------------------------------
// This routine has been copied to the Navigator software
// (MAC/Navigator/scripts/libs/nav_usr/CS1/CS1_Common.ctl)
// if you change anything structural change the Navigator part also please
// -----------------------------------------------------------------------

string compactedArrayString(const string&	orgStr)
{
	string	baseString(orgStr);			// destroyable copy
	ltrim(baseString, " 	[");		// strip of brackets
	rtrim(baseString, " 	]");

	// and split into a vector
	vector<string>	strVector = StringUtil::split(baseString, ',');
	if (strVector.size() < 2) {
		return (orgStr);
	}

	// note: we assume that the format of each element is [xxxx]9999
	string::size_type	firstDigit(strVector[0].find_first_of("0123456789"));
	if (firstDigit == string::npos) {	// no digits? then return org string
		return (orgStr);
	}
	// construct masks for scanning and formatting.
	string	elemName	 (strVector[0].substr(0,firstDigit));
	string	scanMask	 (elemName+"%ld");
	string	rangeScanMask(scanMask+".."+scanMask);
	bool	rangeElem 	 (strVector[0].find("..",0) != string::npos);
	int		numberLength (strVector[0].length() - elemName.length());
	if (rangeElem) {
		numberLength = (numberLength-2)/2;
	}
	string	outMask 	 (formatString("%s%%0%dld", elemName.c_str(), numberLength));

	// process all elements in the vector.
	string 	result("[");
	long 	prevValue(-2);
	bool	firstElem(true);
	bool	endOfArray(false);
	int		elemsInRange(0);
	int		nrElems(strVector.size());
	for (int idx = 0; idx < nrElems; idx++) {
		long	value;
		long	lastValue;
		if (sscanf(strVector[idx].c_str(), scanMask.c_str(), &value) != 1) {
			LOG_DEBUG_STR("Element " << strVector[idx] << " does not match mask " 
						<< scanMask << ". Returning orignal string.");
			return (orgStr);
		}

		// check for already compacted elements.
		rangeElem = (strVector[idx].find("..",0) != string::npos);
		if (rangeElem && (sscanf(strVector[idx].c_str(), rangeScanMask.c_str(), &value, &lastValue) != 2)) {
			LOG_DEBUG_STR("RangeElement " << strVector[idx] << " does not match mask " 
						<< rangeScanMask << ". Returning orignal string.");
			return (orgStr);
		}
			
		// contiquous numbering?
		if (value == prevValue+1) {
			elemsInRange += rangeElem ? (lastValue-value+1) : 1;
			prevValue    =  rangeElem ? lastValue : value;
			if (idx < nrElems-1) {		// when elements left
				continue;
			}
			endOfArray = true;
		}

		// list not contiguous anymore, write results we collected
		if (firstElem) {				// don't start with a comma.
			result += formatString(outMask.c_str(), value);
		}

		if (elemsInRange == 1) {
			result += "," + formatString(outMask.c_str(), value);
		}
		else {
			if (elemsInRange == 2) {		// don't compact xx03,xx04 to xx03..xx04
				result += "," + formatString(outMask.c_str(), prevValue);
			}
			else if (elemsInRange > 2) {
				result += ".." + formatString(outMask.c_str(), prevValue);
			}
			if (!firstElem && !endOfArray) {
				result += "," + formatString(outMask.c_str(), value);
				if (rangeElem) {
					result += ".." + formatString(outMask.c_str(), lastValue);
				}
			}
		}
		elemsInRange = rangeElem ? (lastValue-value+1) : 1;
		prevValue    = rangeElem ? lastValue : value;
		firstElem 	 = false;
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
// ----------------------- ATTENTION !!!----------------------------------
// This routine has been copied to the Navigator software
// (MAC/Navigator/scripts/libs/nav_usr/CS1/CS1_Common.ctl)
// if you change anything structural change the Navigator part also please
// -----------------------------------------------------------------------

string expandedArrayString(const string&	orgStr)
{
	// any ranges in the string?
	if (orgStr.find("..",0) == string::npos) {
		return (orgStr);						// no, just return original
	}

	string	baseString(orgStr);					// destroyable copy
	ltrim(baseString, " 	[");				// strip of brackets
	rtrim(baseString, " 	]");

	// and split into a vector
	vector<string>	strVector = StringUtil::split(baseString, ',');

	// note: we assume that the format of each element is [xxxx]9999
	string::size_type	firstDigit(strVector[0].find_first_of("0123456789"));
	if (firstDigit == string::npos) {	// no digits? then return org string
		return (orgStr);
	}

	// construct scanmask and outputmask.
	string	elemName(strVector[0].substr(0,firstDigit));
	string	scanMask(elemName+"%ld");
	int		nrDigits;
	if (strVector[0].find("..",0) != string::npos) {	// range element?
		nrDigits = ((strVector[0].length() - 2)/2) - elemName.length();
	}
	else {
		nrDigits = strVector[0].length() - elemName.length();
	}
	string	outMask (formatString("%s%%0%dld", elemName.c_str(), nrDigits));

	// handle all elements
	string 	result("[");
	bool	firstElem(true);
	int		nrElems(strVector.size());
	for (int idx = 0; idx < nrElems; idx++) {
		long	firstVal;
		long	lastVal;
		// should match scanmask.
		if (sscanf(strVector[idx].c_str(), scanMask.c_str(), &firstVal) != 1) {
			LOG_DEBUG_STR("Element " << strVector[idx] << " does not match mask " 
						<< scanMask << ". Returning orignal string");
			return (orgStr);
		}

		// range element?
		string::size_type	rangePos(strVector[idx].find("..",0));
		if (rangePos == string::npos) {
			lastVal = firstVal;
		}
		else {	// yes, try to get second element.
			if (sscanf(strVector[idx].data()+rangePos+2, scanMask.c_str(), &lastVal) != 1) {
				LOG_DEBUG_STR("Second part of element " << strVector[idx]
							<< " does not match mask " << scanMask 
							<< ". Returning orignal string");
				return (orgStr);
			}
			// check range
			if (lastVal < firstVal) {
				LOG_DEBUG_STR("Illegal range specified in " << strVector[idx] <<
								". Returning orignal string");
				return (orgStr);
			}
		}

		// finally construct one or more elements
		for	(long val = firstVal ; val <= lastVal; val++) {
			if (firstElem) {
				result += formatString(outMask.c_str(), val);
				firstElem = false;
			}
			else {
				result += "," + formatString(outMask.c_str(), val);
			}
		}
	}

	return (result+"]");
}

} // namespace LOFAR
