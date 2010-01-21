//# pyparameterset.cc: python module for ParameterSet class
//# Copyright (C) 2008
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
#include <Common/ParameterSet.h>
#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycRecord.h>
#include <pyrap/Converters/PycBasicData.h>
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include "Package__Version.cc"
//#include <pyparameterset/Package__Version.h>

using namespace boost::python;
using namespace casa::pyrap;

namespace LOFAR { 

  class PyParameterSet : public ParameterSet
  {
  public:
    PyParameterSet()
      : ParameterSet()
    {}
    PyParameterSet (bool caseInsensitive)
      : ParameterSet (caseInsensitive)
    {}
    PyParameterSet (const string& fileName, bool caseInsensitive)
      : ParameterSet (fileName, caseInsensitive)
    {}
    string version (const string& type) const
      { return Version::getInfo<pyparametersetVersion> (type); }
  };

  // Define function pointers for overloaded functions to be able to tell
  // boost-python which function to take.
  void (ParameterSet::*fadd)(const string&, const string&) =
    &ParameterSet::add;
  void (ParameterSet::*freplace)(const string&, const string&) =
    &ParameterSet::replace;

  bool (ParameterSet::*fgetbool1)(const string&) const =
    &ParameterSet::getBool;
  bool (ParameterSet::*fgetbool2)(const string&, bool) const =
    &ParameterSet::getBool;
  vector<bool> (ParameterSet::*fgetvecbool1)(const string&, bool) const =
    &ParameterSet::getBoolVector;
  vector<bool> (ParameterSet::*fgetvecbool2)(const string&, const vector<bool>&, bool) const =
    &ParameterSet::getBoolVector;

  int (ParameterSet::*fgetint1)(const string&) const =
    &ParameterSet::getInt;
  int (ParameterSet::*fgetint2)(const string&, int) const =
    &ParameterSet::getInt;
  vector<int> (ParameterSet::*fgetvecint1)(const string&, bool) const =
    &ParameterSet::getIntVector;
  vector<int> (ParameterSet::*fgetvecint2)(const string&, const vector<int>&, bool) const =
    &ParameterSet::getIntVector;

  float (ParameterSet::*fgetfloat1)(const string&) const =
    &ParameterSet::getFloat;
  float (ParameterSet::*fgetfloat2)(const string&, float) const =
    &ParameterSet::getFloat;
  vector<float> (ParameterSet::*fgetvecfloat1)(const string&, bool) const =
    &ParameterSet::getFloatVector;
  vector<float> (ParameterSet::*fgetvecfloat2)(const string&, const vector<float>&, bool) const =
    &ParameterSet::getFloatVector;

  double (ParameterSet::*fgetdouble1)(const string&) const =
    &ParameterSet::getDouble;
  double (ParameterSet::*fgetdouble2)(const string&, double) const =
    &ParameterSet::getDouble;
  vector<double> (ParameterSet::*fgetvecdouble1)(const string&, bool) const =
    &ParameterSet::getDoubleVector;
  vector<double> (ParameterSet::*fgetvecdouble2)(const string&, const vector<double>&, bool) const =
    &ParameterSet::getDoubleVector;

  string (ParameterSet::*fgetstring1)(const string&) const =
    &ParameterSet::getString;
  string (ParameterSet::*fgetstring2)(const string&, const string&) const =
    &ParameterSet::getString;
  vector<string> (ParameterSet::*fgetvecstring1)(const string&, bool) const =
    &ParameterSet::getStringVector;
  vector<string> (ParameterSet::*fgetvecstring2)(const string&, const vector<string>&, bool) const =
    &ParameterSet::getStringVector;


