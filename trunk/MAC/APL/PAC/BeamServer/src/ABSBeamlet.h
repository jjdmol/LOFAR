//#  ABSBeamlet.h: interface of the Beamlet class
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

#ifndef ABSBEAMLET_H_
#define ABSBEAMLET_H_

#include "ABSSpectralWindow.h"

namespace ABS
{

  class Beamlet
      {
      public:
	  /**
	   * Get an unallocated beamlet.
	   * @return Pointer to a Beamlet instance, if no
	   * more Beamlet instances are avialable, return 0.
	   * @return 0 if Beamlet::setNInstances has not yet
	   * been called.
	   */
	  static Beamlet* getInstance();

	  /**
	   * Set the number of beamlet instances that
	   * should be created. After calling this method
	   * once you can not alter the number of 
	   * instances by calling it again.
	   * @param ninstances The total number of instances
	   * that should be created.
	   * @return 0 on success, < 0 on failure.
	   */
	  static int setNInstances(int ninstances);

      public:
	  /**
	   * Allocate the beamlet.
	   * @param spw Allocate within this spectral window.
	   * @param subband Index of the subband to allocate
	   * within the spectral window.
	   * @return 0 if allocation succeeded, < 0 otherwise.
	   */
	  int allocate(SpectralWindow const& spw, int subband);

	  /**
	   * Deallocate the beamlet
	   */
	  int deallocate();

	  /**
	   * If the beam is allocated return true,
	   * otherwise return false.
	   */
	  bool allocated() const;

	  /**
	   * Accessor method for index.
	   */
	  const SpectralWindow* spw() const;
	  int subband() const;
	  int index() const;

      protected:
	  Beamlet(); // no direct construction
	  virtual ~Beamlet();

      private:
	  /** is the beamlet allocated? */
	  bool m_allocated;

	  /** spectral window of the subband */
	  const SpectralWindow * m_spw;

	  /** subband within the spectral window */
	  int m_subband;

	  /** index of the beamlet in the total beamles array */
	  int m_index;

      private:
	  //@{
	  /** singleton implementation members */
	  static int   m_ninstances;
	  static Beamlet* m_beamlets; // array of ninstances beamlets
	  //@}

      private:
	  /**
	   * Don't allow copying this object.
	   */
	  Beamlet (const Beamlet&); // not implemented
	  Beamlet& operator= (const Beamlet&); // not implemented
      };

  inline bool Beamlet::allocated() const { return m_allocated; }
  inline const SpectralWindow* Beamlet::spw() const { return m_spw; }
  inline int  Beamlet::subband()   const { return m_subband; }
  inline int  Beamlet::index()     const { return m_index; }

};
     
#endif /* ABSBEAMLET_H_ */
