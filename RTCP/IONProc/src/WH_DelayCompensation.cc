//#  WH_DelayCompensation.cc: Workholder for the delay compensation.
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <WH_DelayCompensation.h>
#include <AMCBase/Direction.h>
#include <AMCBase/Position.h>
#include <AMCBase/Coord3D.h>
#include <AMCBase/Epoch.h>
#include <AMCBase/ConverterClient.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
#include <AMCImpl/ConverterImpl.h>    // Only for testing
#include <Common/LofarLogger.h>
#include <Common/PrettyUnits.h>

namespace LOFAR 
{
  using namespace AMC;

  namespace RTCP 
  {
    INIT_TRACER_CONTEXT(WH_DelayCompensation, LOFARLOGGER_PACKAGE);

    //##----------------  Public methods  ----------------##//

    WH_DelayCompensation::WH_DelayCompensation(const Parset *ps,
                                               const string &stationName) :      
      itsNrBeams   (ps->nrBeams()),
      itsStationName(stationName),
      itsConverter (0)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      
      itsNrCalcDelays = ps->getUint32("OLAP.DelayComp.nrCalcDelays");
     
       // Pre-allocate and initialize storage for the beam direction vectors.
      itsBeamDirectionsAfterEnd.resize(itsNrCalcDelays*itsNrBeams);
      itsObservationEpoch.resize(itsNrCalcDelays);

      getBeamDirections(ps);
      setPositionDiffs(ps);
      
      // Create the AMC converter.
      ASSERT(!itsConverter);
      itsConverter = createConverter();
      ASSERT(itsConverter);
    }


    WH_DelayCompensation::~WH_DelayCompensation()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      // Delete the AMC converter
      ASSERT(itsConverter);
      delete itsConverter;
      itsConverter = 0;
    }

    vector<Direction> WH_DelayCompensation::calculateAllBeamDirections(vector<double> &startIntegrationTime)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      for (uint i = 0; i < startIntegrationTime.size(); i++) {
	itsObservationEpoch[i].utc(startIntegrationTime[i]);
      }	
      
      // Calculate the delays for the epoch after the end of the current time
      // interval. Put the results in itsBeamDirectionsAfterEnd.
      calculateDirections();
      
      return itsBeamDirectionsAfterEnd;
    }

    const Position& WH_DelayCompensation::getPositionDiffs() const
    {
      return itsPhasePositionDiffs;
    }

    double WH_DelayCompensation::getDelay( Direction &dir ) const
    {
      return dir * itsPhasePositionDiffs * (1.0 / speedOfLight);
    }

    //##----------------  Private methods  ----------------##//

    void WH_DelayCompensation::getBeamDirections(const Parset *ps)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      
      // First let's find out how many sources we have.
      LOG_TRACE_VAR_STR(itsNrBeams << " beam direction(s):");

      // What coordinate system is used for these source directions?
      // Currently, we support J2000, ITRF, and AZEL.
      Direction::Types dirType(Direction::INVALID);
      
      itsBeamDirections.resize(itsNrBeams);
      
      // Get the source directions from the parameter set. 
      // Split the \a dir vector into separate Direction objects.
      for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
        string str = toUpper(ps->getBeamDirectionType(beam));
        
	if      (str == "J2000") dirType = Direction::J2000;
        else if (str == "ITRF")  dirType = Direction::ITRF;
        else if (str == "AZEL")  dirType = Direction::AZEL;
        else ASSERTSTR(false, "Observation.BeamDirectionType must be one of "
                       "J2000, ITRF, or AZEL");

        vector<double> beamDir = ps->getBeamDirection(beam);
	
        itsBeamDirections[beam] = Direction(beamDir[0], beamDir[1], dirType);
	LOG_TRACE_VAR_STR(" [" << beam << "] = " << itsBeamDirections[beam]);
      }
    }

    void WH_DelayCompensation::setPositionDiffs(const Parset *ps)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
       // Calculate the station to reference station position difference of apply station.
      LOG_TRACE_VAR("Position difference vectors:");
      
      // Station positions must be given in ITRF; there is currently no
      // support in the AMC package to convert between WGS84 and ITRF.
      Position::Types posType(Position::INVALID);
      string str = toUpper(ps->getString("OLAP.DelayComp.positionType"));
      if (str == "ITRF") posType = Position::ITRF;
      else ASSERTSTR(false, "OLAP.DelayComp.positionType must be ITRF");

      // Get the antenna positions from the parameter set. The antenna
      // positions are stored as one large vector of doubles.
      const Position pRef(Coord3D(ps->getRefPhaseCentres()), posType);
      const Position phaseCentres(Coord3D(ps->getPhaseCentresOf(itsStationName)), posType);
      itsPhaseCentres = phaseCentres;
      
      itsPhasePositionDiffs = itsPhaseCentres - pRef;
      LOG_TRACE_VAR_STR(itsPhasePositionDiffs);
    }

    Converter* WH_DelayCompensation::createConverter()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return new ConverterImpl();  
    }

    void WH_DelayCompensation::calculateDirections()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Convert the source coordinates to ITRF, for all beams and all
      // stations for the epoch "after-end" of the current time interval.
      RequestData request (itsBeamDirections, itsPhaseCentres, 
                           itsObservationEpoch);

      ResultData result;
      itsConverter->j2000ToItrf(result, request); //duur!!
      // Since we've calculated the coordinates for only one epoch, the number
      // of directions in the result vector must be equal to the number of
      // delays per epoch.

      ASSERTSTR(result.direction.size() == itsObservationEpoch.size() * itsNrBeams,
		result.direction.size() << " == " << itsObservationEpoch.size() * itsNrBeams);

      for (uint i = 0; i < result.direction.size(); ++i) {
        itsBeamDirectionsAfterEnd[i] = result.direction[i];
      }
     
      // From the source coordinates in ITRF, calculate the geometrical
      // delays. Please remember that the vector result.direction stores the
      // directions per epoch, per position, per direction. I.e. the first
      // itsBeamDirections.size() elements contain the converted directions
      // for itsPhaseCentres[0], the second for itsPhaseCentres[1],
      // etc.
      LOG_TRACE_CALC("Beamlet geometrical delays:");
      for (uint i = 0; i < itsObservationEpoch.size() * itsNrBeams; ++i) {
	LOG_TRACE_CALC_STR(" [" << i << "]: " << getDelay( itsBeamDirectionsAfterEnd[i]) );
      }
    }

  } // namespace RTCP

} // namespace LOFAR
