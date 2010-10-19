//#  -*- mode: c++ -*-
//#  ACC.h: definition of the Auto Correlation Cube class
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

#ifndef ACC_H_
#define ACC_H_

#include <blitz/array.h>
#include <complex>
#include "Timestamp.h"

namespace CAL
{
  class ACC
  {
  public:
    ACC(blitz::Array<std::complex<double>, 5>& acc);
    virtual ~ACC();

    /**
     * Get a subband slice of the ACC cube.
     * @param subband Subband for which to return the ACM.
     * @param timestamp The timestamp of the specified subband is returned.
     * If an invalid subband is specified Timestamp(0,0) is returned.
     * @return The ACM for the specified subband is returned. If an invalid
     * subband is specified an empty array is returned.
     */
    const blitz::Array<std::complex<double>, 4> getACM(int subband, LOFAR::RSP_Protocol::Timestamp& timestamp) const;

    /**
     * Get the size of the array.
     */
    int getSize() const { return m_acc.size(); }

    /**
     * Get a reference to the ACC array.
     * @return The 5-dimensional ACC array.
     */
    const blitz::Array<std::complex<double>, 5>& getACC() const { return m_acc; }

  private:

    friend class ACCLoader;

    /**
     * ACC is a five dimensional array of complex numbers with dimensions
     * nantennas x nantennas x nsubbands x npol x npol.
     */
    blitz::Array<std::complex<double>, 5>    m_acc;

    /**
     * m_time is a 1 dimensional array with a timestamp for each subband.
     */
    blitz::Array<LOFAR::RSP_Protocol::Timestamp, 1> m_time;
  };


  class ACCLoader
  {
  public:
    static const ACC* loadFromFile(std::string filename);
  };

};

#endif /* ACC_H_ */

