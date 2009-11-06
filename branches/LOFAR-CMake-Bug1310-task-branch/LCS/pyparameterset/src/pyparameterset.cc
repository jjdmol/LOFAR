//# pyparameterset.cc: python module for ParameterSet class
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <Common/ParameterSet.h>

#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycRecord.h>
#include <pyrap/Converters/PycBasicData.h>
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include "Package__Version.cc"

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
            return_value_policy < copy_const_reference> ())
      .def ("expand",   &ParameterValue::expand)
      .def ("isVector", &ParameterValue::isVector)
      .def ("getVector", &ParameterValue::getVector)
      .def ("getBool",   &ParameterValue::getBool)
      .def ("getInt",    &ParameterValue::getInt)
      .def ("getFloat",  &ParameterValue::getFloat)
      .def ("getDouble", &ParameterValue::getDouble)
      .def ("getString", &ParameterValue::getString)
      .def ("getBoolVector",   &ParameterValue::getBoolVector)
      .def ("getIntVector",    &ParameterValue::getIntVector)
      .def ("getFloatVector",  &ParameterValue::getFloatVector)
      .def ("getDoubleVector", &ParameterValue::getDoubleVector)
      .def ("getStringVector", &ParameterValue::getStringVector)
      ;
  }


  // Define the python interface to ParameterSet.
  void pyparameterset()
  {
    class_<PyParameterSet> ("PyParameterSet")
      .def (init<bool>())
      .def (init<std::string, bool>())

      .def ("version", &PyParameterSet::version,
            (boost::python::arg("type")="other"))
      .def ("size", &ParameterSet::size)
      .def ("__len__", &ParameterSet::size)
      .def ("makeSubset", &ParameterSet::makeSubset,
 	    (boost::python::arg("baseKey"),
             boost::python::arg("prefix")=""))
      .def ("subtractSubset", &ParameterSet::subtractSubset,
 	    (boost::python::arg("baseKey")))
      .def ("adoptFile", &ParameterSet::adoptFile,
 	    (boost::python::arg("filename"),
 	     boost::python::arg("prefix")=""))
      .def ("writeFile", &ParameterSet::writeFile,
 	    (boost::python::arg("filename"),
             boost::python::arg("append")=false))
      .def ("add", fadd,
 	    (boost::python::arg("key"),
             boost::python::arg("value")))
      .def ("replace", freplace,
 	    (boost::python::arg("key"),
             boost::python::arg("value")))
      .def ("remove", &ParameterSet::remove,
 	    (boost::python::arg("key")))
      .def ("clear", &ParameterSet::clear)
      .def ("isDefined", &ParameterSet::isDefined,
 	    (boost::python::arg("key")))
      .def ("get", &ParameterSet::get,
            return_value_policy < copy_const_reference> (),
 	    (boost::python::arg("key")))

      .def ("getBool", fgetbool1,
            (boost::python::arg("key")))
      .def ("getBool", fgetbool2,
            (boost::python::arg("key"),
             boost::python::arg("default")))
      .def ("getInt", fgetint1,
            (boost::python::arg("key")))
      .def ("getInt", fgetint2,
            (boost::python::arg("key"),
             boost::python::arg("default")))
      .def ("getFloat", fgetfloat1,
            (boost::python::arg("key")))
      .def ("getFloat", fgetfloat2,
            (boost::python::arg("key"),
             boost::python::arg("default")))
      .def ("getDouble", fgetdouble1,
            (boost::python::arg("key")))
      .def ("getDouble", fgetdouble2,
            (boost::python::arg("key"),
             boost::python::arg("default")))
      .def ("getString", fgetstring1,
            (boost::python::arg("key")))
      .def ("getString", fgetstring2,
            (boost::python::arg("key"),
             boost::python::arg("default")))

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
