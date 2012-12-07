//#  Delays.cc: Workholder for the delay compensation.
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
//#  $Id: Delays.cc 17975 2011-05-10 09:52:51Z mol $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Delays.h>
#include <Common/LofarLogger.h>
#include <Common/PrettyUnits.h>
#include <Interface/Exceptions.h>
#include <Interface/BeamCoordinates.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Cancellation.h>

#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCDirection.h>
#include <casa/Exceptions/Error.h>

#include <pthread.h>
#include <memory>


namespace LOFAR {
namespace RTCP {

using namespace casa;

static LOFAR::Mutex casacoreMutex; // casacore is not thread safe

//##----------------  Public methods  ----------------##//

Delays::Delays(const Parset &parset, const string &stationName, const TimeStamp &startTime)
:
  itsParset(parset),
  stop(false),
  // we need an extra entry for the central beam
  itsBuffer(bufferSize, parset.nrBeams(), parset.nrTABs() + 1),
  head(0),
  tail(0),
  bufferFree(bufferSize),
  bufferUsed(0),
  itsNrCalcDelays(parset.nrCalcDelays()),
  itsNrBeams(parset.nrBeams()),
  itsNrTABs(parset.nrTABs()),
  itsDirectionType(MDirection::J2000),
  itsStartTime(startTime),
  itsNrSamplesPerSec(parset.nrSamplesPerSubband()),
  itsSampleDuration(parset.sampleDuration()),
  itsStationName(stationName),
  itsDelayTimer("delay producer", true, true),
  itsThread(this, &Delays::mainLoop, "[DelayCompensation] ")
{
}


Delays::~Delays()
{
  ScopedDelayCancellation dc; // Semaphores provide cancellation points

  // trigger mainLoop and force it to stop
  stop = true;
  bufferFree.up(itsNrCalcDelays);
}


// convert a time in samples to a (day,fraction) pair in UTC in a CasaCore format
MVEpoch Delays::toUTC(int64 timeInSamples)
{
  double utc_sec = (timeInSamples * itsSampleDuration) / MVEpoch::secInDay;
  double day	 = floor(utc_sec);
  double frac	 = utc_sec - day;

  // (40587 modify Julian day number = 00:00:00 January 1, 1970, GMT)
  return MVEpoch(day + 40587., frac);
}


void Delays::init()
{
  setBeamDirections(itsParset);
  setPositionDiff(itsParset);

  // We need bufferSize to be a multiple of batchSize to avoid wraparounds in
  // the middle of the batch calculations. This makes life a lot easier and there is no
  // need to support other cases.

  if (bufferSize % itsNrCalcDelays > 0)
    THROW(GPUProcException, "nrCalcDelays (" << itsNrCalcDelays << ") must divide bufferSize (" << bufferSize << ")");

  ScopedLock lock(casacoreMutex);
  ScopedDelayCancellation dc;

  // Set an initial epoch for the itsFrame
  itsFrame.set(MEpoch(toUTC(itsStartTime), MEpoch::UTC));

  // Set the position for the itsFrame.
  itsFrame.set(itsPhaseCentre);
  
  // Set-up the conversion engine, using reference direction ITRF.
  itsConverter = new MDirection::Convert(itsDirectionType, MDirection::Ref(MDirection::ITRF, itsFrame));
}


void Delays::mainLoop()
{
  LOG_DEBUG("Delay compensation thread running");

  init();

  // the current time, in samples
  int64 currentTime = itsStartTime;

  try {
    while (!stop) {
      bufferFree.down(itsNrCalcDelays);

      itsDelayTimer.start();

      // Calculate itsNrCalcDelays seconds worth of delays. Technically, we do not have
      // to calculate that many at the end of the run, but there is no need to
      // prevent the few excess delays from being calculated.

      {
	ScopedLock lock(casacoreMutex);
        ScopedDelayCancellation dc;

	// For each given moment in time ...
	for (uint i = 0; i < itsNrCalcDelays; i ++) {
	  // Set the instant in time in the itsFrame (40587 modify Julian day number = 00:00:00 January 1, 1970, GMT)
	  itsFrame.resetEpoch(toUTC(currentTime));

	  // Check whether we will store results in a valid place
	  ASSERTSTR(tail < bufferSize, tail << " < " << bufferSize);
	  
	  // For each given direction in the sky ...
	  for (uint b = 0; b < itsNrBeams; b ++) {
	    for (uint p = 0; p < itsNrTABs + 1; p ++) {

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
	  ++ tail;
	}
      }
      // check for wrap around for the next run
      if (tail >= bufferSize)
	tail = 0;

      itsDelayTimer.stop();

      bufferUsed.up(itsNrCalcDelays);
    }
  } catch (AipsError &ex) {
    THROW(GPUProcException, "AipsError: " << ex.what());
  }

  LOG_DEBUG("Delay compensation thread stopped");
}


void Delays::getNextDelays(Matrix<MVDirection> &directions, Matrix<double> &delays)
{
  ASSERTSTR(directions.num_elements() == itsNrBeams * (itsNrTABs + 1),
	    directions.num_elements() << " == " << itsNrBeams << "*" << (itsNrTABs + 1));

  ASSERTSTR(delays.num_elements() == itsNrBeams * (itsNrTABs + 1),
	    delays.num_elements() << " == " << itsNrBeams << "*" << (itsNrTABs + 1));

  bufferUsed.down();

  // copy the directions at itsBuffer[head] into the provided buffer,
  // and calculate the respective delays
  for (unsigned b = 0; b < itsNrBeams; b ++) {
    for (unsigned p = 0; p < itsNrTABs + 1; p ++) {
      const MVDirection &dir = itsBuffer[head][b][p];

      directions[b][p] = dir;
      delays[b][p] = dir * itsPhasePositionDiff * (1.0 / speedOfLight);
    }  
  }

  // increment the head pointer
  if (++ head == bufferSize)
    head = 0;

  bufferFree.up();
}


void Delays::setBeamDirections(const Parset &parset)
{
  const BeamCoordinates& pencilBeams = parset.pencilBeams();

  // TODO: For now, we include pencil beams for all regular beams,
  // and use the pencil beam offsets as offsets in J2000.
  // To do the coordinates properly, the offsets should be applied
  // in today's coordinates (JMEAN/JTRUE?), not J2000.
  
  itsBeamDirections.resize(itsNrBeams, itsNrTABs + 1);

  // We only support beams of the same direction type for now
  const string type0 = toUpper(parset.getBeamDirectionType(0));

  for (unsigned beam = 1; beam < itsNrBeams; beam ++) {
    const string typeN = toUpper(parset.getBeamDirectionType(beam));

    if (type0 != typeN)
      THROW(GPUProcException, "All beams must use the same coordinate system (beam 0 uses " << type0 << " but beam " << beam << " uses " << typeN << ")");
  }

  if (!MDirection::getType(itsDirectionType, type0))
    THROW(GPUProcException, "Beam direction type unknown: " << type0);
  
  // Get the source directions from the parameter set. 
  // Split the \a dir vector into separate Direction objects.
  for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
    const vector<double> beamDir = parset.getBeamDirection(beam);

    // add central beam coordinates for non-beamforming pipelines
    itsBeamDirections[beam][0] = MVDirection(beamDir[0], beamDir[1]);

    for (unsigned pencil = 0; pencil < itsNrTABs; pencil ++) {
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


void Delays::setPositionDiff(const Parset &parset)
{
  // Calculate the station to reference station position difference of apply station.
  
  // Station positions must be given in ITRF
  string str = toUpper(parset.positionType());

  if (str != "ITRF")
    THROW(GPUProcException, "OLAP.DelayComp.positionType must be ITRF");

  // Get the antenna positions from the parameter set. The antenna
  // positions are stored as one large vector of doubles.
  const MVPosition pRef(parset.getRefPhaseCentre());
  const MVPosition phaseCentre(parset.getPhaseCentreOf(itsStationName));

  itsPhaseCentre = MPosition(phaseCentre, MPosition::ITRF);
  itsPhasePositionDiff = phaseCentre - pRef;
}

} // namespace RTCP
} // namespace LOFAR
