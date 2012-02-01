//# LofarGridder.cc: Gridder for LOFAR data correcting for DD effects
//#
//# Copyright (C) 2009
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
//#
//# @author Ger van Diepen <diepen at astron dot nl>

#include <lofar_config.h>
#include <ParmDB/Grid.h>

//# ASKAP includes
#include <LofarGridder/LofarGridder.h>
#include <gridding/VisGridderFactory.h>
#include <dataaccess/TableConstDataAccessor.h>
#include <dataaccess/TableConstDataIterator.h>
#include <dataaccess/OnDemandBufferDataAccessor.h>

//#casacore includes
#include <tables/Tables/TableRecord.h>

using namespace LOFAR::BBS;
using namespace casa;
using namespace askap;
using namespace askap::synthesis;

namespace LOFAR
{

  LofarGridder::LofarGridder (const ParameterSet& parset)
    : itsInitialized (false)
  {
    itsTimeAvg = parset.getInt ("average.timestep", 1);
    itsFreqAvg = parset.getInt ("average.freqstep", 1);
    itsCorrect = parset.getStringVector ("correct");
    itsParmDBName = parset.getString ("correct.parmdb");
    string gridderName = parset.getString ("name");
    itsIGridder = VisGridderFactory::createGridder (gridderName, parset);
    itsGridder = dynamic_cast<TableVisGridder*>(itsIGridder.get());
    ASSERT (itsGridder != 0);
  }

  LofarGridder::~LofarGridder()
  {}

  // Clone a copy of this Gridder
  IVisGridder::ShPtr LofarGridder::clone()
  {
    return IVisGridder::ShPtr(new LofarGridder(*this));
  }

  IVisGridder::ShPtr LofarGridder::makeGridder (const ParameterSet& parset)
  {
    std::cout << "LofarGridder::makeGridder" << std::endl;
    return IVisGridder::ShPtr(new LofarGridder(parset));
  }

  const std::string& LofarGridder::gridderName()
  {
    static std::string name("LofarGridder");
    return name;
  }

  void LofarGridder::registerGridder()
  {
    VisGridderFactory::registerGridder (gridderName(), &makeGridder);
  }

  void LofarGridder::initialiseGrid(const scimath::Axes& axes,
                                    const casa::IPosition& shape,
                                    const bool dopsf)
  {
    itsGridder->initialiseGrid (axes, shape, dopsf);
  }

  void LofarGridder::grid(IConstDataAccessor& acc)
  {
    if (! itsInitialized) {
      initCorrections (acc);
    }
    // do correction, uvw rotation, and averaging here
    // For time being only correction is done.
    // It is possible to do the following:
    ///  Array<Complex> data (acc.visibility());
    // which makes a reference copy of the array, thus changs it later.
    // This is an optimization we won't make yet.
    // Can use PolConverter::isLinear() to check if data are XX,XY,YX,YY
    OnDemandBufferDataAccessor acc2(acc);
    Array<Complex>& data = acc2.rwVisibility();
    itsGridder->grid (acc2);
    // Get facet center in J2000.
    casa::MVDirection center = itsGridder->getImageCentre();
    double time = acc.time();    // time in sec (do I need exposure?)
    const Vector<double>& freq = acc.frequency();
    Axis::ShPtr timeAxis(new RegularAxis ());
    Axis::ShPtr freqAxis(new RegularAxis ());
  }
      
  void LofarGridder::finaliseGrid(casa::Array<double>& out)
  {
    // If averaging is done, it has to grid the last timeslots.
    itsGridder->finaliseGrid (out);
  }

  void LofarGridder::finaliseWeights(casa::Array<double>& out)
  {
    itsGridder->finaliseWeights (out);
  }

  void LofarGridder::initialiseDegrid(const scimath::Axes& axes,
                                      const casa::Array<double>& image)
  {
    itsGridder->initialiseDegrid (axes, image);
  }

  void LofarGridder::customiseForContext(casa::String context)
  {
    itsGridder->customiseForContext (context);
  }
      
  void LofarGridder::initVisWeights(IVisWeights::ShPtr viswt)
  {
    itsGridder->initVisWeights (viswt);
  }
      
  void LofarGridder::degrid(IDataAccessor& acc)
  {
    if (! itsInitialized) {
      initCorrections (acc);
    }
    // do correction, uvw rotation, and averaging here
    // For time being only correction is done.
    OnDemandBufferDataAccessor acc2(acc);
    Array<Complex>& data = acc2.rwVisibility();
    itsGridder->degrid (acc2);
  }

  void LofarGridder::finaliseDegrid()
  {
    itsGridder->finaliseDegrid();
  }

//   void LofarGridder::initIndices(const IConstDataAccessor& acc) 
//   {
//     itsGridder->initIndices (acc);
//   }

//   void LofarGridder::initConvolutionFunction(const IConstDataAccessor& acc)
//   {
//     itsGridder->initConvolutionFunction (acc);
//   }
    
//   void LofarGridder::correctConvolution(casa::Array<double>& image)
//   {
//     itsGridder->correctConvolution (image);
//   }

  void LofarGridder::initCorrections (const IConstDataAccessor& acc)
  {
    const TableConstDataAccessor& tacc =
      dynamic_cast<const TableConstDataAccessor&>(acc);
    const TableConstDataIterator& titer = tacc.iterator();
    Table ms = titer.table();
    Table antTab(ms.keywordSet().asTable("ANTENNA"));
    itsInitialized = true;
  }

} //# end namespace
