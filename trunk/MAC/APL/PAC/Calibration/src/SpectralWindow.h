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

namespace CAL
{
  class SPWLoader;
  
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
   * The method getSubbandFreq(subband, pos=CENTER) returns the center frequency of a subband by default.
   * Passing position parameter pos=LOW returns the start frequency of the subband.
   * Passing position parameter pos=HIGH return the end frequency of the subband.
   */
  class SpectralWindow
  {
  public:
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
     * Return the LOW, CENTER, HIGH frequency of a specific subband.
     */
    double getSubbandFreq(int subband, int pos = CENTER) const;

    /**
     * Subband frequency position values
     */
    enum {
      LOW = 1,
      CENTER,
      HIGH 
    };

  private:

    friend class SPWLoader; // class to load spectral windows from file

    /**
     * Prevent default construction.
     */
    SpectralWindow();

     /**
     * Private constructor for SPWLoader to use
     */
    SpectralWindow(std::string name, double sampling_freq,
		   int nyquist_zone, int numsubbands) :
      m_name(name), m_sampling_freq(sampling_freq),
      m_nyquist_zone(nyquist_zone), m_numsubbands(numsubbands)
    {
    }
      
    std::string m_name;          // name of the spectral window
    double      m_sampling_freq; // sampling frequency
    int         m_nyquist_zone;  // defines the window
    int         m_numsubbands;   // number of subbands
  };

  /**
   * Class which loads spectral window definitions from file.
   */
  class SPWLoader
  {
  public:
    /**
     * Load one or more spectral window definitions from two Blitz array
     * strings describing the spectral windows.
     *
     * Example strings:
     *
     *             sampling_frequency          nyquist_zone      nsubbands
     *                   |                           |              |
     * params= 6 x 3 [ 160e6 1 512 160e6 2 512 160e6 3 512 200e6 1 512 200e6 2 512 200e6 3 512 ]
     * names=  6     [     "80"        "160"       "240"       "100"       "200"       "300"   ]
     *
     */
    static std::vector<SpectralWindow> loadFromBlitzStrings(std::string params, std::string names);
  };
  
};

#endif /* SPECTRALWINDOW_H_ */

