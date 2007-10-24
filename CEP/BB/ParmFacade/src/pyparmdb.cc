//# pyparms.cc: python module for ParmFacade object.
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

#include <ParmFacade/ParmFacade.h>

#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycRecord.h>
#include <pyrap/Converters/PycBasicData.h>
#include <boost/python.hpp>
#include <boost/python/args.hpp>

using namespace boost::python;
using namespace casa::pyrap;

namespace LOFAR { namespace ParmDB {

  void pyparmdb()
  {
    class_<ParmFacade> ("ParmDB",
			init<std::string>())

      .def ("getrange", &ParmFacade::getRange,
 	    (boost::python::arg("parmnamepattern")=""))
      .def ("getnames", &ParmFacade::getNames,
 	    (boost::python::arg("parmnamepattern")=""))
      .def ("getvalues", &ParmFacade::getValuesRec,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("startx"),
	     boost::python::arg("endx"),
	     boost::python::arg("nx"),
	     boost::python::arg("starty"),
	     boost::python::arg("endy"),
	     boost::python::arg("ny")))
      .def ("gethistory", &ParmFacade::getHistoryRec,
 	    (boost::python::arg("parmnamepattern"),
	     boost::python::arg("startx"),
	     boost::python::arg("endx"),
	     boost::python::arg("starty"),
	     boost::python::arg("endy"),
	     boost::python::arg("startsolvetime")=0,
	     boost::python::arg("ndsolvetime")=1e25))
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

  LOFAR::ParmDB::pyparmdb();
}
