//#  RSPConfig.h: configuration constants read from config file
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

#include "RSPConfig.h"
#include <stdlib.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace RSP;

RSPConfig* RSPConfig::m_instance = 0;

RSPConfig::RSPConfig()
{
}

RSPConfig::~RSPConfig()
{
}

RSPConfig& RSPConfig::getInstance()
{
  if (!m_instance)
  {
    m_instance = new RSPConfig;
  }

  return *m_instance;
}

void RSPConfig::load(const char* filename)
{
  m_config.init(filename, '=', 1);
}
