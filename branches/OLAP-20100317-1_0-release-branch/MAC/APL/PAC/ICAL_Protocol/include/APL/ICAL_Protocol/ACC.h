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
//#  $Id: ACC.h 6967 2005-10-31 16:28:09Z wierenga $

#ifndef ACC_H_
#define ACC_H_

#include <blitz/array.h>
#include <complex>
#include "SharedResource.h"
#include <APL/RTCCommon/Timestamp.h>

namespace LOFAR {
  namespace CAL {

    class ACC : public SharedResource
    {
    public:
      ACC() : m_valid(false), m_antenna_count(0) { }
      ACC(int nsubbands, int nantennas, int npol);
      virtual ~ACC();

      /**
       * Set the antenna selection mask to use in getACM. Dimensions (nantennas (all) x 2 (pol))
       * @param selection 2-dimensional blitz array with selection to apply
       * @note precondition: antenna_selection.extent(firstDim) == m_acc.extent(fourthDim)
       */
      void setSelection(const blitz::Array<bool, 2>& antenna_selection);

      /**
       * Get a subband and single polarized slice of the ACC cube.
       * @param subband Subband for which to return the ACM.
       * @param pol1 0 == x, 1 == y
       * @param pol2 0 == x, 1 == y
       * @param timestamp The timestamp of the specified subband is returned.
       * If an invalid subband is specified Timestamp(0,0) is returned.
       * @return The ACM for the specified subband and polarizations is returned.
       */
      const blitz::Array<std::complex<double>, 2> getACM(int subband, int pol1, int pol2, RTC::Timestamp& timestamp);
    
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
       * Get number of subbands.
       * @return the number of subbands in this ACC
       */
      int getNSubbands() const { return m_acc.extent(blitz::firstDim); }

      /**
       * Get number of antennas
       * @return the number of antennas in this ACC
       */
      int getNAntennas() const { return m_acc.extent(blitz::secondDim); }

      /**
       * Get number of polarizations
       * @return the number of polarizations in this ACC
       */
      int getNPol() const { return m_acc.extent(blitz::fourthDim); }

      /**
       * Get a reference to the ACC array.
       * @return The 5-dimensional ACC array.
       */
      const blitz::Array<std::complex<double>, 5>& getACC() const { return m_acc; }
    
      /**
       * Initialize the ACC array.
       * @note After this call the selection will select all antennas.
       */
      void setACC(blitz::Array<std::complex<double>, 5>& acc);

      /**
       * Is the array valid? If not don't use it.
       */
      bool isValid()
      {
	bool result;
	mutex_lock();
	result = m_valid;
	mutex_unlock();
	return result; 
      }

      /**
       * Set to valid
       */
      void validate()
      {
	mutex_lock();
	m_valid = true; 
	mutex_unlock();
      }

      /**
       * Set to invalid.
       */
      void invalidate() 
      {
	mutex_lock();
	m_valid = false;
	mutex_unlock();
      }

      /**
       * Get the ACC from file. The ACC in the file needs
       * to have the shape that is already defined.
       * @param filename Name of the file to read.
       * @return 0 on success, !0 on failure.
       */
      int getFromFile(std::string filename);

      /**
       * Get ACC from binary file assuming the m_acc dimensions.
       */
      int getFromBinaryFile(std::string filename);

    private:

      /**
       * ACC is a five dimensional array of complex numbers with dimensions
       *
       * nsubbands x  npol x npol x nantennas x nantennas
       */
      blitz::Array<std::complex<double>, 5>    m_acc;

      /**
       * m_time is a 1 dimensional array with a timestamp for each subband.
       */
      blitz::Array<RTC::Timestamp, 1> m_time;

      bool m_valid; // does the array contain valid data?

      blitz::Array<bool, 2>                 m_antenna_selection; // selection of antennas to be used by ::getACM
      int                                   m_antenna_count;     // number of selected antennas
      blitz::Array<std::complex<double>, 2> m_current_acm;       // the ACM last returned by ::getACM
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

  }; // namespace CAL
}; // namespace LOFAR

#endif /* ACC_H_ */

