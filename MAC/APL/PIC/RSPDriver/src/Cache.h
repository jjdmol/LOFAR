//#  -*- mode: c++ -*-
//#
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

#include <APL/RSP_Protocol/AllRegisterState.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP {

    class Cache; // forward declaration
    class CacheBuffer
    {
    public:
      /**
       * Constructors for a Cache object.
       */
      CacheBuffer(Cache* cache);

      /* Destructor for Cache. */
      virtual ~CacheBuffer();

      /*
       * Reset cache to default values.
       * Also called by constructor to initialize the cache.
       */
      void reset(void);

      /*@{*/
      /**
       * Data access methods.
       */
      RTC::Timestamp                   getTimestamp() const;
      RSP_Protocol::BeamletWeights&    getBeamletWeights();
      RSP_Protocol::SubbandSelection&  getSubbandSelection();
      RSP_Protocol::RCUSettings&       getRCUSettings();
      RSP_Protocol::HBASettings&       getHBASettings();
      RSP_Protocol::RSUSettings&       getRSUSettings();
      RSP_Protocol::WGSettings&        getWGSettings();
      RSP_Protocol::SystemStatus&      getSystemStatus();
      RSP_Protocol::Statistics&        getSubbandStats();
      RSP_Protocol::Statistics&        getBeamletStats();
      RSP_Protocol::XCStatistics&      getXCStats();
      RSP_Protocol::Versions&          getVersions();
      uint32&                          getClock();
      /*@}*/

      /**
       * update timestamp
       */
      void setTimestamp(const RTC::Timestamp& timestamp);

      /**
       * Get const pointer to parent cache.
       */
      Cache& getCache() { return *m_cache; }

    private:

      CacheBuffer(); // prevent default construction

      RTC::Timestamp                 m_timestamp;

      RSP_Protocol::BeamletWeights   m_beamletweights;
      RSP_Protocol::SubbandSelection m_subbandselection;
      RSP_Protocol::RCUSettings      m_rcusettings;
      RSP_Protocol::HBASettings      m_hbasettings;
      RSP_Protocol::RSUSettings      m_rsusettings;
      RSP_Protocol::WGSettings       m_wgsettings;
      RSP_Protocol::Statistics       m_subbandstats;
      RSP_Protocol::Statistics       m_beamletstats;
      RSP_Protocol::XCStatistics     m_xcstats;
      RSP_Protocol::SystemStatus     m_systemstatus;
      RSP_Protocol::Versions         m_versions;
      uint32                         m_clock;

      Cache* m_cache;
    };

    /**
     * Singleton class containing the data caches.
     */
    class Cache
    {
    public:
      /*@{*/
      /**
       * Constructor/destructor
       */
      static Cache& getInstance();
      virtual ~Cache();
      /*@}*/

      /*
       * Reset cache front and back buffers.
       */
      void reset(void);

      /**
       * Swap the front and back buffers.
       */
      void swapBuffers();

      /**
       * Get front/back buffers.
       */
      CacheBuffer& getFront();
      CacheBuffer& getBack();

      /**
       * Get register states.
       */
      AllRegisterState& getState() { return m_allstate; }

    private:

      /**
       * Direct construction not allowed.
       */
      Cache();

      /**
       * Keep register update state.
       */
      AllRegisterState m_allstate; // communication status of all register

      /*@{*/
      /**
       * Front and back buffers.
       */
      CacheBuffer* m_front;
      CacheBuffer* m_back;
      /*@}*/

      /**
       * Singleton class.
       */
      static Cache* m_instance;
    };
  };
};
     
#endif /* CACHE_H_ */