  // Define the python interface to ParameterValue.
  void pyparametervalue()
  {
    class_<ParameterValue> ("ParameterValue",
                            init<std::string, bool>())

      .def ("get",      &ParameterValue::get,
            return_value_policy < copy_const_reference> (),
            "Get the original value.")
      .def ("expand",   &ParameterValue::expand,
            "Expand possible range and repeat values (using .. and *)")
      .def ("isVector", &ParameterValue::isVector,
            "Test if the value contains a vector (if enclosed in [])")
      .def ("getVector", &ParameterValue::getVector,
            "Split the vector into its part and return as a list.")
      .def ("getBool",   &ParameterValue::getBool,
            "Get the value as a boolean value.")
      .def ("getInt",    &ParameterValue::getInt,
            "Get the value as an integer value.")
      .def ("getFloat",  &ParameterValue::getFloat,
            "Get the value as a floating point value.")
      .def ("getDouble",  &ParameterValue::getDouble,
            "Get the value as a floating point value.")
      .def ("getString", &ParameterValue::getString,
            "Get the value as a string value (quotes are removed).")
      .def ("getBoolVector",   &ParameterValue::getBoolVector,
            "Get the value vector as a list of boolean values.")
      .def ("getIntVector",    &ParameterValue::getIntVector,
            "Get the value vector as a list of integer values.")
      .def ("getFloatVector",  &ParameterValue::getFloatVector,
            "Get the value vector as a list of floating point values.")
      .def ("getDoubleVector",  &ParameterValue::getDoubleVector,
            "Get the value vector as a list of floating point values.")
      .def ("getStringVector", &ParameterValue::getStringVector,
            "Get the value vector as a list of strings (quotes are removed).")
      ;
  }


