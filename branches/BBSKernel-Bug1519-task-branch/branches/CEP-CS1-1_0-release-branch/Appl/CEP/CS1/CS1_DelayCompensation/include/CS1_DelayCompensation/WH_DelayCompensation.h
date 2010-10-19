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

#ifndef LOFAR_CS1_DELAYCOMPENSATION_WH_DELAYCOMPENSATION_H
#define LOFAR_CS1_DELAYCOMPENSATION_WH_DELAYCOMPENSATION_H

// \file
// Calculate delay compensation for all stations.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <CS1_Interface/CS1_Parset.h>

namespace LOFAR 
{
  //# Forward declarations
  namespace AMC
  { 
    class Converter; 
    class Direction; 
    class Position; 
    class Epoch; 
  }

  namespace CS1 
  {
    // \addtogroup CS1_DelayCompensation
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
    class WH_DelayCompensation : public WorkHolder
    {
    public:
      WH_DelayCompensation(const string& name,
                                 CS1_Parset *ps);

      virtual ~WH_DelayCompensation();

      virtual void preprocess();

      virtual void process();

      virtual void postprocess();

    private:
      // Copying is not allowed
      WH_DelayCompensation (const WH_DelayCompensation& that);
      WH_DelayCompensation& operator= (const WH_DelayCompensation& that);

      virtual WH_DelayCompensation* make(const string& name);

      // AMC converter type
      enum ConverterTypes {
        UNDEFINED,   ///< Default type
        IMPL,        ///< Uses ConverterImpl
        CLIENT       ///< Uses ConverterClient
      };

      // Struct holding the relevant information for creating an AMC Converter.
      struct ConverterConfig
      {
        ConverterConfig() : type(UNDEFINED), server(""), port(0) {}
        ConverterTypes type;
        string         server;
        uint16         port;
      };

      // Get the configuration data for the Converter from the parameter file
      // and initialize \c itsConverterConfig.
      void getConverterConfig();

      // Get the source directions from the parameter file and initialize \c
      // itsBeamDirections. Beam positions must be specified as
      // <tt>(longitude, latitude, direction-type)</tt>. The direction angles
      // are in radians; the direction type must be one of J2000, ITRF, or
      // AZEL.
      void getBeamDirections();

      // Get the station phase centres from the parameter file and
      // initialize \c itsStationPositions. Station phase centres must
      // be specified as <tt>(longitude, latitude, height,
      // position-type)</tt>. The position angles are in radians;
      // height is in meters; the position type must be ITRF.
      void getPhaseCentres();

      // Get the observation epoch from the parameter file and initialize \c
      // itsObservationEpoch. The epoch is a series of times, with an
      // approximately one-second interval, starting at the observation start
      // time, and ending at the observation end time. Start and end time must
      // be specified as Modified Julian Date (MJD).
      void getObservationEpoch();

      // Set the station to reference station position differences for all
      // stations. The choice of reference station is arbitrary (so we choose
      // station 0). The position differences are stored in \c
      // itsPositionDiffs. In other words: we store \f$\mathbf{p}_j -
      // \mathbf{p}_0\f$, where \f$\mathbf{p}_0\f$ is the position of the
      // reference station and \f$\mathbf{p}_j\f$ is the position of station
      // \f$j\f$.
      void setPositionDiffs();

      // Create an instance of an AMC converter, using the data in \c
      // itsConverterConfig.
      AMC::Converter* createConverter();

      // Perform the actual coordinate conversion for the epoch after the end
      // of the current time interval and store the result in
      // itsResultAfterEnd.
      // \post itsResultAtBegin contains the data previously contained by
      // itsResultAfterEnd.
      void doConvert();

      // Calculate the delays for all stations for the epoch after the end of
      // the current time interval and store the results in itsDelaysAfterEnd.
      // \post itsDelaysAtBegin contain the data previously contained by
      // itsDelaysAtEnd.
      void calculateDelays();
      
      // The parameter set, containing all configurable variables.
      // \note Unfortunately we must keep a copy here, because (!@#$%!)
      // make() needs it. Hopefully, we can get rid of it, some day.
      CS1_Parset                   *itsCS1PS;

      // Number of beams.
      const uint                    itsNrBeams;
      
      // Number of stations.
      const uint                    itsNrStations;
      
      // Number of delays that will be calculated per epoch. This number is a
      // run-time constant, because it is equal to the number of beams per
      // station times the number of stations.
      const uint                    itsNrDelays;

      // The sample rate in a subband, in samples per second..
      const double                  itsSampleRate;

      // Counter used to count the number of times that process() was called.
      uint                          itsLoopCount;
  
      // Configuration info required to create our AMC Converter
      ConverterConfig               itsConverterConfig;

      // Pointer to the Converter we're using.
      AMC::Converter*               itsConverter;

      // Beam directions.
      vector<AMC::Direction>        itsBeamDirections;

      // Station phase centres. 
      vector<AMC::Position>         itsPhaseCentres;

      // Observation epoch.
      vector<AMC::Epoch>            itsObservationEpoch;

      // Station to reference station position difference vectors.
      vector<AMC::Position>         itsPhasePositionDiffs;

      // Delays for all stations for the epoch at the begin of the current
      // time interval.
      vector<double>                itsDelaysAtBegin;
      
      // Delays for all stations for the epoch after the end of the current
      // time interval.
      vector<double>                itsDelaysAfterEnd;
      
      // Allocate a tracer context
      ALLOC_TRACER_CONTEXT;
      
    };

    // @}

  } // namespace CS1

} // namespace LOFAR

#endif
