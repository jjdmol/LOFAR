//#  SpectralWindowConfig.h: interface of the SpectralWindow class
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

#ifndef SPECTRALWINDOWCONFIG_H_
#define SPECTRALWINDOWCONFIG_H_

#include <sys/time.h>
#include <RCUSettings.h>

namespace ABS
{
  class SpectralWindow; // forward declaration

  /**
   * Class with 
   */
  class SpectralWindowConfig
    {
      private:
      /**
       * Constructor
       */
      SpectralWindowConfig();

      public:

      /**
       * Destructor
       */
      ~SpectralWindowConfig();

      /**
       * Single get instance.
       */
      static SpectralWindowConfig& getInstance();

      /**
       * Load spectral window config from BeamServer.conf file.
       * @return true if successful, false otherwise
       */
      bool load();
      
      /**
       * Get a pointer to a spectral window.
       */
       const SpectralWindow* get(int index) const;

       /**
	* Increase or decrease reference count for a spectral window.
	* Only one spectral window may have refcount > 0, this is 
	* to ensure that only one spectral window is used at a time.
	*/
       int incRef(int index);
       int decRef(); // index obtained using getCurrent

       /**
	* Get index of currently selected spectral window.
	**/
       inline int getCurrent() const { return m_current; }
      
      private:
       /**
	* Array of spectral windows.
	*/
       SpectralWindow** m_spws;
       int  m_n_spws;
       int* m_refcount; // m_refcount[m_n_spws]

       int m_current; // index of current spw, -1 if no spectral window in use

       // singleton class
       static SpectralWindowConfig* m_config;
    };
};

#endif /* SPECTRALWINDOWCONFIG_H_ */
