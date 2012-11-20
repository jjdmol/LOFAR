//# InputParSet.cc: Parameters using ParameterSet or casacore's Input
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

#include <lofar_config.h>
#include <Common/InputParSet.h>
#include <Common/Exceptions.h>
#ifdef HAVE_AIPSPP
# include <casa/Inputs/Input.h>
#endif

namespace LOFAR {

  InputParSet::InputParSet()
    : itsInput (0),
      itsUsePS (false)
  {
#ifdef HAVE_AIPSPP
    itsInput = new casa::Input;
#endif
  }

  InputParSet::~InputParSet()
  {
#ifdef HAVE_AIPSPP
    delete itsInput;
#endif
  }
  
  void InputParSet::create (const std::string& key,
                            const std::string& defaultValue,
                            const std::string& help,
                            const std::string& type)
  {
#ifdef HAVE_AIPSPP
    itsInput->create (key, defaultValue, help, type);
#endif
    itsParms[key] = IPV(help, type, defaultValue);
  }

  void InputParSet::readArguments (int argc, char const* const* argv)
  {
    if (argc == 2) {
      string value(argv[1]);
      // Use ParameterSet if no =-sign.
      if (value.find('=') == string::npos) {
        if (value == "-h"  ||  value == "--help"  ||  value == "help" ||
            value == "?") {
          showHelp (cerr, argv[0]);
          exit(1);
        }
        itsParSet = ParameterSet (value);
        itsUsePS  = true;
        return;
      }
    }
#ifdef HAVE_AIPSPP
    itsInput->readArguments (argc, argv);
#else
    THROW (APSException, "Program parameters are given on the command line, "
           "but casacore's Input is not available to handle them");
#endif
  }

  string InputParSet::getDefault (map<string,IPV>::const_iterator iter) const
  {
    return iter->second.defVal;
  }

  double InputParSet::getDouble (const std::string& key) const
  {
    map<string,IPV>::const_iterator iter = itsParms.find(key);
    if (iter == itsParms.end()) {
      THROW (APSException, "Parameter " + key +
             " does not exist in InputParSet");
    }
    if (itsUsePS) {
      if (itsParSet.isDefined (key)) {
        return itsParSet.getDouble (key);
      }
      ParameterValue pv(getDefault(iter));
      return pv.getDouble();
    }
#ifdef HAVE_AIPSPP
    return itsInput->getDouble (key);
#endif
    return 0;
  }

  vector<double> InputParSet::getDoubleVector (const std::string& key) const
  {
    map<string,IPV>::const_iterator iter = itsParms.find(key);
    if (iter == itsParms.end()) {
      THROW (APSException, "Parameter " + key +
             " does not exist in InputParSet");
    }
    if (itsUsePS) {
      if (itsParSet.isDefined (key)) {
        return itsParSet.getDoubleVector (key);
      }
      ParameterValue pv('[' + getDefault(iter) + ']');
      return pv.getDoubleVector();
    }
#ifdef HAVE_AIPSPP
    casa::Block<double> vals (itsInput->getDoubleArray (key));
    return vector<double>(vals.begin(), vals.end());
#endif
    return vector<double>();
  }

  int InputParSet::getInt (const std::string& key) const
  {
    map<string,IPV>::const_iterator iter = itsParms.find(key);
    if (iter == itsParms.end()) {
      THROW (APSException, "Parameter " + key +
             " does not exist in InputParSet");
    }
    if (itsUsePS) {
      if (itsParSet.isDefined (key)) {
        return itsParSet.getInt (key);
      }
      ParameterValue pv(getDefault(iter));
      return pv.getInt();
    }
#ifdef HAVE_AIPSPP
    return itsInput->getInt (key);
#endif
    return 0;
  }

  vector<int> InputParSet::getIntVector (const std::string& key) const
  {
    map<string,IPV>::const_iterator iter = itsParms.find(key);
    if (iter == itsParms.end()) {
      THROW (APSException, "Parameter " + key +
             " does not exist in InputParSet");
    }
    if (itsUsePS) {
      if (itsParSet.isDefined (key)) {
        return itsParSet.getIntVector (key);
      }
      ParameterValue pv('[' + getDefault(iter) + ']');
      return pv.getIntVector();
    }
#ifdef HAVE_AIPSPP
    casa::Block<int> vals (itsInput->getIntArray (key));
    return vector<int>(vals.begin(), vals.end());
#endif
    return vector<int>();
  }

  bool InputParSet::getBool (const std::string& key) const
  {
    map<string,IPV>::const_iterator iter = itsParms.find(key);
    if (iter == itsParms.end()) {
      THROW (APSException, "Parameter " + key +
             " does not exist in InputParSet");
    }
    if (itsUsePS) {
      if (itsParSet.isDefined (key)) {
        return itsParSet.getBool (key);
      }
      ParameterValue pv(getDefault(iter));
      return pv.getBool();
    }
#ifdef HAVE_AIPSPP
    return itsInput->getBool (key);
#endif
    return false;
  }

  string InputParSet::getString (const std::string& key) const
  {
    map<string,IPV>::const_iterator iter = itsParms.find(key);
    if (iter == itsParms.end()) {
      THROW (APSException, "Parameter " + key +
             " does not exist in InputParSet");
    }
    if (itsUsePS) {
      if (itsParSet.isDefined (key)) {
        return itsParSet.getString (key);
      }
      return getDefault(iter);
    }
#ifdef HAVE_AIPSPP
    return itsInput->getString (key);
#endif
    return 0;
  }

  void InputParSet::setVersion (const string& version)
  {
    itsVersion = version;
#ifdef HAVE_AIPSPP
    itsInput->version (version);
#endif
  }

  void InputParSet::showHelp (ostream& os, const string& name)
  {
    os << name << "  version " << itsVersion << endl;
    for (map<string,IPV>::const_iterator iter = itsParms.begin();
         iter!=itsParms.end(); ++iter) {
      os << iter->first << ", " << iter->second.type << ", ";
      os << '(' << iter->second.defVal << ')';
      os << endl;
      if (! iter->second.help.empty()) {
        os << "    " << iter->second.help << endl;
      }
    }
  }


  InputParSet::IPV::IPV()
  {}

  InputParSet::IPV::IPV (const string& shelp, const string& stype,
                         const string& sdefVal)
    : help   (shelp),
      type   (stype),
      defVal (sdefVal)
  {}


} // namespace LOFAR
