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
//  $Log$
//  Revision 1.4  2002/06/10 09:45:32  diepen
//
//  %[BugId: 38]%
//  Added command dhfile to the parser and added function setOutFile to
//  Simul.
//
//  Revision 1.3  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.2  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.1  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.5  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
//
//  Revision 1.4  2001/09/24 14:04:08  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.3  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.2  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.1  2001/03/01 13:17:03  gvd
//  New parser source files
//
//
/////////////////////////////////////////////////////////////////////////////

#include "BaseSim/SimulatorParseClass.h"
#include "BaseSim/ParamBlock.h"
#include "BaseSim/Simulator.h"
#include <Common/lofar_vector.h>
#include <Common/lofar_iostream.h>

//# stdlib.h is needed for bison 1.28 and needs to be included here
//# (before the flex/bison files).
//#include <stdlib.h>

//#include <SimulatorParse.ycc>                  // flex output
//#include <SimulatorParse.lcc>                  // bison output

extern char *SimulatorTokenizetext;

#include <strstream.h>


// Initialize statics.
int SimulatorParse::theirLine = 0;
int SimulatorParse::theirPosition = 0;
Simulator* SimulatorParse::theirSimulator = 0;

extern void SimulatorParseparse();

void SimulatorParse::parse (Simulator& simulator)
{
  // Set global Simulator pointer and call the bison parser.
  theirSimulator = &simulator;
  SimulatorParseparse();
  theirSimulator->baseQuit();
}

void SimulatorParse::execute (const ParamValue& value)
{
  if (value.dataType() != ParamValue::DTString) {
    throw SimulatorParseError("command is not a string");
  }
  const string& comm = value.getString();
  if (comm == "define") {
    theirSimulator->baseDefine (ParamBlock());
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

void SimulatorParse::execute (const ParamValue& value,
			      const ParamValue& param)
{
  if (value.dataType() != ParamValue::DTString) {
    throw SimulatorParseError("command is not a string");
  }
  const string& comm = value.getString();
  if (comm == "define") {
    theirSimulator->baseDefine (ParamBlock());
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

void SimulatorParse::execute (const ParamValue& value,
			      const ParamBlock& params)
{
  if (value.dataType() != ParamValue::DTString) {
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
  ostrstream os1;
  os1 << SimulatorParse::line() + 1 << ends;
  ostrstream os2;
  os2 << SimulatorParse::position() << ends;
  string s1(os1.str());
  string s2(os2.str());
  delete [] os1.str();
  delete [] os2.str();
  throw SimulatorParseError("parse error at line " + s1 + ", position "
			    + s2 + " (at or near '" +
			    string(SimulatorTokenizetext) + "')");
}

// Define the yywrap function for flex.
// Note it is not declared in the .h file, but by flex.
int SimulatorTokenizewrap()
{
    return 1;
}
