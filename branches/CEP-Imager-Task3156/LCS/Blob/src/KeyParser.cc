//# KeyParser.cc: Class for parsing a key=value line
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/KeyValueMap.h>
#include <Blob/KeyParser.h>
#include "KeyParse.h"
#include <Common/lofar_iostream.h>
#include <Common/Exception.h>
#include <sstream>
#include <fstream>
#include <stdio.h>

//# Define extern symbols in KeyTokenize.cc.
//extern int yy_start;
extern char* KeyTokenizetext;
extern FILE* KeyTokenizein;
extern void KeyParseparse();
extern void KeyTokenizerestart(FILE*);

namespace LOFAR {

// Initialize statics.
int KeyParser::theirPosition = 0;
const char* KeyParser::theirCommand = 0;
KeyValueMap* KeyParser::theirKeyMap = 0;


KeyValueMap KeyParser::parseFile (const string& fileName)
{
  return parseFile (fileName.c_str());
}

KeyValueMap KeyParser::parseFile (const char* fileName)
{
  string command;
  char buf[4096];
  std::ifstream ifs(fileName);
  if (!ifs) {
    throw Exception("KeyValue file " + string(fileName) +
		    " could not be opened");
  }
  while (ifs.getline(buf, sizeof(buf))) {
    if (buf[0] != '#') {
      command += buf;
      command += '\n';
    }
  }
  return parse(command);
}

KeyValueMap KeyParser::parse (const std::string& command)
{
  // Return an empty map if the command is empty.
  bool empty = true;
  for (uint i=0; i<command.size(); i++) {
    if (command[i] != ' ') {
      empty = false;
      break;
    }
  }
  if (empty) {
    return KeyValueMap();
  }
  //# Parse the command.
  //# Do a yyrestart(yyin) first to make the flex scanner reentrant.
  KeyTokenizerestart (KeyTokenizein);
  //  yy_start = 1;
  theirCommand = command.c_str();     // get pointer to command string
  theirPosition = 0;                  // initialize string position
  delete theirKeyMap;
  theirKeyMap = 0;
  KeyParseparse();                    // parse command string
  KeyValueMap map(*theirKeyMap);
  delete theirKeyMap;
  theirKeyMap = 0;
  return map;
}

int KeyParser::input (char* buf, int max_size)
{
    int nr=0;
    while (*theirCommand != 0) {
	if (nr >= max_size) {
	    break;                         // get max. max_size char.
	}
	buf[nr++] = *theirCommand++;
    }
    return nr;
}

string KeyParser::removeEscapes (const string& in)
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

string KeyParser::removeQuotes (const string& in)
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
      throw KeyParserError ("Ill-formed quoted string: " + in);
    }
    out += in.substr (pos+1, inx-pos-1);             // add substring
    pos = inx+1;
  }
  return out;
}


KeyParserError::KeyParserError (const string& message)
: std::runtime_error ("KeyParser: " + message)
{}


void KeyParseerror (const char*)
{
  std::ostringstream os2;
  os2 << KeyParser::position();
  string s2(os2.str());
  throw KeyParserError("parse error at position "
			    + s2 + " (at or near '" +
			    string(KeyTokenizetext) + "')");
}

} // end namespace


// Define the yywrap function for flex.
// Note it is not declared in the .h file, but by flex.
int KeyTokenizewrap()
{
    return 1;
}
