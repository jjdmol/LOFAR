//  AppControllerParseClass.cc: Class for parsing simulation commands
//
//  Copyright (C) 2001
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
//
/////////////////////////////////////////////////////////////////////////////

#include "AppControllerParseClass.h"
#include "AppController.h"
#include <Common/KeyValueMap.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iostream.h>
#include <sstream>

//# stdlib.h is needed for bison 1.28 and needs to be included here
//# (before the flex/bison files).
//#include <stdlib.h>

//#include <AppControllerParse.ycc>              // flex output
//#include <AppControllerParse.lcc>              // bison output

extern char *AppControllerTokenizetext;

#include <sstream>
using namespace std;

extern void AppControllerParseparse();

namespace LOFAR
{

// Initialize statics.
int AppControllerParse::theirLine = 0;
int AppControllerParse::theirPosition = 0;
AppController* AppControllerParse::theirAppController = 0;

void AppControllerParse::parse (AppController& appctl)
{
  // Set global AppController pointer and call the bison parser.
  theirAppController = &appctl;
  AppControllerParseparse();
  theirAppController->baseQuit();
}

void AppControllerParse::execute (const KeyValue& value)
{
  if (value.dataType() != KeyValue::DTString) {
    throw AppControllerParseError("command is not a string");
  }
  const string& comm = value.getString();
  if (comm == "define") {
    theirAppController->baseDefine (KeyValueMap());
  } else if (comm == "check") {
    theirAppController->baseCheck();
  } else if (comm == "prerun") {
    theirAppController->basePrerun();
  } else if (comm == "run") {
    theirAppController->baseRun (-1);
  } else if (comm == "step") {
    theirAppController->baseRun (1);
  } else if (comm == "dump") {
    theirAppController->baseDump();
  } else {
    throw AppControllerParseError("'" + comm + "' is an unknown command");
  }
}

void AppControllerParse::execute (const KeyValue& value,
			      const KeyValue& param)
{
  if (value.dataType() != KeyValue::DTString) {
    throw AppControllerParseError("command is not a string");
  }
  const string& comm = value.getString();
  if (comm == "define") {
    theirAppController->baseDefine (KeyValueMap());
  } else if (comm == "run"  ||  comm == "step") {
    theirAppController->baseRun (param.getInt());
  } else if (comm == "step") {
    theirAppController->baseRun (1);
  } else if (comm == "dump") {
    theirAppController->baseDump();
  } else {
    throw AppControllerParseError("'" + comm + "' is an unknown command");
  }
}

void AppControllerParse::execute (const KeyValue& value,
			      const KeyValueMap& params)
{
  if (value.dataType() != KeyValue::DTString) {
    throw AppControllerParseError("command is not a string");
  }
  const string& comm = value.getString();
  if (comm == "define") {
    theirAppController->baseDefine (params);
  } else if (comm == "run") {
    theirAppController->baseRun (-1);
  } else if (comm == "dump") {
    theirAppController->baseDump();
  } else if (comm == "dhfile") {
    theirAppController->baseDHFile (params.find("dh")->second.getString(),
				params.getString("file",""));
  } else {
    throw AppControllerParseError("'" + comm + "' is an unknown command");
  }
}

string AppControllerParse::removeEscapes (const string& in)
{
  string out;
  int leng = in.length();
  for (int i=0; i<leng; i++) {
    if (in[i] == '\\') {
      i++;
    }
    if (i < leng) {
      out += in[i];
    }
  }
  return out;
}

string AppControllerParse::removeQuotes (const string& in)
{
  //# A string is formed as "..."'...''...' etc.
  //# All ... parts will be extracted and concatenated into an output string.
  string out;
  string::size_type leng = in.length();
  string::size_type pos = 0;
  while (pos < leng) {
    //# Find next occurrence of leading ' or ""
    string::size_type inx = in.find (in[pos], pos+1);
    if (inx > leng) {
      throw AppControllerParseError ("Ill-formed quoted string: " + in);
    }
    out += in.substr (pos+1, inx-pos-1);             // add substring
    pos = inx+1;
  }
  return out;
}



AppControllerParseError::AppControllerParseError (const string& message)
: std::runtime_error ("SimulateParse: " + message)
{}



void AppControllerParseerror (char*)
{
  std::ostringstream os1;
  os1 << AppControllerParse::line() + 1;
  std::ostringstream os2;
  os2 << AppControllerParse::position();

  throw AppControllerParseError("parse error at line " + os1.str() + ", position "
			    + os2.str() + " (at or near '" +
			    string(AppControllerTokenizetext) + "')");
}

// Define the yywrap function for flex.
// Note it is not declared in the .h file, but by flex.
int AppControllerTokenizewrap()
{
    return 1;
}


}
