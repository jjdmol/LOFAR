//#  -*- mode: c++ -*-
//#  AntennaArray.h: class definition for the AntennaArray class
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

#ifndef ANTENNAARRAY_H_
#define ANTENNAARRAY_H_

#include <string>
#include <blitz/array.h>

namespace CAL
{
  class AntennaArray
  {
  public:
    
    /**
     * Create a new antenna array.
     *
     * @param name   The name of this antenna array.
     * @param pos    The x,y,z positions of the antennas (dimensions: numAntennas x 2 (numPol) x 3 (x,y,z)
     *
     */
    AntennaArray(std::string                    name,
		 const blitz::Array<double, 3>& pos,
		 const blitz::Array<int, 2>*    rcuindex = 0);
    virtual ~AntennaArray();

    /**
     * Get the name of the array.
     * @return The name of the array or subarray (e.g. LBA_ARRAY, HBA_ARRAY, SINGLEPOL_LBA_ARRAY, etc).
     */
    std::string getName() const { return m_name; }

    /**
     * Get the positions of the antennas.
     * @return The array with positions of the antennas.
     */
    const blitz::Array<double, 3>& getAntennaPos() const { return m_pos; }

    /**
     * @return The number of dual polarized antennas (one antenna = two dipoles)
     */
    int getNumAntennas() const { return m_pos.extent(blitz::firstDim); }

  private:
    std::string             m_name;     // name of this antenna array

  protected:
    blitz::Array<double, 3> m_pos;      // three dimensions, Nantennas, Npol, Ncoordinates (x,y,z)
    blitz::Array<int, 2>    m_rcuindex; // the index of the rcu to which a dipole is connected
  };
};

#endif /* ANTENNAARRAY_H_ */

