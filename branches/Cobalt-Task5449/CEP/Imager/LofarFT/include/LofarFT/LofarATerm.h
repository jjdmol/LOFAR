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

#ifndef LOFAR_LOFARFT_LOFARATERM_H
#define LOFAR_LOFARFT_LOFARATERM_H

#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/LofarTypes.h>
#include <ParmDB/ParmFacade.h>
#include <StationResponse/Station.h>
#include <casa/Arrays/Array.h>
#include <casa/Containers/Record.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>

namespace casa
{
  class DirectionCoordinate;
  class MeasurementSet;
}

namespace LOFAR
{
  class LofarATerm
  {
  public:
    /*!
     *  \brief Map of ITRF directions required to compute an image of the
     *  station beam.
     *
     *  The station beam library uses the ITRF coordinate system to express
     *  station positions and source directions. Since the Earth moves with
     *  respect to the sky, the ITRF coordinates of a source vary with time.
     *  This structure stores the ITRF coordinates for the station and tile beam
     *  former reference directions, as well as for a grid of points on the sky,
     *  along with the time for which these ITRF coordinates are valid.
     */
    struct ITRFDirectionMap
    {
      /*!
       *  \brief The time for which this ITRF direction map is valid (MJD(UTC)
       *  in seconds).
       */
      double_t                                  time0;

      /*!
       *  \brief Station beam former reference direction expressed in ITRF
       *  coordinates.
       */
      StationResponse::vector3r_t               station0;

      /*!
       *  \brief Tile beam former reference direction expressed in ITRF
       *  coordinates.
       */
      StationResponse::vector3r_t               tile0;

      /*!
       *  \brief ITRF coordinates for a grid of points on the sky.
       */
      casa::Matrix<StationResponse::vector3r_t> directions;
    };

    LofarATerm(const casa::MeasurementSet &ms, const casa::Record& parameters);

    void setDirection(const casa::DirectionCoordinate &coordinates,
        const casa::IPosition &shape);

    void setEpoch(const casa::MEpoch &epoch);

    /*!
     *  \brief Compute an ITRF direction vector for each pixel at the given
     *  epoch. This map can then be used to call any of the evaluate* functions.
     *
     *  \param coordinates Sky coordinate system definition.
     *  \param shape Number of points along the RA and DEC axis.
     *  \param epoch Time for which to compute the ITRF coordinates.
     *  \param position0 Station beam former reference position (phase reference).
     *  \param station0 Station beam former reference direction (pointing).
     *  \param tile0 Tile beam former reference direction (pointing).
     */
    ITRFDirectionMap
    makeDirectionMap(const casa::DirectionCoordinate &coordinates,
      const casa::IPosition &shape, const casa::MEpoch &epoch) const;

    // Compute the LOFAR station response for the given station. This includes
    // the effects of paralactic rotation, the dual dipole LOFAR antenna, the
    // tile beam former (HBA only), and the station beam former.
    //
    // The freq argument is a list of frequencies at which the response will be
    // evaluated. The reference argument is a list of station beam former
    // reference frequencies. The normalize argument, when set to true, causes
    // the response to be multiplied by the inverse of the response at the
    // central pixel.
    vector<casa::Cube<casa::Complex> > evaluate(uint idStation,
      const casa::Vector<casa::Double> &freq,
      const casa::Vector<casa::Double> &freq0,
      bool normalize = false) const;

    // Compute the array factor for the given station and polarization (0 = X,
    // 1 = Y).
    //
    // The freq argument is a list of frequencies at which the array factor will
    // be evaluated. The reference argument is a list of station beam former
    // reference frequencies. The normalize argument, when set to true, causes
    // the response to be multiplied by the inverse of the array factor at the
    // central pixel.
    casa::Cube<casa::DComplex> evaluateStationScalarFactor(uint idStation,
      uint idPolarization,
      const casa::Vector<casa::Double> &freq,
      const casa::Vector<casa::Double> &freq0,
      bool normalize = false) const;

    vector<casa::Matrix<casa::Complex> > evaluateArrayFactor(uint idStation,
      uint idPolarization,
      const casa::Vector<casa::Double> &freq,
      const casa::Vector<casa::Double> &freq0,
      bool normalize = false) const;

    // Compute the LOFAR element response for the given station and antenna
    // field. This includes the effects of paralactic rotation and the dual
    // dipole LOFAR antenna.
    //
    // The freq argument is a list of frequencies at which the response will be
    // evaluated. The normalize argument, when set to true, causes the response
    // to be multiplied by the inverse of the response at the central pixel.
    vector<casa::Cube<casa::Complex> > evaluateElementResponse(uint idStation,
      uint idField,
      const casa::Vector<casa::Double> &freq,
      bool normalize = false) const;

    casa::Cube<casa::DComplex> evaluateIonosphere(const uint idStation,
      const casa::Vector<casa::Double> &freq) const;

  private:
    void initParmDB(const casa::String &parmdbname);
    double getParmValue(casa::Record &parms, const string &parmname);

    vector<StationResponse::Station::ConstPtr>  itsStations;
    const casa::DirectionCoordinate             *itsDirectionCoordinates;
    const casa::IPosition                       *itsShape;
    casa::MPosition                             itsPosition0;
    casa::MDirection                            itsStation0, itsTile0;
    ITRFDirectionMap                            itsITRFDirectionMap;

    casa::Bool                                  itsApplyBeam;
    casa::Bool                                  itsApplyIonosphere;
    LOFAR::BBS::ParmFacade                      *pdb;
    double                                      time, r0, beta, height;
    casa::Vector<casa::String>                  cal_pp_names;
    casa::Matrix<casa::Double>                  cal_pp;
    casa::Vector<casa::Double>                  tec_white;
    casa::Int                                   itsVerbose;
  };

} // namespace LOFAR

#endif
