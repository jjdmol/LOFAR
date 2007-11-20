//#  APLUtilities.cc: Utility functions
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
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <unistd.h>

//#include <Common/lofar_strstream.h>
#include "APL/APLCommon/APLUtilities.h"

namespace LOFAR {
  namespace APLCommon {

APLUtilities::APLUtilities()
{
}


APLUtilities::~APLUtilities()
{
}

void APLUtilities::decodeCommand(const string&		commandString, 
								 string& 			command, 
								 vector<string>&	parameters, 
								 const char 		delimiter)
{
	unsigned int delim=commandString.find(' ');
	if(delim==string::npos) { // no space found
		command=commandString;
	}
	else {
		command=commandString.substr(0,delim);
		APLUtilities::string2Vector(commandString.substr(delim+1),parameters,delimiter);
	}
}

/*
 * Converts a , delimited string to a vector of strings
 */
void APLUtilities::string2Vector(const string& parametersString, vector<string>& parameters, const char delimiter)
{
  unsigned int parametersStringLen=parametersString.length();
  unsigned int delim(0);
  unsigned int nextDelim;
  do {
    nextDelim=parametersString.find(delimiter,delim);
    if(nextDelim==string::npos) {
      nextDelim=parametersStringLen; // no delim found
    }
    if(nextDelim>delim) {
      string param(parametersString.substr(delim,nextDelim-delim));
      if(param.length()>0) {
        parameters.push_back(param);
      }
      delim=nextDelim+1;
    } 
  } while(delim<parametersStringLen);
}

/*
 * Converts a , delimited string to a vector of ints
 */
void APLUtilities::string2Vector(const string& parametersString, vector<int>& parameters, const char delimiter)
{
  unsigned int parametersStringLen=parametersString.length();
  unsigned int delim(0);
  unsigned int nextDelim;
  do
  {
    nextDelim=parametersString.find(delimiter,delim);
    if(nextDelim==string::npos)
    {
      nextDelim=parametersStringLen; // no delim found
    }
    if(nextDelim>delim)
    {
      string param(parametersString.substr(delim,nextDelim-delim));
      if(param.length()>0)
      {
        parameters.push_back(atoi(param.c_str()));
      }
      delim=nextDelim+1;
    } 
  } while(delim<parametersStringLen);
}

/*
 * Converts a , delimited string to a vector of ints
 */
void APLUtilities::string2Vector(const string& parametersString, vector<int16>& parameters, const char delimiter)
{
  unsigned int parametersStringLen=parametersString.length();
  unsigned int delim(0);
  unsigned int nextDelim;
  do
  {
    nextDelim=parametersString.find(delimiter,delim);
    if(nextDelim==string::npos)
    {
      nextDelim=parametersStringLen; // no delim found
    }
    if(nextDelim>delim)
    {
      string param(parametersString.substr(delim,nextDelim-delim));
      if(param.length()>0)
      {
        parameters.push_back(atoi(param.c_str()));
      }
      delim=nextDelim+1;
    } 
  } while(delim<parametersStringLen);
}

/*
 * Converts a , delimited string to a vector of ints
 */
void APLUtilities::vector2String(const vector<int16>& parameters, string& parametersString, const char delimiter)
{
  parametersString = "";
  vector<int16>::const_iterator it=parameters.begin();
  while(it!=parameters.end())
  {
    stringstream parstream;
    parstream << *it;
    ++it;
    if(it!=parameters.end())
    {
      parstream << delimiter;
    }
    parametersString += parstream.str();
  }
}

time_t APLUtilities::getUTCtime()
{
  return time(0);// current system time in UTC
}

time_t APLUtilities::decodeTimeString(const string& timeStr)
{
	// empty string or -1? -> sometime in the future
	if (timeStr.empty() || (timeStr.find("-1") != string::npos)) {
		return (INT_MAX);
	}

	// does string contain a +? --> add to current time.
	string::size_type plusPos = timeStr.find('+');
	if (plusPos != string::npos) {
		return (APLUtilities::getUTCtime() + atoi(timeStr.substr(plusPos+1).c_str()));
	}

	// specified times are in UTC, seconds since 1-1-1970
	time_t	theTime = atoi(timeStr.c_str());

	// is time 0? return current time.
	if (theTime == 0) {
		return (getUTCtime());
	}

	return(atoi(timeStr.c_str()));	// return specified time.
}

//
// remoteCopy(localFile, remoteHost, remoteFile)
//
int APLUtilities::remoteCopy (const string& localFile, 
							  const string& remoteHost, 
							  const string& remoteFile)
{
	string		tmpResultFile(getTempFileName());
  
	// -B: batch mode; -q: no progress bar
	string command(formatString("scp -Bq %s %s:%s 1>&2 2>%s", localFile.c_str(),
									remoteHost.c_str(), remoteFile.c_str(),
									tmpResultFile.c_str()));
	// execute the command.
	int error = system(command.c_str());
	LOG_DEBUG(formatString("copy command: %s",command.c_str()));

	if(error == 0) {			
		LOG_INFO(formatString("Successfully copied %s to %s:%s",
						localFile.c_str(),remoteHost.c_str(),remoteFile.c_str()));
	}
	else {
		// an error occured, try to reconstruct the message
		char 	outputLine[200];
		string 	outputString;
		FILE* 	f = fopen(tmpResultFile.c_str(),"rt");	// open file with errormsg
		if (f == NULL) {						// oops, problems opening the file
			LOG_ERROR(
				formatString("Unable to remote copy %s to %s:%s: reason unknown",
				localFile.c_str(),remoteHost.c_str(),remoteFile.c_str()));
		}
		else {
			// construct the error message
			while(!feof(f)) {
				fgets(outputLine,200,f);
				if(!feof(f)) {
					outputString+=string(outputLine);
				}
			}
			fclose(f);
			LOG_ERROR(formatString("Unable to remote copy %s to %s:%s: %s",
						localFile.c_str(),remoteHost.c_str(),remoteFile.c_str(),
						outputString.c_str()));
		}
	}

	// remove the temporarely file.
	remove(tmpResultFile.c_str());

	return (error);
}

//
// copyFromRemote(remoteHost, remoteFile, localFile)
//
int APLUtilities::copyFromRemote(const string& remoteHost, 
								 const string& remoteFile,
								 const string& localFile)
{
	string		tmpResultFile(getTempFileName());
  
	// -B: batch mode; -q: no progress bar
	string command(formatString("scp -Bq %s:%s %s 1>&2 2>%s", 
									remoteHost.c_str(), remoteFile.c_str(), localFile.c_str(),
									tmpResultFile.c_str()));
	// execute the command.
	int error = system(command.c_str());
	LOG_DEBUG(formatString("copy command: %s",command.c_str()));

	if(error == 0) {			
		LOG_INFO(formatString("Successfully copied %s:%s to %s",
							  remoteHost.c_str(),remoteFile.c_str(),localFile.c_str()));
	}
	else {
		// an error occured, try to reconstruct the message
		char 	outputLine[200];
		string 	outputString;
		FILE* 	f = fopen(tmpResultFile.c_str(),"rt");	// open file with errormsg
		if (f == NULL) {						// oops, problems opening the file
			LOG_ERROR(
				formatString("Unable to remote copy %s:%s to %s: reason unknown",
							 remoteHost.c_str(),remoteFile.c_str(),localFile.c_str()));
		}
		else {
			// construct the error message
			while(!feof(f)) {
				fgets(outputLine,200,f);
				if(!feof(f)) {
					outputString+=string(outputLine);
				}
			}
			fclose(f);
			LOG_ERROR(formatString("Unable to remote copy %s:%s to %s: %s",
								   remoteHost.c_str(),remoteFile.c_str(),localFile.c_str(),
								   outputString.c_str()));
		}
	}

	// remove the temporarely file.
	remove(tmpResultFile.c_str());

	return (error);
}

//
// getTempFileName([format])
//
string APLUtilities::getTempFileName(const string&	format)
{
	char tempFileName [128];

	if (format.find("XXXXXX", 0) != string::npos) {	// did user specify mask?
		strcpy (tempFileName, format.c_str());		// use user-mask
	}
	else {
		strcpy (tempFileName, "/tmp/MAC_APL_XXXXXX");// use default mask
	}

	mkstemp(tempFileName);							// let OS invent the name

	return string(tempFileName);
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
// if you change anything struktural change the Navigator part also please
// -----------------------------------------------------------------------

string APLUtilities::compactedArrayString(const string&	orgStr)
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
	string	elemName(strVector[0].substr(0,firstDigit));
	string	scanMask(elemName+"%ld");
	string	outMask (formatString("%s%%0%dld", elemName.c_str(), 
											strVector[0].length() - elemName.length()));

	string 	result("[");
	long 	prevValue(-2);
	bool	firstElem(true);
	bool	endOfArray(false);
	int		elemsInRange(0);
	int		nrElems(strVector.size());
	for (int idx = 0; idx < nrElems; idx++) {
		long	value;
		if (sscanf(strVector[idx].c_str(), scanMask.c_str(), &value) != 1) {
			LOG_DEBUG_STR("Element " << strVector[idx] << " does not match mask " 
						<< scanMask << ". Returning orignal string");
			return (orgStr);
		}

		if (value == prevValue+1) {		// contiquous numbering?
			elemsInRange++;
			prevValue = value;
			if (idx < nrElems-1) {		// when elements left
				continue;
			}
			endOfArray = true;
		}

		// broken range
		if (firstElem) {
			result += formatString(outMask.c_str(), value);
			firstElem = false;
		}
		else {
			if (elemsInRange == 1) {
				result += "," + formatString(outMask.c_str(), value);
			}
			else {
				if (elemsInRange == 2) {
					result += "," + formatString(outMask.c_str(), prevValue);
				}
				else {
					result += ".." + formatString(outMask.c_str(), prevValue);
				}
				if (!endOfArray) {
					result += "," + formatString(outMask.c_str(), value);
				}
			}
		}
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
// ----------------------- ATTENTION !!!----------------------------------
// This routine has been copied to the Navigator software
// (MAC/Navigator/scripts/libs/nav_usr/CS1/CS1_Common.ctl)
// if you change anything struktural change the Navigator part also please
// -----------------------------------------------------------------------

string APLUtilities::expandedArrayString(const string&	orgStr)
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

//
// byteSize
//
string byteSize(double	nrBytes)
{
	double	giga (1024.0*1024.0*1024.0);
	if (nrBytes >= giga) {
		return (formatString ("%.1lfGB", nrBytes/giga));
	}

	if (nrBytes >= 1048576) {
		return (formatString ("%.1fMB", (nrBytes*1.0/1048576)));
	}

	if (nrBytes >= 1024) {
		return (formatString ("%.1fKB", (nrBytes*1.0/1024)));
	}

	return (formatString ("%.0fB", nrBytes));
}

  } // namespace APLCommon
} // namespace LOFAR

