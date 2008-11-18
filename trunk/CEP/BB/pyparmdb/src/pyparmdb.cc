//# pyparmdb.cc: python module for ParmFacade object.
//# Copyright (C) 2007
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
      { return Version::getInfo<pyparmdbVersion> (type); }
  };

  Record (ParmFacade::*fgetvalues1)(const string&,
                                    double, double, int,
                                    double, double, int,
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

      .def ("version", &PyParmDB::version,
            (boost::python::arg("type")="other"))
      .def ("getRange", &ParmFacade::getRange,
 	    (boost::python::arg("parmnamepattern")=""))
      .def ("getNames", &ParmFacade::getNames,
 	    (boost::python::arg("parmnamepattern")=""))
      .def ("getValues", fgetvalues1,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("freqv1"),
	     boost::python::arg("freqv2"),
	     boost::python::arg("nfreq"),
	     boost::python::arg("timev1"),
	     boost::python::arg("timev2"),
	     boost::python::arg("ntime"),
	     boost::python::arg("asStartEnd")=false))
      .def ("getValues", fgetvalues2,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("freqv1"),
	     boost::python::arg("freqv2"),
	     boost::python::arg("timev1"),
	     boost::python::arg("timev2"),
	     boost::python::arg("asStartEnd")=false))
      .def ("getValuesGrid", &ParmFacade::getValuesGrid,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("freqv1")=-1e30,
	     boost::python::arg("freqv2")= 1e30,
	     boost::python::arg("timev1")=-1e30,
	     boost::python::arg("timev2")= 1e30))
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