  // Define the python interface to ParameterSet.
  void pyparameterset()
  {
    class_<PyParameterSet> ("PyParameterSet")
      .def (init<bool>())
      .def (init<std::string, bool>())
      .def ("version", &PyParameterSet::version,
            (boost::python::arg("type")="other"),
            "Get the software version.")
      .def ("size", &ParameterSet::size,
            "Get the number of parameters.")
      .def ("__len__", &ParameterSet::size,
            "Get the number of parameters.")
      .def ("makeSubset", &ParameterSet::makeSubset,
 	    (boost::python::arg("baseKey"),
             boost::python::arg("prefix")=""),
            "Return a subset as a new parameterset object.\n"
            "\n"
            "baseKey\n"
            "  The leading part of the parameter name denoting the subset.\n"
            "  A trailing period has to be given.\n"
            "prefix\n"
            "  The baseKey parameter name part is replaced by the prefix.\n"
            "\n"
            "For example::\n"
            "\n"
            "  newps = ps.makeSubset ('p1,p2,', 'pr.')\n"
            "\n"
            "creates a subset of all keys starting with `p`1.p2.` and replaces\n"
            "that prefix by `pr.`.\n"
            )
      .def ("subtractSubset", &ParameterSet::subtractSubset,
 	    (boost::python::arg("baseKey")),
            "Remove all parameters starting with the baseKey.")
      .def ("adoptFile", &ParameterSet::adoptFile,
 	    (boost::python::arg("filename"),
 	     boost::python::arg("prefix")=""),
            "Add the parameters in the parset file with the given prefix.")
      .def ("writeFile", &ParameterSet::writeFile,
 	    (boost::python::arg("filename"),
             boost::python::arg("append")=false),
            "Write the parameterset into a parset file with the given name.")
      .def ("add", fadd,
 	    (boost::python::arg("key"),
             boost::python::arg("value")),
            "Add a parameter key/value pair.")
      .def ("replace", freplace,
 	    (boost::python::arg("key"),
             boost::python::arg("value")),
            "Replace the value of a parameter.")
      .def ("remove", &ParameterSet::remove,
 	    (boost::python::arg("key")))
      .def ("clear", &ParameterSet::clear,
            "Clear this parameterset object.")
      .def ("isDefined", &ParameterSet::isDefined,
 	    (boost::python::arg("key")),
            "Does a parameter with the given name exist? ")
      .def ("_get", &ParameterSet::get,
            return_value_policy < copy_const_reference> (),
 	    (boost::python::arg("key")))

      .def ("getBool", fgetbool1,
            (boost::python::arg("key")),
            "Get a boolean parameter value. Exception if undefined.")
      .def ("getBool", fgetbool2,
            (boost::python::arg("key"),
             boost::python::arg("default")),
            "Get a boolean parameter value. Use default if undefined.")
      .def ("getInt", fgetint1,
            (boost::python::arg("key")),
            "Get an integer parameter value. Exception if undefined.")
      .def ("getInt", fgetint2,
            (boost::python::arg("key"),
             boost::python::arg("default")),
            "Get an integer parameter value. Use default if undefined.")
      .def ("getFloat", fgetfloat1,
            (boost::python::arg("key")),
            "Get a floating point parameter value. Exception if undefined.")
      .def ("getFloat", fgetfloat2,
            (boost::python::arg("key"),
             boost::python::arg("default")),
            "Get a floating point parameter value. Use default if undefined.")
      .def ("getDouble", fgetdouble1,
            (boost::python::arg("key")),
            "Get a floating point parameter value. Exception if undefined.")
      .def ("getDouble", fgetdouble2,
            (boost::python::arg("key"),
             boost::python::arg("default")),
            "Get a floating point parameter value. Use default if undefined.")
      .def ("getString", fgetstring1,
            (boost::python::arg("key")),
            "Get a string parameter value. Exception if undefined.")
      .def ("getString", fgetstring2,
            (boost::python::arg("key"),
             boost::python::arg("default")),
            "Get a string parameter value. Use default if undefined.")

      .def ("_getBoolVector1", fgetvecbool1,
            (boost::python::arg("key"),
             boost::python::arg("expandable")))
      .def ("_getBoolVector2", fgetvecbool2,
            (boost::python::arg("key"),
             boost::python::arg("default"),
             boost::python::arg("expandable")))
      .def ("_getIntVector1", fgetvecint1,
            (boost::python::arg("key"),
             boost::python::arg("expandable")))
      .def ("_getIntVector2", fgetvecint2,
            (boost::python::arg("key"),
             boost::python::arg("default"),
             boost::python::arg("expandable")))
      .def ("_getFloatVector1", fgetvecfloat1,
            (boost::python::arg("key"),
             boost::python::arg("expandable")))
      .def ("_getFloatVector2", fgetvecfloat2,
            (boost::python::arg("key"),
             boost::python::arg("default"),
             boost::python::arg("expandable")))
      .def ("_getDoubleVector1", fgetvecdouble1,
            (boost::python::arg("key"),
             boost::python::arg("expandable")))
      .def ("_getDoubleVector2", fgetvecdouble2,
            (boost::python::arg("key"),
             boost::python::arg("default"),
             boost::python::arg("expandable")))
      .def ("_getStringVector1", fgetvecstring1,
            (boost::python::arg("key"),
             boost::python::arg("expandable")))
      .def ("_getStringVector2", fgetvecstring2,
            (boost::python::arg("key"),
             boost::python::arg("default"),
             boost::python::arg("expandable")))

      ;
  }
    
}


// Define the python module itself.
BOOST_PYTHON_MODULE(_pyparameterset)
{
  casa::pyrap::register_convert_excp();
  casa::pyrap::register_convert_basicdata();
  casa::pyrap::register_convert_std_vector<LOFAR::ParameterValue>();
  casa::pyrap::register_convert_std_vector<bool>();
  casa::pyrap::register_convert_std_vector<int>();
  casa::pyrap::register_convert_std_vector<float>();
  casa::pyrap::register_convert_std_vector<double>();
  casa::pyrap::register_convert_std_vector<std::string>();
  //  casa::pyrap::register_convert_casa_record();

  LOFAR::pyparametervalue();
  LOFAR::pyparameterset();
}
