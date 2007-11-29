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
#include <CS1_DelayCompensation/WH_DelayCompensation.h>
#include <CS1_Interface/DH_Delay.h>
#include <AMCBase/Direction.h>
#include <AMCBase/Position.h>
#include <AMCBase/Epoch.h>
#include <AMCBase/ConverterClient.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
#include <AMCImpl/ConverterImpl.h>    // Only for testing
#include <Common/LofarLogger.h>

namespace LOFAR 
{
  using namespace AMC;
  using ACC::APS::ParameterSet;

  namespace CS1 
  {
    INIT_TRACER_CONTEXT(WH_DelayCompensation, LOFARLOGGER_PACKAGE);

    //##----------------  Public methods  ----------------##//

    WH_DelayCompensation::WH_DelayCompensation(const string& name,
                                                     CS1_Parset *ps) :
      WorkHolder   (0,                                   // inputs
                    ps->getUint32("OLAP.nrRSPboards"),    // outputs
                    name,                                // name
                    "WH_DelayCompensation"),             // type
      itsCS1PS     (ps),
      itsNrBeams   (ps->getUint32("Observation.nrBeams")),
      itsNrStations(itsCS1PS->nrStations()),
      itsNrDelays  (itsNrBeams*itsNrStations),
      itsSampleRate(itsCS1PS->sampleRate()),
      itsLoopCount (0),
      itsConverter (0)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Initialize our private data structures.
      getConverterConfig();
      getBeamDirections();
      getPhaseCentres();
      getObservationEpoch();
      setPositionDiffs();

      // Create outgoing dataholders for the delay information; 
      // one for each RSP board.
      for (int i = 0; i < itsNoutputs; ++i) {
        string str = "DH_Delay_out_" + formatString("%02d", i);
        LOG_TRACE_LOOP_STR("Creating " << str);
	getDataManager().addOutDataHolder(i, new DH_Delay(str, itsNrDelays));
      }
    }


    WH_DelayCompensation::~WH_DelayCompensation()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    void WH_DelayCompensation::preprocess()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      itsLoopCount = 0;

      // Create the AMC converter.
      ASSERT(!itsConverter);
      itsConverter = createConverter();
      ASSERT(itsConverter);

