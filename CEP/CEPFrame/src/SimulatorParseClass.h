//  SimulatorParse.h: Class for parsing simulation commands
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

#ifndef BASESIM_SIMULATORPARSE_H
#define BASESIM_SIMULATORPARSE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//# Includes
#include <Common/lofar_complex.h>
#include <Common/lofar_string.h>
#include <stdexcept>

extern "C" {
  int SimulatorTokenizewrap();
}

//# Forward Declarations
class Simulator;
class ParamValue;
class ParamBlock;

/**
   SimulatorParse is the class for parsing simulation commands.
   Its main function is the static function parse. This function
   reads, scans, and parses the commands and executes the corresponding
   functions.

   Furthermore the class is used as a container for the various kinds
   of values that a user can specify in the simulation commands.

   See Simulator_Example.cc for how this class can be used.
*/

class SimulatorParse
{
public:
  /// Parse and execute the commands to be read from standard input.
  static void parse (Simulator&);

  /// Execute the command given in the value.
  static void execute (const ParamValue&);

  /** Execute the command given in the value.
      The 2nd argument gives a parameter. Currently it is only
      used for the step and run command in which case it gives the number
      of steps to execute.
  */
  static void execute (const ParamValue&, const ParamValue& param);

  /** Execute the command given in the value.
      The 2nd argument gives a set of parameters, which can be used
      by the define.
  */
  static void execute (const ParamValue&, const ParamBlock& parameters);

  /// Give the current line (for read or update).
  static int& line()
    { return theirLine; }

  /// Give the current position (for read or update).
  static int& position()
    { return theirPosition; }

  /// A function to remove escaped characters.
  static string removeEscapes (const string& in);

  /// A function to remove quotes from a quoted string.
  static string removeQuotes (const string& in);

private:
  static int theirLine;
  static int theirPosition;
  static Simulator* theirSimulator;
};



/**
   SimulatorParseError is used in case of parser errors.
   An exception with this class is thrown. The object contains
   the actual error message.

   One can put a try/catch block around SimulatorParse::parse to
   catch this error object and, for example, to output a message.
*/

class SimulatorParseError: public std::runtime_error
{
public:
  /// Construct the error object with the given message.
  SimulatorParseError (const string& message);
};



/** The global yyerror function for the parser.
    It throws an exception with the current token.
*/
void SimulatorParseerror (char*);


#endif
