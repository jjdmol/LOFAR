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

#include "RSP_Protocol.ph"
#include <blitz/array.h>

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

      /*@{*/
      /**
       * Data access methods.
       */
      RSP_Protocol::Timestamp         getTimestamp() const;
      RSP_Protocol::BeamletWeights&   getBeamletWeights();
      RSP_Protocol::SubbandSelection& getSubbandSelection();
      RSP_Protocol::RCUSettings&      getRCUSettings(int rcu);
      RSP_Protocol::WGSettings&       getWGSettings(int rcu);
      RSP_Protocol::SystemStatus&     getSystemStatus();
      RSP_Protocol::Statistics&       getStatistics(int rcu);
      RSP_Protocol::Versions&         getVersions();
      /*@}*/

    private:
      RSP_Protocol::Timestamp                    m_timestamp;

      RSP_Protocol::BeamletWeights               m_beamletweights;
      RSP_Protocol::SubbandSelection             m_subbandselection;
      blitz::Array<RSP_Protocol::RCUSettings, 1> m_rcusettings;
      blitz::Array<RSP_Protocol::WGSettings, 1>  m_wgsettings;
      blitz::Array<RSP_Protocol::Statistics, 1>  m_statistics;
      RSP_Protocol::SystemStatus                 m_systemstatus;
      RSP_Protocol::Versions                     m_versions;
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
       * Swap front and back buffers
       */
      void swapBuffers();

      /**
       * Get front/back buffers.
       */
      CacheBuffer& getFront();
      CacheBuffer& getBack();

    private:

      /**
       * Direct construction not allowed.
       */
      Cache();

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
     
#endif /* CACHE_H_ */
