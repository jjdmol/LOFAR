//#  ABSSubband.h: interface of the Subband class
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

#ifndef ABSSUBBAND_H_
#define ABSSUBBAND_H_

#include "ABSSpectralWindow.h"

namespace ABS
{

  class Subband
      {
      public:
	  /**
	   * Get a subband with the specified subband index.
	   * @param beam The index of the beam to get.
	   * 0 <= beam < ninstances.
	   */
	  static Subband* getInstance(int subband);

	  /**
	   * Set the number of subband instances that
	   * should be created. After calling this method
	   * once you can not alter the number of 
	   * instances by calling it again.
	   * @param nspws The total number of spectral
	   * windows.
	   * @param ninstances The total number of instances
	   * that should be created.
	   */
	  static int setNInstances(int nspws,
				   int ninstances);

	  /**
	   * Allocate the subband. A subband can be allocated
	   * more than once, the m_refcount is increased on
	   * each allocation.
	   * @return 0 on success, < 0 on failure.
	   */
	  int allocate();

      private:
	  /** Center frequency of the subband */
	  double m_centerfreq;

	  /** subband index */
	  int    m_index;

      protected:
	  Subband(); // no direct construction
	  ~Subband();

      private:
	  //@{
	  /** singleton implementation members */
	  static int   m_ninstances;
	  static Subband* m_subbands; // array of ninstances subbands
	  //@}

      private:
	  /**
	   * Don't allow copying this object.
	   */
	  Subband (const Subband&); // not implemented
	  Subband& operator= (const Subband&); // not implemented
      };

};
     
#endif /* ABSSUBBAND_H_ */
