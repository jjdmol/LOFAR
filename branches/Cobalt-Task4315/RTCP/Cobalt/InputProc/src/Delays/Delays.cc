//# Delays.cc: Workholder for the delay compensation.
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "Delays.h"

#include <Common/LofarLogger.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Cancellation.h>
#include <CoInterface/Exceptions.h>

#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCDirection.h>
#include <casa/Exceptions/Error.h>


namespace LOFAR
{
  namespace Cobalt
  {

    //##----------------  Public methods  ----------------##//

    Delays::Delays(const Parset &parset, const std::string &stationName, const TimeStamp &startTime, size_t blockSize)
      :
      parset(parset),
      itsStationName(stationName),
      itsStartTime(startTime),
      blockSize(blockSize),

      stop(false),
      // we need an extra entry for the central beam
      itsBuffer(bufferSize),
      head(0),
      tail(0),
      bufferFree(bufferSize),
      bufferUsed(0),
      itsDelayTimer("delay producer", true, true)
    {
    }


    void Delays::start()
    {
      itsThread = new Thread(this, &Delays::mainLoop, "[DelayCompensation] ");
    }


    Delays::~Delays()
    {
      ScopedDelayCancellation dc; // Semaphores provide cancellation points

      // trigger mainLoop and force it to stop
      stop = true;
      bufferFree.up(nrCalcDelays);
    }


    void Delays::setAllDelaysSize( AllDelays &result ) const {
        result.resize(parset.settings.SAPs.size());

        for (size_t sap = 0; sap < parset.settings.SAPs.size(); ++sap) {
          if (parset.settings.beamFormer.enabled) {
            const struct ObservationSettings::BeamFormer::SAP &bfSap = parset.settings.beamFormer.SAPs[sap];

            result[sap].TABs.resize(bfSap.TABs.size());
          }
        }
    }


#ifdef HAVE_CASACORE
    using namespace casa;

    static LOFAR::Mutex casacoreMutex; // casacore is not thread safe

    // convert a time in samples to a (day,fraction) pair in UTC in a CasaCore format
    MVEpoch Delays::toUTC(const TimeStamp &timestamp) const
    {
      double utc_sec = timestamp.getSeconds() / MVEpoch::secInDay;
      double day = floor(utc_sec);
      double frac = utc_sec - day;

      // (40587 modify Julian day number = 00:00:00 January 1, 1970, GMT)
      return MVEpoch(day + 40587., frac);
    }


    void Delays::init()
    {
      ScopedLock lock(casacoreMutex);
      ScopedDelayCancellation dc;

      // We need bufferSize to be a multiple of batchSize to avoid wraparounds in
      // the middle of the batch calculations. This makes life a lot easier and there is no
      // need to support other cases.

      ASSERT(bufferSize % nrCalcDelays == 0);

      // Set an initial epoch for the itsFrame
      itsFrame.set(MEpoch(toUTC(itsStartTime), MEpoch::UTC));

      // Set the position for the itsFrame.
      const MVPosition phaseCentre(parset.getPhaseCentreOf(itsStationName));
      itsFrame.set(MPosition(phaseCentre, MPosition::ITRF));

      // Cache the difference with CS002LBA
      const MVPosition pRef(parset.getRefPhaseCentre());
      itsPhasePositionDiff = phaseCentre - pRef;

      // Set-up the direction cache and conversion engines, using reference direction ITRF.
      itsDirectionTypes.resize(parset.settings.SAPs.size());

      for (size_t sap = 0; sap < parset.settings.SAPs.size(); sap++) {
        const std::string typeName = toUpper(parset.settings.SAPs[sap].direction.type);
        MDirection::Types &casaType = itsDirectionTypes[sap];

        if (!MDirection::getType(casaType, typeName))
          THROW(Exception, "Beam direction type unknown: " << typeName);

        if (itsConverters.find(casaType) == itsConverters.end())
          itsConverters[casaType] = MDirection::Convert(casaType, MDirection::Ref(MDirection::ITRF, itsFrame));
      }
    }


