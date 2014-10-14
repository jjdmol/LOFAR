//#  -*- mode: c++ -*-
//#  AntennaArrayData.h: class definition for the AntennaArrayData class
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

#ifndef ANTENNAARRAYDATA_H_
#define ANTENNAARRAYDATA_H_

#include "AntennaArray.h"
#include <string>
#include <fstream>

namespace LOFAR {
  namespace CAL {

    /**
     * This class is responsible for managing the AntennaArray data.
     */
    class AntennaArrayData
    {
    public:
      AntennaArrayData();
      virtual ~AntennaArrayData();

      /**
       * Load an antenna array from file. A second call to this method
       * will override the data from the first.
       * @param filename The file to load data from.
       * @return true when an antenna array was succesfully loaded, false when
       * no more arrays could be loaded.
       */
      bool getNextFromFile(std::string filename);

      /**
       * Get the name.
       * @return the name of the antenna array
       */
      std::string getName() const { return m_name; }

      /**
       * Get the antenna positions.
       * @return the antenna positions.
       */
      const blitz::Array<double, 3>& getPositions() const { return m_positions; }

      /**
       * Get the geographical location of the array.
       * @return 1-d array with 3 values for longitude, latitude and height.
       */
      const blitz::Array<double, 1>& getGeoLoc() const { return m_geoloc; }

    private:
      std::string             m_filename;
      std::ifstream           m_file;

      std::string             m_name;
      blitz::Array<double, 1> m_geoloc;
      blitz::Array<double, 3> m_positions;
    };
  }; // namespace CAL
}; // namespace LOFAR

#endif /* ANTENNAARRAYDATA_H_ */

