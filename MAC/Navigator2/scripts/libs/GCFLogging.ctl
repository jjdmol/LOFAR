// GCFLogging.ctl
//
//  Copyright (C) 2002-2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
///////////////////////////////////////////////////////////////////
// Contains the funtions of the logging-system of the Navigator.
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// loggingsetLoglevel		: Set loglevel of the logging system.
// LOG_DYN		  	 : Constructs a string of the given array with the format '[xx,xx,xx,xx]'
// LOG_FATAL			 : Fatal level
// LOG_ERROR			 : Error level
// LOG_WARN		   	 : Warn level
// LOG_INFO		  	 : Info level
// LOG_DEBUG			 : Debug level
// LOG_TRACE			 : Trace level
// LOG_MESSAGE	   : Does the real logging. Tests level of message against level set by user.
// initLog			   : Connects the loglevelUpdated function to the loglevel DP to capture changes.
// loglevelUpdated : Copies value to own admin and logs the change.
//



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

// set the loglevel throughout the navigator
global int  g_logLevel = LOGLEVEL_WARN;
global dyn_string g_logScope = "";
global dyn_string g_searchString="";

global string DPNAME_LOGLEVEL = "";
global bool g_logInitialized = false;
////////////////////////////////////////////////////////////////////////////////
//
// Function setLoglevel(level)
//
// Set loglevel of the logging system.
//
////////////////////////////////////////////////////////////////////////////////
void setLoglevel(int newLevel)
{
  DebugTN("setting loglevel to:", newLevel);
  if(dpExists(DPNAME_LOGLEVEL)) {
    dyn_errClass err;
    dpSet(DPNAME_LOGLEVEL+".logLevel",newLevel);
    err = getLastError();
    if (dynlen(err) > 0) {
      errorDialog(err);
    }
  } else {
    g_logLevel=newLevel;
  }  
}

