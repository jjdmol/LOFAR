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

#include <APL/RTCCommon/RegisterState.h>
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

      /*@{*/
      /**
       * Data access methods.
       */
      RTC::Timestamp                   getTimestamp() const;
      RSP_Protocol::BeamletWeights&    getBeamletWeights();
      RSP_Protocol::SubbandSelection&  getSubbandSelection();
      RSP_Protocol::RCUSettings&       getRCUSettings();
      RSP_Protocol::RSUSettings&       getRSUSettings();
      RSP_Protocol::WGSettings&        getWGSettings();
      RSP_Protocol::SystemStatus&      getSystemStatus();
      RSP_Protocol::Statistics&        getSubbandStats();
      RSP_Protocol::Statistics&        getBeamletStats();
      RSP_Protocol::XCStatistics&      getXCStats();
      RSP_Protocol::Versions&          getVersions();
      RSP_Protocol::Clocks&            getClocks();
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
      RSP_Protocol::RSUSettings      m_rsusettings;
      RSP_Protocol::WGSettings       m_wgsettings;
      RSP_Protocol::Statistics       m_subbandstats;
      RSP_Protocol::Statistics       m_beamletstats;
      RSP_Protocol::XCStatistics     m_xcstats;
      RSP_Protocol::SystemStatus     m_systemstatus;
      RSP_Protocol::Versions         m_versions;
      RSP_Protocol::Clocks           m_clocks;

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
      RTC::RegisterState& getRSUClearState()       { return rsuclear_state; }
      RTC::RegisterState& getDIAGWGSettingsState() { return diagwgsettings_state; }
      RTC::RegisterState& getRCUSettingsState()    { return rcusettings_state; }
      RTC::RegisterState& getRCUProtocolState()    { return rcuprotocol_state; }
      RTC::RegisterState& getCDOState()            { return cdo_state; }
      RTC::RegisterState& getBSState()             { return bs_state; }
      RTC::RegisterState& getTDSState()            { return tds_state; }

    private:

      /**
       * Direct construction not allowed.
       */
      Cache();

      /**
       * Keep register update state.
       */
      RTC::RegisterState rsuclear_state;       // RSU clear state
      RTC::RegisterState diagwgsettings_state; // DIAG WG settings state
      RTC::RegisterState rcusettings_state;    // RCU settings state
      RTC::RegisterState rcuprotocol_state;    // RCU protocol state
      RTC::RegisterState cdo_state;            // CDO state
      RTC::RegisterState bs_state;             // BS register state
      RTC::RegisterState tds_state;            // TDS register state (Clock board)

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
