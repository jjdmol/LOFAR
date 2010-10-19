//#  WH_DelayCompensation.h: Calculate delay compensation for all stations.
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

#ifndef LOFAR_IONPROC_WH_DELAYCOMPENSATION_H
#define LOFAR_IONPROC_WH_DELAYCOMPENSATION_H

// \file
// Calculate delay compensation for all stations.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Interface/Parset.h>
#include <AMCBase/Epoch.h>
#include <AMCBase/Position.h>
#include <Common/Timer.h>
#include <Common/Semaphore.h>
#include <AMCBase/ResultData.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Thread.h>

#include <boost/noncopyable.hpp>
#include <pthread.h>

namespace LOFAR 
{
  //# Forward declarations
  namespace AMC
  { 
    class Converter; 
    class Direction; 
  }

  namespace RTCP 
  {
    // \addtogroup DelayCompensation
    // @{

    // Speed of light in vacuum, in m/s.
    const double speedOfLight = 299792458;

    // Workholder for calculating the delay compensation that must be applied
    // per beam per station. We start by calculating the path length
    // difference for beam \f$\mathbf{b}_i\f$ between station \f$j\f$ at
    // position \f$\mathbf{p}_j\f$ and the reference station 0 at position
    // \f$\mathbf{p}_0\f$.
    // \f[
    //   d_{ij} - d_{i0} = \mathbf{b}_i \cdot \mathbf{p}_j 
    //                   - \mathbf{b}_i \cdot \mathbf{p}_0
    //                   = \mathbf{b}_i \cdot (\mathbf{p}_j - \mathbf{p}_0)
    // \f]
    // The choice of reference station is arbitrary, so we simply choose the
    // first station from the parameter set. From the equation above it is
    // clear that we can reduce the number of dot products if we precalculate
    // the position difference vectors \f$\mathbf{p}_j - \mathbf{p}_0$\f,
    // which we will store in \c itsPositionDiffs.
    //
    // The geometrical delay is easily obtained by dividing the path length
    // difference by the speed of light in vacuum. We don't need to know the
    // speed of light in the atmosphere, because the AZEL directions that
    // we've calculated are valid for vacuum (only!). This is the delay that
    // must be compensated for.
    //
    // The calculated delay compensation must be split into a coarse (whole
    // sample) delay and a fine (subsample) delay.  The coarse delay will be
    // applied in the input section as a true time delay, by shifting the
    // input samples. The fine delay will be applied in the correlator as a
    // phase shift in each frequency channel.
    class WH_DelayCompensation: boost::noncopyable
    {
    public:
      WH_DelayCompensation(const Parset *ps,
                           const string &stationName,
			   const TimeStamp &startTime);

      ~WH_DelayCompensation();

      // get the set of directions and delays for the beams, for the next CN integration time
      // Both matrices must have dimensions [itsNrBeams][itsNrPencilBeams]
      void getNextDelays( Matrix<AMC::Direction> &directions, Matrix<double> &delays );
      
    private:
      // do the delay compensation calculations in a separate thread to allow bulk
      // calculations and to avoid blocking other threads
      void                      mainLoop();

      // the circular buffer to hold the moving beam directions for every second of data
      Matrix<AMC::Direction>	itsBuffer;
      size_t			head, tail;

      // two semaphores are used: one to trigger the producer that free space is available,
      // another to trigger the consumer that data is available.
      Semaphore			bufferFree, bufferUsed;

      // the number of seconds to maintain in the buffer
      static const size_t	bufferSize = 128;

      // the number of delays to calculate in a single run
      const unsigned            itsNrCalcDelays;

      // Get the source directions from the parameter file and initialize \c
      // itsBeamDirections. Beam positions must be specified as
      // <tt>(longitude, latitude, direction-type)</tt>. The direction angles
      // are in radians; the direction type must be one of J2000, ITRF, or
      // AZEL.
      void setBeamDirections(const Parset *);

      // Set the station to reference station position differences for all
      // stations. The choice of reference station is arbitrary (so we choose
      // station 0). The position differences are stored in \c
      // itsPositionDiffs. In other words: we store \f$\mathbf{p}_j -
      // \mathbf{p}_0\f$, where \f$\mathbf{p}_0\f$ is the position of the
      // reference station and \f$\mathbf{p}_j\f$ is the position of station
      // \f$j\f$.
      void setPositionDiffs(const Parset *);

      // Beam info.
      const unsigned                itsNrBeams;
      const unsigned                itsNrPencilBeams;
      vector<AMC::Direction>        itsBeamDirections;

      // Sample timings.
      const TimeStamp               itsStartTime;
      const unsigned                itsNrSamplesPerSec;
      const double                  itsSampleDuration;

      // Station Name.
      const string                  itsStationName;
      
      // Station phase centres. 
      AMC::Position                 itsPhaseCentres;
      
      // Station to reference station position difference vectors.
      AMC::Position                 itsPhasePositionDiffs;
      
      NSTimer                       itsDelayTimer;

      Thread                        *thread;
    };

    // @}

  } // namespace RTCP

} // namespace LOFAR

#endif
