//#  WH_DelayCompensation.cc: one line description
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
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/DH_Delay.h>
// #include <APS/ParameterSet.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
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

    WH_DelayCompensation::WH_DelayCompensation(const string& name,
                                               const ParameterSet& ps) :
      WorkHolder(0,                                      // inputs
                 ps.getUint32("Observation.NStations"),  // outputs
//                  ps.getUint32("Observation.NRSPBoards"), // outputs
                 name,                                   // name
                 "WH_DelayCompensation"),                // type
      itsParameterSet(ps),
      itsConverter(0),
      itsLoopCount(0)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Get relevant data from the parameter set and initialize private data.
      getConverterConfig(ps);
      getBeamDirections(ps);
      getStationPositions(ps);
      getObservationEpoch(ps);

      // Create outgoing dataholders for the delay information; one for each
      // stations (should be one for each RSP board).
      for (int i = 0; i < itsNoutputs; ++i) {
        string str = "DH_Delay_out_" + formatString("%02d", i);
        LOG_TRACE_LOOP_STR("Creating " << str);
        getDataManager().addOutDataHolder(i, new DH_Delay(str, itsNoutputs));
      }
    }


    WH_DelayCompensation::~WH_DelayCompensation()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    void WH_DelayCompensation::preprocess()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Set the process loop counter to zero.
      itsLoopCount = 0;
      
      // Create the AMC converter.
      ASSERT(!itsConverter);
      itsConverter = createConverter();
      ASSERT(itsConverter);
    }


    void WH_DelayCompensation::process()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Each integration period, we must send the new CoarseDelay,
      // FineDelayAtBegin, and FineDelayAfterEnd data for all stations.

      // For the time being we will call itsConverter->j2000ToAzel() every
      // cycle. This can be made more efficient by precalculating, say, for 30
      // integration periods, and only call itsConverter->j2000ToAzel() once
      // every 30 cycles.
      RequestData request (itsBeamDirections, itsStationPositions, 
                           itsObservationEpoch[itsLoopCount]);
      ResultData result;
      itsConverter->j2000ToAzel(result, request);

      cout.precision(9);
      cout << "skies = " << endl;
      for (uint i=0; i<itsBeamDirections.size(); ++i) {
        cout << "[" << i << "]: " << itsBeamDirections[i] << endl;
      }
      cout << "poss = " << endl;
      for (uint i=0; i<itsStationPositions.size(); ++i) {
        cout << "[" << i << "]: " << itsStationPositions[i] << endl;
      }
      cout << "times = " << endl;
      cout << "[" << itsLoopCount << "]: " 
           << itsObservationEpoch[itsLoopCount] << endl;
      
      cout << "Conversion result " << itsLoopCount << ": ";
      for (uint i=0; i<result.skyCoord.size(); ++i) {
        cout << endl << "[" << i << "]: " << result.skyCoord[i];
      }
            
      // Finally, we must update the process loop counter. 
      itsLoopCount++;
    }


    void WH_DelayCompensation::postprocess()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Delete the AMC converter
      ASSERT(itsConverter);
      delete itsConverter;
      itsConverter = 0;
    }
    

    WH_DelayCompensation* WH_DelayCompensation::make(const string& name)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return new WH_DelayCompensation(name, itsParameterSet);
    }


    void WH_DelayCompensation::getConverterConfig(const ParameterSet& ps)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      string type = toUpper(ps.getString("DelayComp.ConverterType"));
      if (type == "IMPL") itsConverterConfig.type = IMPL;
      else if (type == "CLIENT") {
        itsConverterConfig.type   = CLIENT;
        itsConverterConfig.server = ps.getString("Connections.AMC.ServerHost");
        itsConverterConfig.port   = ps.getUint16("Connections.AMC.Port");
      }
    }


    void WH_DelayCompensation::getBeamDirections(const ParameterSet& ps)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // First let's find out how many sources we have.
      uint32 nrBeams = ps.getUint32("Observation.NBeams");
      LOG_TRACE_VAR_STR(nrBeams << " beam direction(s)");

      // What coordinate system is used for these source directions?
      // Currently, we support J2000, ITRF, and AZEL.
      SkyCoord::Types dirType(SkyCoord::INVALID);
      string str = toUpper(ps.getString("Observation.DirectionType"));
      if      (str == "J2000") dirType = SkyCoord::J2000;
      else if (str == "ITRF")  dirType = SkyCoord::ITRF;
      else if (str == "AZEL")  dirType = SkyCoord::AZEL;
      else ASSERTSTR(false, "Observation.BeamDirectionType must be one of "
                     "J2000, ITRF, or AZEL");

      // Get the source directions from the parameter set. The source
      // directions are stored as one large vector of doubles. A direction
      // consists of two doubles.
      vector<double> dir;
      dir = ps.getDoubleVector("Observation.BeamDirections");
      ASSERTSTR(dir.size() == 2 * nrBeams, 
                dir.size() << " == " << 2 * nrBeams);
      
      // Reserve space in \a itsBeamDirections to avoid reallocations.
      itsBeamDirections.reserve(nrBeams);
      
      // Split the \a dir vector into separate SkyCoord objects.
      for (uint i = 0; i < nrBeams; ++i) {
        itsBeamDirections.push_back(SkyCoord(dir[2*i], dir[2*i+1], dirType));
      }
    }


    void WH_DelayCompensation::getStationPositions(const ParameterSet& ps)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // First, let's find out how many stations we have.
      uint32 nrStations = ps.getUint32("Observation.NStations");
      LOG_TRACE_VAR_STR(nrStations << " station position(s)");
      ASSERTSTR(nrStations <= NR_STATIONS, 
                nrStations << " <= " << NR_STATIONS);

      // Station positions must be given in ITRF; there is currently no
      // support in the AMC package to convert between WGS84 and ITRF.
      EarthCoord::Types posType(EarthCoord::INVALID);
      string str = toUpper(ps.getString("Observation.PositionType"));
      if (str == "ITRF") posType = EarthCoord::ITRF;
      else ASSERTSTR(false, "Observation.PositionType must be ITRF");

      // Get the antenna positions from the parameter set. The antenna
      // positions are stored as one large vector of doubles.
      vector<double> pos;
      pos = ps.getDoubleVector("Observation.StationPositions");
      for (uint i=0; i<pos.size(); ++i) cout << "pos["<<i<<"]="<<pos[i]<<endl;
      ASSERTSTR(pos.size() == 3 * nrStations,
                pos.size() << " == " << 3 * nrStations);

      // Reserve space in \a itsStationPositions to avoid reallocations.
      itsStationPositions.reserve(nrStations);

      // Split the \a pos vector into separate EarthCoord objects.
      for (uint i = 0; i < nrStations; ++i) {
        itsStationPositions.
          push_back(EarthCoord(pos[3*i], pos[3*i+1], pos[3*i+2], posType));
      }
    }


    void WH_DelayCompensation::getObservationEpoch(const ParameterSet& ps)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // The observation epoch should be given by a start time and a stop
      // time. Both times must be specified in Modified Julian Date (MJD).
      // The integration period must be specified in seconds.
      double startTime = ps.getDouble("Observation.StartTime");
      double stopTime  = ps.getDouble("Observation.StopTime");
      double delta     = ps.getDouble("Observation.IntegrationPeriod") / 86400;
      ASSERTSTR(startTime <= stopTime, startTime << " <= " << stopTime);

      // Reserve space in \a itsObservationEpoch to avoid reallocations
      uint sz = uint(ceil((stopTime - startTime) / delta));
      LOG_TRACE_VAR_STR(sz << " integration period(s)");
      itsObservationEpoch.reserve(sz);
      
      // Fill a vector of TimeCoord, with a time interval \a delta, starting
      // at \a startTime and ending at \a stopTime.
      // \todo The current implementation, using an increment in the for-loop,
      // introduces a rounding error of approx 1 ms per hour. Too much, IMHO.
      for (double time = startTime; time <= stopTime; time += delta) {
        itsObservationEpoch.push_back(TimeCoord(time));
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
    }


  } // namespace CS1

} // namespace LOFAR
