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
#include <Interface/Exceptions.h>
#include <Interface/PencilCoordinates.h>
#include <Interface/Mutex.h>

#include <pthread.h>
#include <memory>

// casacore is not thread safe
static LOFAR::RTCP::Mutex casacoreMutex;

namespace LOFAR 
{
  using namespace AMC;

  namespace RTCP 
  {
    //##----------------  Public methods  ----------------##//

    WH_DelayCompensation::WH_DelayCompensation(const Parset *ps,
                                               const string &stationName,
					       const TimeStamp &startTime) :
      stop	   (false),
      itsBuffer    (bufferSize,ps->nrBeams() * ps->nrPencilBeams()),
      head         (0),
      tail         (0),
      bufferFree   (bufferSize),
      bufferUsed   (0),

      itsNrCalcDelays    (ps->nrCalcDelays()),
      itsNrBeams         (ps->nrBeams()),
      itsNrPencilBeams   (ps->nrPencilBeams()),
      itsStartTime       (startTime),
      itsNrSamplesPerSec (ps->nrSubbandSamples()),
      itsSampleDuration  (ps->sampleDuration()),
      itsStationName     (stationName),
      itsDelayTimer      ("delay producer",true,true)
    {
      setBeamDirections(ps);
      setPositionDiffs(ps);
      itsThread = new Thread(this, &WH_DelayCompensation::mainLoop);
    }

    WH_DelayCompensation::~WH_DelayCompensation()
    {
      // trigger mainLoop and force it to stop
      stop = true;
      bufferFree.up( itsNrCalcDelays );
      delete itsThread;
    }

    void WH_DelayCompensation::mainLoop()
    {
      // We need bufferSize to be a multiple of batchSize to avoid wraparounds in
      // the middle of the batch calculations. This makes life a lot easier and there is no
      // need to support other cases.

      if( bufferSize % itsNrCalcDelays > 0 ) {
        THROW(IONProcException, "nrCalcDelays (" << itsNrCalcDelays << ") must divide bufferSize (" << bufferSize << ")" );
      }

      vector<AMC::Epoch>	        observationEpochs( itsNrCalcDelays );
      std::auto_ptr<AMC::Converter>     converter( new ConverterImpl() );

      // the current time, in samples
      int64 currentTime = itsStartTime;

      while (!stop) {
        bufferFree.down( itsNrCalcDelays );

	itsDelayTimer.start();

	// Calculate itsNrCalcDelays seconds worth of delays. Technically, we do not have
	// to calculate that many at the end of the run, but there is no need to
	// prevent the few excess delays from being calculated.

        // Derive the next list of timestamps
	for( unsigned i = 0; i < itsNrCalcDelays; i++ ) {
	  observationEpochs[i].utc(currentTime * itsSampleDuration);
	  currentTime += itsNrSamplesPerSec;
	}

        // Convert the source coordinates to ITRF, for all beams and all
        // stations for the epoch "after-end" of the current time interval.
        RequestData request (itsBeamDirections, itsPhaseCentres, 
                             observationEpochs);

        // From the source coordinates in ITRF, calculate the geometrical
        // delays. Please remember that the vector result.direction stores the
        // directions per epoch, per position, per direction. I.e. the first
        // itsNrBeams elements contain the converted directions
        // for itsPhaseCentres[0], the second for itsPhaseCentres[1],
        // etc.
        ResultData result;

        ASSERTSTR(request.direction.size() == itsNrBeams * itsNrPencilBeams,
	  	  request.direction.size() << " == " << itsNrBeams * itsNrPencilBeams );
        ASSERTSTR(request.position.size() == 1,
	  	  request.position.size() << " == 1");

        ASSERTSTR(request.epoch.size() == itsNrCalcDelays,
	  	  request.epoch.size() << " == " << itsNrCalcDelays);

        casacoreMutex.lock();
        // TODO: hangs if exception is thrown here -- should abort everything instead, since
        // we can't restore casacore.
        converter->j2000ToItrf(result, request); // expensive
        casacoreMutex.unlock();

        ASSERTSTR(result.direction.size() == itsNrCalcDelays * itsNrBeams * itsNrPencilBeams,
	  	  result.direction.size() << " == " << itsNrCalcDelays * itsNrBeams * itsNrPencilBeams );

	// append the results to the circular buffer
	unsigned resultIndex = 0;

        for( unsigned t = 0; t < itsNrCalcDelays; t++ ) {
	  for( unsigned b = 0; b < itsNrBeams * itsNrPencilBeams; b++ ) {
	    ASSERTSTR( tail < bufferSize, tail << " < " << bufferSize );

	    itsBuffer[tail][b] = result.direction[resultIndex++];
	  }

	  // increment the tail pointer. since itsNrCalcDelays % bufferSize == 0, wrap
	  // around can only occur between runs
	  ++tail;
	}

	// check for wrap around for the next run
	if( tail >= bufferSize ) {
	  tail = 0;
	}

	itsDelayTimer.stop();

	bufferUsed.up( itsNrCalcDelays );
      }
      
      LOG_DEBUG( "delay compensation thread stopped." );
    }

