//#  ABSSpectralWindow.h: interface of the SpectralWindow class
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

#ifndef ABSSPECTRALWINDOW_H_
#define ABSSPECTRALWINDOW_H_

#include "ABSDirection.h"
#include <sys/time.h>

namespace ABS
{

  /**
   * Class with 
   */
  class SpectralWindow
      {
      public:
	  //@{
	  /**
	   * Constructors for a spectralwindow.
	   */
	  SpectralWindow(double start, double width, int n_subbands);
	  //@}

	  //@{
	  /**
	   * Accessor methods.
	   */
	  double start() const;
	  double width() const;
	  int    nsubbands() const;
          //@}

	  /**
	   * Get frequency of one of the subbands.
	   * @param subband Index of the subbands for which
	   * the frequency should be calculated. Indexing starts
	   * from 0 so that getFrequency(0) returns the center
	   * frequency of the first subband.
	   */
	  double getFrequency(int subband) const;

      private:
	  /** the center frequency in Hz of the first subband */
	  double m_start;

	  /** the width in Hz of a subband */
	  double m_width;

	  /** the number of subbands */
	  int m_nsubbands;
      };

  inline double SpectralWindow::start() const     { return m_start; }
  inline double SpectralWindow::width() const     { return m_width; }
  inline int    SpectralWindow::nsubbands() const { return m_nsubbands; }

  inline double SpectralWindow::getFrequency(int subband) const
      {
	//return (0.5 * m_width) + m_start + (subband * m_width);
	return m_start + (subband * m_width);
      }
};

#endif /* ABSSPECTRALWINDOW_H_ */
