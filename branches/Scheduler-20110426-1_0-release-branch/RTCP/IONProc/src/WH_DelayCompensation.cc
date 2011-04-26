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
#include <Common/LofarLogger.h>
#include <Common/PrettyUnits.h>
#include <Interface/Exceptions.h>
#include <Interface/BeamCoordinates.h>
#include <Thread/Mutex.h>

#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCDirection.h>
#include <casa/Exceptions/Error.h>

#include <pthread.h>
#include <memory>

// casacore is not thread safe
static LOFAR::Mutex casacoreMutex;

namespace LOFAR 
{
  using namespace casa;

  namespace RTCP 
  {
    //##----------------  Public methods  ----------------##//

    WH_DelayCompensation::WH_DelayCompensation(const Parset *ps,
                                               const string &stationName,
					       const TimeStamp &startTime) :
      stop	   (false),
      // we need an extra entry for the central beam
      itsBuffer    (bufferSize, ps->nrBeams(), ps->nrPencilBeams()+1),
      head         (0),
      tail         (0),
      bufferFree   (bufferSize),
      bufferUsed   (0),

      itsNrCalcDelays    (ps->nrCalcDelays()),
      itsNrBeams         (ps->nrBeams()),
      itsNrPencilBeams   (ps->nrPencilBeams()),
      itsDirectionType   (MDirection::J2000),
      itsStartTime       (startTime),
      itsNrSamplesPerSec (ps->nrSubbandSamples()),
      itsSampleDuration  (ps->sampleDuration()),
      itsStationName     (stationName),
      itsConverter       (0),
      itsDelayTimer      ("delay producer",true,true)
    {
      setBeamDirections(ps);
      setPositionDiff(ps);

      // We need bufferSize to be a multiple of batchSize to avoid wraparounds in
      // the middle of the batch calculations. This makes life a lot easier and there is no
      // need to support other cases.

      if( bufferSize % itsNrCalcDelays > 0 ) {
        THROW(IONProcException, "nrCalcDelays (" << itsNrCalcDelays << ") must divide bufferSize (" << bufferSize << ")" );
      }

      try {
        ScopedLock lock(casacoreMutex);

        // Set an initial epoch for the itsFrame
        itsFrame.set(MEpoch(toUTC( itsStartTime ),MEpoch::UTC));

        // Set the position for the itsFrame.
        itsFrame.set( itsPhaseCentre );
        
        // Set-up the conversion engine, using reference direction ITRF.
        itsConverter = new MDirection::Convert(itsDirectionType, MDirection::Ref (MDirection::ITRF, itsFrame));
      } catch (AipsError& e) {
        THROW (IONProcException, "AipsError: " << e.what());
      }

      itsThread = new Thread(this, &WH_DelayCompensation::mainLoop, "[DelayCompensation] " );
    }

    WH_DelayCompensation::~WH_DelayCompensation()
    {
      // trigger mainLoop and force it to stop
      stop = true;
      bufferFree.up( itsNrCalcDelays );
      delete itsThread;

      delete itsConverter;
    }

    // convert a time in samples to a (day,fraction) pair in UTC in a CasaCore format
    MVEpoch WH_DelayCompensation::toUTC( int64 timeInSamples )
    {
      double utc_sec = (timeInSamples * itsSampleDuration)/MVEpoch::secInDay;
      double day  = floor( utc_sec );
      double frac = utc_sec - day;

      // (40587 modify Julian day number = 00:00:00 January 1, 1970, GMT)
      return MVEpoch( day + 40587., frac );
    }

    void WH_DelayCompensation::mainLoop()
    {
      LOG_DEBUG( "Delay compensation thread running" );

      // the current time, in samples
      int64 currentTime = itsStartTime;

      while (!stop) {
        bufferFree.down( itsNrCalcDelays );

	itsDelayTimer.start();

	// Calculate itsNrCalcDelays seconds worth of delays. Technically, we do not have
	// to calculate that many at the end of the run, but there is no need to
	// prevent the few excess delays from being calculated.

        try {
          ScopedLock lock(casacoreMutex);

          // For each given moment in time ...
          for (uint i = 0; i < itsNrCalcDelays; i++) {
            // Set the instant in time in the itsFrame (40587 modify Julian day number = 00:00:00 January 1, 1970, GMT)
            itsFrame.resetEpoch(toUTC( currentTime ));

            // Check whether we will store results in a valid place
	    ASSERTSTR( tail < bufferSize, tail << " < " << bufferSize );
            
            // For each given direction in the sky ...
            for (uint b = 0; b < itsNrBeams; b++) {
              for (uint p = 0; p < itsNrPencilBeams+1; p++) {

                // Define the astronomical direction as a J2000 direction.
                MVDirection &sky = itsBeamDirections[b][p];

                // Convert this direction, using the conversion engine.
                MDirection dir = (*itsConverter)(sky);

                // Add to the return vector
                itsBuffer[tail][b][p] = dir.getValue();
              }
            }  

            // Advance time for the next calculation
	    currentTime += itsNrSamplesPerSec;

            // Advance to the next result set.
            // since bufferSize % itsNrCalcDelays == 0, wrap
            // around can only occur between runs
            ++tail;
          }
        } catch (AipsError& e) {
          THROW (IONProcException, "AipsError: " << e.what());
        }

	// check for wrap around for the next run
	if( tail >= bufferSize ) {
	  tail = 0;
	}

	itsDelayTimer.stop();

	bufferUsed.up( itsNrCalcDelays );
      }

      LOG_DEBUG( "Delay compensation thread stopped" );
    }

