//#  -*- mode: c++ -*-
//#
//#  RSPConfig.h: Singleton class containing the
//#               configuration file configurable constants.
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

#ifndef RSPCONFIG_H_
#define RSPCONFIG_H_

#include "Config.h"
#include <string>

#define RSPDRIVER_BLOCK "RSPDRIVER"

#define GET_CONFIG(var, type) \
(RSP::RSPConfig::getInstance()()(RSPDRIVER_BLOCK, var)? \
 (ato##type(RSP::RSPConfig::getInstance()()(RSPDRIVER_BLOCK, var))) : ato##type(""))

#define GET_CONFIG_STRING(var) \
(RSP::RSPConfig::getInstance()()(RSPDRIVER_BLOCK, var)?RSP::RSPConfig::getInstance()()(RSPDRIVER_BLOCK, var):"unset")

namespace RSP
{
  /**
   * Singleton class holding the constants that
   * are configurable from a configuration file.
   *
   * These constants can only be changed before starting
   * the RSP driver.
   */
  class RSPConfig
  {
    public:
      static RSPConfig& getInstance();
      virtual ~RSPConfig();

      void load(const char* filename);

      inline Config& operator()() { return m_config; }

    private:
      RSPConfig();

      static RSPConfig* m_instance;

      Config  m_config;
  };
};

#endif /* RSPCONFIG_H_ */
