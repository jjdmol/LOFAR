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

#include <Common/LofarLogger.h>
#include "APLCommon/APLUtilities.h"

using namespace LOFAR;
using namespace APLCommon;

INIT_TRACER_CONTEXT(APLUtilities,LOFARLOGGER_PACKAGE);

APLUtilities::APLUtilities()
{
}


APLUtilities::~APLUtilities()
{
}

void APLUtilities::decodeCommand(const string& commandString, string& command, vector<string>& parameters)
{
  unsigned int delim=commandString.find(' ');
  if(delim==string::npos) // no space found
  {
    command=commandString;
  }
  else
  {
    command=commandString.substr(0,delim);
    APLUtilities::string2Vector(commandString.substr(delim+1),parameters);
  }
}

/*
 * Converts a , delimited string to a vector of strings
 */
void APLUtilities::string2Vector(const string& parametersString, vector<string>& parameters)
{
  unsigned int parametersStringLen=parametersString.length();
  unsigned int delim(0);
  unsigned int nextDelim;
  do
  {
    nextDelim=parametersString.find(',',delim);
    if(nextDelim==string::npos)
    {
      nextDelim=parametersStringLen; // no delim found
    }
    if(nextDelim>delim)
    {
      string param(parametersString.substr(delim,nextDelim-delim));
      if(param.length()>0)
      {
        parameters.push_back(param);
      }
      delim=nextDelim+1;
    } 
  } while(delim<parametersStringLen);
}

time_t APLUtilities::getUTCtime()
{
  return time(0);// current system time in UTC
}

int APLUtilities::remoteCopy(const string& localFile, const string& remoteHost, const string& remoteFile)
{
  char tempFileName [L_tmpnam];
  tmpnam(tempFileName);
  
  string command("rcp ");
  command += localFile + string(" ");
  command += remoteHost + string(":");
  command += remoteFile;
  command += string(" 1>&2 2>") + string(tempFileName);
  int result = system(command.c_str());
  if(result != 0)
  {
    char outputLine[200];
    string outputString;
    FILE* f=fopen(tempFileName,"rt");
    if(f != NULL)
    {
      while(!feof(f))
      {
        fgets(outputLine,200,f);
        if(!feof(f))
          outputString+=string(outputLine);
      }
      fclose(f);
      LOG_ERROR(formatString("Unable to remote copy %s to %s:%s: %s",localFile.c_str(),remoteHost.c_str(),remoteFile.c_str(),outputString.c_str()));
    }
  }
  else
  {
    LOG_INFO(formatString("Successfully copied %s to %s:%s",localFile.c_str(),remoteHost.c_str(),remoteFile.c_str()));
  }
  remove(tempFileName);
  return result;
}

string APLUtilities::getTempFileName()
{
  char tempFileName [L_tmpnam];
  tmpnam(tempFileName);
  return string(tempFileName);
}