    void WH_DelayCompensation::getNextDelays( Matrix<Direction> &directions, Matrix<double> &delays )
    {
      ASSERTSTR(directions.num_elements() == itsNrBeams * itsNrPencilBeams,
                directions.num_elements() << " == " << itsNrBeams << "*" << itsNrPencilBeams);

      ASSERTSTR(delays.num_elements() == itsNrBeams * itsNrPencilBeams,
                delays.num_elements() << " == " << itsNrBeams << "*" << itsNrPencilBeams);

      bufferUsed.down();

      // copy the directions at itsBuffer[head] into the provided buffer,
      // and calculate the respective delays
      for( unsigned b = 0; b < itsNrBeams; b++ ) {
        for( unsigned p = 0; p < itsNrPencilBeams; p++ ) {
          const Direction &dir = itsBuffer[head][b*itsNrPencilBeams + p];

          directions[b][p] = dir;
	  delays[b][p] = dir * itsPhasePositionDiffs * (1.0 / speedOfLight);
	}  
      }

      // increment the head pointer
      if( ++head == bufferSize ) {
        head = 0;
      }

      bufferFree.up();
    }

    //##----------------  Private methods  ----------------##//
    
    void WH_DelayCompensation::setBeamDirections(const Parset *ps)
    {
      const PencilCoordinates& pencilBeams = ps->pencilBeams();

      // What coordinate system is used for these source directions?
      // Currently, we support J2000, ITRF, and AZEL.
      Direction::Types dirType(Direction::INVALID);

      // TODO: For now, we include pencil beams for all regular beams,
      // and use the pencil beam offsets as offsets in J2000.
      // To do the coordinates properly, the offsets should be applied
      // in today's coordinates (JMEAN/JTRUE?), not J2000.
      
      itsBeamDirections.resize(itsNrBeams*itsNrPencilBeams);
      
      // Get the source directions from the parameter set. 
      // Split the \a dir vector into separate Direction objects.
      for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
        const string str = toUpper(ps->getBeamDirectionType(beam));
        const vector<double> beamDir = ps->getBeamDirection(beam);
	
	if      (str == "J2000") dirType = Direction::J2000;
        else if (str == "ITRF")  dirType = Direction::ITRF;
        else if (str == "AZEL")  dirType = Direction::AZEL;
        else THROW(IONProcException, "Observation.BeamDirectionType must be one of "
                       "J2000, ITRF, or AZEL");

        for (unsigned pencil = 0; pencil < itsNrPencilBeams; pencil ++) {
	  // obtain pencil coordinate
	  const PencilCoord3D &pencilCoord = pencilBeams[pencil];

	  // apply angle modification (TODO: different calculates for J2000 (ra/dec) and AZEL and ITRF!)
	  const double angle1 = beamDir[0] + pencilCoord[0];
	  const double angle2 = beamDir[1] + pencilCoord[1];

	  // store beam
          itsBeamDirections[beam*itsNrPencilBeams + pencil] = Direction(angle1, angle2, dirType);
	}
      }
    }

    void WH_DelayCompensation::setPositionDiffs(const Parset *ps)
    {
       // Calculate the station to reference station position difference of apply station.
      
      // Station positions must be given in ITRF; there is currently no
      // support in the AMC package to convert between WGS84 and ITRF.
      Position::Types posType(Position::INVALID);
      string str = toUpper(ps->positionType());
      if (str == "ITRF")
        posType = Position::ITRF;
      else
        THROW(IONProcException, "OLAP.DelayComp.positionType must be ITRF");

      // Get the antenna positions from the parameter set. The antenna
      // positions are stored as one large vector of doubles.
      const Position pRef(Coord3D(ps->getRefPhaseCentres()), posType);
      const Position phaseCentres(Coord3D(ps->getPhaseCentresOf(itsStationName)), posType);
      itsPhaseCentres = phaseCentres;
      
      itsPhasePositionDiffs = itsPhaseCentres - pRef;
    }
  } // namespace RTCP

} // namespace LOFAR