    void WH_DelayCompensation::getNextDelays( Matrix<MVDirection> &directions, Matrix<double> &delays )
    {
      ASSERTSTR(directions.num_elements() == itsNrBeams * (itsNrPencilBeams+1),
                directions.num_elements() << " == " << itsNrBeams << "*" << (itsNrPencilBeams+1));

      ASSERTSTR(delays.num_elements() == itsNrBeams * (itsNrPencilBeams+1),
                delays.num_elements() << " == " << itsNrBeams << "*" << (itsNrPencilBeams+1));

      bufferUsed.down();

      // copy the directions at itsBuffer[head] into the provided buffer,
      // and calculate the respective delays
      for( unsigned b = 0; b < itsNrBeams; b++ ) {
        for( unsigned p = 0; p < itsNrPencilBeams+1; p++ ) {
          const MVDirection &dir = itsBuffer[head][b][p];

          directions[b][p] = dir;
	  delays[b][p] = dir * itsPhasePositionDiff * (1.0 / speedOfLight);
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
      const BeamCoordinates& pencilBeams = ps->pencilBeams();

      // TODO: For now, we include pencil beams for all regular beams,
      // and use the pencil beam offsets as offsets in J2000.
      // To do the coordinates properly, the offsets should be applied
      // in today's coordinates (JMEAN/JTRUE?), not J2000.
      
      itsBeamDirections.resize(itsNrBeams,itsNrPencilBeams+1);

      // We only support beams of the same direction type for now
      const string type0 = toUpper(ps->getBeamDirectionType(0));
      for (unsigned beam = 1; beam < itsNrBeams; beam ++) {
        const string typeN = toUpper(ps->getBeamDirectionType(beam));

        if (type0 != typeN)
          THROW( IONProcException, "All beams must use the same coordinate system (beam 0 uses " << type0 << " but beam " << beam << " uses " << typeN << ")" );
      }

      if (!MDirection::getType( itsDirectionType, type0 ))
        THROW(IONProcException, "Beam direction type unknown: " << type0 );
      
      // Get the source directions from the parameter set. 
      // Split the \a dir vector into separate Direction objects.
      for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
        const vector<double> beamDir = ps->getBeamDirection(beam);

        // add central beam coordinates for non-beamforming pipelines
        itsBeamDirections[beam][0] = MVDirection(beamDir[0], beamDir[1]);

        for (unsigned pencil = 0; pencil < itsNrPencilBeams; pencil ++) {
	  // obtain pencil coordinate
	  const BeamCoord3D &pencilCoord = pencilBeams[pencil];

	  // apply angle modification
	  const double angle1 = beamDir[0] + pencilCoord[0];
	  const double angle2 = beamDir[1] + pencilCoord[1];

	  // store beam
          itsBeamDirections[beam][pencil + 1] = MVDirection(angle1, angle2);
	}
      }
    }

    void WH_DelayCompensation::setPositionDiff(const Parset *ps)
    {
      // Calculate the station to reference station position difference of apply station.
      
      // Station positions must be given in ITRF
      string str = toUpper(ps->positionType());
      if (str != "ITRF")
        THROW(IONProcException, "OLAP.DelayComp.positionType must be ITRF");

      // Get the antenna positions from the parameter set. The antenna
      // positions are stored as one large vector of doubles.
      const MVPosition pRef(ps->getRefPhaseCentre());
      const MVPosition phaseCentre(ps->getPhaseCentreOf(itsStationName));

      itsPhaseCentre = MPosition(phaseCentre, MPosition::ITRF);
      itsPhasePositionDiff = phaseCentre - pRef;
    }
  } // namespace RTCP

} // namespace LOFAR
