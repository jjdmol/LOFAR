/////boost::python::converter::registry
//# PycBasicData.cc: Convert LOFAR data types to/from python
//# Copyright (C) 2006
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <lofar_config.h>
#include <pytools/PycBasicData.h>

using namespace boost::python;


namespace LOFAR { namespace pytools {

  std::map<std::string,bool> pyregistry::_registry;
  bool pyregistry::get (const std::string& name)
  {
    return _registry[name];
  }
  void pyregistry::set (const std::string& name)
  {
    _registry[name] = true;
  }


  void register_convert_basicdata()
  {
    LOFAR::pytools::register_convert_std_vector<bool>();
    LOFAR::pytools::register_convert_std_vector<int>();
    LOFAR::pytools::register_convert_std_vector<double>();
    LOFAR::pytools::register_convert_std_vector<float>();
    LOFAR::pytools::register_convert_std_vector<dcomplex>();
    LOFAR::pytools::register_convert_std_vector<fcomplex>();
    LOFAR::pytools::register_convert_std_vector<std::string>();
  }


  bool getSeqObject (object& py_obj)
  {
    // Restriction to list, tuple, iter, xrange until
    // Boost.Python overload resolution is enhanced.
    //  PySequence_Check() is used for numarray.
    PyObject* obj_ptr = py_obj.ptr();
    if (!(PyList_Check(obj_ptr)
	  || PyTuple_Check(obj_ptr)
	  || PyIter_Check(obj_ptr)
	  || PyRange_Check(obj_ptr)
	  || PySequence_Check(obj_ptr) )) return false;
    // Must be a measurable sequence.
    int obj_size = -1;
    bool done = false;
    // Try to get attribute size, because length fails for a numpy scalar.
    try {
      // A numpy scalar size should be 1.
      object py_tmp = py_obj.attr("size");
      if (extract<int>(py_tmp) == 1) {
	done = true;
      }
    } catch (...) {
      PyErr_Clear();
    }
    // If it failed, try to get the length.
    if (!done) {
      obj_size = PyObject_Length(obj_ptr);
      if (obj_size < 0) {
	done = true;
	PyErr_Clear();
      }
    }
    // If we seem to have a numpy/numarray scalar, try to flatten it.
    // Return the flattened object.
    if (done) {
      done = false;
      object py_flat;
      // Try if the object is a scalar numarray/numpy object which
      // can be flattened to a vector num object.
      try {
	py_flat = py_obj.attr("flatten")();    // numpy attr name
	done = true;
      } catch (...) {
	PyErr_Clear();
      }
      if (!done) {
	try {
	  py_flat = py_obj.attr("flat");       // numarray attr name
	  done = true;
	} catch (...) {
	  PyErr_Clear();
	}
      }
      if (done) py_obj = py_flat;
    }
    // If it failed, try to get the length.
    if (!done) {
      obj_size = PyObject_Length(obj_ptr);
      if (obj_size >= 0) {
	done = true;
      } else {
	PyErr_Clear();
      }
    }
    return done;
  }


}}
