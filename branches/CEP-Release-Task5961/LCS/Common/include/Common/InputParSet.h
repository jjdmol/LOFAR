//# InputParSet.h: Parameters using ParameterSet or casacore's Input
//#
//# Copyright (C) 2011
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

#ifndef LOFAR_COMMON_INPUTPARSET_H
#define LOFAR_COMMON_INPUTPARSET_H

// \file
// Parameters using ParameterSet or casacore's Input

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/ParameterSet.h>

//# Forward declare Input.
namespace casa {
  class Input;
}


namespace LOFAR {

  // \addtogroup Common
  // @{

  // This class makes it possible to get parameters from a parset file using
  // ParameterSet or from the command line arguments using casacore' Input.
  //
  // It will be determined from the command line argumenst if Input
  // or ParameterSet is used. If a single argument without an =-sign is used,
  // the ParameterSet is used with that argument as parset name.

  class InputParSet
  {
  public:
    // The default constructor enables the creation of parameters. 
    InputParSet();

    ~InputParSet();
  
    // Create a new parameter.
    void create (const string& key, const string& defaultValue,
                 const string& help, const string& type); 

    // Fill the parameter list from argc, argv command line args.
    // If a single argument without =-sign is given, it is the name of a parset.
    // If '-h' is given, help info will be printed and exit(1) is called.
    // '?', '--help' and 'help' are also considered as help request.
    void readArguments (int argc, char const* const* argv);

    // Get the value of the parameter as a double.
    double getDouble (const string& key) const;

    // Get the value of the parameter as a vector of doubles.
    vector<double> getDoubleVector (const string& key) const;

    // Get the value of the parameter as an integer.
    int getInt (const string& key) const;

    // Get the value of the parameter as a vector of integers.
    vector<int> getIntVector (const string& key) const;

    // Get the value of the parameter as a string.
    string getString (const string& key) const;

    // Get the value of the parameter as a bool.
    bool getBool (const string& key) const;

    // Set version string for announcements.
    void setVersion (const string&);

    // Show the help info.
    void showHelp (ostream& os, const string& name);

  private:
    struct IPV {
      IPV();
      IPV (const string& shelp, const string& stype,
           const string& sdefVal);
      string help;
      string type;
      string defVal;
    };

    // Get the default value for the given parameter iterator.
    // It throws an exception if it has no default value.
    string getDefault (map<string,IPV>::const_iterator iter) const;

    ParameterSet itsParSet;
    casa::Input* itsInput;
    bool         itsUsePS;     //# true = use ParameterSet
    string       itsVersion;
    map<string, IPV> itsParms;
  };

} // namespace LOFAR

#endif
