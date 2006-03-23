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
#include <APS/ParameterSet.h>

namespace LOFAR 
{
  //# Forward declarations
//   namespace ACC { 
//     namespace APS { 
//       class ParameterSet; 
//     } 
//   }
  namespace AMC { 
    class Converter; 
    class SkyCoord; 
    class EarthCoord; 
    class TimeCoord; 
  }


  namespace CS1 
  {
    // \addtogroup CS1_DelayCompensation
    // @{

    // Description of class.
    class WH_DelayCompensation : public WorkHolder
    {
    public:
      WH_DelayCompensation(const string& name,
                           const ACC::APS::ParameterSet& ps);

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
      void getConverterConfig(const ACC::APS::ParameterSet& ps);

      // Get the source directions from the parameter file and initialize \c
      // itsBeamDirections. Beam positions must be specified as
      // <tt>(longitude, latitude, direction-type)</tt>. The direction angles
      // are in radians; the direction type must be one of J2000, ITRF, or
      // AZEL.
      void getBeamDirections(const ACC::APS::ParameterSet& ps);

      // Get the station positions from the parameter file and initialize \c
      // itsStationPositions. Station positions must be specified as
      // <tt>(longitude, latitude, height, position-type)</tt>. The position
      // angles are in radians; height is in meters; the position type must be
      // ITRF.
      void getStationPositions(const ACC::APS::ParameterSet& ps);

      // Get the observation epoch from the parameter file and initialize \c
      // itsObservationEpoch. The epoch is a series of times, with an
      // approximately one-second interval, starting at the observation start
      // time, and ending at the observation end time. Start and end time must
      // be specified as Modified Julian Date (MJD).
      void getObservationEpoch(const ACC::APS::ParameterSet& ps);

      // Create an instance of an AMC converter, using the data in \c
      // itsConverterConfig.
      AMC::Converter* createConverter();


      // The parameter set, containing all configurable variables.
      // \note Unfortunately we must keep a copy here, because (!@#$%!)
      // make() needs it. Hopefully, we can get rid of it, some day.
      ACC::APS::ParameterSet  itsParameterSet;

      // Beam directions.
      vector<AMC::SkyCoord>   itsBeamDirections;

      // Station positions. 
      vector<AMC::EarthCoord> itsStationPositions;

      // Observation epoch.
      vector<AMC::TimeCoord>  itsObservationEpoch;

      // Configuration info required to create our AMC Converter
      ConverterConfig         itsConverterConfig;

      // Pointer to the Converter we're using.
      AMC::Converter*         itsConverter;

    };

    // @}

  } // namespace CS1

} // namespace LOFAR

#endif
