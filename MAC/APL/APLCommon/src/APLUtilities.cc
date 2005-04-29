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
