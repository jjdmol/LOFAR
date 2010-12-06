//# pyparmdb.cc: python module for ParmFacade object.
//# Copyright (C) 2007
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
#include <ParmDB/ParmFacade.h>

#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycRecord.h>
#include <pyrap/Converters/PycBasicData.h>
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include "Package__Version.cc"

using namespace boost::python;
using namespace casa;
using namespace casa::pyrap;

namespace LOFAR { namespace BBS  {

  class PyParmDB : public ParmFacade
  {
  public:
    PyParmDB (const string& tableName)
      : ParmFacade (tableName)
    {}
    string version (const string& type) const
    { return Version::getInfo<pyparmdbVersion> ("parmdb", type); }
  };

  Record (ParmFacade::*fgetvalues0)(const string&,
                                    double, double,
                                    double, double,
                                    bool) = &ParmFacade::getValues;
  Record (ParmFacade::*fgetvalues1)(const string&,
                                    double, double, double,
                                    double, double, double,
                                    bool) = &ParmFacade::getValues;
  Record (ParmFacade::*fgetvalues2)(const string&,
                                    const vector<double>&,
                                    const vector<double>&,
                                    const vector<double>&,
                                    const vector<double>&,
                                    bool) = &ParmFacade::getValues;

  void pyparmdb()
  {
    class_<PyParmDB> ("ParmDB",
                      init<std::string>())

      .def ("_version", &PyParmDB::version,
            (boost::python::arg("type")="other"))
      .def ("_getRange", &ParmFacade::getRange,
 	    (boost::python::arg("parmnamepattern")=""))
      .def ("_getNames", &ParmFacade::getNames,
 	    (boost::python::arg("parmnamepattern")=""))
      .def ("_getDefNames", &ParmFacade::getDefNames,
 	    (boost::python::arg("parmnamepattern")=""))
      .def ("_getDefValues", &ParmFacade::getDefValues,
 	    (boost::python::arg("parmnamepattern")=""))
      .def ("_getValues", fgetvalues0,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("sfreq")=-1e30,
	     boost::python::arg("efreq")= 1e30,
	     boost::python::arg("stime")=-1e30,
	     boost::python::arg("etime")= 1e30,
	     boost::python::arg("asStartEnd")=true))
      .def ("_getValues", fgetvalues1,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("sfreq"),
	     boost::python::arg("efreq"),
	     boost::python::arg("freqstep"),
	     boost::python::arg("stime"),
	     boost::python::arg("etime"),
	     boost::python::arg("timestep"),
	     boost::python::arg("asStartEnd")=true))
      .def ("_getValuesVec", fgetvalues2,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("sfreq"),
	     boost::python::arg("efreq"),
	     boost::python::arg("stime"),
	     boost::python::arg("etime"),
	     boost::python::arg("asStartEnd")=true))
      .def ("_getValuesGrid", &ParmFacade::getValuesGrid,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("sfreq")=-1e30,
	     boost::python::arg("efreq")= 1e30,
	     boost::python::arg("stime")=-1e30,
	     boost::python::arg("etime")= 1e30,
	     boost::python::arg("asStartEnd")=true))
      .def ("_getCoeff", &ParmFacade::getCoeff,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("sfreq")=-1e30,
	     boost::python::arg("efreq")= 1e30,
	     boost::python::arg("stime")=-1e30,
	     boost::python::arg("etime")= 1e30,
	     boost::python::arg("asStartEnd")=true))
      ;
  }
    
}}


// Define the python module itself.
BOOST_PYTHON_MODULE(_parmdb)
{
  casa::pyrap::register_convert_excp();
  casa::pyrap::register_convert_basicdata();
  casa::pyrap::register_convert_std_vector<double>();
  casa::pyrap::register_convert_std_vector<std::string>();
  casa::pyrap::register_convert_casa_record();

  LOFAR::BBS::pyparmdb();
}
