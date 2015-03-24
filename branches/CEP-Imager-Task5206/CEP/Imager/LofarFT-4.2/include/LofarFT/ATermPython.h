//# ATermPython.h: Compute the LOFAR beam response on the sky.
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
//# $Id: LOFARATerm.h 18046 2011-05-19 20:58:40Z diepen $

#ifndef LOFAR_LOFARFT_ATERMPYTHON_H
#define LOFAR_LOFARFT_ATERMPYTHON_H

#include <LofarFT/ATermLofar.h>
#include <LofarFT/DynamicObjectFactory.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/ParameterSet.h>
#include <ParmDB/ParmFacade.h>

#include <casa/Arrays/Array.h>
#include <casa/Containers/Record.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>

#include <boost/python.hpp>

namespace casa
{
  class DirectionCoordinate;
  class MeasurementSet;
}

namespace LOFAR {
namespace LofarFT {

class ATermPython : public ATermLofar
{
public:
  ATermPython(const casa::MeasurementSet &ms, const ParameterSet& parameters);
  
  virtual vector<casa::Cube<casa::Complex> > evaluate(
    uint idStation,
    const casa::Vector<casa::Double> &freq,
    const casa::Vector<casa::Double> &reference, 
    bool normalize = false)
    const;

  void setDirection(const casa::DirectionCoordinate &coordinates, const casa::IPosition &shape);

  void setEpoch(const casa::MEpoch &epoch);
    
protected:    
  boost::python::object itsPyaterm;
};


} // namespace LofarFT
} // namespace LOFAR

#endif
