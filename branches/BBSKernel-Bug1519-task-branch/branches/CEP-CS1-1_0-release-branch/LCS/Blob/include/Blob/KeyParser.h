//# KeyParser.h: Class for parsing a key=value line
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_BLOB_KEYPARSER_H
#define LOFAR_BLOB_KEYPARSER_H

// \file
// Class for parsing a key=value line.

//# Includes
//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/lofar_complex.h>
#include <Common/lofar_string.h>
#include <stdexcept>

extern "C" {
  int KeyTokenizewrap();        // yywrap
}

namespace LOFAR {

  // \ingroup Blob
  // \addtogroup KeyValue
  // <group>

  //# Forward Declarations
  class KeyValue;
  class KeyValueMap;
  
  // KeyParser is the class for parsing a key=value command.
  // Its main function is the static function parse. This function
  // reads, scans, and parses the key=value pairs.

  class KeyParser
    {
    public:
      // Parse the command in the given string and return the resulting map.
      static KeyValueMap parse (const std::string& command);
      
      // \name Parse the command in the given file.
      // Parse the command in the given file and return the resulting map.
      // Lines starting with # are comments and ignored.
      // <group>
      static KeyValueMap parseFile (const std::string& fileName);
      static KeyValueMap parseFile (const char* fileName);
      // </group>
      
      // Give the next chunk of input for the scanner.
      static int input (char* buf, int max_size);
      
      // Give the current position (for read or update).
      static int& position()
	{ return theirPosition; }
      
      // A function to remove escaped characters.
      static string removeEscapes (const string& in);
      
      // A function to remove quotes from a quoted string.
      static string removeQuotes (const string& in);
      
      // Let the parser set the final KeyValueMap.
      static void setMap (KeyValueMap* map)
	{ theirKeyMap = map; }
      
    private:
      static int theirPosition;
      static const char* theirCommand;
      static KeyValueMap* theirKeyMap;
    };
  
  
  
  // KeyParserError is used in case of parser errors.
  // An exception with this class is thrown. The object contains
  // the actual error message.
  // 
  // One can put a try/catch block around KeyParser::parse to
  // catch this error object and, for example, to output a message.
  
  class KeyParserError: public std::runtime_error
    {
    public:
      // Construct the error object with the given message.
      KeyParserError (const string& message);
    };
  
  
  
  // The global yyerror function for the parser.
  // It throws an exception with the current token.

  void KeyParseerror (char*);
  
  // </group>

} // end namespace

#endif
