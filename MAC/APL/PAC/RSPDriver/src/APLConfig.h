//#  -*- mode: c++ -*-
//#
//#  APLConfig.h: Singleton class containing the
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

#ifndef APLCONFIG_H_
#define APLCONFIG_H_

#include "Config.h"
#include <string>

#if 1

#include <GCF/ParameterSet.h>

// Replace APLConfig calls with ParameterSet calls

#define convert_i getInt
#define convert_f getFloat
#define GET_CONFIG(var, type) (GCF::ParameterSet::instance()->convert_##type(var))

#define GET_CONFIG_STRING(var) (GCF::ParameterSet::instance()->getString(var).c_str())

#else

#define GET_CONFIG(var, type) \
(APLConfig::getInstance()()(APLConfig::getInstance().getBlockname(), var)? \
 (ato##type(APLConfig::getInstance()()(APLConfig::getInstance().getBlockname(), var))) : ato##type(""))

#define GET_CONFIG_STRING(var) \
(APLConfig::getInstance()()(APLConfig::getInstance().getBlockname(), var)?APLConfig::getInstance()()(APLConfig::getInstance().getBlockname(), var):"unset")

#endif

/**
 * Singleton class holding the constants that
 * are configurable from a configuration file.
 *
 * These constants can only be changed before starting
 * the RSP driver.
 */
class APLConfig
{
  public:
    static APLConfig& getInstance();
    virtual ~APLConfig();

    void load(const char* blockname, const char* filename);
    const char* getBlockname();

    inline Config& operator()() { return m_config; }

  private:
    APLConfig();

    static APLConfig* m_instance;

    std::string m_blockname;
    Config      m_config;
};

#endif /* APLCONFIG_H_ */
