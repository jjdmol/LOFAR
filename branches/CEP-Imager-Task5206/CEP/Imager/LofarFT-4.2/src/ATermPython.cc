//# ATermPython.cc: Compute the LOFAR beam response on the sky.
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
//# $Id: $

#include <lofar_config.h>
#include <LofarFT/ATermPython.h>
#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycValueHolder.h>
#include <pyrap/Converters/PycRecord.h>
#include <pyrap/Converters/PycArray.h>

#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <ms/MeasurementSets/MeasurementSet.h>

using namespace casa;

namespace
{
  bool dummy = LOFAR::LofarFT::ATermFactory::instance().registerClass<LOFAR::LofarFT::ATermPython>("ATermPython");
}

// BOOST_PYTHON_MODULE(aterm)
// {
//   class_<LOFAR::LofarFT::ATerm::ITRFDirectionMap>("ITRFDirectionMap", "Hi there");
// }

namespace LOFAR {
namespace LofarFT {
 
ATermPython::ATermPython(const MeasurementSet& ms, const ParameterSet& parameters) :
  ATermLofar(ms, parameters)
{

  cout << "==============" << endl;  
  #pragma omp critical
  {
  // Initialize Python interpreter
  Py_Initialize();
  
  boost::python::class_<LOFAR::LofarFT::ATerm::ITRFDirectionMap>("ITRFDirectionMap", "Hi there");
  boost::python::class_<casa::MEpoch>("MEpoch", "Mepoch");
  
  // Register converters for casa types from/to python types
  casa::pyrap::register_convert_excp();
  casa::pyrap::register_convert_basicdata();
  casa::pyrap::register_convert_casa_valueholder();
  casa::pyrap::register_convert_casa_record();
  try {
    // First import main
    boost::python::object main_module = boost::python::import("__main__");
    
    // Import the module
    boost::python::object embedded_module = boost::python::import(parameters.getString("ATermPython.module").c_str());
    
    // Get the python class object from the imported module
    boost::python::object pyATerm = embedded_module.attr(parameters.getString("ATermPython.class").c_str());

    // Import the lofar.parameterset module
    boost::python::object lofar_parameterset_module = boost::python::import("lofar.parameterset");
    
    boost::python::object pyparameterset = lofar_parameterset_module.attr("parameterset")();
    
    ParameterSet ps = boost::python::extract<ParameterSet>(pyparameterset);
    
    ps.adoptCollection(parameters);
    
    // create an instance of the python class
    itsPyaterm = pyATerm(pyparameterset); 
  }
  catch(boost::python::error_already_set const &)
  {
    // handle the exception in some way
    PyErr_Print();
  }
  }
}

vector<casa::Cube<casa::Complex> > ATermPython::evaluate(
  uint idStation,
  const casa::Vector<casa::Double> &freq,
  const casa::Vector<casa::Double> &reference, 
  bool normalize) const
{
  // call the evaluta method of the python ATerm instance
  vector<Cube<Complex> > result_vector;
  #pragma omp critical
  {
    boost::python::object result;
    try
    {
      result = itsPyaterm.attr("evaluate")(idStation, freq, reference, normalize);
    }
    catch(boost::python::error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
    ValueHolder v = boost::python::extract<ValueHolder>(result);
    Array<Complex> result_array = v.asArrayComplex();
    
    for(ArrayIterator<Complex> it(result_array, 3); !it.pastEnd(); it.next())
    {
      Cube<Complex> slice(it.array().shape());
      convertArray(slice, it.array());
      result_vector.push_back(slice);
    }
  }
  return result_vector;
}

void ATermPython::setDirection(
  const casa::DirectionCoordinate &coordinates,
  const IPosition &shape)
{
  Record r;
  coordinates.save(r, "");
  Record r1 = r.asRecord(0);
  
  #pragma omp critical
  {
    try
    {
      itsPyaterm.attr("setDirection")(r1, shape);
    }
    catch(boost::python::error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }  
}

void ATermPython::setEpoch( const MEpoch &epoch )
{
  Double time = epoch.get(casa::Unit("s")).getValue();
  
  #pragma omp critical
  {
    try
    {
      itsPyaterm.attr("setEpoch")(time);
    }
    catch(boost::python::error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }  
}

} // end namespace LofarFT
} // end namespace LOFAR
