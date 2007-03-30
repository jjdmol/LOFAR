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
//# - Logging facility
//# - Message box function
//# - give child border a color
//# - give Ownlight a color
//# - get stateName from int
//# - get stateColor from int


const int OFF         = 0;
const int OPERATIONAL = 1;
const int MAINTENANCE = 2;
const int TEST        = 3;
const int SUSPICIOUS  = 4;
const int BROKEN      = 5;

const string DPNAME_LOGLEVEL = "__loglevel";

const int LOGLEVEL_FATAL = 50000;
const int LOGLEVEL_ERROR = 40000;
const int LOGLEVEL_WARN  = 30000;
const int LOGLEVEL_INFO  = 20000;
const int LOGLEVEL_DEBUG = 10000;
const int LOGLEVEL_TRACE = 0;

const string LOGMESSAGE_FATAL = "FATAL";
const string LOGMESSAGE_ERROR = "ERROR";
const string LOGMESSAGE_WARN  = "WARN";
const string LOGMESSAGE_INFO  = "INFO";
const string LOGMESSAGE_DEBUG = "DEBUG";
const string LOGMESSAGE_TRACE = "TRACE";

global bool g_logInitialized = false;
global int  g_loglevel = 30000;
global dyn_string stateColor;
global dyn_string stateName;

void gcfUtilMessageWarning(string caption, string message)
{
  dyn_string messageboxParams = makeDynString("$1:"+message);
  ChildPanelOnCentral("vision/MessageWarning",caption, messageboxParams);
}

// helper function to convert arrays to strings
string LOG_DYN(dyn_anytype array)
{
  int i;
  string result = "[";
  for(i=1;i<dynlen(array);i++)
    result += array[i] + ",";
  if(i<=dynlen(array))
    result += array[i];
  result += "]";
  return result;
}

void LOG_FATAL(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
  dyn_anytype message;
  int i,len;
  va_list parameters;
  dynAppend(message,prefix);
  len = va_start(parameters); // returns the number of parameters
  for(i=1;i<=len;i++)
    dynAppend(message,va_arg(parameters));
  va_end(parameters);
  LOG_MESSAGE(LOGLEVEL_FATAL,LOGMESSAGE_FATAL,message);
}

void LOG_ERROR(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
  dyn_anytype message;
  int i,len;
  va_list parameters;
  dynAppend(message,prefix);
  len = va_start(parameters); // returns the number of parameters
  for(i=1;i<=len;i++)
    dynAppend(message,va_arg(parameters));
  va_end(parameters);
  LOG_MESSAGE(LOGLEVEL_ERROR,LOGMESSAGE_ERROR,message);
}

void LOG_WARN(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
  dyn_anytype message;
  int i,len;
  va_list parameters;
  dynAppend(message,prefix);
  len = va_start(parameters); // returns the number of parameters
  for(i=1;i<=len;i++)
    dynAppend(message,va_arg(parameters));
  va_end(parameters);
  LOG_MESSAGE(LOGLEVEL_WARN,LOGMESSAGE_WARN,message);
}

void LOG_INFO(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
  dyn_anytype message;
  int i,len;
  va_list parameters;
  dynAppend(message,prefix);
  len = va_start(parameters); // returns the number of parameters
  for(i=1;i<=len;i++)
    dynAppend(message,va_arg(parameters));
  va_end(parameters);
  LOG_MESSAGE(LOGLEVEL_INFO,LOGMESSAGE_INFO,message);
}

void LOG_DEBUG(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
  dyn_anytype message;
  int i,len;
  va_list parameters;
  dynAppend(message,prefix);
  len = va_start(parameters); // returns the number of parameters
  for(i=1;i<=len;i++)
  {
    string blubs=va_arg(parameters);
    if(strlen(blubs)>0)
      dynAppend(message,blubs);
//    dynAppend(message,va_arg(parameters));
  }
  va_end(parameters);
  LOG_MESSAGE(LOGLEVEL_DEBUG,LOGMESSAGE_DEBUG,message);
}