////////////////////////////////////////////////////////////////////////////////
//
// Function LOG_DYN(dyn array) : string
//
// Constructs a string of the given array with the format '[xx,xx,xx,xx]'
//
////////////////////////////////////////////////////////////////////////////////
// helper function to convert arrays to strings
string LOG_DYN(dyn_anytype array)
{
	int 	i;
	string	result = "[";

	for (i = 1; i < dynlen(array); i++) {
		result += array[i] + ",";
	}
	if (i <= dynlen(array)) {
		result += array[i];
	}
	result += "]";

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function LOG_FATAL (...)
//
////////////////////////////////////////////////////////////////////////////////
void LOG_FATAL(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
	dyn_anytype message;
	int 		i,len;
	va_list 	parameters;

	dynAppend(message,prefix);
	len = va_start(parameters); // returns the number of parameters
	for (i = 1;i <= len; i++) {
		// dynAppend(message,va_arg(parameters));
		string blubs=va_arg(parameters);
		if(strlen(blubs)>0) {
			dynAppend(message,blubs);
		}
	}
	va_end(parameters);

	LOG_MESSAGE(LOGLEVEL_FATAL, LOGMESSAGE_FATAL, message);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function LOG_ERROR
//
////////////////////////////////////////////////////////////////////////////////
void LOG_ERROR(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
	dyn_anytype message;
	int 		i,len;
	va_list 	parameters;

	dynAppend(message,prefix);
	len = va_start(parameters); // returns the number of parameters
	for (i = 1;i <= len; i++) {
		// dynAppend(message,va_arg(parameters));
		string blubs=va_arg(parameters);
		if(strlen(blubs)>0) {
			dynAppend(message,blubs);
		}
	}
	va_end(parameters);

	LOG_MESSAGE (LOGLEVEL_ERROR, LOGMESSAGE_ERROR, message);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function LOG_WARN
//
////////////////////////////////////////////////////////////////////////////////
void LOG_WARN(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
	dyn_anytype message;
	int 		i,len;
	va_list 	parameters;

	dynAppend(message,prefix);
	len = va_start(parameters); // returns the number of parameters
	for (i = 1;i <= len; i++) {
		// dynAppend(message,va_arg(parameters));
		string blubs=va_arg(parameters);
		if(strlen(blubs)>0) {
			dynAppend(message,blubs);
		}
	}
	va_end(parameters);

	LOG_MESSAGE (LOGLEVEL_WARN, LOGMESSAGE_WARN, message);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function LOG_INFO
//
////////////////////////////////////////////////////////////////////////////////
void LOG_INFO(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
	dyn_anytype message;
	int 		i,len;
	va_list 	parameters;

	dynAppend(message,prefix);
	len = va_start(parameters); // returns the number of parameters
	for (i = 1;i <= len; i++) {
		// dynAppend(message,va_arg(parameters));
		string blubs=va_arg(parameters);
		if(strlen(blubs)>0) {
			dynAppend(message,blubs);
		}
	}
	va_end(parameters);

	LOG_MESSAGE (LOGLEVEL_INFO, LOGMESSAGE_INFO, message);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function LOG_DEBUG
//
////////////////////////////////////////////////////////////////////////////////
void LOG_DEBUG(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
	dyn_anytype message;
	int 		i,len;
	va_list 	parameters;

	dynAppend(message,prefix);
	len = va_start(parameters); // returns the number of parameters
	for (i = 1; i <= len; i++) {
		// dynAppend(message,va_arg(parameters));
		string blubs=va_arg(parameters);
		if(strlen(blubs)>0) {
			dynAppend(message,blubs);
		}
	}
	va_end(parameters);

	LOG_MESSAGE (LOGLEVEL_DEBUG, LOGMESSAGE_DEBUG, message);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function LOG_TRACE
//
////////////////////////////////////////////////////////////////////////////////
void LOG_TRACE(string prefix, ...) // the prefix is necessary, otherwise PVSS won't eat it!
{
	dyn_anytype message;
	int 		i,len;
	va_list 	parameters;

	dynAppend(message,prefix);
	len = va_start(parameters); // returns the number of parameters
	for (i = 1; i <= len; i++) {
		// dynAppend(message,va_arg(parameters));
		string blubs=va_arg(parameters);
		if(strlen(blubs)>0) {
			dynAppend(message,blubs);
		}
	}
	va_end(parameters);

	LOG_MESSAGE (LOGLEVEL_TRACE, LOGMESSAGE_TRACE, message);
}

//=============================== Private functions ==============================

////////////////////////////////////////////////////////////////////////////////
//
// Private Function LOG_MESSAGE
//
// Does the real logging. Tests level of message against level set by user.
// Takes care for the initialisation of the logsystem.
//
////////////////////////////////////////////////////////////////////////////////
void LOG_MESSAGE(int level, string logmessage, dyn_anytype message)
{
  // check if level is wanted
  if (level >= g_logLevel) {
    
    // split the original string and check against scopes and searchstrings wanted
    string originator="";
    string function="";
    string ms = "";
    splitLogString(message,originator,function,ms);
    bool logflag=false;
    if (dynlen(g_logScope) < 1 && dynlen(g_searchString) < 1) {      // If No search criteria, just print
      logflag = true;
    } else if (originator == "" && function == "" ) {                // If empty split, just print
      logflag = true;
    } else if (matchLogScope(originator,function)) {     // if logscope contains originator or function
      logflag = true;
    } else if (matchSearchString(ms)) {    // if searchstring found in ms
      logflag = true;
    }
    
    if (logflag) {
      string msg = logmessage;
      for (int i = 1; i <= dynlen(message); i++) {
        msg += ", " + message[i];
      }
      DebugTN (msg);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function initLog()
//
// Connects the loglevelUpdated function to the loglevel DP to capture changes.
//
////////////////////////////////////////////////////////////////////////////////
void initLog(string logDP)
{
  DPNAME_LOGLEVEL=logDP;
  if(!g_logInitialized) {
    if(dpExists(DPNAME_LOGLEVEL)) {
      dyn_errClass err;
      if (dpGet(DPNAME_LOGLEVEL+".logLevel",g_logLevel)== -1) {
        err = getLastError();
        if (dynlen(err) > 0) {
          errorDialog(err);
        }
      }
      // monitor future updates:
      if (dpConnect("loglevelUpdated",DPNAME_LOGLEVEL+".logLevel",
                                      DPNAME_LOGLEVEL+".logScope",
                                      DPNAME_LOGLEVEL+".searchString") == -1) {
        err = getLastError();
        if (dynlen(err) > 0) {
          errorDialog(err);
        }
      }
    }
    g_logInitialized=true;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// EventHandler loglevelUpdated
//
// Event: __loglevel was changed.
//
// Copies value to own admin and logs the change.
//
////////////////////////////////////////////////////////////////////////////////
void loglevelUpdated(string dp1, int newLevel,
                     string dp2, dyn_string scope,
                     string dp3, dyn_string search)
{
	DebugTN("new loglevel     : ", newLevel);
	DebugTN("new scopes       : ", scope);
	DebugTN("new searchStrings: ", search);
	g_logLevel = newLevel;
        g_logScope = scope;
        g_searchString = search;
}



////////////////////////////////////////////////////////////////////////////////
//
// splitLogString
//
// LogMsg's should look like:
//
//   LOG_TRACE("navigator.pnl:fw_viewBoxEvent| trigger: " + someinfo);
//
//   This routine will split the 3 main streams from a log msg into seperate vars.
//
////////////////////////////////////////////////////////////////////////////////

void splitLogString(string aString, string& originator, string& function, string& msg) {
  dyn_string aS1= strsplit(aString,"|");
  if (dynlen(aS1) > 1) {
    msg=aS1[2];
    for (int i=3;i<=dynlen(aS1);i++) {
      msg= msg+"|"+aS1[i];
    }
    dyn_string aS2=strsplit(aS1[1],":");
    if (dynlen(aS2) > 1) {
      function = aS2[2];
      originator = aS2[1];
    } else if (dynlen(aS2) == 1) {
      originator = aS2[1];
      function = "";
    }    
  } else if (dynlen(aS1) == 1) {
    msg=aS1[1];
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Function matchLogScope
//
// Checks if entries in the g_logScope array match with the originator or function
//
////////////////////////////////////////////////////////////////////////////////
bool matchLogScope(string originator,string function)
{
  int i;
  for (i=1; i<= dynlen(g_logScope); i++) {
    if (strpos(originator,g_logScope[i]) > -1) {
      return true;
    }
  }
  for (i=1; i<= dynlen(g_logScope); i++) {
    if (strpos(function,g_logScope[i]) > -1) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function matchSearchString
//
// Checks if entries in the g_searchString array match with the remaining msg
//
////////////////////////////////////////////////////////////////////////////////
bool matchSearchString(string msg)
{
  int i;
  for (i=1; i<= dynlen(g_searchString); i++) {
    if (strpos(msg,g_searchString[i]) > -1 && g_searchString[i] != "") {
      return true;
    }
  }
  return false;
}