      // Pre-allocate and initialize storage for the delay vectors.
      itsDelaysAtBegin.resize(itsNrDelays);
      itsDelaysAfterEnd.resize(itsNrDelays);
      // Initialize \c itsDelaysAfterEnd with the conversion results for the
      // epoch after the end of the first time interval.
      calculateDelays();
    }


    void WH_DelayCompensation::process()
    {
      // Calculate the delays for the epoch after the end of the current time
      // interval. Put the results in itsDelaysAtBegin and itsDelaysAfterEnd.
      if (itsLoopCount < itsObservationEpoch.size()) {
        calculateDelays();

        // Incrementing the loop count
        LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_FLOW, "count = " << itsLoopCount);
        itsLoopCount++;
            
        // The delays -- split into a coarse (sample) delay and a fine
        // (subsample) delay -- need to be put into a DelayInfo struct.
        vector<DH_Delay::DelayInfo> delayInfo(itsNrDelays);
        for (uint i = 0; i < itsNrDelays; ++i) {

	  // Get delays "at begin" and "after end".
	  double db = itsDelaysAtBegin[i];
	  double de = itsDelaysAfterEnd[i];

	  // The coarse delay will be determined for the center of the current
	  // time interval and will be expressed in \e samples.
	  delayInfo[i].coarseDelay = 
	    (int32)floor(0.5 * (db + de) * itsSampleRate + 0.5);
	
	  // The fine delay will be determined for the boundaries of the current
	  // time interval and will be expressed in seconds.
	  double d = delayInfo[i].coarseDelay / itsSampleRate;
	  delayInfo[i].fineDelayAtBegin  = (float)(db - d);
	  delayInfo[i].fineDelayAfterEnd = (float)(de - d);

	  LOG_TRACE_CALC_STR("Beamlet #" << i << ":");
	  LOG_TRACE_CALC_STR(" coarseDelay       = " << 
	  		   delayInfo[i].coarseDelay);
	  LOG_TRACE_CALC_STR(" fineDelayAtBegin  = " << 
	  		   delayInfo[i].fineDelayAtBegin);
	  LOG_TRACE_CALC_STR(" fineDelayAfterEnd = " << 
			     delayInfo[i].fineDelayAfterEnd);
        }

        // We need to send the coarse and fine delay info to all RSP boards.
        for (int rsp = 0; rsp < itsNoutputs; ++rsp) {
	  DH_Delay* dh;
	  ASSERT(dh = dynamic_cast<DH_Delay*>(getDataManager().getOutHolder(rsp)));
	  for (uint i = 0; i < itsNrDelays; ++i) {
  	    (*dh)[i] = delayInfo[i];
	  }
        }
      }
      else{
	sleep(1);
      }
    }


    void WH_DelayCompensation::postprocess()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      // Delete the AMC converter
      ASSERT(itsConverter);
      delete itsConverter;
      itsConverter = 0;
    }
    

    WH_DelayCompensation* WH_DelayCompensation::make(const string& name)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return new WH_DelayCompensation(name, itsCS1PS);
    }


    //##----------------  Private methods  ----------------##//

    void WH_DelayCompensation::getConverterConfig()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      string type = toUpper(itsCS1PS->getString("OLAP.DelayComp.converterType"));
      if (type == "IMPL") itsConverterConfig.type = IMPL;
      else if (type == "CLIENT") {
        itsConverterConfig.type   = CLIENT;
        itsConverterConfig.server = itsCS1PS->getString("OLAP.OLAP_Conn.AMCServerHost");
        itsConverterConfig.port   = itsCS1PS->getUint16("OLAP.OLAP_Conn.AMCServerPort");
      }
    }


    void WH_DelayCompensation::getBeamDirections()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      // First let's find out how many sources we have.
      LOG_TRACE_VAR_STR(itsNrBeams << " beam direction(s):");

      // What coordinate system is used for these source directions?
      // Currently, we support J2000, ITRF, and AZEL.
      Direction::Types dirType(Direction::INVALID);
      string str = toUpper(itsCS1PS->getString("Observation.Beam.directionTypes"));
      if      (str == "J2000") dirType = Direction::J2000;
      else if (str == "ITRF")  dirType = Direction::ITRF;
      else if (str == "AZEL")  dirType = Direction::AZEL;
      else ASSERTSTR(false, "Observation.BeamDirectionType must be one of "
                     "J2000, ITRF, or AZEL");

      // Get the source directions from the parameter set. The source
      // directions are stored as one large vector of doubles. A direction
      // consists of two doubles.
      vector<double> angle1, angle2;

      angle1 = itsCS1PS->getDoubleVector("Observation.Beam.angle1");
      angle2 = itsCS1PS->getDoubleVector("Observation.Beam.angle2");
      ASSERTSTR(angle1.size() == itsNrBeams, 
                angle1.size() << " == " << itsNrBeams);
      ASSERTSTR(angle2.size() == itsNrBeams, 
                angle2.size() << " == " << itsNrBeams);
      
      // Reserve space in \a itsBeamDirections to avoid reallocations.
      itsBeamDirections.reserve(itsNrBeams);
      
      // Split the \a dir vector into separate Direction objects.
      for (uint i = 0; i < itsNrBeams; ++i) {
        itsBeamDirections.push_back(Direction(angle1[i], angle2[i], dirType));
	LOG_TRACE_VAR_STR(" [" << i << "] = " << itsBeamDirections[i]);
      }
    }


    void WH_DelayCompensation::getPhaseCentres()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      // First, let's find out how many stations we have.
      LOG_TRACE_VAR_STR(itsNrStations << " station position(s):");

      // Station positions must be given in ITRF; there is currently no
      // support in the AMC package to convert between WGS84 and ITRF.
      Position::Types posType(Position::INVALID);
      string str = toUpper(itsCS1PS->getString("OLAP.DelayComp.positionType"));
      if (str == "ITRF") posType = Position::ITRF;
      else ASSERTSTR(false, "OLAP.DelayComp.positionType must be ITRF");

      // Get the antenna positions from the parameter set. The antenna
      // positions are stored as one large vector of doubles.
      vector<double> pos;
      pos = itsCS1PS->phaseCenters();
      ASSERTSTR(pos.size() == 3 * itsNrStations,
                pos.size() << " == " << 3 * itsNrStations);

      // Reserve space in \a itsPhaseCentres to avoid reallocations.
      itsPhaseCentres.reserve(itsNrStations);

      // Split the \a pos vector into separate Position objects.
      for (uint i = 0; i < itsNrStations; ++i) {
        itsPhaseCentres.
          push_back(Position(pos[3*i], pos[3*i+1], pos[3*i+2], posType));
	LOG_TRACE_VAR_STR(" [" << i << "] = " << itsPhaseCentres[i]);
      }
    }


    void WH_DelayCompensation::getObservationEpoch()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      // The observation epoch should be given by a start time and a stop
      // time. Both times must be specified in Modified Julian Date (MJD).
      Epoch startTime;
      startTime.utc(itsCS1PS->startTime());
      Epoch stopTime;
      stopTime.utc(itsCS1PS->stopTime());

      // The time step is equal to one integration period, which, in turn, is
      // defined as the number of samples used per integration period divided
      // by the sample rate in seconds. Divide by 86400 to get it in days.
      Epoch stepTime  = itsCS1PS->BGLintegrationTime() / 86400;
 
      LOG_TRACE_VAR("Observation:");
      LOG_TRACE_VAR_STR(" startTime = " << startTime);
      LOG_TRACE_VAR_STR(" stopTime  = " << stopTime);
      LOG_TRACE_VAR_STR(" stepTime  = " << stepTime);
      
      ASSERTSTR(startTime <= stopTime, startTime << " <= " << stopTime);
 
      // Reserve space in \a itsObservationEpoch to avoid reallocations
      uint sz = uint(1 + ceil((stopTime - startTime).mjd() / stepTime.mjd()));
      
      // Variable sz must be equal to noRuns. sz and noRuns must be a multiple 
      // of 16, plus 16. 
      sz = ((sz+15)&~15) + 16;
      
      LOG_TRACE_VAR_STR(sz << " epoch(s)");
      itsObservationEpoch.reserve(sz);
      
      // Fill a vector of Epoch, with a time interval \a stepTime, starting
      // at \a startTime and ending at \a stopTime.
      Epoch time = startTime;
      for (uint i = 0; i < sz; i++, time += stepTime) {
        itsObservationEpoch.push_back(Epoch(time));
	LOG_TRACE_LOOP_STR(" [" << i << "] = " << itsObservationEpoch[i]);
      }
    }


    void WH_DelayCompensation::setPositionDiffs()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      // Reserve space in \a itsPhasePositionDiffs to avoid reallocations.
      itsPhasePositionDiffs.reserve(itsPhaseCentres.size());

      // Calculate the station to reference station position difference vector
      // all stations.
      LOG_TRACE_VAR("Position difference vectors:");
      const Position& p0 = itsPhaseCentres[0];
      for (uint i = 0; i < itsPhaseCentres.size(); ++i) {
        itsPhasePositionDiffs.push_back(itsPhaseCentres[i] - p0);
        LOG_TRACE_VAR_STR(" [" << i << "] = " << itsPhasePositionDiffs[i]);
      }
    }


    Converter* WH_DelayCompensation::createConverter()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      if (itsConverterConfig.type == IMPL) return new ConverterImpl();
      if (itsConverterConfig.type == CLIENT) {
        return new ConverterClient(itsConverterConfig.server,
                                   itsConverterConfig.port);
      }
      return 0;
    }


    void WH_DelayCompensation::calculateDelays()
    {
      // Avoid invalid memory reads.
      ASSERTSTR(itsLoopCount < itsObservationEpoch.size(),
		itsLoopCount << " < " << itsObservationEpoch.size());
      
      LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_FLOW,
			     itsObservationEpoch[itsLoopCount]);

      // Convert the source coordinates to ITRF, for all beams and all
      // stations for the epoch "after-end" of the current time interval.
      RequestData request (itsBeamDirections, itsPhaseCentres, 
                           itsObservationEpoch[itsLoopCount]);
      ResultData result;
      itsConverter->j2000ToItrf(result, request);

      // Since we've calculated the coordinates for only one epoch, the number
      // of directions in the result vector must be equal to the number of
      // delays per epoch.
      ASSERTSTR(result.direction.size() == itsNrDelays,
		result.direction.size() << " == " << itsNrDelays);
      
      LOG_TRACE_CALC("Beamlet directions:");
      for (uint i = 0; i < result.direction.size(); ++i) {
        LOG_TRACE_CALC_STR(" [" << i << "] = " << result.direction[i]);
      }

      // Copy the current contents of itsDelaysAfterEnd to itsDelaysAtBegin.
      itsDelaysAtBegin = itsDelaysAfterEnd;
      
      // From the source coordinates in ITRF, calculate the geometrical
      // delays. Please remember that the vector result.direction stores the
      // directions per epoch, per position, per direction. I.e. the first
      // itsBeamDirections.size() elements contain the converted directions
      // for itsPhaseCentres[0], the second for itsPhaseCentres[1],
      // etc.
      LOG_TRACE_CALC("Beamlet geometrical delays:");
      for (uint i = 0; i < itsNrDelays; ++i) {
        uint j = i / itsBeamDirections.size();
        itsDelaysAfterEnd[i] = 
	  (result.direction[i] * itsPhasePositionDiffs[j]) / speedOfLight;
        LOG_TRACE_CALC_STR(" [" << i << "]: " << itsDelaysAfterEnd[i]);
      }
    }
  } // namespace CS1

} // namespace LOFAR