void LOG_TRACE(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
  dyn_anytype message;
  int i,len;
  va_list parameters;
  dynAppend(message,prefix);
  len = va_start(parameters); // returns the number of parameters
  for(i=1;i<=len;i++)
  {
    string blubs=va_arg(parameters);
    if(strlen(blubs)>0)
      dynAppend(message,blubs);
  }
  va_end(parameters);
  LOG_MESSAGE(LOGLEVEL_TRACE,LOGMESSAGE_TRACE,message);
}

void LOG_MESSAGE(int level, string logmessage, dyn_anytype message)
{
  if(!g_logInitialized)
    initLog();

  if(level >= g_loglevel)
  {
    string msg = logmessage;
    for(int i=1;i<=dynlen(message);i++)
      msg += ", " + message[i];
    DebugTN(msg);
  }
}

void initLog()
{
  if(!g_logInitialized)
  {
    if(dpExists(DPNAME_LOGLEVEL))
    {
      dyn_errClass err;
      dpGet(DPNAME_LOGLEVEL+".",g_loglevel);
      err=getLastError();
      if(dynlen(err)>0)
        errorDialog(err);
      // monitor future updates:
      dpConnect("loglevelUpdated",DPNAME_LOGLEVEL+".");
      err=getLastError();
      if(dynlen(err)>0)
        errorDialog(err);
    }
    g_logInitialized=true;
  }
}

void loglevelUpdated(string dp, int newLevel)
{
  DebugTN("new loglevel:",newLevel);
  g_loglevel = newLevel;
}

// helper function to return the colorName based on a state
// dyn_arrays start count at 1!!!

string getStateColor(int aState) {
  if (aState < dynlen(stateColor) & aState >= 0) {
    return stateColor[aState+1];
  } else { 
    return "";
  }
}

// helper function to return the stateName based on a state
// dyn_arrays start count at 1!!!

string getStateName(int aState) {
  if (aState < dynlen(stateName) & aState >= 0) {
    return stateName[aState+1];
  } else { 
    return "";
  }
}

// helper function to return the stateNumber based on a stateName
// dyn_arrays start count at 1!!!

int getStateNumber(string aState) {
  if (aState != "") {
    for (int i = 1; i <= dynlen(stateName); i++) {
      if (aState == stateName[i]) {
        return (i-1);
      }
    }
  }
  return (-1);
}

// does all the colorsettings based on the DP's state
void showSelfState(string aDP) {
  // check if the requiered datapoint for this view are enabled and accessible
  if (dpAccessable(aDP+".state")) {
    dpConnect("updateSelfState",aDP + ".state",
   	                        aDP + ".state:_online.._invalid");
  } else {
    setValue("selfState.light","backCol","_dpdoesnotexist");
  }
}

void showChildState(string aDP) {
  // check if the requiered datapoint for this view are enabled and accessible
  if (dpAccessable(aDP+".childState")) {
    dpConnect("updateChildState",aDP + ".childState",
   	                         aDP + ".childState:_online.._invalid");
  } else {
    setValue("childStateBorder","foreCol","_dpdoesnotexist");
  }
}


updateChildState(string dp1, int state,
 		 string dp2, bool invalid)
{
  string SymbolCol;
	
  if (invalid) 
  {
    SymbolCol = "Lofar_invalid";
  } else {
    SymbolCol = getStateColor(state);
  }
  setValue("childStateBorder", "foreCol", SymbolCol);
}

updateSelfState(string dp1, int state,
 		string dp2, bool invalid)
{
  string SymbolCol;

  if (invalid) 
  {
    SymbolCol = "Lofar_invalid";
  } else {
    SymbolCol = getStateColor(state);
  }
  setValue("selfState.light", "backCol", SymbolCol);
}