    struct Delays::Delay Delays::convert( casa::MDirection::Convert &converter, const casa::MVDirection &direction ) const {
      struct Delay d;

      MVDirection casaDir = converter(direction).getValue();

      // Compute direction and convert it 
      casa::Vector<double> dir = casaDir.getValue();
      std::copy(dir.begin(), dir.end(), d.direction);

      d.delay = casaDir * itsPhasePositionDiff * (1.0 / speedOfLight);

      return d;
    }


    void Delays::calcDelays( const TimeStamp &timestamp, AllDelays &result ) {
      try {
        ScopedLock lock(casacoreMutex);
        ScopedDelayCancellation dc;

        // Set the instant in time in the itsFrame
        itsFrame.resetEpoch(toUTC(timestamp));

        // Convert directions for all beams
        for (size_t sap = 0; sap < parset.settings.SAPs.size(); ++sap) {
          const struct ObservationSettings::SAP &sapInfo = parset.settings.SAPs[sap];

          // Fetch the relevant convert engine
          MDirection::Convert &converter = itsConverters[itsDirectionTypes[sap]];

          // Convert the SAP directions using the convert engine
          result[sap].SAP = convert(converter, MVDirection(sapInfo.direction.angle1, sapInfo.direction.angle2));

          if (parset.settings.beamFormer.enabled) {
            // Convert the TAB directions using the convert engine
            const struct ObservationSettings::BeamFormer::SAP &bfSap = parset.settings.beamFormer.SAPs[sap];
            for (size_t tab = 0; tab < bfSap.TABs.size(); tab++) {
              const MVDirection dir(sapInfo.direction.angle1 + bfSap.TABs[tab].directionDelta.angle1,
                                    sapInfo.direction.angle2 + bfSap.TABs[tab].directionDelta.angle2);

              result[sap].TABs[tab] = convert(converter, dir);
            }
          }
        }
      } catch (AipsError &ex) {
        THROW(Exception, "AipsError: " << ex.what());
      }
    }
#else
    void Delays::init() {
    }

    void Delays::calcDelays( const TimeStamp &timestamp, AllDelays &result ) {
      (void)timestamp;
      (void)result;
    }
#endif


    void Delays::mainLoop()
    {
      LOG_DEBUG("Delay compensation thread running");

      init();

      // the current time, in samples
      TimeStamp currentTime = itsStartTime;

      try {
        while (!stop) {
          bufferFree.down(nrCalcDelays);

          itsDelayTimer.start();

          // Calculate nrCalcDelays seconds worth of delays. Technically, we do not have
          // to calculate that many at the end of the run, but there is no need to
          // prevent the few excess delays from being calculated.

          {
            for (size_t i = 0; i < nrCalcDelays; i++) {
              // Check whether we will store results in a valid place
              ASSERTSTR(tail < bufferSize, tail << " < " << bufferSize);

              // Calculate the delays and store them in itsBuffer[tail]
              setAllDelaysSize(itsBuffer[tail]);
              calcDelays(currentTime, itsBuffer[tail]);

              // Advance time for the next calculation
              currentTime += blockSize;

              // Advance to the next result set.
              // since bufferSize % nrCalcDelays == 0, wrap
              // around can only occur between runs
              ++tail;
            }
          }

          // check for wrap around for the next run
          if (tail >= bufferSize)
            tail = 0;

          itsDelayTimer.stop();

          bufferUsed.up(nrCalcDelays);
        }
      } catch (Exception &ex) {
        // trigger getNextDelays and force it to stop
        stop = true;
        bufferUsed.up(1);

        throw;
      }

      LOG_DEBUG("Delay compensation thread stopped");
    }


    void Delays::getNextDelays( AllDelays &result )
    {
      ASSERT(itsThread);

      bufferUsed.down();

      if (stop)
        THROW(Exception, "Cannot obtain delays -- delay thread stopped running");

      // copy the directions at itsBuffer[head]
      result = itsBuffer[head];

      // increment the head pointer
      if (++head == bufferSize)
        head = 0;

      bufferFree.up();
    }
  } // namespace Cobalt
} // namespace LOFAR

