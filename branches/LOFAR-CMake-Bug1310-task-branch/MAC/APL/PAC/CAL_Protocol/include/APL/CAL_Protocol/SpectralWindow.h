//#  -*- mode: c++ -*-
//#  SpectralWindow.h: class definition for the SpectralWindow class
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

#ifndef SPECTRALWINDOW_H_
#define SPECTRALWINDOW_H_

#include <string>
#include <vector>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace CAL {

    /**
     * Class which represents a window on the electromagnetic spectrum.
     *
     * A window is defined by three parameters:
     * - sampling frequency
     * - nyquist_zone
     * - nsubbands
     *
     * The band starting at frequency ((nyquist_zone - 1) * (sampling_frequency / 2.0)) and
     * ending at (nyquist_zone * (sampling_frequency / 2.0)) is filtered into
     * nsubband equal frequency bins.
     * 
     * The method getSubbandFreq(subband) returns the center frequency of a subband.
     */
    class SpectralWindow
    {
    public:
      /**
       * Default constructor.
       */
      SpectralWindow();

      /**
       * Constructor
       */
      SpectralWindow(std::string name, double sampling_freq,
		     int nyquist_zone, int numsubbands, uint32 rcucontrol) :
	m_name(name), m_sampling_freq(sampling_freq),
	m_nyquist_zone(nyquist_zone), m_numsubbands(numsubbands), m_rcucontrol(rcucontrol) {}
      virtual ~SpectralWindow();

      /**
       * Return the name of the spectral window.
       */
      std::string getName() const { return m_name; }
      
      /**
       * Return the sampling frequency for this window
       */
      double getSamplingFrequency() const { return m_sampling_freq; }
      
      /**
       * Return the nyquist zone for this window.
       */
      int getNyquistZone() const { return m_nyquist_zone; }

      /**
       * Return the number of subbands for the spectral window.
       */
      int getNumSubbands() const { return m_numsubbands; }
      
      /**
       * Return the width of the subbands.
       */
      double getSubbandWidth() const { return m_sampling_freq / (2.0 * m_numsubbands); }
      
      /**
       * Return frequency of a specific subband.
       */
      double getSubbandFreq(int subband) const;

      /**
       * Based on the subband, the spectral window and
       * the rcucontrol. Determine whether a subband
       * is suitable for calibration. A subband is not
       * suitable for calibration when it is in the stop-band
       * of the filter or antenna or if it is aliased
       * by frequencies around the sampling frequency or
       * one of its harmonics.
       */
      bool isSuitable(int subband) const;

	  // Returns try if spectralWindow is ment for the HBA antennas.
	  bool isForHBA() const;

    public:
      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize() const;
      unsigned int pack   (void* buffer) const;
      unsigned int unpack (void* buffer);
      /*@}*/

    private:

      std::string m_name;          // name of the spectral window
      double      m_sampling_freq; // sampling frequency
      uint16      m_nyquist_zone;  // defines the window
      uint16      m_numsubbands;   // number of subbands
      uint32      m_rcucontrol;    // RCU control setting
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* SPECTRALWINDOW_H_ */

