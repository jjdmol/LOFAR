//# LofarATerm.h: Compute the LOFAR beam response on the sky.
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

#ifndef LOFAR_LOFARFT_LOFARATERMIONOSPHERE_H
#define LOFAR_LOFARFT_LOFARATERMIONOSPHERE_H

#include <Common/LofarTypes.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>

#include <ParmDB/ParmFacade.h>

#include <LofarFT/LofarATerm.h>
#include <LofarFT/LofarATermBeam.h>

#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>

namespace casa
{
  class DirectionCoordinate;
  class MEpoch;
  class MeasurementSet;
  class Path;
}

namespace LOFAR
{

  class LofarATermIonosphere : public LofarATerm
  {
  public:
    LofarATermIonosphere(const casa::MeasurementSet &ms, const casa::String& parmdbname);

    virtual vector<casa::Cube<casa::Complex> > evaluate(const casa::IPosition &shape,
      const casa::DirectionCoordinate &coordinates,
      uint station,
      const casa::MEpoch &epoch,
      const casa::Vector<casa::Double> &freq,
      bool normalize = false);

    virtual double resolution();

  private:
    casa::Array<casa::DComplex>
    normalize(const casa::Array<casa::DComplex> &response) const;

    casa::Cube<casa::Double>
    computeITRFMap(const casa::DirectionCoordinate &coordinates,
      const casa::IPosition &shape,
      casa::MDirection::Convert convertor) const;

    void initInstrument(const casa::MeasurementSet &ms);

    void initParmDB(const casa::String &parmdbname);

    Station initStation(const casa::MeasurementSet &ms,
      uint id,
      const casa::String &name,
      const casa::MPosition &position) const;

    void initReferenceFreq(const casa::MeasurementSet &ms,
      uint idDataDescription);

    void setEpoch( const casa::MEpoch &epoch );
  
    double get_parmvalue( std::string parmname );
  
    casa::MDirection refDelay, refTile;
    double           refFreq;
    double           time;
    double           r0, beta, height;
    Instrument       instrument;
    LOFAR::BBS::ParmFacade* pdb;
    casa::Vector<casa::String>   cal_pp_names;
    casa::Matrix<casa::Double> cal_pp;
    casa::Vector<casa::Double> tec_white;
    
  };
} // namespace LOFAR

#endif
