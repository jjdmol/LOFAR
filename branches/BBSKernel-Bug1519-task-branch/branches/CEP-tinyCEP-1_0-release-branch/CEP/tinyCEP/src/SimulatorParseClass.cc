//  SimulatorParseClass.cc: Class for parsing simulation commands
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

#include <tinyCEP/SimulatorParseClass.h>
//#include <tinyCEP/TinyApplicationHolder.h>
#include <Common/KeyValueMap.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iostream.h>
#include <sstream>

//# stdlib.h is needed for bison 1.28 and needs to be included here
//# (before the flex/bison files).
//#include <stdlib.h>

//#include <SimulatorParse.ycc>                  // flex output
//#include <SimulatorParse.lcc>                  // bison output

extern char *SimulatorTokenizetext;

#include <sstream>
using namespace std;

extern void SimulatorParseparse();

namespace LOFAR
{

// Initialize statics.
int SimulatorParse::theirLine = 0;
int SimulatorParse::theirPosition = 0;
TinyApplicationHolder* SimulatorParse::theirSimulator = 0;

void SimulatorParse::parse (TinyApplicationHolder& simulator)
{
  // Set global Simulator pointer and call the bison parser.
  theirSimulator = &simulator;
  SimulatorParseparse();
  theirSimulator->baseQuit();
}

void SimulatorParse::execute (const KeyValue& value)
{
  if (value.dataType() != KeyValue::DTString) {
    throw SimulatorParseError("command is not a string");
  }
  const string& comm = value.getString();
  if (comm == "define") {
    theirSimulator->baseDefine (KeyValueMap());
  } else if (comm == "check") {
    theirSimulator->baseCheck();
  } else if (comm == "prerun") {
    theirSimulator->basePrerun();
  } else if (comm == "run") {
    theirSimulator->baseRun (-1);
  } else if (comm == "step") {
    theirSimulator->baseRun (1);
  } else if (comm == "dump") {
    theirSimulator->baseDump();
  } else {
    throw SimulatorParseError("'" + comm + "' is an unknown command");
  }
}

void SimulatorParse::execute (const KeyValue& value,
			      const KeyValue& param)
{
  if (value.dataType() != KeyValue::DTString) {
    throw SimulatorParseError("command is not a string");
  }
  const string& comm = value.getString();
  if (comm == "define") {
    theirSimulator->baseDefine (KeyValueMap());
  } else if (comm == "run"  ||  comm == "step") {
    theirSimulator->baseRun (param.getInt());
  } else if (comm == "step") {
    theirSimulator->baseRun (1);
  } else if (comm == "dump") {
    theirSimulator->baseDump();
  } else {
    throw SimulatorParseError("'" + comm + "' is an unknown command");
  }
}

void SimulatorParse::execute (const KeyValue& value,
			      const KeyValueMap& params)
{
  if (value.dataType() != KeyValue::DTString) {
    throw SimulatorParseError("command is not a string");
  }
  const string& comm = value.getString();
  if (comm == "define") {
    theirSimulator->baseDefine (params);
  } else if (comm == "run") {
    theirSimulator->baseRun (-1);
  } else if (comm == "dump") {
    theirSimulator->baseDump();
  } else if (comm == "dhfile") {
    theirSimulator->baseDHFile (params.find("dh")->second.getString(),
				params.getString("file",""));
  } else {
    throw SimulatorParseError("'" + comm + "' is an unknown command");
  }
}

string SimulatorParse::removeEscapes (const string& in)
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

string SimulatorParse::removeQuotes (const string& in)
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
      throw SimulatorParseError ("Ill-formed quoted string: " + in);
    }
    out += in.substr (pos+1, inx-pos-1);             // add substring
    pos = inx+1;
  }
  return out;
}



SimulatorParseError::SimulatorParseError (const string& message)
: std::runtime_error ("SimulateParse: " + message)
{}



void SimulatorParseerror (char*)
{
  std::ostringstream os1;
  os1 << SimulatorParse::line() + 1;
  std::ostringstream os2;
  os2 << SimulatorParse::position();

  throw SimulatorParseError("parse error at line " + os1.str() + ", position "
			    + os2.str() + " (at or near '" +
			    string(SimulatorTokenizetext) + "')");
}

// Define the yywrap function for flex.
// Note it is not declared in the .h file, but by flex.
int SimulatorTokenizewrap()
{
    return 1;
}


}
