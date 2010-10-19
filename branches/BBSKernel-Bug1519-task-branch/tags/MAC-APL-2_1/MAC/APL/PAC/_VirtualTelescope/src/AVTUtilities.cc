//#  AVTUtilities.cc: Utility functions
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

#include "AVTUtilities.h"
#include "AVTDefines.h"

AVTUtilities::AVTUtilities()
{
}


AVTUtilities::~AVTUtilities()
{
}

void AVTUtilities::decodeCommand(const string& commandString, string& command, vector<string>& parameters)
{
  unsigned int delim=commandString.find(' ');
  if(delim==string::npos) // no space found
  {
    command=commandString;
  }
  else
  {
    command=commandString.substr(0,delim);
    AVTUtilities::decodeParameters(commandString.substr(delim+1),parameters);
  }
}

void AVTUtilities::decodeParameters(const string& parametersString, vector<string>& parameters)
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
      parameters.push_back(param);
      delim=nextDelim+1;
    } 
  } while(delim<parametersStringLen);
}

void AVTUtilities::decodeSubbandsParameter(const string& subbandsString, vector<int>& subbands)
{
  unsigned int subbandsStringLen=subbandsString.length();
  unsigned int delim(0);
  unsigned int nextDelim;
  do
  {
    nextDelim=subbandsString.find('|',delim);
    if(nextDelim==string::npos) // no more '|' found
    {
      nextDelim=subbandsStringLen;
    }
    if(nextDelim>delim)
    {
      subbands.push_back(atoi(subbandsString.substr(delim,nextDelim-delim).c_str()));
      delim=nextDelim+1;
    }
  } while(delim<subbandsStringLen);
}

void AVTUtilities::encodeParameters(const vector<string>& parameters,string& encodedParameters)
{
  encodedParameters="";
  vector<string>::const_iterator parameterIt=parameters.begin();
  while(parameterIt!=parameters.end())
  {
    if(parameterIt!=parameters.begin())
    {
      encodedParameters += ",";
    }
    encodedParameters += (*parameterIt);
    
    ++parameterIt;
  }
}
