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
#include <Common/ParameterRecord.h>
#include <pytools/PycExcp.h>
#include <pytools/PycBasicData.h>
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include "Package__Version.cc"
//#include <pyparameterset/Package__Version.h>

using namespace boost::python;

namespace LOFAR { 

  //# Forward declarations.
  class PyParameterSet;
  class PyParameterValue;

  // Declare function to convert vector of ParameterValue to PyParameterValue.
  vector<PyParameterValue> convertVectorPV (const vector<ParameterValue>& vec);

  //# Make a class PyParameterSet to be able to add the function version().
  //# We also need a PyParameterValue.
  class PyParameterValue: public ParameterValue
  {
  public:
    PyParameterValue()
      : ParameterValue()
    {}
    PyParameterValue (const string& value, bool trim=true)
      : ParameterValue (value, trim)
    {}
    PyParameterValue (const ParameterValue& pvalue)
      : ParameterValue (pvalue)
    {}
    PyParameterValue expand() const
      { return ParameterValue::expand(); }
    vector<PyParameterValue> getVector() const
      { return convertVectorPV (ParameterValue::getVector()); }
    PyParameterSet getRecord() const;
  };    

  class PyParameterSet : public ParameterSet
  {
  public:
    PyParameterSet()
      : ParameterSet()
    {}
    PyParameterSet (bool caseInsensitive, int, int)
      : ParameterSet (caseInsensitive)
    {}
    PyParameterSet (const string& fileName, bool caseInsensitive)
      : ParameterSet (fileName, caseInsensitive)
    {}
    PyParameterSet (const ParameterSet& pset)
      : ParameterSet (pset)
    {}
    string version (const string& type) const
      { return Version::getInfo<pyparametersetVersion> ("parameterset",
                                                        type); }

    PyParameterSet makeSubset (const string& baseKey, const string& prefix)
      { return ParameterSet::makeSubset (baseKey, prefix); }

    void adoptCollection (const PyParameterSet& parset, const string& prefix)
      { return ParameterSet::adoptCollection (parset, prefix); }

    PyParameterValue get (const string& key) const
      { return ParameterSet::get (key); }

    vector<PyParameterValue> getVector (const string& key) const
      { return convertVectorPV (ParameterSet::getVector(key)); }

    PyParameterSet getRecord (const string& key)
      { return ParameterSet::getRecord (key); }

    // Return the list of keywords.
    vector<string> keywords() const
    {
      vector<string> result;
      result.reserve (size());
      for (ParameterSet::const_iterator iter=begin(); iter!=end(); ++iter) {
        result.push_back (iter->first);
      }
      return result;
    }
  };
 
 inline PyParameterSet PyParameterValue::getRecord() const
    { return ParameterValue::getRecord(); }

  vector<PyParameterValue> convertVectorPV (const vector<ParameterValue>& vec)
  {
    vector<PyParameterValue> out;
    out.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter=vec.begin();
         iter!=vec.end(); ++iter) {
      out.push_back (*iter);
    }
    return out;
  }

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
    class_<PyParameterValue> ("PyParameterValue")

      .def (init<PyParameterValue>())
      .def (init<std::string, bool>())
      .def ("get",      &ParameterValue::get,
            return_value_policy < copy_const_reference> (),
            "Get the original value.")
      .def ("_expand",   &PyParameterValue::expand,
            "Expand possible range and repeat values (using .. and *)")
      .def ("isVector", &ParameterValue::isVector,
            "Test if the value contains a vector (if enclosed in [])")
      .def ("isRecord", &ParameterValue::isRecord,
            "Test if the value contains a record (if enclosed in {})")
      .def ("_getVector", &PyParameterValue::getVector,
            "Split the vector value into its parts and return as a list.")
      .def ("_getRecord", &PyParameterValue::getRecord,
            "Split the record value into its parts and return as a PyParameterSet.")
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
      .def (init<PyParameterSet>())
      .def (init<bool, int, int>())
      .def (init<std::string, bool>())
      .def ("version", &PyParameterSet::version,
            (boost::python::arg("type")="other"),
            "Get the software version.")
      .def ("size", &ParameterSet::size,
            "Get the number of parameters.")
      .def ("keywords", &PyParameterSet::keywords)
      .def ("_makeSubset", &PyParameterSet::makeSubset,
 	    (boost::python::arg("baseKey"),
             boost::python::arg("prefix")))
      .def ("subtractSubset", &ParameterSet::subtractSubset,
 	    (boost::python::arg("baseKey")),
            "Remove all parameters starting with the baseKey.")
      .def ("adoptFile", &ParameterSet::adoptFile,
 	    (boost::python::arg("filename"),
 	     boost::python::arg("prefix")=""),
            "Add the parameters from a parset file and prefix their names "
            "with the given prefix.")
      .def ("adoptCollection", &PyParameterSet::adoptCollection,
 	    (boost::python::arg("parameterset"),
 	     boost::python::arg("prefix")=""),
            "Add the parameters from a parset object and prefix their names "
            "with the given prefix.")
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
 	    (boost::python::arg("key")),
            "Remove a parameter.")
      .def ("clear", &ParameterSet::clear,
            "Clear this parameterset object.")
      .def ("locateModule", &ParameterSet::locateModule,
 	    (boost::python::arg("key")),
            "Search for a module whose name ends in the given modulename.")
      .def ("fullModuleName", &ParameterSet::fullModuleName,
 	    (boost::python::arg("key")),
            "Search the module name or module hierarchy and return its full position.")
      .def ("isDefined", &ParameterSet::isDefined,
 	    (boost::python::arg("key")),
            "Does a parameter with the given name exist? ")
      .def ("unusedKeys", &ParameterSet::unusedKeys,
            "Get the list of parameter keys not asked for")
      .def ("_get", &PyParameterSet::get,
            ///            return_value_policy < copy_const_reference> (),
 	    (boost::python::arg("key")))

      .def ("_getVector", &PyParameterSet::getVector,
            (boost::python::arg("key")),
            "Get a vector of parameter values. Exception if undefined.")
      .def ("_getRecord", &PyParameterSet::getRecord,
            (boost::python::arg("key")),
            "Get a parameter record. Exception if undefined.")
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
  LOFAR::pytools::register_convert_excp();
  LOFAR::pytools::register_convert_basicdata();
  LOFAR::pytools::register_convert_std_vector<LOFAR::PyParameterValue>();
  LOFAR::pytools::register_convert_std_vector<bool>();
  LOFAR::pytools::register_convert_std_vector<int>();
  LOFAR::pytools::register_convert_std_vector<float>();
  LOFAR::pytools::register_convert_std_vector<double>();
  LOFAR::pytools::register_convert_std_vector<std::string>();

  LOFAR::pyparametervalue();
  LOFAR::pyparameterset();
}
