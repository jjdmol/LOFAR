//#  APLConfig.h: configuration constants read from config file
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

#include "APLConfig.h"
#include <stdlib.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

APLConfig* APLConfig::m_instance = 0;

APLConfig::APLConfig()
{
}

APLConfig::~APLConfig()
{
}

APLConfig& APLConfig::getInstance()
{
  if (!m_instance)
  {
    m_instance = new APLConfig;
  }

  return *m_instance;
}

void APLConfig::load(const char* blockname, const char* filename)
{
  m_blockname = std::string(blockname);
  m_config.init(filename, '=', 1);
}

const char* APLConfig::getBlockname()
{
  return m_blockname.c_str();
}
