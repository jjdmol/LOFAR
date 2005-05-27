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
#include "SharedResource.h"
#include "Timestamp.h"

namespace LOFAR {
  namespace CAL {

    class ACC : public SharedResource
    {
    public:
      ACC() {}
      ACC(int nsubbands, int nantennas, int npol);
      virtual ~ACC();

      /**
       * Get a subband slice of the ACC cube.
       * @param subband Subband for which to return the ACM.
       * @param timestamp The timestamp of the specified subband is returned.
       * If an invalid subband is specified Timestamp(0,0) is returned.
       * @return The ACM for the specified subband is returned. If an invalid
       * subband is specified an empty array is returned.
       */
      const blitz::Array<std::complex<double>, 4> getACM(int subband, RTC::Timestamp& timestamp) const;
    
      /**
       * Update an ACM.
       */
      void updateACM(int subband, RTC::Timestamp timestamp,
		     blitz::Array<std::complex<double>, 4>& newacm);

      /**
       * Get the size of the array.
       */
      int getSize() const { return m_acc.size(); }

      /**
       * Get a reference to the ACC array.
       * @return The 5-dimensional ACC array.
       */
      const blitz::Array<std::complex<double>, 5>& getACC() const { return m_acc; }
    
      /**
       * Initialize the ACC array.
       */
      void setACC(blitz::Array<std::complex<double>, 5>& acc);

    private:

      friend class ACCLoader;

      /**
       * ACC is a five dimensional array of complex numbers with dimensions
       * nsubbands x  nantennas x nantennas x npol x npol.
       */
      blitz::Array<std::complex<double>, 5>    m_acc;

      /**
       * m_time is a 1 dimensional array with a timestamp for each subband.
       */
      blitz::Array<RTC::Timestamp, 1> m_time;
    };
  
    /**
     * Factory class for ACC (Array Correlation Cube) instances.
     * This class manages ACC instances: the front and back ACC buffers.
     * The calibration algorithm works with the front ACC buffer while 
     * in the background the next ACC buffer is filled by the CAL::ACCService.
     */
    class ACCs
    {
    public:

      ACCs(int nsubbands, int nantennas, int npol);
      virtual ~ACCs();
	
      /**
       * @return a reference to the front ACC buffer. This buffer can be used
       * to pass to the calibration algorithm.
       */
      ACC& getFront() const;
	
      /**
       * @return a reference to the back ACC buffer. This buffer can be filled
       * for the next calibration iteration.
       */
      ACC& getBack() const;
	
      /**
       * Swap the front and back buffers after the calibration has completed and
       * the back buffer is filled with the most recent ACC for use in the next
       * calibration iteration.
       */
      void swap();
		
    private:
      int m_front;
      int m_back;
      ACC* m_buffer[2];
    };

    class ACCLoader
    {
    public:
      static const ACC* loadFromFile(std::string filename);
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* ACC_H_ */

