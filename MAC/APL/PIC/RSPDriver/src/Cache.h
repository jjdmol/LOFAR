//#  Cache.h: RSP Driver data cache
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

#ifndef CACHE_H_
#define CACHE_H_

#include "RSP_Protocol.ph"
#if 0
#include "Timestamp.h"
#include "BeamletWeights.h"
#include "SubbandSelection.h"
#include "RCUSettings.h"
#include "WGSettings.h"
#include "SystemStatus.h"
#include "Statistics.h"
#include "Versions.h"
#endif

namespace RSP
{

  class CacheBuffer
    {
    public:
	  /**
	   * Constructors for a Cache object.
	   */
	  CacheBuffer();
	  
	  /* Destructor for Cache. */
	  virtual ~CacheBuffer();

	  /**
	   * Data access methods.
	   */
	  /*@{*/
	  const RSP_Protocol::BeamletWeights&   getBeamletWeights()   const;
	  const RSP_Protocol::SubbandSelection& getSubbandSelection() const;
	  const RSP_Protocol::RCUSettings&      getRCUSettings()      const;
	  const RSP_Protocol::WGSettings&       getWGSettings()       const;
	  const RSP_Protocol::SystemStatus&     getSystemStatus()     const;
	  const RSP_Protocol::Statistics&       getStatistics()       const;
	  const RSP_Protocol::Versions&         getVersions()         const;
	  /*@}*/

    private:
	  RSP_Protocol::Timestamp        m_timestamp;

	  RSP_Protocol::BeamletWeights   m_beamletweights;
	  RSP_Protocol::SubbandSelection m_subbandselection;
	  RSP_Protocol::RCUSettings      m_rcusettings;
	  RSP_Protocol::WGSettings       m_wgsettings;
	  RSP_Protocol::SystemStatus     m_systemstatus;
	  RSP_Protocol::Statistics       m_statistics;
	  RSP_Protocol::Versions         m_versions;
    };

  /**
   * Singleton class containing the data caches.
   */
  class Cache
      {
      public:
	/**
	 * Get single instance of this singleton class.
	 */
	static Cache& getInstance();
	  
	  /**
	   * Swap front and back buffers
	   */
	  void swapBuffers();

      private:
	  /**
	   * Constructor/destructor
	   */
	  /*@{*/
	  Cache();
	  virtual ~Cache();
	  /*@}*/

	  /**
	   * Front and back buffers.
	   */
	  /*@{*/
	  CacheBuffer* m_front;
	  CacheBuffer* m_back;
	  /*@}*/
      };
};
     
#endif /* CACHE_H_ */
