//#  DH_Converter.h: one line description
//#
//#  Copyright (C) 2002-2004
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

#ifndef LOFAR_AMCBASE_AMCCLIENT_DH_CONVERTER_H
#define LOFAR_AMCBASE_AMCCLIENT_DH_CONVERTER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Transport/DataHolder.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace AMC
  {
    //# Forward declarations
    class SkyCoord;
    class EarthCoord;
    class TimeCoord;
    class ConverterCommand;

    // This class contains the data that will be transferred back and forth
    // between the converter client and converter server. As both client and
    // server use vectors of SkyCoord, EarthCoord, and TimeCoord, it is easier
    // to use one DataHolder class.
    class DH_Converter : public DataHolder
    {
    public:
      DH_Converter();

      // Destructor.
      virtual ~DH_Converter();

      // Make a deep copy.
      // \note Must be redefined, because it's defined pure virtual in the
      // base class; however we won't implement it, because we won't use it.
      virtual DH_Converter* clone() const;

      // Send a conversion command to the server. The input data are stored in
      // three vectors.
      void send(const ConverterCommand&,
                const vector<SkyCoord>&,
                const vector<EarthCoord>&,
                const vector<TimeCoord>&);
      
      // Receive the converted data from the server.
      void receive(vector<SkyCoord>&);
        
    private:

      // Version number for this class
      const static uint32 theirVersionNr;
     
//       // @name Definition of pointers within the DH_Conversion data blob.
//       // @{

//       // Version number of this class.
//       uint32*     theirVersionNr;

//       // Type of conversion that must be done by the server.
//       uint32*     itsConversionType;

//       // Number of elements in the vector of sky coordinates.
//       uint32*     itsNrOfSkyCoords;

//       // Vector of sky coordinates.
//       SkyCoord*   itsSkyCoords;

//       // Number of elements in the vector of earth coordinates.
//       uint32*     itsNrOfEarthCoords;

//       // Vector of earth coordinates.
//       EarthCoord* itsEarthCoords;

//       // Number of elements in the vector of time coordinates.
//       uint32*     itsNrOfTimeCoords;

//       // Vector of time coordinates.
//       TimeCoord*  itsTimeCoords;

//       // @}
    };

  } // namespace AMC

} // namespace LOFAR

#endif
